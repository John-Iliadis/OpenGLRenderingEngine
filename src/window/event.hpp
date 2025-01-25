//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_EVENT_HPP
#define OPENGLRENDERINGENGINE_EVENT_HPP

class Event
{
public:
    struct WindowResize
    {
        int width;
        int height;
    };

    struct Key
    {
        int key;
        int action;
    };

    struct MouseButton
    {
        int button;
        int action;
    };

    struct MouseWheel
    {
        double x;
        double y;
    };

    struct MouseMove
    {
        double x;
        double y;
    };

    std::variant<WindowResize,
        Key,
        MouseButton,
        MouseWheel,
        MouseMove> data;

    template<typename T>
    Event(const T& event) : data(event) {}

    template<typename T>
    T* getIf() { return std::get_if<T>(&data); }

    template<typename T>
    const T* getIf() const { return std::get_if<T>(&data); }
};

#endif //OPENGLRENDERINGENGINE_EVENT_HPP
