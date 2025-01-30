//
// Created by Gianni on 21/01/2025.
//

#include "application.hpp"

static constexpr uint32_t sInitialWindowWidth = 1920;
static constexpr uint32_t sInitialWindowHeight = 1080;

Application::Application()
    : mWindow(sInitialWindowWidth, sInitialWindowHeight)
    , mRenderer(std::make_shared<Renderer>())
    , mResourceManager(std::make_shared<ResourceManager>())
    , mEditor(mRenderer, mResourceManager)
{
}

Application::~Application()
{
}

void Application::run()
{
    float currentTime = glfwGetTime();

    while (mWindow.opened())
    {
        float dt = glfwGetTime() - currentTime;
        currentTime = glfwGetTime();

        handleEvents();
        update(dt);
        render();
    }
}

void Application::handleEvents()
{
    mWindow.pollEvents();
    for (const Event& event : mWindow.events())
    {
    }
}

void Application::update(float dt)
{
    mEditor.update(dt);
    mResourceManager->processMainThreadTasks();
    countFPS(dt);
}

void Application::render()
{
    mEditor.render();
    mWindow.swapBuffers();
}

void Application::countFPS(float dt)
{
    static float frameCount = 0;
    static float accumulatedTime = 0.f;

    ++frameCount;
    accumulatedTime += dt;

    if (accumulatedTime >= 1.f)
    {
        debugLog(std::format("FPS: {}", frameCount));

        frameCount = 0;
        accumulatedTime = 0.f;
    }
}
