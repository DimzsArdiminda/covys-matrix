#ifndef ACTIVITY_MANAGER_H
#define ACTIVITY_MANAGER_H

#include <vector>
#include <string>

struct Activity {
    std::string name;
    bool important;
    bool urgent;
};

class ActivityManager {
private:
    std::vector<Activity> q1, q2, q3, q4;

public:
    void addActivity(const std::string& name, bool important, bool urgent);
    void renderUI();
};

#endif
