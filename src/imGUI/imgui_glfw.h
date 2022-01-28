// ImGui GLFW binding with OpenGL3 + shaders
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan
// graphics context creation, etc.) (GL3W is a helper library to access OpenGL functions since there
// is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad,
// etc.)

// Implemented features:
//  [X] User texture binding. Cast 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the
//  FAQ about ImTextureID in imgui.cpp. [X] Gamepad navigation mapping. Enable with 'io.ConfigFlags
//  |= ImGuiConfigFlags_NavEnableGamepad'.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example
// of using this. If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(),
// ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to
// ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#pragma once
#include "imgui.h"
#include "GL/glew.h"
#include <string>
#include <vector>
struct GLFWwindow;
struct GLFWcursor;

namespace ImGui
{
class ImGui
{
public:
    ImGui(GLFWwindow* window, bool installCallbacks = true,
          const std::string& glslVersion = "#version 430");
    ~ImGui();

    void newFrame();
    void render() const;

    // GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
    // Provided here if you want to chain callbacks.
    // You can also handle inputs yourself and use those as a reference.
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) const;
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) const;
    void charCallback(GLFWwindow* window, unsigned int c) const;

private:
    // Use if you want to reset your rendering device without losing ImGui state.
    void invalidateDeviceObjects();
    bool createDeviceObjects();
    bool createFontsTexture();

    ImGuiContext*            m_context         = nullptr;
    GLFWwindow*              m_window          = nullptr;
    double                   m_time            = 0.0;
    bool                     m_mouseButtons[3] = {false, false, false};
    std::vector<GLFWcursor*> m_mouseCursors;

    // OpenGL3
    std::string m_glslVersion       = "#version 150";
    GLuint      m_fontTexture       = 0;
    int         m_shaderHandle      = 0;
    int         m_vertHandle        = 0;
    int         m_fragHandle        = 0;
    int         m_attrLocationTex   = 0;
    int         m_attrLocationProj  = 0;
    int         m_attrLocationPos   = 0;
    int         m_attrLocationUv    = 0;
    int         m_attrLocationColor = 0;
    uint32_t    m_vboHandle         = 0;
    uint32_t    m_elementHandle     = 0;
};
} // namespace ImGui
