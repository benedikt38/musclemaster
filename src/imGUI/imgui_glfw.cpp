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
// ImGui_ImplXXXX_NewFrame(), Render() and ImGui_ImplXXXX_Shutdown(). If you are new to
// ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2018-03-20: Misc: Setup io.BackendFlags ImGuiBackendFlags_HasMouseCursors and
//  ImGuiBackendFlags_HasSetMousePos flags + honor ImGuiConfigFlags_NoMouseCursorChange flag.
//  2018-03-06: OpenGL: Added const char* glsl_version parameter to Init() so user can override the
//  GLSL version e.g. "#version 150". 2018-02-23: OpenGL: Create the VAO in the render function so
//  the setup can more easily be used with multiple shared GL context. 2018-02-20: Inputs: Added
//  support for mouse cursors (GetMouseCursor() value, passed to glfwSetCursor()).
//  2018-02-20: Inputs: Renamed GLFW callbacks exposed in .h to not include GL3 in their name.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed RenderDrawData() in
//  the .h file so you can call it yourself. 2018-02-06: Misc: Removed call to Shutdown()
//  which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext
//  themselves. 2018-02-06: Inputs: Added mapping for ImGuiKey_Space. 2018-01-25: Inputs: Added
//  gamepad support if ImGuiConfigFlags_NavEnableGamepad is set. 2018-01-25: Inputs: Honoring the
//  io.WantSetMousePos flag by repositioning the mouse (ImGuiConfigFlags_NavEnableSetMousePos is
//  set). 2018-01-20: Inputs: Added Horizontal Mouse Wheel support. 2018-01-18: Inputs: Added
//  mapping for ImGuiKey_Insert. 2018-01-07: OpenGL: Changed GLSL shader version from 330 to 150.
//  (Also changed GL context from 3.3 to 3.2 in example's main.cpp) 2017-09-01: OpenGL: Save and
//  restore current bound sampler. Save and restore current polygon mode. 2017-08-25: Inputs:
//  MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2017-05-01: OpenGL: Fixed save and restore of current blend function state.
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.
//  2016-09-05: OpenGL: Fixed save and restore of current scissor rectangle.
//  2016-04-30: OpenGL: Fixed save and restore of current GL_ACTIVE_TEXTURE.

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui_glfw.h"
#include "imgui.h"
#include "src/iconfont/IconsMaterialDesignIcons.h"
#include <filesystem>

// GL3W/GLFW
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <iostream>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

//#include "dino/input/Window.h"
//#include <dino/Paths.h>

