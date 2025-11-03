#include "ScenegraphPrinter.h"

namespace sgraph {

void ScenegraphPrinter::visitGroupNode(GroupNode* node) {
    visitComposite(node);
}

void ScenegraphPrinter::visitLeafNode(LeafNode* node) {
    // Leaf: just print the name
    printLine(node->getName());
}

void ScenegraphPrinter::visitTransformNode(TransformNode* node) {
    // Print transform node name and recurse
    visitComposite(node);
}

void ScenegraphPrinter::visitScaleTransform(ScaleTransform* node) {
    visitTransformNode(node);
}

void ScenegraphPrinter::visitTranslateTransform(TranslateTransform* node) {
    visitTransformNode(node);
}

void ScenegraphPrinter::visitRotateTransform(RotateTransform* node) {
    visitTransformNode(node);
}

} // namespace sgraph
