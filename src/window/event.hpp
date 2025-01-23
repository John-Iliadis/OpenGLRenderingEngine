//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_EVENT_HPP
#define OPENGLRENDERINGENGINE_EVENT_HPP

class Event
{
public:
    struct ResizeEvent
    {
        int width;
        int height;
    };

    struct KeyEvent
    {
        int key;
        int action;
    };

    struct MouseButtonEvent
    {
        int button;
        int action;
    };

    struct MouseWheelEvent
    {
        double x;
        double y;
    };

    struct MouseMoveEvent
    {
        double x;
        double y;
    };

    enum EventType
    {
        Resized,
        Key,
        MouseWheelScrolled,
        MouseButton,
        MouseMoved,
        WindowMinimized,
        WindowRestored
    };

    EventType type;

    union
    {
        ResizeEvent size;
        KeyEvent key;
        MouseButtonEvent mouseButton;
        MouseWheelEvent mouseWheel;
        MouseMoveEvent mouseMove;
    };
};

#endif //OPENGLRENDERINGENGINE_EVENT_HPP
