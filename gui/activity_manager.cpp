#include "activity_manager.h"
#include <sstream>

Activity::Activity(std::string name, bool isImportant, bool isUrgent)
    : name(name), isImportant(isImportant), isUrgent(isUrgent) {}

void ActivityManager::addActivity(const std::string& name, bool isImportant, bool isUrgent) {
    allActivities.emplace_back(name, isImportant, isUrgent);
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
