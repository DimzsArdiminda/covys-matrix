#ifndef ACTIVITY_MANAGER_H
#define ACTIVITY_MANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

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
    
    // CSV functions
    void saveToCSV(const std::string& filename = "activities.csv") const;
    void loadFromCSV(const std::string& filename = "activities.csv");
    void clearAllActivities();
};

#endif
