//
// Created by Gianni on 25/01/2025.
//

#include "simple_notification_service.hpp"

Topic::Topic()
    : mType(Topic::Type::None)
{
}

Topic::Topic(Topic::Type topicType)
    : mType(topicType)
{
}

void Topic::addSubscriber(SubscriberSNS *subscriber)
{
    mSubscribers.insert(subscriber);
}

void Topic::removeSubscriber(SubscriberSNS *subscriber)
{
    mSubscribers.erase(subscriber);
}

const std::unordered_set<SubscriberSNS*>& Topic::subscriberList() const
{
    return mSubscribers;
}

void SimpleNotificationService::init()
{
    for (uint32_t i = 0; i < Topic::Type::Count; ++i)
        mTopics.at(i) = Topic(static_cast<Topic::Type>(i));
}

void SimpleNotificationService::subscribe(Topic::Type topicType, SubscriberSNS *subscriber)
{
    mTopics.at(topicType).addSubscriber(subscriber);
}

void SimpleNotificationService::unsubscribe(Topic::Type topicType, SubscriberSNS *subscriber)
{
    mTopics.at(topicType).removeSubscriber(subscriber);
}

void SimpleNotificationService::publishMessage(Topic::Type topicType, const Message& message)
{
    for (SubscriberSNS* subscriber : mTopics.at(topicType).subscriberList())
    {
        subscriber->notify(message);
    }
}
