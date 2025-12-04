#ifndef __KDLEAFNODE_H__
#define __KDLEAFNODE_H__

#include "KDAbstractNode.h"

class KDLeafNode: public KDAbstractNode {
    public:
    KDLeafNode(vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,KDTree *t)
    :KDAbstractNode(listOfPoints,listOfNormals,indices,t) {

    }

    

    


};

#endif