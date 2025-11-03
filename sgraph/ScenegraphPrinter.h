#pragma once
/**
 * ScenegraphPrinter
 * -----------------
 * Visitor that prints the scene graph as an indented text tree:
 *
 * - root
 *     - childA
 *         - grandChild
 *     - childB
 *
 * Each line: tabs for depth + "- " + node name.
 */

#include <iostream>
#include <string>

#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "TransformNode.h"
#include "LeafNode.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include "RotateTransform.h"
#include "AnimationNode.h"

namespace sgraph {

class ScenegraphPrinter : public SGNodeVisitor {
public:
    explicit ScenegraphPrinter(std::ostream& os) : out(os) {}

    void visitGroupNode(GroupNode* node) override;
    void visitLeafNode(LeafNode* node) override;
    void visitTransformNode(TransformNode* node) override;
    void visitScaleTransform(ScaleTransform* node) override;
    void visitTranslateTransform(TranslateTransform* node) override;
    void visitRotateTransform(RotateTransform* node) override;
    void visitAnimationNode(AnimationNode* node) override;

private:
    std::ostream& out;
    int level = 0;  // current depth

    inline void printLine(const std::string& name) {
        // Match assignment's expected format: root without "- ", children with hyphens
        if (level == 0) {
            out << name << '\n';
            return;
        }
        for (int i = 0; i < level; ++i) out << '\t';
        out << "- " << name << '\n';
    }
    inline void enter() { ++level; }
    inline void leave() { --level; }

    // Common traversal for composite nodes that expose getChildren()
    template <typename CompositeNodeT>
    void visitComposite(CompositeNodeT* node) {
        printLine(node->getName());
        enter();
        for (auto* child : node->getChildren()) {
            child->accept(this);
        }
        leave();
    }
};

} // namespace sgraph
