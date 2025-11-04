#ifndef _ANIMATIONNODE_H_
#define _ANIMATIONNODE_H_

#include "TransformNode.h"

namespace sgraph {
    /**
     * @brief Base class for time-varying transforms that update using an integer tick.
     * 
     * AnimationNode extends TransformNode to support animations driven by a unitless
     * "tick" counter rather than real time. This allows for frame-independent,
     * repeatable animations that can be easily controlled by changing playback speed.
     * 
     * Key concepts:
     * - "Tick" is a unitless measure of animation progress (e.g., frame number)
     * - Separates animation cadence from actual time/speed
     * - Similar to a metronome in music: uniform beats that can be played faster/slower
     * 
     * Usage in Assignment 5:
     * - Part of the scene graph animation system (requirement 2.1)
     * - Subclassed by KeyframeAnimationNode for keyframe-based animation
     * - Visited by GLScenegraphRenderer which calls setTransform(tick) before rendering
     * 
     * @see KeyframeAnimationNode for keyframe-based implementation
     * @see GLScenegraphRenderer for animation rendering logic
     */
    class AnimationNode : public TransformNode {
    public:
        /**
         * @brief Construct a new AnimationNode
         * 
         * @param name Unique identifier for this node in the scene graph
         * @param graph Pointer to parent scene graph (may be NULL during construction)
         */
        AnimationNode(const std::string& name, sgraph::IScenegraph *graph)
            : TransformNode(name, graph) {}
        
        virtual ~AnimationNode() = default;

        /**
         * @brief Update this node's internal transform matrix based on the supplied tick.
         * 
         * This pure virtual function must be implemented by subclasses to define
         * how the transformation changes over time. The tick parameter represents
         * animation progress and is typically the current frame number.
         * 
         * @param tick Current animation tick (unitless progress counter)
         * 
         * Implementation notes:
         * - Called by renderer before each frame
         * - Should update the internal transformation matrix
         * - Use tick % keyframeCount for cyclic animations
         */
        virtual void setTransform(int tick) = 0;

        /**
         * @brief Accept a visitor for traversing the scene graph.
         * 
         * Overrides SGNode::accept to route to the animation-specific visit method.
         * This enables the visitor pattern for scene graph operations.
         * 
         * @param visitor Pointer to visitor that will process this node
         */
        void accept(SGNodeVisitor* visitor) override {
            visitor->visitAnimationNode(this);
        }
    };
}

#endif
