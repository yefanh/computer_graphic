#ifndef _RAYTRACEMESH_H_
#define _RAYTRACEMESH_H_

#include "Ray.h"
#include "HitRecord.h"
#include "VertexAttrib.h"
#include "PolygonMesh.h"
#include "KDTree.h"
#include <glm/glm.hpp>
#include <vector>
using namespace std;

/**
 * RaytraceMesh represents a ray-traceable object.
 * An object of this class is associated with exactly one polygon mesh.
 * 
 * This class uses a KD-Tree for accelerated ray-mesh intersection testing.
 * Per Section 2.2: The KD-Tree's intersect function is called with tmin=0 and tmax=infinity.
 */
class RaytraceMesh {
public:
    /**
     * @brief Construct a RaytraceMesh from a polygon mesh
     * 
     * Creates a KD-Tree from the mesh for accelerated ray casting.
     * 
     * @param mesh The polygon mesh to associate with this object
     */
    RaytraceMesh(util::PolygonMesh<VertexAttrib>& mesh) : kdTree(mesh) {
        // KD-Tree is constructed in initializer list
        // All triangle data is now managed by the KD-Tree
    }

    /**
     * @brief Find the closest intersection of a ray with this mesh
     * 
     * This function takes in:
     * (a) a ray in the object space
     * (b) the same ray in view space (for convenience)
     * (c) the modelview and normal matrices
     * 
     * Per Section 2.2: Delegates to KD-Tree's intersect function with tmin=0, tmax=infinity.
     * 
     * @param objectRay Ray in object space
     * @param viewRay Ray in view space (for convenience)
     * @param modelviewMatrix The modelview matrix for transforming to view space
     * @param normalMatrix The normal matrix for transforming normals
     * @param material The material properties for this object
     * @param textureName The texture name for this object
     * @return HitRecord of the closest intersection, or empty if no hit
     */
    HitRecord intersect(const Ray& objectRay,
                        const Ray& viewRay,
                        const glm::mat4& modelviewMatrix,
                        const glm::mat4& normalMatrix,
                        const util::Material& material,
                        const string& textureName,
                        const string& meshName) {
        // Set the mesh name in the KD-Tree so nodes can access it
        kdTree.setMeshName(meshName);
        
        // Delegate to KD-Tree's intersect function
        // Per Section 2.2: "For now, use tmin = 0 and tmax = infinity when calling this function."
        return kdTree.intersect(objectRay, viewRay, modelviewMatrix, normalMatrix, 
                               material, textureName);
    }

private:
    // KD-Tree for accelerated ray-mesh intersection
    KDTree kdTree;
};

#endif
