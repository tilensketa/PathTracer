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
    /*scene.Models.push_back(Model("Models/leftmodel.obj"));
    scene.Models.push_back(Model("Models/rightmodel.obj"));
    scene.Models.push_back(Model("Models/light.obj"));
    scene.Models.push_back(Model("Models/leftwall.obj"));
    scene.Models.push_back(Model("Models/rightwall.obj"));
    scene.Models.push_back(Model("Models/backwall.obj"));
    scene.Models.push_back(Model("Models/ceiling.obj"));
    scene.Models.push_back(Model("Models/floor.obj"));
    scene.Models.push_back(Model("Models/texturedcube.obj"));*/

    scene.Models.push_back(Model("Models/cornellbox.obj"));
    scene.Models.push_back(Model("Models/texcube.obj"));

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
                    if (material.DiffuseTextureIndex >= 0)
                        ImGui::Text("Diffused texture used");
                    else
                        ImGui::ColorEdit3("DiffuseColor", glm::value_ptr(material.DiffuseColor));

                    ImGui::ColorEdit3("Emissive Color", glm::value_ptr(material.EmissionColor));
                    ImGui::DragFloat("Emissive", &material.EmissionPower, 0.01f, 0.0f, FLT_MAX);
                    ImGui::DragFloat("Metallic", &material.Metallic, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Shininess", &material.Shininess, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
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