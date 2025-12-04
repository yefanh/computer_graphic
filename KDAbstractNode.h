#ifndef __KDABSTRACTNODE_H__
#define __KDABSTRACTNODE_H__

#include <glm/glm.hpp>
#include <vector>
using namespace std;
#include "KDNode.h"

class KDAbstractNode: public KDNode {
    public:
    virtual ~KDAbstractNode() {
        
    }
    protected:
    KDAbstractNode(vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,KDTree *t);

    
    protected:

    
    vector<glm::vec3> *listOfPoints;
    vector<glm::vec3> *listOfNormals;
    vector<int> indices; //vertex indices
};

#endif