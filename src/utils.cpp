//
// Created by Gianni on 21/01/2025.
//

#include "utils.hpp"

void debugLog(const std::string& logMSG)
{
    std::cout << "[Debug Log] " << logMSG << std::endl;
}

void check(bool result, const char* msg, std::source_location location)
{
    if (!result)
    {
        std::stringstream errorMSG;
        errorMSG << '`' << location.function_name() << "`: " << msg;

        throw std::runtime_error(errorMSG.str());
    }
}

uint32_t stringHash(const std::string& str)
{
    return static_cast<uint32_t>(std::hash<std::string>()(str));
}

uint32_t filesystemHash(const std::filesystem::path& path)
{
    return static_cast<uint32_t>(std::hash<std::filesystem::path>()(path));
}

std::filesystem::path fileDialog()
{
    char filename[260] {};
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lpstrFile = filename;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = glfwGetWin32Window(glfwGetCurrentContext());
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "All Files";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn) == TRUE)
        return filename;
    return {};
}

void MainThreadTaskQueue::push(Task &&task)
{
    std::lock_guard<std::mutex> lock(mTaskQueueMutex);
    mTaskQueue.push_back(std::move(task));
}

std::optional<Task> MainThreadTaskQueue::pop()
{
    std::lock_guard<std::mutex> lock(mTaskQueueMutex);

    if (!mTaskQueue.empty())
    {
        Task task = std::move(mTaskQueue.front());
        mTaskQueue.pop_front();
        return task;
    }

    return std::nullopt;
}
