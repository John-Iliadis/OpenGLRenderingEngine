//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_APPLICATION_HPP
#define OPENGLRENDERINGENGINE_APPLICATION_HPP

#include "window/window.hpp"

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
};


#endif //OPENGLRENDERINGENGINE_APPLICATION_HPP
