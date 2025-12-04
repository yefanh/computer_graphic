#include "KDAbstractNode.h"
#include "KDTree.h"

KDAbstractNode::KDAbstractNode(vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,KDTree *t) 
: KDNode(t) {
    this->listOfPoints = listOfPoints;
    this->listOfNormals = listOfNormals;
    this->indices = indices;
}

