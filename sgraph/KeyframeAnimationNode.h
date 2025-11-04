#ifndef _KEYFRAME_ANIMATION_NODE_H_
#define _KEYFRAME_ANIMATION_NODE_H_

#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include "AnimationNode.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace sgraph {

/**
 * @brief Keyframe-based animation node for animating objects along a path.
 * 
 * KeyframeAnimationNode implements path-based animation using keyframes, where each
 * keyframe stores a position and "up" vector. The object follows the path by interpolating
 * between keyframes based on the current tick.
 * 
 * Keyframe Representation:
 * - Each keyframe = (position, up_vector)
 * - Position: 3D location of the object at this frame
 * - Up vector: Defines the object's orientation (which direction is "up")
 * - Forward direction: Computed from current → next position
 * 
 * Transform Calculation:
 * - Forward = normalize(nextPos - currentPos)
 * - Right = cross(forward, up)
 * - Final up = cross(right, forward)  [ensures orthogonality]
 * - Transform matrix built from these basis vectors + position
 * 
 * Assignment 5 Usage:
 * - Requirement 2.2: Keyframe animation support
 * - Requirement 2.3: Animates the plane around Hogwarts castle
 * - Requirement 2.4: Provides transform data for plane camera
 * 
 * File Format (e.g., aeroplane-path.txt):
 * ```
 * <num_keyframes>
 * x y z upx upy upz
 * x y z upx upy upz
 * ...
 * ```
 * 
 * @see AnimationNode for base animation functionality
 * @see code/aeroplane-path.txt for example keyframe file
 */
class KeyframeAnimationNode : public AnimationNode {
public:
    /**
     * @brief Construct a new KeyframeAnimationNode
     * 
     * @param name Unique identifier for this animation node (e.g., "plane-anim")
     * @param graph Pointer to parent scene graph
     */
    KeyframeAnimationNode(const std::string& name, sgraph::IScenegraph* graph);

    /**
     * @brief Load keyframe data from a file.
     * 
     * Reads keyframes from a text file with format:
     * - First line: number of keyframes
     * - Following lines: x y z upx upy upz (position and up vector)
     * 
     * @param filepath Path to keyframe file (relative or absolute)
     * @return true if file loaded successfully, false otherwise
     * 
     * Example usage:
     * ```cpp
     * node->loadKeyframesFromFile("code/aeroplane-path.txt");
     * ```
     */
    bool loadKeyframesFromFile(const std::string& filepath);
    
    /**
     * @brief Set keyframe data programmatically.
     * 
     * @param positions Vector of 3D positions for each keyframe
     * @param upVectors Vector of up vectors for each keyframe (must match size)
     * 
     * Note: If sizes don't match, both vectors are truncated to the smaller size
     */
    void setKeyframes(const std::vector<glm::vec3>& positions,
                      const std::vector<glm::vec3>& upVectors);

    /**
     * @brief Get the total number of keyframes.
     * @return Size of the keyframe sequence
     */
    size_t getKeyframeCount() const { return positions.size(); }
    
    /**
     * @brief Get the position vector for all keyframes.
     * @return Const reference to positions vector (used by Plane camera)
     */
    const std::vector<glm::vec3>& getPositions() const { return positions; }
    
    /**
     * @brief Get the up vectors for all keyframes.
     * @return Const reference to up directions vector (used by Plane camera)
     */
    const std::vector<glm::vec3>& getUpVectors() const { return upDirs; }

    /**
     * @brief Update transformation matrix based on current tick.
     * 
     * Computes the transformation for keyframe at index (tick % keyframeCount).
     * The forward direction is computed from current → next position.
     * 
     * Algorithm:
     * 1. Calculate keyframe index from tick (with wraparound)
     * 2. Get position from current keyframe
     * 3. Compute forward = normalize(nextPos - currentPos)
     * 4. Compute right = cross(forward, up)
     * 5. Orthogonalize up = cross(right, forward)
     * 6. Build 4x4 transform matrix from basis vectors + position
     * 
     * Handles degenerate cases:
     * - Coincident positions (forward vector = 0)
     * - Parallel forward/up vectors
     * - Zero-length up vectors
     * 
     * @param tick Current animation frame/tick
     * @override Implements AnimationNode::setTransform
     */
    void setTransform(int tick) override;

protected:
    /**
     * @brief Create a copy of this node for scene graph operations.
     * @return Pointer to new KeyframeAnimationNode with same keyframe data
     */
    ParentSGNode* copyNode() override;

private:
    std::vector<glm::vec3> positions; ///< Position for each keyframe
    std::vector<glm::vec3> upDirs;    ///< Up vector for each keyframe

    /**
     * @brief Helper to compute transform matrix for a specific keyframe index.
     * @param index Keyframe index (0-based)
     * @return 4x4 transformation matrix for this keyframe
     */
    glm::mat4 computeTransformForIndex(size_t index) const;
};

} // namespace sgraph

#endif
