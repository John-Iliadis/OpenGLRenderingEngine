//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_WINDOW_HPP
#define OPENGLRENDERINGENGINE_WINDOW_HPP

#include <glfw/glfw3.h>
#include <glad/glad.h>
#include "../utils.hpp"
#include "event.hpp"
#include "input.hpp"

class Window
{
public:
    Window(uint32_t width, uint32_t height);
    ~Window();

    void pollEvents();
    void waitEvents();
    void swapBuffers();

    uint32_t width() const;
    uint32_t height() const;
    bool opened() const;

    operator GLFWwindow*() const;

    const std::vector<Event>& events() const;

private:
    static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* glfwWindow, double x, double y);
    static void mouseScrollCallback(GLFWwindow* glfwWindow, double x, double y);
    static void framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);

private:
    GLFWwindow* mWindow;
    std::vector<Event> mEventQueue;
    uint32_t mWidth;
    uint32_t mHeight;
};

#endif //OPENGLRENDERINGENGINE_WINDOW_HPP
