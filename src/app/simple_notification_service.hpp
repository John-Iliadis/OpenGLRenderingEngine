//
// Created by Gianni on 25/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_SIMPLE_NOTIFICATION_SERVICE_HPP
#define OPENGLRENDERINGENGINE_SIMPLE_NOTIFICATION_SERVICE_HPP

#include "../renderer/instanced_mesh.hpp"

class Message
{
public:
    struct ModelDeleted
    {
        uint32_t modelID;
        std::vector<uint32_t> meshIDs;
    };

    struct MaterialDeleted
    {
        uint32_t removeIndex;
        std::optional<uint32_t> transferIndex;
    };

    struct TextureDeleted
    {
        uint32_t removedIndex;
        std::optional<uint32_t> transferIndex;
    };

    struct MeshInstanceUpdate
    {
        uint32_t meshID;
        uint32_t objectID;
        uint32_t instanceID;
        uint32_t materialIndex;
        glm::mat4 transformation;
    };

    struct MaterialRemap
    {
        uint32_t meshID;
        uint32_t materialIndex;
    };

    std::variant<ModelDeleted,
        MaterialDeleted,
        TextureDeleted,
        MeshInstanceUpdate,
        MaterialRemap> message;

    template<typename T>
    Message(const T& message) : message(message) {}

    template <typename T, typename... Args>
    Message(std::in_place_type_t<T>, Args&&... args)
        : message(std::in_place_type<T>, std::forward<Args>(args)...)
    {
    }

    template<typename T>
    T* getIf() { return std::get_if<T>(&message); }

    template<typename T>
    const T* getIf() const { return std::get_if<T>(&message); }

    template<typename T, typename... Args>
    static Message create(Args&&... args)
    {
        return Message(std::in_place_type<T>, std::forward<Args>(args)...);
    }
};

class SubscriberSNS
{
public:
    SubscriberSNS() = default;
    virtual ~SubscriberSNS() = default;
    virtual void notify(const Message& message) {};
};

class Topic
{
public:
    enum Type
    {
        None = 0,
        Resource,
        Count
    };

public:
    Topic() = default;

    void addSubscriber(SubscriberSNS* subscriber);
    void removeSubscriber(SubscriberSNS* subscriber);
    void publish(const Message& message);

private:
    std::unordered_set<SubscriberSNS*> mSubscribers;
};

class SimpleNotificationService
{
public:
    static void subscribe(Topic::Type topicType, SubscriberSNS* subscriber);
    static void unsubscribe(Topic::Type topicType, SubscriberSNS* subscriber);
    static void publishMessage(Topic::Type topicType, const Message& message);

private:
    inline static std::array<Topic, Topic::Type::Count> mTopics;
};

#endif //OPENGLRENDERINGENGINE_SIMPLE_NOTIFICATION_SERVICE_HPP
