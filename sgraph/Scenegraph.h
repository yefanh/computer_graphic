#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include "IScenegraph.h"
#include "SGNode.h"
#include "ParentSGNode.h"
#include "TransformNode.h"
#include "glm/glm.hpp"
#include "IVertexData.h"
#include "PolygonMesh.h"
#include <string>
#include <map>
#include <vector>
using namespace std;

namespace sgraph {

  /**
 * A specific implementation of this scene graph. This implementation is still independent
 * of the rendering technology (i.e. OpenGL)
 * \author Amit Shesh
 */

  class Scenegraph: public IScenegraph {
    /**
     * The root of the scene graph tree
     */
  protected:
    SGNode *root;
    map<string,util::PolygonMesh<VertexAttrib> > meshes;
    map<string,string> meshPaths;


    /**
     * A map to store the (name,node) pairs. A map is chosen for efficient search
     */
    map<string,SGNode *> nodes;

  public:
    Scenegraph() {
      root = NULL;
    }

    ~Scenegraph() {
      dispose();
    }

    void dispose() {

      if (root!=NULL) {
          delete root;
          root = NULL;
      }
    }

    

    /**
     * Set the root of the scenegraph, and then pass a reference to this scene graph object
     * to all its node. This will enable any node to call functions of its associated scene graph
     * \param root
     */

    void makeScenegraph(SGNode *root) {
      this->root = root;
      if (root!=NULL) {
        this->root->setScenegraph(this);
      }

    }

   

    void addNode(const string& name, SGNode *node) {
      nodes[name]=node;
    }


    SGNode *getRoot() {
      return root;
    }



    map<string, SGNode *> getNodes() {
      return nodes;
    }

    void setMeshes(map<string,util::PolygonMesh<VertexAttrib> >& meshes) {
      this->meshes = meshes;
    }

    map<string,util::PolygonMesh<VertexAttrib> > getMeshes() {
      return this->meshes;
    }

    void setMeshPaths(map<string,string>& meshPaths) {
      this->meshPaths = meshPaths;
    }

    map<string,string> getMeshPaths() {
      return this->meshPaths;
    }

    vector<util::Light> getLightsInViewCoordinates(const glm::mat4& view) {
      vector<util::Light> lights;
      if (root==NULL) {
        return lights;
      }
      collectLights(root,view,lights);
      return lights;
    }

  private:
    void collectLights(SGNode *node,const glm::mat4& transform,vector<util::Light>& result) {
      if (node==NULL) return;

      glm::mat4 currentTransform = transform;
      TransformNode *transformNode = dynamic_cast<TransformNode *>(node);
      if (transformNode!=NULL) {
        currentTransform = currentTransform * transformNode->getTransform();
      }

      const vector<util::Light>& attachedLights = node->getLights();
      for (int i=0;i<attachedLights.size();i++) {
        result.push_back(transformLightToView(attachedLights[i],currentTransform));
      }

      ParentSGNode *parentNode = dynamic_cast<ParentSGNode *>(node);
      if (parentNode!=NULL) {
        vector<SGNode *> children = parentNode->getChildren();
        for (int i=0;i<children.size();i++) {
          collectLights(children[i],currentTransform,result);
        }
      }
    }

    util::Light transformLightToView(const util::Light& light,const glm::mat4& transform) {
      util::Light converted(light);
      glm::vec4 pos = light.getPosition();
      glm::vec4 transformedPos = transform * pos;
      if (pos.w==0.0f) {
        glm::vec3 dir = glm::vec3(transformedPos);
        float len = glm::length(dir);
        if (len>0.0f) {
          dir = dir/len;
        }
        converted.setDirection(dir.x,dir.y,dir.z);
      }
      else {
        converted.setPosition(transformedPos);
      }

      glm::vec4 spotDir = light.getSpotDirection();
      glm::vec3 spotDir3 = glm::vec3(spotDir);
      if (glm::length(spotDir3) > 0.0f) {
        glm::vec4 transformedSpot = transform * glm::vec4(spotDir3,0.0f);
        glm::vec3 normalized = glm::normalize(glm::vec3(transformedSpot));
        converted.setSpotDirection(normalized.x,normalized.y,normalized.z);
      }

      return converted;
    }
  };
}
#endif
