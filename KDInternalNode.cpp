#include "KDInternalNode.h"
#include "KDTree.h"
#include <cmath>

KDInternalNode::KDInternalNode(KDNode *left,KDNode *right,vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,glm::vec4 plane_equation,KDTree *t) 
:KDAbstractNode(listOfPoints,listOfNormals,indices,t)
,plane(plane_equation)
{
    this->left = left;
    this->right = right;
}

KDInternalNode::~KDInternalNode() {
    if (left!=NULL) {
        delete left;
    }
    if (right!=NULL) {
        delete right;
    }
}

/**
 * @brief Intersect ray with internal node using front-to-back traversal
 * 
 * Section 2.2.2 Algorithm:
 * 1. Find P (point at tmin) and Q (point at tmax) on the ray
 * 2. Find R (intersection with split plane) at time t
 * 3. Based on where t falls relative to tmin/tmax:
 *    - Case 1: tmin <= t <= tmax: ray crosses plane, traverse both sides front-to-back
 *    - Case 2: tmin <= tmax <= t: ray doesn't reach plane, only traverse side with P
 *    - Case 3: t <= tmin <= tmax: ray starts past plane, only traverse side with P
 *    - Case 4: ray lies on plane: check both sides and plane triangles
 */
HitRecord KDInternalNode::intersect(const Ray& objectRay,
                                    const Ray& viewRay,
                                    const glm::mat4& modelviewMatrix,
                                    const glm::mat4& normalMatrix,
                                    const util::Material& material,
                                    const string& textureName,
                                    float tmin,
                                    float tmax,
                                    set<int>& testedTriangles) {
    HitRecord closestHit;
    const float EPSILON = 0.0001f;
    
    // Get ray start and direction
    glm::vec3 rayStart = glm::vec3(objectRay.getStart());
    glm::vec3 rayDir = glm::vec3(objectRay.getDirection());
    
    // Calculate P (point at tmin) and Q (point at tmax)
    // Per Section 2.2.2: "Find the points P and Q on the ray corresponding to tmin and tmax"
    glm::vec3 P = rayStart + tmin * rayDir;
    glm::vec3 Q = rayStart + tmax * rayDir;
    
    // Split plane equation: plane.x*x + plane.y*y + plane.z*z + plane.w = 0
    // plane.xyz is the normal, plane.w is the negative distance
    glm::vec3 planeNormal = glm::vec3(plane);
    float planeD = plane.w;
    
    // Signed distance of P from split plane
    float distP = glm::dot(planeNormal, P) + planeD;
    // Signed distance of Q from split plane  
    float distQ = glm::dot(planeNormal, Q) + planeD;
    
    // Calculate the denominator for ray-plane intersection
    // Ray: point = rayStart + t * rayDir
    // Plane: dot(planeNormal, point) + planeD = 0
    // Substituting: dot(planeNormal, rayStart) + t * dot(planeNormal, rayDir) + planeD = 0
    // t = -(dot(planeNormal, rayStart) + planeD) / dot(planeNormal, rayDir)
    float denom = glm::dot(planeNormal, rayDir);
    
    // Case 4: Ray lies on (or is parallel to) the split plane
    if (abs(denom) < EPSILON) {
        // Ray is parallel to split plane
        // Check if ray origin is on the plane
        float distStart = glm::dot(planeNormal, rayStart) + planeD;
        
        if (abs(distStart) < EPSILON) {
            // Per Section 2.2.2 Case 4: "The ray lies on the split plane: 
            // check both sides as well as the split plane itself"
            
            // Test triangles on the split plane (stored in this internal node)
            HitRecord planeHit = testTriangles(triangleIndices, objectRay, viewRay, modelviewMatrix,
                                              normalMatrix, material, textureName,
                                              tmin, tmax, testedTriangles);
            if (planeHit.hasHit() && planeHit.getT() < closestHit.getT()) {
                closestHit = planeHit;
            }
            
            // Check both children
            if (left != NULL) {
                HitRecord leftHit = left->intersect(objectRay, viewRay, modelviewMatrix,
                                                   normalMatrix, material, textureName,
                                                   tmin, tmax, testedTriangles);
                if (leftHit.hasHit() && leftHit.getT() < closestHit.getT()) {
                    closestHit = leftHit;
                }
            }
            if (right != NULL) {
                HitRecord rightHit = right->intersect(objectRay, viewRay, modelviewMatrix,
                                                     normalMatrix, material, textureName,
                                                     tmin, tmax, testedTriangles);
                if (rightHit.hasHit() && rightHit.getT() < closestHit.getT()) {
                    closestHit = rightHit;
                }
            }
            return closestHit;
        }
        else {
            // Ray is parallel but not on plane - only traverse one side
            KDNode* nearSide = (distStart < 0) ? left : right;
            if (nearSide != NULL) {
                return nearSide->intersect(objectRay, viewRay, modelviewMatrix,
                                          normalMatrix, material, textureName,
                                          tmin, tmax, testedTriangles);
            }
            return closestHit;
        }
    }
    
    // Calculate t where ray intersects split plane
    float t = -(glm::dot(planeNormal, rayStart) + planeD) / denom;
    
    // Determine which side P is on (front side)
    // left = negative side (signed distance < 0)
    // right = positive side (signed distance > 0)
    KDNode* frontNode = (distP < 0) ? left : right;
    KDNode* backNode = (distP < 0) ? right : left;
    
    // Handle precision: if P or Q is very close to the plane, we need to check plane triangles
    // Per Section 2.2.2 Warning: "if P or Q are close to the split plane, 
    // you must check the triangles on the split plane"
    bool pNearPlane = abs(distP) < EPSILON;
    bool qNearPlane = abs(distQ) < EPSILON;
    bool needCheckPlane = pNearPlane || qNearPlane;
    
    // Case 1: tmin <= t <= tmax
    // The ray overlaps with both sides of the split plane
    if (t >= tmin - EPSILON && t <= tmax + EPSILON) {
        // Per Section 2.2.2 Case 1:
        // "First traverse the side of the split plane where P lies,
        //  then the plane and finally the side where Q lies.
        //  Report the closest intersection that is within (tmin, tmax)."
        
        // 1. First traverse front side (where P lies) with range [tmin, t]
        if (frontNode != NULL) {
            HitRecord frontHit = frontNode->intersect(objectRay, viewRay, modelviewMatrix,
                                                     normalMatrix, material, textureName,
                                                     tmin, t, testedTriangles);
            if (frontHit.hasHit() && frontHit.getT() < closestHit.getT()) {
                closestHit = frontHit;
            }
        }
        
        // 2. Then check triangles on the split plane
        HitRecord planeHit = testTriangles(triangleIndices, objectRay, viewRay, modelviewMatrix,
                                          normalMatrix, material, textureName,
                                          tmin, tmax, testedTriangles);
        if (planeHit.hasHit() && planeHit.getT() < closestHit.getT()) {
            closestHit = planeHit;
        }
        
        // 3. Finally traverse back side (where Q lies) with range [t, tmax]
        // Only if we haven't found a closer hit already
        if (backNode != NULL) {
            // We can potentially early-exit if we already have a hit closer than t
            // But for correctness, we check the back side too
            float newTmax = closestHit.hasHit() ? min(tmax, closestHit.getT()) : tmax;
            if (t < newTmax) {
                HitRecord backHit = backNode->intersect(objectRay, viewRay, modelviewMatrix,
                                                       normalMatrix, material, textureName,
                                                       t, newTmax, testedTriangles);
                if (backHit.hasHit() && backHit.getT() < closestHit.getT()) {
                    closestHit = backHit;
                }
            }
        }
    }
    // Case 2: tmin <= tmax <= t
    // Ray segment is entirely before the plane intersection
    else if (t > tmax) {
        // Per Section 2.2.2 Case 2:
        // "Only traverse the side of the split plane where P lies."
        if (frontNode != NULL) {
            HitRecord frontHit = frontNode->intersect(objectRay, viewRay, modelviewMatrix,
                                                     normalMatrix, material, textureName,
                                                     tmin, tmax, testedTriangles);
            if (frontHit.hasHit() && frontHit.getT() < closestHit.getT()) {
                closestHit = frontHit;
            }
        }
        
        // Check plane triangles if P or Q is near the plane
        if (needCheckPlane) {
            HitRecord planeHit = testTriangles(triangleIndices, objectRay, viewRay, modelviewMatrix,
                                              normalMatrix, material, textureName,
                                              tmin, tmax, testedTriangles);
            if (planeHit.hasHit() && planeHit.getT() < closestHit.getT()) {
                closestHit = planeHit;
            }
        }
    }
    // Case 3: t <= tmin <= tmax
    // Ray segment is entirely after the plane intersection
    else if (t < tmin) {
        // Per Section 2.2.2 Case 3:
        // "Only traverse the side of the split plane where P lies."
        // Note: When t < tmin, P is on the opposite side from the ray direction
        // But P is still at tmin, so we use distP to determine which side
        if (frontNode != NULL) {
            HitRecord frontHit = frontNode->intersect(objectRay, viewRay, modelviewMatrix,
                                                     normalMatrix, material, textureName,
                                                     tmin, tmax, testedTriangles);
            if (frontHit.hasHit() && frontHit.getT() < closestHit.getT()) {
                closestHit = frontHit;
            }
        }
        
        // Check plane triangles if P or Q is near the plane
        if (needCheckPlane) {
            HitRecord planeHit = testTriangles(triangleIndices, objectRay, viewRay, modelviewMatrix,
                                              normalMatrix, material, textureName,
                                              tmin, tmax, testedTriangles);
            if (planeHit.hasHit() && planeHit.getT() < closestHit.getT()) {
                closestHit = planeHit;
            }
        }
    }
    
    return closestHit;
}
