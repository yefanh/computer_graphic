#include "KDTree.h"
#include <iostream>
using namespace std;

KDTree::KDTree(util::PolygonMesh<VertexAttrib>& mesh) {
    vector<VertexAttrib> vertices = mesh.getVertexAttributes();
    vector<float> v;
    minBounds = glm::vec3(numeric_limits<float>::max(),numeric_limits<float>::max(),numeric_limits<float>::max());
    // Use lowest() instead of min() - min() returns smallest positive number, not negative infinity!
    maxBounds = glm::vec3(-numeric_limits<float>::max(),-numeric_limits<float>::max(),-numeric_limits<float>::max());
    for (int i=0;i<vertices.size();i+=1) {
        v = vertices[i].getData("position");
        glm::vec3 glmv = glm::vec3(v[0],v[1],v[2]);
        minBounds = glm::min(minBounds,glmv);
        maxBounds = glm::max(maxBounds,glmv);
        this->vertices.push_back(glmv);
        
        v = vertices[i].getData("normal");
        this->normals.push_back(glm::vec3(v[0],v[1],v[2]));

        v = vertices[i].getData("texcoord");
        this->texcoords.push_back(glm::vec2(v[0],v[1]));
    }

    // Extract triangles from the mesh
    // Each triangle is stored as 3 vertex indices
    vector<unsigned int> indices = mesh.getPrimitives();
    int primitiveSize = mesh.getPrimitiveSize();
    if (primitiveSize == 3) {
        for (size_t i = 0; i < indices.size(); i += 3) {
            this->triangles.push_back(glm::ivec3(indices[i], indices[i+1], indices[i+2]));
        }
    }
    
    maxPointsPerLeaf = 5;
    root = buildKDTree(maxPointsPerLeaf);

    // Now add the triangles to the KD-tree nodes
    addTrianglesToTree();
        
}

KDTree::~KDTree() {
    if (root!=NULL) {
        delete root;
    }
}

/**
 * @brief Main intersection function for KD-Tree
 * 
 * Per Section 2.2.3: First check if ray intersects bounding box.
 * If yes, use the bounding box intersection to find tmin and tmax.
 * If no, return immediately without traversing the tree.
 */
HitRecord KDTree::intersect(const Ray& objectRay,
                            const Ray& viewRay,
                            const glm::mat4& modelviewMatrix,
                            const glm::mat4& normalMatrix,
                            const util::Material& material,
                            const string& textureName) {
    HitRecord hitRecord;
    
    if (root == NULL) {
        return hitRecord;
    }
    
    // Section 2.2.3: Check if ray intersects bounding box first
    float tmin, tmax;
    bool hitBox = intersect_bounding_box(objectRay, &tmin, &tmax);
    
    if (!hitBox) {
        // Ray completely misses the object - no need to traverse KD-tree
        return hitRecord;
    }
    
    // Ensure tmin is at least 0 (we don't want intersections behind the ray origin)
    if (tmin < 0.0f) {
        tmin = 0.0f;
    }
    
    // Create a set to track tested triangles across the entire traversal
    // This prevents testing the same triangle multiple times
    set<int> testedTriangles;
    
    // Delegate to root node's intersect method with computed tmin/tmax
    HitRecord result = root->intersect(objectRay, viewRay, modelviewMatrix, normalMatrix,
                          material, textureName, tmin, tmax, testedTriangles);
    
    return result;
}


KDNode *KDTree::buildKDTree(int maxPointsPerLeaf) {

    vector<int> sortedByX;
    vector<int> sortedByY;
    vector<int> sortedByZ;
    for (int i=0;i<vertices.size();i+=1){
        sortedByX.push_back(i);
        sortedByY.push_back(i);
        sortedByZ.push_back(i);
    }
    // Sort the vector in ascending order using x
    std::sort(sortedByX.begin(), sortedByX.end(), [this](int a, int b) {
        return vertices[a][0] < vertices[b][0]; // Returns true if 'a' should come before 'b'
    });
    // Sort the vector in ascending order using y
    std::sort(sortedByY.begin(), sortedByY.end(), [this](int a, int b) {
        return vertices[a][1] < vertices[b][1]; // Returns true if 'a' should come before 'b'
    });
    // Sort the vector in ascending order using z
    std::sort(sortedByZ.begin(), sortedByZ.end(), [this](int a, int b) {
        return vertices[a][2] < vertices[b][2]; // Returns true if 'a' should come before 'b'
    });
    return buildKDTree(sortedByX,sortedByY,sortedByZ,maxPointsPerLeaf,0); 
}