namespace ImGui
{
void ImGui::render() const
{
    SetCurrentContext(m_context);
    Render();
    ImDrawData* drawData = GetDrawData();

    if(!drawData)
        return;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates !=
    // framebuffer coordinates)
    ImGuiIO&  io       = GetIO();
    const int fbWidth  = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    const int fbHeight = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if(fbWidth == 0 || fbHeight == 0)
        return;
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLenum lastActiveTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&lastActiveTexture));
    glActiveTexture(GL_TEXTURE0);
    GLint lastProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
    GLint lastTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    GLint lastSampler;
    glGetIntegerv(GL_SAMPLER_BINDING, &lastSampler);
    GLint lastArrayBuffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
    GLint lastElementArrayBuffer;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastElementArrayBuffer);
    GLint lastVertexArray;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArray);
    GLint lastPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, lastPolygonMode);
    GLint lastViewport[4];
    glGetIntegerv(GL_VIEWPORT, lastViewport);
    GLint lastScissorBox[4];
    glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox);
    GLenum lastBlendSrcRgb;
    glGetIntegerv(GL_BLEND_SRC_RGB, reinterpret_cast<GLint*>(&lastBlendSrcRgb));
    GLenum lastBlendDstRgb;
    glGetIntegerv(GL_BLEND_DST_RGB, reinterpret_cast<GLint*>(&lastBlendDstRgb));
    GLenum lastBlendSrcAlpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, reinterpret_cast<GLint*>(&lastBlendSrcAlpha));
    GLenum lastBlendDstAlpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, reinterpret_cast<GLint*>(&lastBlendDstAlpha));
    GLenum lastBlendEquationRgb;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, reinterpret_cast<GLint*>(&lastBlendEquationRgb));
    GLenum lastBlendEquationAlpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, reinterpret_cast<GLint*>(&lastBlendEquationAlpha));
    const GLboolean lastEnableBlend       = glIsEnabled(GL_BLEND);
    const GLboolean lastEnableCullFace    = glIsEnabled(GL_CULL_FACE);
    const GLboolean lastEnableDepthTest   = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor
    // enabled, polygon fill
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));
    const float ortho_projection[4][4] = {
            {2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f, 1.0f},
    };
    glUseProgram(m_shaderHandle);
    glUniform1i(m_attrLocationTex, 0);
    glUniformMatrix4fv(m_attrLocationProj, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindSampler(0, 0); // Rely on combined texture/sampler state.

    // Recreate the VAO every time
    // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we
    // don't track creation/deletion of windows so we don't have an obvious key to use to cache
    // them.)
    GLuint vaoHandle = 0;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle);
    glEnableVertexAttribArray(m_attrLocationPos);
    glEnableVertexAttribArray(m_attrLocationUv);
    glEnableVertexAttribArray(m_attrLocationColor);
    glVertexAttribPointer(m_attrLocationPos,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ImDrawVert),
                          reinterpret_cast<const void*>(offsetof(ImDrawVert, pos)));
    glVertexAttribPointer(m_attrLocationUv,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ImDrawVert),
                          reinterpret_cast<const void*>(offsetof(ImDrawVert, uv)));
    glVertexAttribPointer(m_attrLocationColor,
                          4,
                          GL_UNSIGNED_BYTE,
                          GL_TRUE,
                          sizeof(ImDrawVert),
                          reinterpret_cast<const void*>(offsetof(ImDrawVert, col)));

    // Draw
    for(int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList         = drawData->CmdLists[n];
        const ImDrawIdx*  idxBufferOffset = nullptr;

        glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(cmdList->VtxBuffer.Size * sizeof(ImDrawVert)),
                     static_cast<const void*>(cmdList->VtxBuffer.Data),
                     GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(cmdList->IdxBuffer.Size * sizeof(ImDrawIdx)),
                     static_cast<const void*>(cmdList->IdxBuffer.Data),
                     GL_STREAM_DRAW);

        for(int i = 0; i < cmdList->CmdBuffer.Size; i++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[i];
            if(pcmd->UserCallback)
            {
                pcmd->UserCallback(cmdList, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D,
                              static_cast<uint32_t>(reinterpret_cast<uint64_t>(pcmd->TextureId)));
                glScissor(static_cast<int>(pcmd->ClipRect.x),
                          static_cast<int>(fbHeight - pcmd->ClipRect.w),
                          static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x),
                          static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES,
                               static_cast<GLsizei>(pcmd->ElemCount),
                               sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                               idxBufferOffset);
            }
            idxBufferOffset += pcmd->ElemCount;
        }
    }
    glDeleteVertexArrays(1, &vaoHandle);

    // Restore modified GL state
    glUseProgram(lastProgram);
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glBindSampler(0, lastSampler);
    glActiveTexture(lastActiveTexture);
    glBindVertexArray(lastVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastElementArrayBuffer);
    glBlendEquationSeparate(lastBlendEquationRgb, lastBlendEquationAlpha);
    glBlendFuncSeparate(lastBlendSrcRgb, lastBlendDstRgb, lastBlendSrcAlpha, lastBlendDstAlpha);
    if(lastEnableBlend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    if(lastEnableCullFace)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    if(lastEnableDepthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    if(lastEnableScissorTest)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(lastPolygonMode[0]));
    glViewport(lastViewport[0],
               lastViewport[1],
               static_cast<GLsizei>(lastViewport[2]),
               static_cast<GLsizei>(lastViewport[3]));
    glScissor(lastScissorBox[0],
              lastScissorBox[1],
              static_cast<GLsizei>(lastScissorBox[2]),
              static_cast<GLsizei>(lastScissorBox[3]));
}

void ImGui::mouseButtonCallback(GLFWwindow*, const int button, const int action, int /*mods*/)
{
    SetCurrentContext(m_context);
    if(action == GLFW_PRESS && button >= 0 && button < 3)
        m_mouseButtons[button] = true;
}

void ImGui::scrollCallback(GLFWwindow*, const double xoffset, const double yoffset) const
{
    SetCurrentContext(m_context);
    ImGuiIO& io = GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);
}

