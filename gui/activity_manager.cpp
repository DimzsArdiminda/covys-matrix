#include "activity_manager.h"
#include <sstream>

Activity::Activity(std::string name, bool isImportant, bool isUrgent)
    : name(name), isImportant(isImportant), isUrgent(isUrgent) {}

void ActivityManager::addActivity(const std::string& name, bool isImportant, bool isUrgent) {
    allActivities.emplace_back(name, isImportant, isUrgent);
    // Auto-save to CSV after adding activity
    saveToCSV();
}

std::vector<Activity> ActivityManager::searchActivities(const std::string& keyword) const {
    std::vector<Activity> results;
    for (const auto& act : allActivities) {
        if (act.name.find(keyword) != std::string::npos) {
            results.push_back(act);
        }
    }
    return results;
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
    
    ss << "*** COVIE'S MATRIX - KATEGORISASI AKTIVITAS ***\n";
    ss << "==================================================\n\n";
    
    // Header tabel
    ss << "+-----+--------------------------------+----------+----------+----------+\n";
    ss << "| No  | Nama Aktivitas                 | Penting  | Mendesak | Kuadran  |\n";
    ss << "+-----+--------------------------------+----------+----------+----------+\n";
    
    int no = 1;
    
    // Function untuk format aktivitas dalam tabel
    auto addToTable = [&ss, &no](const std::vector<Activity>& list, const std::string& quadrant) {
        for (const auto& activity : list) {
            std::string name = activity.name;
            if (name.length() > 30) {
                name = name.substr(0, 27) + "...";
            }
            
            ss << "| " << std::setw(3) << no++ << " | ";
            ss << std::left << std::setw(30) << name << " | ";
            ss << std::setw(8) << (activity.isImportant ? "Ya" : "Tidak") << " | ";
            ss << std::setw(8) << (activity.isUrgent ? "Ya" : "Tidak") << " | ";
            ss << std::setw(8) << quadrant << " |\n";
        }
    };
    
    // Tambahkan semua aktivitas ke tabel berdasarkan kuadran
    addToTable(quadrantI, "I");
    addToTable(quadrantII, "II");
    addToTable(quadrantIII, "III");
    addToTable(quadrantIV, "IV");
    
    ss << "+-----+--------------------------------+----------+----------+----------+\n\n";
    
    // Summary per kuadran
    ss << "*** RINGKASAN PER KUADRAN ***\n";
    ss << "===============================\n";
    ss << "[I]   Penting & Mendesak      : " << quadrantI.size() << " aktivitas\n";
    ss << "[II]  Penting & Tidak Mendesak: " << quadrantII.size() << " aktivitas\n";
    ss << "[III] Tidak Penting & Mendesak: " << quadrantIII.size() << " aktivitas\n";
    ss << "[IV]  Tidak Penting & Tidak M.: " << quadrantIV.size() << " aktivitas\n";
    ss << "Total Aktivitas               : " << allActivities.size() << " aktivitas\n\n";

    return ss.str();
}

std::string ActivityManager::getRecommendations() const {
    return "*** PANDUAN KUADRAN COVIE'S MATRIX ***\n"
           "========================================\n"
           "+----------+-------------------+---------------------------+\n"
           "| Kuadran  | Kategori          | Tindakan yang Disarankan  |\n"
           "+----------+-------------------+---------------------------+\n"
           "| I        | Penting &         | KERJAKAN SEGERA!          |\n"
           "|          | Mendesak          | Prioritas tertinggi       |\n"
           "+----------+-------------------+---------------------------+\n"
           "| II       | Penting &         | JADWALKAN dengan baik     |\n"
           "|          | Tidak Mendesak    | Fokus untuk jangka panjang|\n"
           "+----------+-------------------+---------------------------+\n"
           "| III      | Tidak Penting &   | DELEGASIKAN jika bisa     |\n"
           "|          | Mendesak          | Atau batasi waktu         |\n"
           "+----------+-------------------+---------------------------+\n"
           "| IV       | Tidak Penting &   | ELIMINASI atau MINIMALKAN |\n"
           "|          | Tidak Mendesak    | Hindari jika memungkinkan |\n"
           "+----------+-------------------+---------------------------+\n\n"
           ">> TIPS PRODUKTIVITAS:\n"
           "   * Fokus utama pada Kuadran II untuk hasil optimal\n"
           "   * Kurangi waktu di Kuadran III dan IV\n"
           "   * Selesaikan Kuadran I secepat mungkin\n";
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

void ActivityManager::removeActivity(int index) {
    if (index >= 0 && index < allActivities.size()) {
        allActivities.erase(allActivities.begin() + index);
        saveToCSV(); // Auto-save after removal
    }
}

void ActivityManager::editActivity(int index, const std::string& newName, bool newImportant, bool newUrgent) {
    if (index >= 0 && index < allActivities.size()) {
        allActivities[index].name = newName;
        allActivities[index].isImportant = newImportant;
        allActivities[index].isUrgent = newUrgent;
        saveToCSV(); // Auto-save after edit
    }
}
