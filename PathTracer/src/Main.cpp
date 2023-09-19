#define STB_IMAGE_IMPLEMENTATION

#include "Model.h"

#include "Renderer.h"
#include "Image.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>

int main(void)
{
    spdlog::info("Path tracer");

    int screenWidth = 1000;
    int screenHeight = 600;
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(screenWidth, screenHeight, "PATH TRACER", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


#pragma region ImGuiInit
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 460");
#pragma endregion

#pragma region Scene
    Scene scene;

    std::string folderPath = "Textures/Environment";
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        // Check if the entry is a regular file and has an image file extension
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            std::string fileExtension = entry.path().extension().string();

            if (fileExtension == ".hdr") {
                Texture image(filePath.c_str());
                scene.EnvironmentImages.push_back(image);
            }
        }
    }

    //scene.Models.push_back(Model("Models/cornellbox.obj"));
    scene.Models.push_back(Model("Models/monkeys.obj"));
#pragma endregion

    Renderer renderer;
    Camera camera(45.0f, screenWidth, screenHeight, 0.1f, 100.0f);
    Image image(screenWidth, screenHeight);
    Image accumulationImage(screenWidth, screenHeight);

    double prevTime = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currTime = glfwGetTime();
        double delta = currTime - prevTime;
        prevTime = currTime;

        /* Poll for and process events */
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glfwGetWindowSize(window, &screenWidth, &screenHeight);
        
        if (camera.OnUpdate(window, delta))
            renderer.ResetFrameIndex();
        image.OnResize(screenWidth, screenHeight);
        accumulationImage.OnResize(screenWidth, screenHeight);
        camera.OnResize(screenWidth, screenHeight);
        renderer.OnResize(screenWidth, screenHeight);
        renderer.Render(scene, camera, image, accumulationImage);

        glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image.GetPixels().data());

        ImGui::Begin("PATH TRACER");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Checkbox("Accumulate", &renderer.GetSettings().Accumulate);
        ImGui::Checkbox("Environment", &renderer.GetSettings().ShowEnvironment);
        if (ImGui::BeginCombo("Choose an Image", scene.EnvironmentImages[scene.SelectedEnvironment].GetName().c_str())) {
            for (int i = 0; i < scene.EnvironmentImages.size(); ++i) {
                bool isSelected = (i == scene.SelectedEnvironment);
                if (ImGui::Selectable(scene.EnvironmentImages[i].GetName().c_str(), isSelected)) {
                    scene.SelectedEnvironment = i;
                    // Handle the selection change here if needed
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::DragFloat("EnvironmentStrength", &scene.EnvironmetStrength, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("EnvironmentRotation", &scene.EnvironmentRotation, 0.1f, 0.0f, 360.0f);
        for (uint32_t i = 0; i < scene.Models.size(); i++)
        {
            Model& model = scene.Models[i];
            for (uint32_t j = 0; j < model.GetMeshes().size(); j++)
            {
                ImGui::PushID(i + j);
                Mesh& mesh = model.GetMeshes()[j];
                Material& material = mesh.GetMaterial();
                std::string title = mesh.GetName() + " (" + material.Name + ")";
                if (ImGui::CollapsingHeader(title.c_str())) {
                    if (material.DiffuseTextureIndex >= 0) {
                        ImGui::Text("Diffused texture used");
                    }
                    else {
                        ImGui::ColorEdit3("DiffuseColor", glm::value_ptr(material.DiffuseColor));
                    }

                    ImGui::ColorEdit3("Emissive Color", glm::value_ptr(material.EmissionColor));
                    ImGui::DragFloat("Emissive", &material.EmissionPower, 0.01f, 0.0f, FLT_MAX);
                    //ImGui::DragFloat("Metallic", &material.Metallic, 0.01f, 0.0f, 1.0f);
                    if (material.ShininessTextureIndex >= 0) {
                        ImGui::Text("Roughness texture used");
                    }
                    else {
                        ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
                    }
                    //ImGui::DragFloat("Shininess", &material.Shininess, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Specular", &material.Specular, 0.01f, 0.0f, 1.0f);
                    ImGui::Separator();
                }
                ImGui::PopID();
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glViewport(0, 0, screenWidth, screenHeight);

        glfwSwapBuffers(window);
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}