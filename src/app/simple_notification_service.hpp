//
// Created by Gianni on 25/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_SIMPLE_NOTIFICATION_SERVICE_HPP
#define OPENGLRENDERINGENGINE_SIMPLE_NOTIFICATION_SERVICE_HPP

#include "../renderer/instanced_mesh.hpp"

class Message
{
public:
    struct MaterialDeleted
    {
        uint32_t deletedIndex;
        uint32_t defaultMaterialIndex;
        std::optional<uint32_t> movedMaterialIndex;
    };

    struct TextureDeleted
    {
        uint32_t deletedIndex;
        uint32_t defaultTextureIndex;
        std::optional<uint32_t> movedTextureIndex;
    };

    struct ModelDeleted
    {
        std::unordered_set<std::shared_ptr<InstancedMesh>> removedMeshes;
    };

    std::variant<MaterialDeleted,
        TextureDeleted,
        ModelDeleted> message;

    template<typename T>
    Message(const T& message) : message(message) {}

    template<typename T>
    T* getIf() { return std::get_if<T>(&message); }

    template<typename T>
    const T* getIf() const { return std::get_if<T>(&message); }
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
    Topic();
    Topic(Type topicType);

    void addSubscriber(SubscriberSNS* subscriber);
    void removeSubscriber(SubscriberSNS* subscriber);

    const std::unordered_set<SubscriberSNS*>& subscriberList() const;

private:
    Type mType;
    std::unordered_set<SubscriberSNS*> mSubscribers;
};

class SimpleNotificationService
{
public:
    static void init();
    static void subscribe(Topic::Type topicType, SubscriberSNS* subscriber);
    static void unsubscribe(Topic::Type topicType, SubscriberSNS* subscriber);
    static void publishMessage(Topic::Type topicType, const Message& message);

private:
    inline static std::array<Topic, Topic::Type::Count> mTopics;
};

#endif //OPENGLRENDERINGENGINE_SIMPLE_NOTIFICATION_SERVICE_HPP