void ImGui::keyCallback(GLFWwindow*, const int key, int, const int action, int mods) const
{
    SetCurrentContext(m_context);
    ImGuiIO& io = GetIO();
    if(action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if(action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    (void)mods; // Modifiers are not reliable across systems
    io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void ImGui::charCallback(GLFWwindow*, const unsigned int c) const
{
    SetCurrentContext(m_context);
    ImGuiIO& io = GetIO();
    if(c > 0 && c < 0x10000)
        io.AddInputCharacter(static_cast<ImWchar>(c));
}

bool ImGui::createFontsTexture()
{
    SetCurrentContext(m_context);
    // Build texture atlas
    ImGuiIO&       io = GetIO();
    unsigned char* pixels;
    int            width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); // Load as RGBA 32-bits (75% of the
                                                            // memory is wasted, but default font is
                                                            // so small) because it is more likely
                                                            // to be compatible with user's existing
                                                            // shaders. If your ImTextureId
                                                            // represent a higher-level concept than
                                                            // just a GL texture id, consider
                                                            // calling GetTexDataAsAlpha8() instead
                                                            // to save on GPU memory.

    // Upload texture to graphics system
    GLint lastTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = reinterpret_cast<ImTextureID>(uint64_t(m_fontTexture));

    // Restore state
    glBindTexture(GL_TEXTURE_2D, lastTexture);

    return true;
}

bool ImGui::createDeviceObjects()
{
    SetCurrentContext(m_context);
    // Backup GL state
    GLint lastTexture, lastArrayBuffer, lastVertexArray;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArray);

    const GLchar* vertexShader = "\nlayout(location=1) uniform mat4 ProjMtx;\n"
                                 "layout(location=0) in vec2 Position;\n"
                                 "layout(location=1) in vec2 UV;\n"
                                 "layout(location=2) in vec4 Color;\n"
                                 "out vec2 Frag_UV;\n"
                                 "out vec4 Frag_Color;\n"
                                 "void main()\n"
                                 "{\n"
                                 "	Frag_UV = UV;\n"
                                 "	Frag_Color = Color;\n"
                                 "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                                 "}\n";

    const GLchar* fragmentShader = "\nlayout(location=0) uniform sampler2D Texture;\n"
                                   "in vec2 Frag_UV;\n"
                                   "in vec4 Frag_Color;\n"
                                   "out vec4 Out_Color;\n"
                                   "void main()\n"
                                   "{\n"
                                   "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
                                   "}\n";

    const GLchar* vertexShaderSource[2]   = {m_glslVersion.c_str(), vertexShader};
    const GLchar* fragmentShaderSource[2] = {m_glslVersion.c_str(), fragmentShader};

    m_shaderHandle = glCreateProgram();
    m_vertHandle   = glCreateShader(GL_VERTEX_SHADER);
    m_fragHandle   = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_vertHandle, 2, vertexShaderSource, nullptr);
    glShaderSource(m_fragHandle, 2, fragmentShaderSource, nullptr);
    glCompileShader(m_vertHandle);
    glCompileShader(m_fragHandle);
    glAttachShader(m_shaderHandle, m_vertHandle);
    glAttachShader(m_shaderHandle, m_fragHandle);
    glLinkProgram(m_shaderHandle);

    GLint       status;
    std::string log;
    glGetProgramiv(m_shaderHandle, GL_LINK_STATUS, &status);
    if(GL_FALSE == status)
    {
        GLint logLen;
        glGetProgramiv(m_shaderHandle, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0)
        {
            log.resize(logLen);
            GLsizei written;
            glGetProgramInfoLog(m_shaderHandle, logLen, &written, &log[0]);
        }
    }

    m_attrLocationTex   = glGetUniformLocation(m_shaderHandle, "Texture");
    m_attrLocationProj  = glGetUniformLocation(m_shaderHandle, "ProjMtx");
    m_attrLocationPos   = glGetAttribLocation(m_shaderHandle, "Position");
    m_attrLocationUv    = glGetAttribLocation(m_shaderHandle, "UV");
    m_attrLocationColor = glGetAttribLocation(m_shaderHandle, "Color");

    glGenBuffers(1, &m_vboHandle);
    glGenBuffers(1, &m_elementHandle);

    createFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
    glBindVertexArray(lastVertexArray);

    return true;
}

