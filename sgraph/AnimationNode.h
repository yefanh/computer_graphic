#ifndef _ANIMATIONNODE_H_
#define _ANIMATIONNODE_H_

#include "TransformNode.h"

namespace sgraph {
    /**
     * Base class for time-varying transforms that update using an integer tick.
     */
    class AnimationNode : public TransformNode {
    public:
        AnimationNode(const std::string& name, sgraph::IScenegraph *graph)
            : TransformNode(name, graph) {}
        virtual ~AnimationNode() = default;

        /**
         * Update this node's internal transform matrix based on the supplied tick.
         */
        virtual void setTransform(int tick) = 0;

        void accept(SGNodeVisitor* visitor) override {
            visitor->visitAnimationNode(this);
        }
    };
}

#endif
