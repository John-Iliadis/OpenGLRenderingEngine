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

    std::variant<ResizeEvent,
        KeyEvent,
        MouseButtonEvent,
        MouseWheelEvent,
        MouseMoveEvent> data;

    template<typename T>
    constexpr bool is() const { return static_cast<bool>(std::get_if<T>(data)); }
};

#endif //OPENGLRENDERINGENGINE_EVENT_HPP
