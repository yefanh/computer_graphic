#include "KDAbstractNode.h"
#include "KDTree.h"

KDAbstractNode::KDAbstractNode(vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,KDTree *t) 
: KDNode(t) {
    this->listOfPoints = listOfPoints;
    this->listOfNormals = listOfNormals;
    this->indices = indices;
}

/**
 * @brief Test intersection with a list of triangles
 * 
 * This helper function is used by both leaf nodes and internal nodes
 * to test ray-triangle intersection for their stored triangles.
 * 
 * @param triangleList List of triangle indices to test
 * @param testedTriangles Set of already tested triangles to avoid duplicates
 * @return HitRecord of closest intersection within (tmin, tmax)
 */
HitRecord KDAbstractNode::testTriangles(const vector<int>& triangleList,
                                        const Ray& objectRay,
                                        const glm::mat4& modelviewMatrix,
                                        const glm::mat4& normalMatrix,
                                        const util::Material& material,
                                        const string& textureName,
                                        float tmin,
                                        float tmax,
                                        set<int>& testedTriangles) {
    HitRecord closestHit;
    
    // Get triangle and vertex data from the KD-tree
    const vector<glm::ivec3>& triangles = tree->getTriangles();
    const vector<glm::vec3>& vertices = tree->getVertices();
    const vector<glm::vec3>& normals = tree->getNormals();
    const vector<glm::vec2>& texcoords = tree->getTexcoords();
    
    // Test each triangle in the list
    for (int triIndex : triangleList) {
        glm::ivec3 tri = triangles[triIndex];
        
        // Get vertex positions
        glm::vec3 v0 = vertices[tri.x];
        glm::vec3 v1 = vertices[tri.y];
        glm::vec3 v2 = vertices[tri.z];
        
        // Test ray-triangle intersection
        float t;
        glm::vec3 baryCoords;
        if (rayTriangleIntersect(objectRay, v0, v1, v2, t, baryCoords)) {
            // Per Section 2.2.1: "report the closest intersection that is within (tmin, tmax)"
            // Use a small tolerance for boundary cases (precision issues near split planes)
            const float EPSILON = 0.0001f;
            
            // Check if intersection is within the current node's range
            if (t > tmin - EPSILON && t < tmax + EPSILON && t > EPSILON) {
                // Valid intersection in range
                if (t < closestHit.getT()) {
                    // Compute intersection point in object space
                    glm::vec4 objIntersection = objectRay.getPointAt(t);
                    
                    // Transform intersection point to view space
                    glm::vec4 viewIntersection = modelviewMatrix * objIntersection;
                    float viewT = glm::length(glm::vec3(viewIntersection));
                    
                    // Get vertex normals
                    glm::vec3 n0 = normals[tri.x];
                    glm::vec3 n1 = normals[tri.y];
                    glm::vec3 n2 = normals[tri.z];
                    
                    // Interpolate normal using barycentric coordinates
                    glm::vec3 objNormal = baryCoords.x * n0 + baryCoords.y * n1 + baryCoords.z * n2;
                    objNormal = glm::normalize(objNormal);
                    
                    // Transform normal to view space
                    glm::vec4 viewNormal = normalMatrix * glm::vec4(objNormal, 0.0f);
                    viewNormal = glm::normalize(viewNormal);
                    
                    // Fill hit record
                    closestHit.setT(t);
                    closestHit.setIntersectionPoint(viewIntersection);
                    closestHit.setNormal(viewNormal);
                    closestHit.setMaterial(material);
                    closestHit.setTextureName(textureName);
                    closestHit.setViewT(viewT);
                    
                    // Interpolate texture coordinates
                    glm::vec2 tc0 = texcoords[tri.x];
                    glm::vec2 tc1 = texcoords[tri.y];
                    glm::vec2 tc2 = texcoords[tri.z];
                    glm::vec2 tc = baryCoords.x * tc0 + baryCoords.y * tc1 + baryCoords.z * tc2;
                    closestHit.setTextureCoordinates(tc);
                }
            }
        }
        else {
            // No intersection at all with this triangle for this segment; allow other nodes to test it.
        }
    }
    
    return closestHit;
}

/**
 * @brief Moller-Trumbore ray-triangle intersection algorithm
 * 
 * Same algorithm as used in RaytraceMesh, but operates on glm::vec3 directly.
 */
bool KDAbstractNode::rayTriangleIntersect(const Ray& ray, 
                                          const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                          float& t, glm::vec3& baryCoords) {
    const float EPSILON = 0.0000001f;
    
    glm::vec3 rayOrigin = glm::vec3(ray.getStart());
    glm::vec3 rayDir = glm::vec3(ray.getDirection());
    
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);
    
    // Ray is parallel to triangle
    if (a > -EPSILON && a < EPSILON) {
        return false;
    }
    
    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
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
