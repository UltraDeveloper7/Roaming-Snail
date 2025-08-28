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

    [[nodiscard]] bool ShouldClose() const;
    void SetCloseFlag();
    [[nodiscard]] GLFWwindow* GetGLFWWindow() const;
    [[nodiscard]] bool Resized() const;
    [[nodiscard]] int GetWidth() const;
    [[nodiscard]] int GetHeight() const;
    void ResetResizedFlag();

    // (Optional helpers—safe to ignore; no breaking changes)
    void MakeContextCurrent();
    void SwapBuffers();
    void SetVSync(bool on);

private:
    // GLFW callbacks (static C hooks) -> forward to 'this'
    static void OnFramebufferResized(GLFWwindow* window, int width, int height);
    static void OnError(int error, const char* description);

    // init stages
    void initGlfw();
    void createWindow();
    void loadGL();

    // state
    int          width_ = 0;
    int          height_ = 0;
    bool         resized_ = false;
    std::string  title_;
    GLFWwindow* handle_ = nullptr;
};
