#ifndef __KDNODE_H__
#define __KDNODE_H__
#include <glm/glm.hpp>
#include "HitRecord.h"
#include "Ray.h"
#include "Material.h"
#include <unordered_set>
#include <set>
#include <string>
using namespace std;

class KDTree;

class KDNode {
    public:
    KDNode(KDTree *t) {
        tree = t;
    }
    virtual ~KDNode() {};

    /**
     * @brief Intersect ray with this node and its subtree
     * 
     * @param objectRay Ray in object space
     * @param viewRay Ray in view space
     * @param modelviewMatrix Modelview matrix for transforming points to view space
     * @param normalMatrix Normal matrix for transforming normals
     * @param material Material properties for shading
     * @param textureName Texture name for this mesh
     * @param tmin Minimum t value to consider (exclusive)
     * @param tmax Maximum t value to consider (exclusive)
     * @param testedTriangles Set of already tested triangles (shared across traversal)
     * @return HitRecord of closest intersection within (tmin, tmax), or empty if no hit
     */
    virtual HitRecord intersect(const Ray& objectRay,
                                const Ray& viewRay,
                                const glm::mat4& modelviewMatrix,
                                const glm::mat4& normalMatrix,
                                const util::Material& material,
                                const string& textureName,
                                float tmin,
                                float tmax,
                                set<int>& testedTriangles) = 0;

    protected:
    KDTree *tree; //pointer to the kdtree of which this node is a part

};

#endif