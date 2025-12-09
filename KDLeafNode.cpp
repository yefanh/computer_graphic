#include "KDLeafNode.h"
#include "KDTree.h"

/**
 * @brief Intersect ray with all triangles in this leaf node
 * 
 * Per Section 2.2.1:
 * "If traversal reaches a leaf node, simply test all the triangles in that leaf
 *  for intersection and report the closest intersection that is within (tmin, tmax)."
 */
HitRecord KDLeafNode::intersect(const Ray& objectRay,
                                const Ray& viewRay,
                                const glm::mat4& modelviewMatrix,
                                const glm::mat4& normalMatrix,
                                const util::Material& material,
                                const string& textureName,
                                float tmin,
                                float tmax,
                                set<int>& testedTriangles) {
    // Test all triangles in this leaf node within (tmin, tmax)
    return testTriangles(triangleIndices, objectRay, viewRay, modelviewMatrix,
                        normalMatrix, material, textureName,
                        tmin, tmax, testedTriangles);
}
