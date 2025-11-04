#include "KeyframeAnimationNode.h"

#include <algorithm>
#include <fstream>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace sgraph {
namespace {
    constexpr float kEpsilon = 1e-5f;

    inline glm::vec3 safeNormalize(const glm::vec3& v, const glm::vec3& fallback) {
        float len2 = glm::dot(v, v);
        if (len2 >= kEpsilon) {
            return glm::normalize(v);
        }
        float fallbackLen2 = glm::dot(fallback, fallback);
        if (fallbackLen2 >= kEpsilon) {
            return glm::normalize(fallback);
        }
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }
}

KeyframeAnimationNode::KeyframeAnimationNode(const std::string& name, sgraph::IScenegraph* graph)
    : AnimationNode(name, graph) {
}

bool KeyframeAnimationNode::loadKeyframesFromFile(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) {
        std::cerr << "Failed to open keyframe file: " << filepath << std::endl;
        return false;
    }

    size_t count = 0;
    in >> count;
    if (!in || count == 0) {
        std::cerr << "Invalid keyframe file header: " << filepath << std::endl;
        return false;
    }

    std::vector<glm::vec3> pos(count);
    std::vector<glm::vec3> ups(count);
    for (size_t i = 0; i < count; ++i) {
        float px, py, pz, ux, uy, uz;
        in >> px >> py >> pz >> ux >> uy >> uz;
        if (!in) {
            std::cerr << "Keyframe file truncated: " << filepath << std::endl;
            return false;
        }
        pos[i] = glm::vec3(px, py, pz);
        ups[i] = glm::vec3(ux, uy, uz);
    }

    setKeyframes(pos, ups);
    return true;
}

void KeyframeAnimationNode::setKeyframes(const std::vector<glm::vec3>& pos,
                                         const std::vector<glm::vec3>& ups) {
    positions = pos;
    upDirs = ups;
    if (positions.size() != upDirs.size()) {
        size_t minCount = std::min(positions.size(), upDirs.size());
        positions.resize(minCount);
        upDirs.resize(minCount);
    }
}

void KeyframeAnimationNode::setTransform(int tick) {
    if (positions.empty()) {
        glm::mat4 identity(1.0f);
        TransformNode::setTransform(identity);
        return;
    }

    size_t count = positions.size();
    size_t index = static_cast<size_t>(tick) % count;
    size_t nextIndex = (index + 1) % count;

    const glm::vec3& pos = positions[index];
    glm::vec3 front = positions[nextIndex] - pos;
    if (glm::dot(front, front) < kEpsilon) {
        // Fall back to a default forward if consecutive points coincide
        front = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    front = glm::normalize(front);

    glm::vec3 up = safeNormalize(upDirs[index], glm::vec3(0.0f, 1.0f, 0.0f));
    if (glm::abs(glm::dot(front, up)) > 0.99f) {
        up = safeNormalize(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        if (glm::abs(glm::dot(front, up)) > 0.99f) {
            up = safeNormalize(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    glm::vec3 right = glm::cross(front, up);
    if (glm::dot(right, right) < kEpsilon) {
        right = glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    right = safeNormalize(right, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec3 correctedUp = glm::normalize(glm::cross(right, front));

    glm::mat4 transform(1.0f);
    transform[0] = glm::vec4(right, 0.0f);
    transform[1] = glm::vec4(correctedUp, 0.0f);
    transform[2] = glm::vec4(-front, 0.0f);
    transform[3] = glm::vec4(pos, 1.0f);

    TransformNode::setTransform(transform);
}

ParentSGNode* KeyframeAnimationNode::copyNode() {
    auto* node = new KeyframeAnimationNode(name, scenegraph);
    node->setKeyframes(positions, upDirs);
    glm::mat4 current = this->getTransform();
    node->TransformNode::setTransform(current);
    return node;
}

} // namespace sgraph
