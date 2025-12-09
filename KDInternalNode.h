#ifndef __KDINTERNALNODE_H__
#define __KDINTERNALNODE_H__

#include "KDAbstractNode.h"


/**
 * @brief Internal node of KD-Tree
 * 
 * Section 2.2.2: Implements front-to-back traversal based on ray position
 * relative to the split plane.
 */
class KDInternalNode:public KDAbstractNode {
    public:
        KDInternalNode(KDNode *left,KDNode *right,vector<glm::vec3> *listOfPoints,vector<glm::vec3> *listOfNormals,vector<int> indices,glm::vec4 plane_equation,KDTree *t);
        virtual ~KDInternalNode();

        // Get the split plane equation
        glm::vec4 getPlane() const { return plane; }

        // Get child nodes
        KDNode* getLeft() const { return left; }
        KDNode* getRight() const { return right; }

        /**
         * @brief Intersect ray with this internal node using front-to-back traversal
         * 
         * Per Section 2.2.2:
         * - Find points P (at tmin) and Q (at tmax) on the ray
         * - Find intersection R with split plane at time t
         * - Case 1: tmin <= t <= tmax: traverse both sides in front-to-back order
         * - Case 2: tmin <= tmax <= t: only traverse side where P lies
         * - Case 3: t <= tmin <= tmax: only traverse side where P lies  
         * - Case 4: ray lies on plane: check both sides and plane triangles
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

    private:
     KDNode *left,*right;
     const glm::vec4 plane; //equation of the dividing plane

};

#endif
