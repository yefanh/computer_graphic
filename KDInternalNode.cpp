#include "KDInternalNode.h"
#include "KDTree.h"

KDInternalNode::KDInternalNode(KDNode *left,KDNode *right,vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,glm::vec4 plane_equation,KDTree *t) 
:KDAbstractNode(listOfPoints,listOfNormals,indices,t)
,plane(plane_equation)
{
    this->left = left;
    this->right = right;
}

KDInternalNode::~KDInternalNode() {
    if (left!=NULL) {
        delete left;
    }
    if (right!=NULL) {
        delete right;
    }
}