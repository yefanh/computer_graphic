#ifndef __KDNODE_H__
#define __KDNODE_H__
#include <glm/glm.hpp>
#include "HitRecord.h"
#include "Ray.h"
#include <unordered_set>
using namespace std;

class KDTree;

class KDNode {
    public:
    KDNode(KDTree *t) {
        tree = t;
    }
    virtual ~KDNode() {};

    protected:
    KDTree *tree; //pointer to the kdtree of which this node is a part

};

#endif