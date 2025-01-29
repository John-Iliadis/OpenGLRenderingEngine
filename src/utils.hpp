//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_UTILS_HPP
#define OPENGLRENDERINGENGINE_UTILS_HPP

#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

void debugLog(const std::string& logMSG);

void check(bool result, const char* msg, std::source_location location = std::source_location::current());

uint32_t stringHash(const std::string& str);
uint32_t filesystemHash(const std::filesystem::path& path);

std::filesystem::path fileDialog();

std::string fileExtension(const std::filesystem::path& path);

using Task = std::function<void()>;

class MainThreadTaskQueue
{
public:
    void push(Task&& task);
    std::optional<Task> pop();

private:
    std::deque<Task> mTaskQueue;
    std::mutex mTaskQueueMutex;
};

#endif //OPENGLRENDERINGENGINE_UTILS_HPP