KDNode *KDTree::buildKDTree(vector<int>& sortedByX,vector<int>& sortedByY,vector<int>& sortedByZ,int maxPointsPerLeaf,int depth) {
    int splitDimension = depth % 3;
    int numPoints = sortedByX.size();

    if (numPoints == 0) {
        return NULL;
    }

    if (numPoints<=maxPointsPerLeaf) {
        return new KDLeafNode(&vertices,&normals,sortedByX,this);
    }

    glm::vec4 plane_equation;
    switch(splitDimension) {
        case 0: 
        plane_equation = glm::vec4(0.0f,0.0f,0.0f,-vertices[sortedByX[numPoints/2]][splitDimension]);
        break;
        case 1: 
        plane_equation = glm::vec4(0.0f,0.0f,0.0f,-vertices[sortedByY[numPoints/2]][splitDimension]);
        break;
        case 2: 
        plane_equation = glm::vec4(0.0f,0.0f,0.0f,-vertices[sortedByZ[numPoints/2]][splitDimension]);
        break;
    }
    plane_equation[splitDimension] = 1;

    //now split X, Y, Z lists into two, for the two halves
    vector<int> xBefore,yBefore,zBefore,xAfter,yAfter,zAfter,on;
    for (int i=0;i<sortedByX.size();i+=1) {
        float signed_distance = glm::dot(plane_equation,glm::vec4(vertices[sortedByX[i]],1.0f));
        if (signed_distance<0) { // ray's start is on the left side
            xBefore.push_back(sortedByX[i]);
        }
        else if (signed_distance>0) { 
            xAfter.push_back(sortedByX[i]);
        }
        else {
            on.push_back(sortedByX[i]);
        }
    }

    for (int i=0;i<sortedByY.size();i+=1) {
        float signed_distance = glm::dot(plane_equation,glm::vec4(vertices[sortedByY[i]],1.0f));
        if (signed_distance<0) { // ray's start is on the left side
            yBefore.push_back(sortedByY[i]);
        }
        else if (signed_distance>0) { 
            yAfter.push_back(sortedByY[i]);
        }
        
    }

    for (int i=0;i<sortedByZ.size();i+=1) {
        float signed_distance = glm::dot(plane_equation,glm::vec4(vertices[sortedByZ[i]],1.0f));
        if (signed_distance<0) { // ray's start is on the left side
            zBefore.push_back(sortedByZ[i]);
        }
        else if (signed_distance>0) { 
            zAfter.push_back(sortedByZ[i]);
        }
    }

    KDNode *left,*right;

    left = buildKDTree(xBefore,yBefore,zBefore,maxPointsPerLeaf,depth+1);
    right = buildKDTree(xAfter,yAfter,zAfter,maxPointsPerLeaf,depth+1);
    return new KDInternalNode(left,right,&vertices,&normals,on,plane_equation,this);


}

/**
 * @brief Test if ray intersects the axis-aligned bounding box
 * 
 * Section 2.2.3: Use the bounds stored in the KD-tree to check if the ray
 * will hit the corresponding bounding box. If yes, compute tmin and tmax.
 * 
 * Uses the "slab method" for ray-AABB intersection.
 * For each axis, compute the t values where the ray enters and exits the slab.
 * The ray intersects the box if and only if all intervals overlap.
 * 
 * @param objectRay The ray in object space
 * @param min_t Output: minimum t value (entry point)
 * @param max_t Output: maximum t value (exit point)
 * @return true if ray intersects bounding box, false otherwise
 */
