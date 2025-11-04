#include "KeyframeAnimationNode.h"

#include <algorithm>
#include <fstream>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace sgraph {
namespace {
    // Epsilon value for floating-point comparisons to avoid division by zero
    constexpr float kEpsilon = 1e-5f;

    /**
     * @brief Safely normalize a vector with fallback handling.
     * 
     * Attempts to normalize the primary vector. If it's too small (near-zero),
     * tries the fallback vector. If both fail, returns (1,0,0) as last resort.
     * 
     * @param v Primary vector to normalize
     * @param fallback Fallback vector if primary is degenerate
     * @return Normalized vector, or fallback, or (1,0,0)
     */
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
    // Open the keyframe data file
    std::ifstream in(filepath);
    if (!in.is_open()) {
        std::cerr << "Failed to open keyframe file: " << filepath << std::endl;
        return false;
    }

    // Read number of keyframes (first line)
    size_t count = 0;
    in >> count;
    if (!in || count == 0) {
        std::cerr << "Invalid keyframe file header: " << filepath << std::endl;
        return false;
    }

    // Read keyframe data: x y z upx upy upz per line
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
    // Ensure both vectors have the same size (truncate to smaller)
    if (positions.size() != upDirs.size()) {
        size_t minCount = std::min(positions.size(), upDirs.size());
        positions.resize(minCount);
        upDirs.resize(minCount);
    }
}

void KeyframeAnimationNode::setTransform(int tick) {
    // Handle empty keyframe case
    if (positions.empty()) {
        glm::mat4 identity(1.0f);
        TransformNode::setTransform(identity);
        return;
    }

    // Compute cyclic keyframe index from tick
    size_t count = positions.size();
    size_t index = static_cast<size_t>(tick) % count;
    size_t nextIndex = (index + 1) % count;

    // Get current position
    const glm::vec3& pos = positions[index];
    
    // Compute forward direction from current → next position
    glm::vec3 front = positions[nextIndex] - pos;
    if (glm::dot(front, front) < kEpsilon) {
        // Consecutive positions coincide, use default forward
        front = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    front = glm::normalize(front);

    // Get up vector from keyframe data and normalize safely
    glm::vec3 up = safeNormalize(upDirs[index], glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Handle degenerate case: forward and up nearly parallel
    if (glm::abs(glm::dot(front, up)) > 0.99f) {
        up = safeNormalize(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        if (glm::abs(glm::dot(front, up)) > 0.99f) {
            up = safeNormalize(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    // Compute right vector: right = cross(front, up)
    glm::vec3 right = glm::cross(front, up);
    if (glm::dot(right, right) < kEpsilon) {
        // Fallback if cross product fails
        right = glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    right = safeNormalize(right, glm::vec3(1.0f, 0.0f, 0.0f));

    // Re-orthogonalize up vector: up = cross(right, front)
    glm::vec3 correctedUp = glm::normalize(glm::cross(right, front));

    // Build 4x4 transformation matrix from basis vectors
    // Column 0: right (X axis)
    // Column 1: up (Y axis)
    // Column 2: -front (Z axis, negated for right-handed system)
    // Column 3: position (translation)
    glm::mat4 transform(1.0f);
    transform[0] = glm::vec4(right, 0.0f);
    transform[1] = glm::vec4(correctedUp, 0.0f);
    transform[2] = glm::vec4(-front, 0.0f);
    transform[3] = glm::vec4(pos, 1.0f);

    TransformNode::setTransform(transform);
}

ParentSGNode* KeyframeAnimationNode::copyNode() {
    // Create a new node with same name and scenegraph
    auto* node = new KeyframeAnimationNode(name, scenegraph);
    // Copy keyframe data
    node->setKeyframes(positions, upDirs);
    // Copy current transformation state
    glm::mat4 current = this->getTransform();
    node->TransformNode::setTransform(current);
    return node;
}

} // namespace sgraph
