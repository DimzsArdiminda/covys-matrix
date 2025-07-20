#include "activity_manager.h"
#include "imgui.h"

void ActivityManager::addActivity(const std::string& name, bool important, bool urgent) {
    Activity a{name, important, urgent};
    if (important && urgent)
        q1.push_back(a);
    else if (important && !urgent)
        q2.push_back(a);
    else if (!important && urgent)
        q3.push_back(a);
    else
        q4.push_back(a);
}

void renderList(const std::vector<Activity>& list) {
    for (const auto& a : list) {
        ImGui::BulletText("%s", a.name.c_str());
    }
}

void ActivityManager::renderUI() {
    if (ImGui::CollapsingHeader("Kuadran I - Penting & Mendesak"))
        renderList(q1);
    if (ImGui::CollapsingHeader("Kuadran II - Penting & Tidak Mendesak"))
        renderList(q2);
    if (ImGui::CollapsingHeader("Kuadran III - Tidak Penting & Mendesak"))
        renderList(q3);
    if (ImGui::CollapsingHeader("Kuadran IV - Tidak Penting & Tidak Mendesak"))
        renderList(q4);

    ImGui::Spacing();
    ImGui::Text("Rekomendasi:");
    ImGui::BulletText("Kerjakan Kuadran I segera!");
    ImGui::BulletText("Jadwalkan Kuadran II untuk jangka panjang");
    ImGui::BulletText("Minimalkan Kuadran III dan IV");
}