bool KDTree::intersect_bounding_box(const Ray& objectRay, float *min_t, float *max_t) {
    glm::vec3 rayOrigin = glm::vec3(objectRay.getStart());
    glm::vec3 rayDir = glm::vec3(objectRay.getDirection());
    
    float tmin = -numeric_limits<float>::infinity();
    float tmax = numeric_limits<float>::infinity();
    
    // For each axis (x, y, z)
    for (int i = 0; i < 3; i++) {
        float origin = rayOrigin[i];
        float dir = rayDir[i];
        float boxMin = minBounds[i];
        float boxMax = maxBounds[i];
        
        if (abs(dir) < 0.0000001f) {
            // Ray is parallel to this slab
            // Check if origin is within the slab
            if (origin < boxMin || origin > boxMax) {
                // Ray is parallel and outside the slab - no intersection
                return false;
            }
            // Ray is parallel and inside the slab - this axis doesn't constrain t
        } else {
            // Compute t values for intersection with the two planes of this slab
            float invDir = 1.0f / dir;
            float t1 = (boxMin - origin) * invDir;
            float t2 = (boxMax - origin) * invDir;
            
            // Make sure t1 is the near intersection, t2 is the far
            if (t1 > t2) {
                float temp = t1;
                t1 = t2;
                t2 = temp;
            }
            
            // Update the interval
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            
            // Check if the interval is valid
            if (tmin > tmax) {
                return false;
            }
        }
    }
    
    // Check if the intersection is in the positive direction of the ray
    if (tmax < 0) {
        return false;
    }
    
    *min_t = tmin;
    *max_t = tmax;
    return true;
}

// Add all triangles to the KD-tree nodes
void KDTree::addTrianglesToTree() {
    if (root == NULL) return;
    
    // For each triangle, find which nodes it belongs to
    for (int triIndex = 0; triIndex < triangles.size(); triIndex++) {
        addTriangleToNode(root, triIndex);
    }
}

// Recursively add a triangle to the appropriate nodes
// Per instructor: "When a triangle straddles the split plane, it must be added to both halves."
// This means a triangle can appear in multiple nodes.
void KDTree::addTriangleToNode(KDNode* node, int triangleIndex) {
    if (node == NULL) return;
    
    glm::ivec3 tri = triangles[triangleIndex];
    
    // Check if this is a leaf node or internal node
    KDLeafNode* leaf = dynamic_cast<KDLeafNode*>(node);
    KDInternalNode* internal = dynamic_cast<KDInternalNode*>(node);
    
    if (leaf != NULL) {
        // For leaf node: add any triangle that reaches this leaf
        // The triangle was already determined to belong here by the internal node logic above
        leaf->addTriangle(triangleIndex);
    }
    else if (internal != NULL) {
        // For internal node: check where each vertex lies relative to split plane
        glm::vec4 plane = internal->getPlane();
        const float EPS = 0.0001f;
        float d0 = glm::dot(plane, glm::vec4(vertices[tri.x], 1.0f));
        float d1 = glm::dot(plane, glm::vec4(vertices[tri.y], 1.0f));
        float d2 = glm::dot(plane, glm::vec4(vertices[tri.z], 1.0f));
        
        bool on0 = abs(d0) < EPS;
        bool on1 = abs(d1) < EPS;
        bool on2 = abs(d2) < EPS;
        bool allOnPlane = (on0 && on1 && on2);
        bool hasLeft = (d0 < -EPS || d1 < -EPS || d2 < -EPS);
        bool hasRight = (d0 > EPS || d1 > EPS || d2 > EPS);
        bool touchesPlane = on0 || on1 || on2; // any vertex on plane should go to both sides
        
        // Triangle lies completely on the split plane - store in internal node
        if (allOnPlane) {
            internal->addTriangle(triangleIndex);
        }
        else {
            // Triangle straddles the plane or is on one side
            bool sendLeft = hasLeft || touchesPlane;
            bool sendRight = hasRight || touchesPlane;
            if (sendLeft) {
                addTriangleToNode(internal->getLeft(), triangleIndex);
            }
            if (sendRight) {
                addTriangleToNode(internal->getRight(), triangleIndex);
            }
        }
    }
}
