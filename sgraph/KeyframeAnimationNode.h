#ifndef _KEYFRAME_ANIMATION_NODE_H_
#define _KEYFRAME_ANIMATION_NODE_H_

#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include "AnimationNode.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace sgraph {

class KeyframeAnimationNode : public AnimationNode {
public:
    KeyframeAnimationNode(const std::string& name, sgraph::IScenegraph* graph);

    bool loadKeyframesFromFile(const std::string& filepath);
    void setKeyframes(const std::vector<glm::vec3>& positions,
                      const std::vector<glm::vec3>& upVectors);

    size_t getKeyframeCount() const { return positions.size(); }
    const std::vector<glm::vec3>& getPositions() const { return positions; }
    const std::vector<glm::vec3>& getUpVectors() const { return upDirs; }

    void setTransform(int tick) override;

protected:
    ParentSGNode* copyNode() override;

private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> upDirs;

    glm::mat4 computeTransformForIndex(size_t index) const;
};

} // namespace sgraph

#endif
