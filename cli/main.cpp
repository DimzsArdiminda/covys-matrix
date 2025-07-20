#include "./header/activity_manager.h"

int main() {
    ActivityManager manager;
    manager.collectActivities();
    manager.categorizeActivities();
    manager.displayQuadrants();
    manager.showRecommendations();

    return 0;
}
