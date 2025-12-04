#ifndef _LIGHTGATHERER_H_
#define _LIGHTGATHERER_H_

#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include <ShaderProgram.h>
#include <ShaderLocationsVault.h>
#include "ObjectInstance.h"
#include <stack>
#include <iostream>
using namespace std;

namespace sgraph {
    /**
     * This visitor implements getting the lights from  scene graph 
     * 
     */
    class LightGatherer: public SGNodeVisitor {
        public:
        /**
         * @brief Construct a new LightGatherer object
         * 
         * @param mv a reference to modelview stack that will be used while rendering
         */

        LightGatherer(stack<glm::mat4>& mv) 
            : modelview(mv) {
           
        }

        vector<util::Light> getLightsInViewSpace() {
            return lightsInViewSpace;
        }

        /**
         * @brief Recur to the children for drawing
         * 
         * @param groupNode 
         */
        void visitGroupNode(GroupNode *groupNode) {
            addLights(groupNode);
            for (int i=0;i<groupNode->getChildren().size();i=i+1) {
                groupNode->getChildren()[i]->accept(this);
            }
        }

        /**
         * @brief Draw the instance for the leaf, after passing the 
         * modelview and color to the shader
         * 
         * @param leafNode 
         */
        void visitLeafNode(LeafNode *leafNode) {
            addLights(leafNode);
        }

        /**
         * @brief Multiply the transform to the modelview and recur to child
         * 
         * @param transformNode 
         */
        void visitTransformNode(TransformNode * transformNode) {
            modelview.push(modelview.top());
            modelview.top() = modelview.top() * transformNode->getTransform();
            addLights(transformNode);
            if (transformNode->getChildren().size()>0) {
                transformNode->getChildren()[0]->accept(this);
            }
            modelview.pop();
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         * 
         * @param scaleNode 
         */
        void visitScaleTransform(ScaleTransform *scaleNode) {
            visitTransformNode(scaleNode);
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         * 
         * @param translateNode 
         */
        void visitTranslateTransform(TranslateTransform *translateNode) {
            visitTransformNode(translateNode);
        }

        void visitRotateTransform(RotateTransform *rotateNode) {
            visitTransformNode(rotateNode);
        }

        private:
        stack<glm::mat4>& modelview;    
        vector<util::Light> lightsInViewSpace;

        void addLights(AbstractSGNode *node) {
            vector<util::Light> lights = node->getLights();
            glm::mat4 transform = modelview.top();
            for (int i=0;i<lights.size();i=i+1) {
                util::Light l = lights[i];
                //transform its position
                glm::vec4 p = l.getPosition();
                p = transform * p;
                l.setPosition(p);

                //transform its spot direction
                p = l.getSpotDirection();
                p = transform * p;
                l.setSpotDirection(p.x,p.y,p.z);

                //push into vector
                lightsInViewSpace.push_back(l);
            }
        }

   };
}

#endif