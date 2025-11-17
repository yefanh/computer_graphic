#ifndef _KEYFRAMETRANSFORMNODE_H_
#define _KEYFRAMETRANSFORMNODE_H_

#include "AnimationTransformNode.h"
#include "SGNodeVisitor.h"
#include "glm/glm.hpp"
#include <vector>
#include <string>
using namespace std;

namespace sgraph
{

  /**
 * This node represents a transformation in the scene graph that is based on keyframes. 
 * \author Amit Shesh
 */
  class KeyframeTransformNode: public AnimationTransformNode {
    protected:
      vector<glm::mat4> keyframe_transforms;

      

    public:
      KeyframeTransformNode(const string& name,vector<glm::vec4>& positions,vector<glm::vec4>& ups,sgraph::IScenegraph *graph)
        :AnimationTransformNode(name,graph) {
        this->transform = glm::mat4(1.0);
        createKeyframes(positions,ups);
      }
    
    ~KeyframeTransformNode()	{
    }

    ParentSGNode *copyNode() {
        return new KeyframeTransformNode(name,keyframe_transforms,scenegraph);
    }

    void setTransform(int tick) {
      AnimationTransformNode::setTransform(keyframe_transforms[tick%keyframe_transforms.size()]);
    }

    /**
     * Visit this node.
     * 
     */
    void accept(SGNodeVisitor* visitor) {
      return visitor->visitKeyframeTransformNode(this);
    }

    void createKeyframes(vector<glm::vec4>& positions,vector<glm::vec4>& ups) {
      for (int i=0;i<positions.size()-1;i+=1) {
        glm::vec3 w = glm::normalize(positions[i].xyz()-positions[i+1].xyz());
        glm::vec3 v = glm::normalize(ups[i].xyz());
        glm::vec3 u = glm::cross(v,w);
        glm::mat4 keyframe_transform = glm::translate(glm::mat4(1.0),positions[i].xyz())
                                      * glm::mat4(u.x,u.y,u.z,0,v.x,v.y,v.z,0,w.x,w.y,w.z,0,0,0,0,1);
        keyframe_transforms.push_back(keyframe_transform);
      }
    }

    private:
    KeyframeTransformNode(const string& name,vector<glm::mat4>& keyframe_transforms,sgraph::IScenegraph *graph) 
    :AnimationTransformNode(name,graph) {
      this->keyframe_transforms = keyframe_transforms;
    }
  };
}
#endif
