#ifndef __KDLEAFNODE_H__
#define __KDLEAFNODE_H__

#include "KDAbstractNode.h"

/**
 * @brief Leaf node of KD-Tree
 * 
 * Section 2.2.1: If traversal reaches a leaf node, simply test all the
 * triangles in that leaf for intersection and report the closest
 * intersection that is within (tmin, tmax).
 */
class KDLeafNode: public KDAbstractNode {
    public:
    KDLeafNode(vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,KDTree *t)
    :KDAbstractNode(listOfPoints,listOfNormals,indices,t) {

    }

    /**
     * @brief Intersect ray with all triangles in this leaf node
     * 
     * Per Section 2.2.1: Test all triangles in this leaf for intersection
     * and report the closest intersection within (tmin, tmax).
     */
    virtual HitRecord intersect(const Ray& objectRay,
                                const Ray& viewRay,
                                const glm::mat4& modelviewMatrix,
                                const glm::mat4& normalMatrix,
                                const util::Material& material,
                                const string& textureName,
                                float tmin,
                                float tmax,
                                set<int>& testedTriangles) override;

};

#endif