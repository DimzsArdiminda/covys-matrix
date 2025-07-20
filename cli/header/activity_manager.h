#ifndef ACTIVITY_MANAGER_H
#define ACTIVITY_MANAGER_H

#include <iostream>
#include <vector>
#include <string>

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
    void collectActivities();
    void categorizeActivities();
    void displayQuadrants();
    void showRecommendations();
};

#endif
