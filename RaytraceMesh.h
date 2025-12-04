#ifndef _RAYTRACEMESH_H_
#define _RAYTRACEMESH_H_

#include "Ray.h"
#include "HitRecord.h"
#include "VertexAttrib.h"
#include "PolygonMesh.h"
#include <glm/glm.hpp>
#include <vector>
using namespace std;

/**
 * RaytraceMesh represents a ray-traceable object.
 * An object of this class is associated with exactly one polygon mesh.
 * 
 * This class implements the ray-mesh intersection test by cycling through
 * all triangles in the mesh (inefficient but correct implementation).
 */
class RaytraceMesh {
public:
    /**
     * @brief Construct a RaytraceMesh from a polygon mesh
     * 
     * Pre-extracts all vertex data for efficiency during ray casting.
     * 
     * @param mesh The polygon mesh to associate with this object
     */
    RaytraceMesh(util::PolygonMesh<VertexAttrib>& mesh) {
        // Store indices and primitive size
        indices = mesh.getPrimitives();
        primitiveSize = mesh.getPrimitiveSize();
        
        // Pre-extract all vertex data to avoid repeated getData() calls
        vector<VertexAttrib> vertices = mesh.getVertexAttributes();
        positions.resize(vertices.size());
        normals.resize(vertices.size());
        texcoords.resize(vertices.size());
        
        for (size_t v = 0; v < vertices.size(); v++) {
            vector<float> pos = vertices[v].getData("position");
            vector<float> norm = vertices[v].getData("normal");
            vector<float> tex = vertices[v].getData("texcoord");
            positions[v] = glm::vec3(pos[0], pos[1], pos[2]);
            normals[v] = glm::vec3(norm[0], norm[1], norm[2]);
            texcoords[v] = glm::vec2(tex[0], tex[1]);
        }
    }

    /**
     * @brief Find the closest intersection of a ray with this mesh
     * 
     * This function takes in:
     * (a) a ray in the object space
     * (b) the same ray in view space (for convenience)
     * (c) the modelview and normal matrices
     * 
     * @param objectRay Ray in object space
     * @param viewRay Ray in view space (for convenience, currently unused)
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
                        const string& textureName) {
        HitRecord closestHit;
        
        // Only handle triangles (primitiveSize == 3)
        if (primitiveSize != 3) {
            return closestHit;
        }

        // Iterate through all triangles (inefficient but correct)
        for (size_t i = 0; i < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i+1];
            unsigned int i2 = indices[i+2];
            
            // Get vertex positions
            glm::vec4 v0 = glm::vec4(positions[i0], 1.0f);
            glm::vec4 v1 = glm::vec4(positions[i1], 1.0f);
            glm::vec4 v2 = glm::vec4(positions[i2], 1.0f);

            // Test ray-triangle intersection
            float t;
            glm::vec3 baryCoords;
            if (rayTriangleIntersect(objectRay, v0, v1, v2, t, baryCoords)) {
                if (t > 0.0001f && t < closestHit.getT()) {
                    // Compute intersection point in object space
                    glm::vec4 objIntersection = objectRay.getPointAt(t);
                    
                    // Transform intersection point to view space
                    glm::vec4 viewIntersection = modelviewMatrix * objIntersection;

                    // Get vertex normals
                    glm::vec4 n0 = glm::vec4(normals[i0], 0.0f);
                    glm::vec4 n1 = glm::vec4(normals[i1], 0.0f);
                    glm::vec4 n2 = glm::vec4(normals[i2], 0.0f);

                    // Interpolate normal using barycentric coordinates
                    glm::vec4 objNormal = baryCoords.x * n0 + baryCoords.y * n1 + baryCoords.z * n2;
                    objNormal.w = 0.0f;
                    objNormal = glm::normalize(objNormal);

                    // Transform normal to view space
                    glm::vec4 viewNormal = normalMatrix * objNormal;
                    viewNormal.w = 0.0f;
                    viewNormal = glm::normalize(viewNormal);

                    // Fill hit record
                    closestHit.setT(t);
                    closestHit.setIntersectionPoint(viewIntersection);
                    closestHit.setNormal(viewNormal);
                    closestHit.setMaterial(material);
                    closestHit.setTextureName(textureName);

                    // Interpolate texture coordinates
                    glm::vec2 tc = baryCoords.x * texcoords[i0] + 
                                   baryCoords.y * texcoords[i1] + 
                                   baryCoords.z * texcoords[i2];
                    closestHit.setTextureCoordinates(tc);
                }
            }
        }

        return closestHit;
    }

private:
    // Pre-extracted vertex data for efficiency
    vector<glm::vec3> positions;
    vector<glm::vec3> normals;
    vector<glm::vec2> texcoords;
    vector<unsigned int> indices;
    int primitiveSize;

    /**
     * @brief Moller-Trumbore ray-triangle intersection algorithm
     */
    bool rayTriangleIntersect(const Ray& ray, 
                              const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2,
                              float& t, glm::vec3& baryCoords) {
        const float EPSILON = 0.0000001f;
        
        glm::vec3 rayOrigin = glm::vec3(ray.getStart());
        glm::vec3 rayDir = glm::vec3(ray.getDirection());
        glm::vec3 vertex0 = glm::vec3(v0);
        glm::vec3 vertex1 = glm::vec3(v1);
        glm::vec3 vertex2 = glm::vec3(v2);

        glm::vec3 edge1 = vertex1 - vertex0;
        glm::vec3 edge2 = vertex2 - vertex0;
        glm::vec3 h = glm::cross(rayDir, edge2);
        float a = glm::dot(edge1, h);

        // Ray is parallel to triangle
        if (a > -EPSILON && a < EPSILON) {
            return false;
        }

        float f = 1.0f / a;
        glm::vec3 s = rayOrigin - vertex0;
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f) {
            return false;
        }

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(rayDir, q);

        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }

        // Compute t to find intersection point
        t = f * glm::dot(edge2, q);

        if (t > EPSILON) {
            // Barycentric coordinates: (1-u-v, u, v) for vertices (v0, v1, v2)
            baryCoords = glm::vec3(1.0f - u - v, u, v);
            return true;
        }

        return false;
    }
};

#endif
