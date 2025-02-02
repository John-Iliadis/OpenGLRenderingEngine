//
// Created by Gianni on 24/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MODEL_HPP
#define OPENGLRENDERINGENGINE_MODEL_HPP

#include <glm/glm.hpp>
#include "../app/simple_notification_service.hpp"
#include "bounding_box.hpp"
#include "instanced_mesh.hpp"

class Model : public SubscriberSNS
{
public:
    struct Node
    {
        std::string name;
        glm::mat4 transformation;
        std::optional<uuid64_t> meshID;
        std::optional<std::string> materialName;
        std::vector<Node> children;
    };

public:
    Node root;
    BoundingBox bb;

public:
    Model();

    void notify(const Message &message) override;

    std::optional<uuid64_t> getMaterialID(const Model::Node& node) const;

private:
    std::unordered_map<std::string, uuid64_t> mMappedMaterials;
};

#endif //OPENGLRENDERINGENGINE_MODEL_HPP
