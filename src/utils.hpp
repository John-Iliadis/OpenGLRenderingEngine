//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_UTILS_HPP
#define OPENGLRENDERINGENGINE_UTILS_HPP

void debugLog(const std::string& logMSG);

void check(bool result, const char* msg, std::source_location location = std::source_location::current());

#endif //OPENGLRENDERINGENGINE_UTILS_HPP