void ImGui::invalidateDeviceObjects()
{
    SetCurrentContext(m_context);
    if(m_vboHandle)
        glDeleteBuffers(1, &m_vboHandle);
    if(m_elementHandle)
        glDeleteBuffers(1, &m_elementHandle);
    m_vboHandle = m_elementHandle = 0;

    if(m_shaderHandle && m_vertHandle)
        glDetachShader(m_shaderHandle, m_vertHandle);
    if(m_vertHandle)
        glDeleteShader(m_vertHandle);
    m_vertHandle = 0;

    if(m_shaderHandle && m_fragHandle)
        glDetachShader(m_shaderHandle, m_fragHandle);
    if(m_fragHandle)
        glDeleteShader(m_fragHandle);
    m_fragHandle = 0;

    if(m_shaderHandle)
        glDeleteProgram(m_shaderHandle);
    m_shaderHandle = 0;

    if(m_fontTexture)
    {
        glDeleteTextures(1, &m_fontTexture);
        GetIO().Fonts->TexID = 0;
        m_fontTexture        = 0;
    }
}

ImGui::ImGui(GLFWwindow* window, const bool installCallbacks, const std::string& glslVersion)
        : m_context(CreateContext())
        , m_window(window)
        , m_glslVersion(glslVersion)
        , m_mouseCursors(ImGuiMouseCursor_COUNT, nullptr)
{
    SetCurrentContext(m_context);
    // Setup back-end capabilities flags
    ImGuiIO& io = GetIO();
    io.BackendFlags |=
            ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests
                                                         // (optional, rarely used)

    auto p = RESOURCES_PATH "/ui/Roboto-Medium.ttf";
    if(std::filesystem::exists(p))
        io.Fonts->AddFontFromFileTTF(RESOURCES_PATH "/ui/Roboto-Medium.ttf", 14.f);
    static const ImWchar icon_range[] = {ICON_MIN_MDI, ICON_MAX_MDI, 0};
    ImFontConfig         icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF(
                RESOURCES_PATH "/ui/materialdesignicons-webfont.ttf", 14.f, &icons_config, icon_range);
    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]        = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]   = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]       = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End]        = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert]     = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete]     = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]      = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter]      = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape]     = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C]          = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V]          = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X]          = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y]          = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z]          = GLFW_KEY_Z;

    io.GetClipboardTextFn = [](void* user_data) {
        return glfwGetClipboardString(static_cast<GLFWwindow*>(user_data));
    };

    io.SetClipboardTextFn = [](void* user_data, const char* text) {
        glfwSetClipboardString(static_cast<GLFWwindow*>(user_data), text);
    };

    io.ClipboardUserData = m_window;
    io.UserData          = this;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(m_window);
#endif

    // Load cursors
    // FIXME: GLFW doesn't expose suitable cursors for ResizeAll, ResizeNESW, ResizeNWSE. We revert
    // to arrow cursor for those.
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR > 1
    m_mouseCursors[ImGuiMouseCursor_Arrow]      = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    m_mouseCursors[ImGuiMouseCursor_TextInput]  = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    m_mouseCursors[ImGuiMouseCursor_ResizeAll]  = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    m_mouseCursors[ImGuiMouseCursor_ResizeNS]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    m_mouseCursors[ImGuiMouseCursor_ResizeEW]   = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    m_mouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    m_mouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif

    /*if(installCallbacks)
    {
        glfwSetMouseButtonCallback(
                window, [](GLFWwindow* w, const int button, const int action, const int m) {
                    auto * window =
                            static_cast<dino::Window*>(glfwGetWindowUserPointer(w));
                    window->m_gui.mouseButtonCallback(w, button, action, m);
                });
        glfwSetScrollCallback(window,
                              [](GLFWwindow* w, const double xoffset, const double yoffset) {
                                  const dino::Window* window =
                                          static_cast<dino::Window*>(glfwGetWindowUserPointer(w));
                                  window->m_gui.scrollCallback(w, xoffset, yoffset);
                              });
        glfwSetKeyCallback(
                window,
                [](GLFWwindow* w, const int key, const int sc, const int action, const int mods) {
                    const dino::Window* window =
                            static_cast<dino::Window*>(glfwGetWindowUserPointer(w));
                    window->m_gui.keyCallback(w, key, sc, action, mods);
                });
        glfwSetCharCallback(window, [](GLFWwindow* w, const unsigned int c) {
            const dino::Window* window = static_cast<dino::Window*>(glfwGetWindowUserPointer(w));
            window->m_gui.charCallback(w, c);
        });
    }*/
    ImGuiStyle& style       = GetStyle();
    style.WindowRounding    = 0.f;
    style.ScrollbarRounding = 0.f;
    style.WindowBorderSize  = 0.f;
    style.ChildBorderSize   = 0.f;
}

