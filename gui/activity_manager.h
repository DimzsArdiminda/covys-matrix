#ifndef ACTIVITY_MANAGER_H
#define ACTIVITY_MANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

class Activity {
public:
    std::string name;
    bool isImportant;
    bool isUrgent;

    Activity(std::string name, bool isImportant, bool isUrgent);
};

class ActivityManager {
private:
    std::vector<Activity> allActivities;
    std::vector<Activity> quadrantI;
    std::vector<Activity> quadrantII;
    std::vector<Activity> quadrantIII;
    std::vector<Activity> quadrantIV;

    bool askYesNo(const std::string& question);

public:
    void addActivity(const std::string& name, bool isImportant, bool isUrgent);
    void collectActivities();
    void categorizeActivities();
    void displayQuadrants();
    void showRecommendations();
    std::string getQuadrantAsString() const;
    std::string getRecommendations() const;
    std::vector<Activity> searchActivities(const std::string& keyword) const;

    
        // CSV operations
    void saveToCSV(const std::string& filename = "activities.csv") const;
    void loadFromCSV(const std::string& filename = "activities.csv");
    void clearAllActivities();
    
    // Getter for all activities
    const std::vector<Activity>& getAllActivities() const { return allActivities; }
    
    // Edit and delete operations
    void removeActivity(int index);
    void editActivity(int index, const std::string& newName, bool newImportant, bool newUrgent);
};

#endif
