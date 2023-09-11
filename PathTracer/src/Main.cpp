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
    {
        Material defaultMaterial;
        defaultMaterial.Albedo = glm::vec3(1.0f, 0.0f, 1.0f);
        defaultMaterial.Roughness = 0.0f;
        defaultMaterial.Metallic = 0.0f;
        scene.Materials.push_back(defaultMaterial);
    }
    {
        Material material;
        material.Albedo = glm::vec3(1.0f, 1.0f, 0.0f);
        material.Roughness = 1.0f;
        material.Metallic = 0.0f;
        scene.Materials.push_back(material);
    }
    {
        Material material;
        material.Albedo = glm::vec3(0.0f, 1.0f, 0.0f);
        material.Roughness = 0.2f;
        material.Metallic = 0.0f;
        scene.Materials.push_back(material);
    }
    {
        Material material;
        material.Albedo = glm::vec3(0.8f, 0.5f, 0.2f);
        material.Roughness = 0.1f;
        material.EmissionColor = material.Albedo;
        material.EmissionPower = 10.0f;
        scene.Materials.push_back(material);
    }
    /*
    {
        Sphere sphere;
        sphere.Position = glm::vec3(0.5f, -0.3f, 0.0f);
        sphere.Radius = 0.25f;
        sphere.MaterialIndex = 1;
        scene.Spheres.push_back(sphere);
    }
    {
        Sphere sphere;
        sphere.Position = glm::vec3(1.0f, -10.5f, -1.0f);
        sphere.Radius = 10.0f;
        sphere.MaterialIndex = 2;
        scene.Spheres.push_back(sphere);
    }
    {
        Sphere sphere;
        sphere.Position = glm::vec3(5.5f, 1.5f, -5.0f);
        sphere.Radius = 3.5f;
        sphere.MaterialIndex = 3;
        scene.Spheres.push_back(sphere);
    }
    */
    {
        Mesh monkey("Models/monkey.obj", glm::vec3(0.0f, 0.0f, -1.0f));
        monkey.SetMaterialIndex(2);
        scene.Meshes.push_back(monkey);
    }
    {
        Mesh plane("Models/plane.obj", glm::vec3(0.0f, -1.0f, 0.0f));
        plane.SetMaterialIndex(1);
        scene.Meshes.push_back(plane);
    }
    {
        Mesh light("Models/cube1.obj", glm::vec3(2.0f, 2.0f, 3.0f));
        light.SetMaterialIndex(3);
        scene.Meshes.push_back(light);
    }
#pragma endregion

    Renderer renderer;
    Camera camera(45.0f, screenWidth, screenHeight, 0.1f, 100.0f);
    Image image(screenWidth, screenHeight);
    Image accumulationImage(screenWidth, screenHeight);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {

        /* Poll for and process events */
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glfwGetWindowSize(window, &screenWidth, &screenHeight);
        
        if (camera.OnUpdate(window, io.Framerate / 100.0f))
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
        ImGui::Text("MATERIALS");
        for (uint32_t i = 1; i < scene.Materials.size(); i++)
        {
            ImGui::PushID(i);
            Material& material = scene.Materials[i];
            ImGui::Text("Material %i", i);
            ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
            ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
            ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.EmissionColor));
            ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);
            ImGui::Separator();
            ImGui::PopID();
        }
        /*
        ImGui::Text("MESHES");
        for (uint32_t i = 0; i < scene.Meshes.size(); i++)
        {
            ImGui::PushID(i);
            Mesh& mesh = scene.Meshes[i];
            ImGui::Text("Mesh %i", i);
            ImGui::DragFloat3("Position", glm::value_ptr(mesh.Position()), 0.01f);
            ImGui::DragFloat("Scale", &mesh.Scale());
            ImGui::Text("Uses material %i", mesh.GetMaterialIndex());
            ImGui::Separator();
            ImGui::PopID();
        }
        */
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