ImGui::~ImGui()
{
    SetCurrentContext(m_context);
    invalidateDeviceObjects();
    // Destroy GLFW mouse cursors
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR > 1
    for(ImGuiMouseCursor cur = 0; cur < ImGuiMouseCursor_COUNT; cur++)
        glfwDestroyCursor(m_mouseCursors[cur]);
#endif
    m_mouseCursors.clear();

    // Destroy OpenGL objects
    DestroyContext(m_context);
}

void ImGui::newFrame()
{
    SetCurrentContext(m_context);
    if(!m_fontTexture)
        createDeviceObjects();

    ImGuiIO& io = GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int displayWidth, displayHeight;
    glfwGetWindowSize(m_window, &w, &h);
    glfwGetFramebufferSize(m_window, &displayWidth, &displayHeight);
    io.DisplaySize             = ImVec2(static_cast<float>(w), static_cast<float>(h));
    io.DisplayFramebufferScale = ImVec2(w > 0 ? (static_cast<float>(displayWidth) / w) : 0,
                                        h > 0 ? (static_cast<float>(displayHeight) / h) : 0);

    // Setup time step
    const double currentTime = glfwGetTime();
    io.DeltaTime             = m_time > 0.0 ? static_cast<float>(currentTime - m_time)
                                : static_cast<float>(1.0f / 60.0f);
    m_time = currentTime;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in
    // glfwPollEvents())
    if(glfwGetWindowAttrib(m_window, GLFW_FOCUSED))
    {
        // Set OS mouse position if requested (only used when ImGuiConfigFlags_NavEnableSetMousePos
        // is enabled by user)
        if(io.WantSetMousePos)
        {
            glfwSetCursorPos(m_window,
                             static_cast<double>(io.MousePos.x),
                             static_cast<double>(io.MousePos.y));
        }
        else
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(m_window, &mouse_x, &mouse_y);
            io.MousePos = ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
        }
    }
    else
    {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    for(int i = 0; i < 3; i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss
        // click-release events that are shorter than 1 frame.
        io.MouseDown[i]   = m_mouseButtons[i] || glfwGetMouseButton(m_window, i) != 0;
        m_mouseButtons[i] = false;
    }

    // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
    if((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0 &&
       glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    {
        const ImGuiMouseCursor cursor = GetMouseCursor();
        if(io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        else
        {
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR > 1
            glfwSetCursor(m_window,
                          m_mouseCursors[cursor] ? m_mouseCursors[cursor]
                                                 : m_mouseCursors[ImGuiMouseCursor_Arrow]);
#endif
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // Gamepad navigation mapping [BETA]
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if(io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)
    {
// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)                                                              \
    {                                                                                              \
        if(buttonsCount > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS)                           \
            io.NavInputs[NAV_NO] = 1.0f;                                                           \
    }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1)                                                        \
    {                                                                                              \
        float v = (axesCount > AXIS_NO) ? axes[AXIS_NO] : V0;                                      \
        v       = (v - V0) / (V1 - V0);                                                            \
        if(v > 1.0f)                                                                               \
            v = 1.0f;                                                                              \
        if(io.NavInputs[NAV_NO] < v)                                                               \
            io.NavInputs[NAV_NO] = v;                                                              \
    }
        int                  axesCount = 0, buttonsCount = 0;
        const float*         axes    = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonsCount);
        MAP_BUTTON(ImGuiNavInput_Activate, 0);   // Cross / A
        MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
        MAP_BUTTON(ImGuiNavInput_Menu, 2);       // Square / X
        MAP_BUTTON(ImGuiNavInput_Input, 3);      // Triangle / Y
        MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);  // D-Pad Left
        MAP_BUTTON(ImGuiNavInput_DpadRight, 11); // D-Pad Right
        MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
        MAP_BUTTON(ImGuiNavInput_DpadDown, 12);  // D-Pad Down
        MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);  // L1 / LB
        MAP_BUTTON(ImGuiNavInput_FocusNext, 5);  // R1 / RB
        MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);  // L1 / LB
        MAP_BUTTON(ImGuiNavInput_TweakFast, 5);  // R1 / RB
        MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
        if(axesCount > 0 && buttonsCount > 0)
            io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
        else
            io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    }

    // Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag
    // that you can use to dispatch inputs (or not) to your application.
    NewFrame();
}
} // namespace ImGui
