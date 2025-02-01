//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_RENDERER_HPP
#define OPENGLRENDERINGENGINE_RENDERER_HPP

#include <glad/glad.h>
#include "../window/event.hpp"
#include "../editor/camera.hpp"

class Editor;

class Renderer
{
public:
    Renderer();
    ~Renderer();

private:
    friend class Editor;
};

#endif //OPENGLRENDERINGENGINE_RENDERER_HPP
