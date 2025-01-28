//
// Created by Gianni on 25/01/2025.
//

#include "simple_notification_service.hpp"

void Topic::addSubscriber(SubscriberSNS *subscriber)
{
    mSubscribers.insert(subscriber);
}

void Topic::removeSubscriber(SubscriberSNS *subscriber)
{
    mSubscribers.erase(subscriber);
}

void Topic::publish(const Message &message)
{
    for (auto subscriber : mSubscribers)
        subscriber->notify(message);
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
    mTopics.at(topicType).publish(message);
}
