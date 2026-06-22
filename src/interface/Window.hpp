#pragma once

class Window
{
public:
    Window();
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    // Thin GLFW wrapper API used by App.
    [[nodiscard]] bool ShouldClose() const;
    void SetCloseFlag();
    [[nodiscard]] GLFWwindow* GetGLFWWindow() const;
    [[nodiscard]] bool Resized() const;
    [[nodiscard]] int GetWidth() const;
    [[nodiscard]] int GetHeight() const;
    void ResetResizedFlag();

    // Context/presentation helpers.
    void MakeContextCurrent();
    void SwapBuffers();
    void SetVSync(bool on);
    void SetWin32WindowIconFromICO(const wchar_t* path);

private:
    // GLFW callbacks are static C hooks; implementation forwards them to the
    // Window instance stored in glfwSetWindowUserPointer.
    static void OnFramebufferResized(GLFWwindow* window, int width, int height);
    static void OnError(int error, const char* description);

    // Initialization stages kept separate for easier debugging/presentation.
    void initGlfw();
    void createWindow();
    void loadGL();

    // Window state mirrored from GLFW callbacks.
    int          width_ = 0;
    int          height_ = 0;
    bool         resized_ = false;
    std::string  title_;
    GLFWwindow* handle_ = nullptr;
};
