//
// Created by Gianni on 2/02/2025.
//

#ifndef OPENGLRENDERINGENGINE_UUID_REGISTRY_HPP
#define OPENGLRENDERINGENGINE_UUID_REGISTRY_HPP

#include "types.hpp"

enum class ObjectType
{
    Model,
    Material,
    Texture,
    Mesh,
    SceneNode
};

class UUIDRegistry
{
public:
    static uuid64_t generateModelID();
    static uuid64_t generateTextureID();
    static uuid64_t generateMaterialID();
    static uuid64_t generateMeshID();
    static uuid64_t generateSceneNodeID();
    static ObjectType getObjectType(uuid64_t id);

private:
    static uuid64_t generateID(ObjectType type);
    static inline std::unordered_map<uuid64_t, ObjectType> mIdToType;
};


#endif //OPENGLRENDERINGENGINE_UUID_REGISTRY_HPP
