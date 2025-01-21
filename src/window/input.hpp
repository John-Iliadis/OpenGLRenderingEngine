//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_INPUT_HPP
#define OPENGLRENDERINGENGINE_INPUT_HPP

#include <glfw/glfw3.h>
#include <glm/vec2.hpp>

class Input
{
public:
    static bool keyPressed(int key);
    static bool keyReleased(int key);
    static bool keyPressedCtrl(int key);
    static bool keyPressedShift(int key);

    static bool mouseButtonPressed(int button);
    static bool mouseButtonReleased(int button);
    static glm::vec2 mousePosition();

    static void updateKeyState(int key, int state);
    static void updateMouseButtonState(int button, int state);
    static void updateMousePosition(float x, float y);

private:
    inline static std::unordered_map<int, int> mKeyData;
    inline static std::unordered_map<int, int> mMouseData;
    inline static glm::vec2 mMousePosition;
};


#endif //OPENGLRENDERINGENGINE_INPUT_HPP
