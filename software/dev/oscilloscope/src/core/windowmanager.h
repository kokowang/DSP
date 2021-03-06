#pragma once

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


// ImPlot
#include "implot.h"

// GLFW
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Vulkan
#include <vulkan/vulkan.h>

// C++
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

class WindowManager {
    public:
        GLFWwindow* window;
        int windowWidth, windowHeight;
        bool vsync;

        // Vulkan Variables
        ImGui_ImplVulkanH_Window* wd;
        ImDrawData* draw_data;

        VkResult err;

        VkAllocationCallbacks*   g_Allocator = NULL;
        VkInstance               g_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                 g_Device = VK_NULL_HANDLE;
        uint32_t                 g_QueueFamily = (uint32_t)-1;
        VkQueue                  g_Queue = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
        VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

        ImGui_ImplVulkanH_Window g_MainWindowData;
        int                      g_MinImageCount = 2;
        bool                     g_SwapChainRebuild = false;

        int initialize(const char* windowName, int windowWidth, int windowHeight, bool vsync = true);

        void destroy();
        
        bool shouldClose();

        void pollEvents();

        bool isMinimized();

        void checkSwapChainRebuild();

        void renderImGui();

        void renderFrame();

        void drawFrame();

        void glfw_key_callback(int key, int scancode, int actions, int mods);

        void glfw_framebuffer_size_callback(int width, int height); //todo

    private:
        // Returns no error, or ERR_CODE
        int initializeWindow(const char* windowName, int windowWidth, int windowHeight);

        static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int actions, int mods);

        static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);

        static void glfw_error_callback(int error, const char* description);

        void setupVulkan(const char** extensions, uint32_t extensions_count);

        void setupVulkanWindow(VkSurfaceKHR surface, int width, int height);

        void cleanupVulkan();

        void cleanupVulkanWindow();
};