#include "activity_manager.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <string>

int main() {
    // Setup GLFW
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Covey's Matrix Manager", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();

    ActivityManager manager;
    static char activityName[128] = "";
    static bool isImportant = false;
    static bool isUrgent = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Covey's Matrix Activity Manager");

        ImGui::InputText("Nama Aktivitas", activityName, IM_ARRAYSIZE(activityName));
        ImGui::Checkbox("Penting", &isImportant);
        ImGui::Checkbox("Mendesak", &isUrgent);

        if (ImGui::Button("Tambah Kegiatan")) {
            if (std::string(activityName).length() > 0) {
                manager.addActivity(activityName, isImportant, isUrgent);
                strcpy(activityName, "");
                isImportant = false;
                isUrgent = false;
            }
        }

        ImGui::Separator();
        manager.renderUI();

        ImGui::End();
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
