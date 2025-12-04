#ifndef __KDABSTRACTNODE_H__
#define __KDABSTRACTNODE_H__

#include <glm/glm.hpp>
#include <vector>
#include <set>
using namespace std;
#include "KDNode.h"

class KDAbstractNode: public KDNode {
    public:
    virtual ~KDAbstractNode() {
        
    }

    // Add a triangle index to this node
    void addTriangle(int triangleIndex) {
        triangleIndices.push_back(triangleIndex);
    }

    // Get the triangle indices stored in this node
    const vector<int>& getTriangleIndices() const {
        return triangleIndices;
    }

    // Get vertex indices for this node
    vector<int>& getVertexIndices() {
        return indices;
    }

    protected:
    KDAbstractNode(vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,KDTree *t);

    /**
     * @brief Test intersection with a specific set of triangles
     * 
     * Helper function used by both leaf and internal nodes to test
     * intersection with triangles stored in this node.
     * 
     * @param triangleList List of triangle indices to test
     * @param objectRay Ray in object space
     * @param modelviewMatrix Modelview matrix
     * @param normalMatrix Normal matrix
     * @param material Material properties
     * @param textureName Texture name
     * @param tmin Minimum t value
     * @param tmax Maximum t value
     * @param testedTriangles Set of already tested triangles (to avoid duplicates)
     * @return HitRecord of closest intersection
     */
    HitRecord testTriangles(const vector<int>& triangleList,
                           const Ray& objectRay,
                           const glm::mat4& modelviewMatrix,
                           const glm::mat4& normalMatrix,
                           const util::Material& material,
                           const string& textureName,
                           float tmin,
                           float tmax,
                           set<int>& testedTriangles);

    /**
     * @brief Moller-Trumbore ray-triangle intersection algorithm
     */
    bool rayTriangleIntersect(const Ray& ray, 
                              const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                              float& t, glm::vec3& baryCoords);
    
    protected:

    
    vector<glm::vec3> *listOfPoints;
    vector<glm::vec3> *listOfNormals;
    vector<int> indices; //vertex indices
    vector<int> triangleIndices; // triangle indices stored in this node
};

#endif