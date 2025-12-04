#include "KDTree.h"
#include <iostream>
using namespace std;

KDTree::KDTree(util::PolygonMesh<VertexAttrib>& mesh) {
    vector<VertexAttrib> vertices = mesh.getVertexAttributes();
    vector<float> v;
    minBounds = glm::vec3(numeric_limits<float>::max(),numeric_limits<float>::max(),numeric_limits<float>::max());
    maxBounds = glm::vec3(numeric_limits<float>::min(),numeric_limits<float>::min(),numeric_limits<float>::min());
    for (int i=0;i<vertices.size();i+=1) {
        v = vertices[i].getData("position");
        glm::vec3 glmv = glm::vec3(v[0],v[1],v[2]);
        minBounds = glm::min(minBounds,glmv);
        maxBounds = glm::max(maxBounds,glmv);
        this->vertices.push_back(glmv);
        
        v = vertices[i].getData("normal");
        this->normals.push_back(glm::vec3(v[0],v[1],v[2]));
    }
    
    maxPointsPerLeaf = 5;
    root = buildKDTree(maxPointsPerLeaf);

    //now extract and add the triangles to the kdtree
    
        
}

KDTree::~KDTree() {
    if (root!=NULL) {
        delete root;
    }
}

HitRecord KDTree::intersect(Ray& objectRay,Ray& viewRay,glm::mat4 normalMatrix) {
    HitRecord hitRecord;

    return hitRecord;
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

bool KDTree::intersect_bounding_box(Ray& objectRay,float *min_t,float *max_t) {
    
    return false;
}
