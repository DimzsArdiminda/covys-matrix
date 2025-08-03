#include "activity_manager.h"
#include <sstream>

Activity::Activity(std::string name, bool isImportant, bool isUrgent)
    : name(name), isImportant(isImportant), isUrgent(isUrgent) {}

void ActivityManager::addActivity(const std::string& name, bool isImportant, bool isUrgent) {
    allActivities.emplace_back(name, isImportant, isUrgent);
    // Auto-save to CSV after adding activity
    saveToCSV();
}

bool ActivityManager::askYesNo(const std::string& question) {
    std::string input;
    std::cout << question << " (ya/tidak): ";
    std::getline(std::cin, input); 
    return input == "ya";
}

void ActivityManager::collectActivities() {
    int count;
    std::cout << "Berapa banyak kegiatan yang ingin Anda masukkan? ";
    std::cin >> count;
    std::cin.ignore();

    for (int i = 0; i < count; ++i) {
        std::string name;
        std::cout << "\nMasukkan nama kegiatan ke-" << i + 1 << ": ";
        std::getline(std::cin, name);

        bool important = askYesNo("Apakah kegiatan ini PENTING?");
        bool urgent = askYesNo("Apakah kegiatan ini MENDESAK?");

        allActivities.emplace_back(name, important, urgent);
    }
}

void ActivityManager::categorizeActivities() {
    // Clear previous categorization
    quadrantI.clear();
    quadrantII.clear();
    quadrantIII.clear();
    quadrantIV.clear();
    
    for (const auto& activity : allActivities) {
        if (activity.isImportant && activity.isUrgent) {
            quadrantI.push_back(activity);
        } else if (activity.isImportant && !activity.isUrgent) {
            quadrantII.push_back(activity);
        } else if (!activity.isImportant && activity.isUrgent) {
            quadrantIII.push_back(activity);
        } else {
            quadrantIV.push_back(activity);
        }
    }
}

void ActivityManager::displayQuadrants() {
    auto display = [](const std::vector<Activity>& list, const std::string& title) {
        std::cout << "\n" << title << ":\n";
        if (list.empty()) {
            std::cout << "  (Tidak ada kegiatan)\n";
        } else {
            for (const auto& act : list) {
                std::cout << "  - " << act.name << "\n";
            }
        }
    };

    display(quadrantI, "Kuadran I (Penting & Mendesak)");
    display(quadrantII, "Kuadran II (Penting & Tidak Mendesak)");
    display(quadrantIII, "Kuadran III (Tidak Penting & Mendesak)");
    display(quadrantIV, "Kuadran IV (Tidak Penting & Tidak Mendesak)");
}

void ActivityManager::showRecommendations() {
    std::cout << "\nRekomendasi Tindakan:\n";
    std::cout << "  - Kerjakan Kuadran I segera!\n";
    std::cout << "  - Jadwalkan Kuadran II untuk pengembangan jangka panjang\n";
    std::cout << "  - Minimalkan Kuadran III dan IV, hindari jika bisa\n";
}

std::string ActivityManager::getQuadrantAsString() const {
    std::stringstream ss;
    auto writeList = [&ss](const std::vector<Activity>& list, const std::string& title) {
        ss << title << ":\n";
        if (list.empty())
            ss << "  (Tidak ada kegiatan)\n";
        else {
            for (const auto& act : list)
                ss << "  - " << act.name << "\n";
        }
        ss << "\n";
    };

    writeList(quadrantI, "Kuadran I (Penting & Mendesak)");
    writeList(quadrantII, "Kuadran II (Penting & Tidak Mendesak)");
    writeList(quadrantIII, "Kuadran III (Tidak Penting & Mendesak)");
    writeList(quadrantIV, "Kuadran IV (Tidak Penting & Tidak Mendesak)");

    return ss.str();
}

std::string ActivityManager::getRecommendations() const {
    return "Rekomendasi:\n"
           "  - Kerjakan Kuadran I segera!\n"
           "  - Jadwalkan Kuadran II untuk pengembangan jangka panjang\n"
           "  - Minimalkan Kuadran III dan IV, hindari jika bisa\n";
}

void ActivityManager::saveToCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        // Write header
        file << "Name,Important,Urgent\n";
        
        // Write activities
        for (const auto& activity : allActivities) {
            file << "\"" << activity.name << "\"," 
                 << (activity.isImportant ? "1" : "0") << ","
                 << (activity.isUrgent ? "1" : "0") << "\n";
        }
        
        file.close();
    }
}

void ActivityManager::loadFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        
        // Skip header line
        if (std::getline(file, line)) {
            // Read data lines
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                
                // Parse CSV line
                std::stringstream ss(line);
                std::string name, importantStr, urgentStr;
                
                // Parse name (handle quoted strings)
                if (line[0] == '"') {
                    size_t closeQuote = line.find('"', 1);
                    if (closeQuote != std::string::npos) {
                        name = line.substr(1, closeQuote - 1);
                        std::string remaining = line.substr(closeQuote + 1);
                        
                        // Parse remaining comma-separated values
                        std::stringstream remainingSS(remaining);
                        std::string temp;
                        if (std::getline(remainingSS, temp, ',')) {
                            if (std::getline(remainingSS, importantStr, ',')) {
                                std::getline(remainingSS, urgentStr);
                            }
                        }
                    }
                } else {
                    // Simple parsing for unquoted strings
                    std::getline(ss, name, ',');
                    std::getline(ss, importantStr, ',');
                    std::getline(ss, urgentStr);
                }
                
                if (!name.empty() && !importantStr.empty() && !urgentStr.empty()) {
                    bool isImportant = (importantStr == "1");
                    bool isUrgent = (urgentStr == "1");
                    allActivities.emplace_back(name, isImportant, isUrgent);
                }
            }
        }
        
        file.close();
    }
}

void ActivityManager::clearAllActivities() {
    allActivities.clear();
    quadrantI.clear();
    quadrantII.clear();
    quadrantIII.clear();
    quadrantIV.clear();
}
