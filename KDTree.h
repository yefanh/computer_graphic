#ifndef __KDTREE_H__
#define __KDTREE_H__

#include "KDNode.h"
#include "KDInternalNode.h"
#include "KDLeafNode.h"
#include <limits>
#include <vector>
using namespace std;
#include "VertexAttrib.h"
#include "PolygonMesh.h"
#include <glm/glm.hpp>

class KDTree {
    friend class KDInternalNode;
    friend class KDAbstractNode;
    friend class KDLeafNode;
    public:
    KDTree(util::PolygonMesh<VertexAttrib>& mesh);

    ~KDTree();

    /**
     * @brief Original intersect function (calls the new one with default tmin/tmax)
     * 
     * Per Section 2.2: "For now, use tmin = 0 and tmax = infinity when calling this function."
     */
    virtual HitRecord intersect(const Ray& objectRay,
                                const Ray& viewRay,
                                const glm::mat4& modelviewMatrix,
                                const glm::mat4& normalMatrix,
                                const util::Material& material,
                                const string& textureName);

    // Get triangle data for intersection tests
    const vector<glm::ivec3>& getTriangles() const { return triangles; }
    const vector<glm::vec3>& getVertices() const { return vertices; }
    const vector<glm::vec3>& getNormals() const { return normals; }
    const vector<glm::vec2>& getTexcoords() const { return texcoords; }

    void setMeshName(const string& name) { meshName = name; }
    string getMeshName() const { return meshName; }

    private:
    KDNode *buildKDTree(int maxPointsPerLeaf);
    KDNode *buildKDTree(vector<int>& sortedByX,vector<int>& sortedByY,vector<int>& sortedByZ,int maxPointsPerLeaf,int depth);
    bool intersect_bounding_box(const Ray& objectRay, float *min_t, float *max_t);
    void addTrianglesToTree();
    void addTriangleToNode(KDNode* node, int triangleIndex);
    int countTrianglesInNode(KDNode* node); // Debug helper
    
    string meshName;

    private:
    KDNode *root;
    int maxPointsPerLeaf;
    vector<glm::vec3> vertices;
    vector<glm::vec3> normals;
    vector<glm::vec2> texcoords;
    vector<glm::ivec3> triangles;
    glm::vec3 minBounds,maxBounds;
};

#endif