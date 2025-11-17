#ifndef _ANIMATIONTRANSFORMNODE_H_
#define _ANIMATIONTRANSFORMNODE_H_

#include "TransformNode.h"
#include "SGNodeVisitor.h"
#include "glm/glm.hpp"
#include <vector>
#include <string>
using namespace std;

namespace sgraph
{

  /**
 * This node represents a transformation in the scene graph that changes with time. 
 * \author Amit Shesh
 */
  class AnimationTransformNode: public TransformNode {
    protected:
      void setTransform(glm::mat4& transform) {
        this->transform = transform;
      }

    public:
      AnimationTransformNode(const string& name,sgraph::IScenegraph *graph)
        :TransformNode(name,graph) {
        this->transform = glm::mat4(1.0);
      }

      virtual void setTransform(int tick)=0;
    
    ~AnimationTransformNode()	{
    }

    /**
     * Visit this node.
     * 
     */
    void accept(SGNodeVisitor* visitor) {
      return visitor->visitTransformNode(this);
    }
  };
}
#endif
