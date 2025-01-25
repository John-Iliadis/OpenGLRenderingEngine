//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_APPLICATION_HPP
#define OPENGLRENDERINGENGINE_APPLICATION_HPP

#include "../window/window.hpp"
#include "../renderer/renderer.hpp"
#include "../resource/resource_manager.hpp"
#include "simple_notification_service.hpp"
#include "editor.hpp"

class Application
{
public:
    Application();
    ~Application();

    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();

    void countFPS(float dt);

private:
    Window mWindow;
    std::shared_ptr<Renderer> mRenderer;
    std::shared_ptr<ResourceManager> mResourceManager;
    Editor mEditor;
};

#endif //OPENGLRENDERINGENGINE_APPLICATION_HPP
