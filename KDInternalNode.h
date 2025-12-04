#ifndef __KDINTERNALNODE_H__
#define __KDINTERNALNODE_H__

#include "KDAbstractNode.h"


class KDInternalNode:public KDAbstractNode {
    public:
        KDInternalNode(KDNode *left,KDNode *right,vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,glm::vec4 plane_equation,KDTree *t);
        virtual ~KDInternalNode();

    private:
     KDNode *left,*right;
     const glm::vec4 plane; //equation of the dividing plane

};

#endif