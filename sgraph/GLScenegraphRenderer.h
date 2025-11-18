#ifndef _GLSCENEGRAPHRENDERER_H_
#define _GLSCENEGRAPHRENDERER_H_

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
#include <glad/glad.h>
#include <map>
#include <string>
#include <stack>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
using namespace std;

namespace sgraph {
    /**
     * This visitor implements drawing the scene graph using OpenGL
     * 
     */
    class GLScenegraphRenderer: public SGNodeVisitor {
        public:
        /**
         * @brief Construct a new GLScenegraphRenderer object
         * 
         * @param mv a reference to modelview stack that will be used while rendering
         * @param os the map of ObjectInstance objects
         * @param shaderLocations the shader locations for the program used to render
         */
        GLScenegraphRenderer(stack<glm::mat4>& mv,
                             map<string,util::ObjectInstance *>& os,
                             util::ShaderLocationsVault& shaderLocations,
                             map<string,GLuint>& textures,
                             GLuint defaultTexture) 
            : modelview(mv)
            , objects(os)
            , textureTable(textures)
            , fallbackTexture(defaultTexture) {
            this->shaderLocations = shaderLocations;
            for (map<string,util::ObjectInstance *>::iterator it=objects.begin();it!=objects.end();it++) {
                cout << "Mesh with name: "<< it->first << endl;
            }
        }

        /**
         * @brief Recur to the children for drawing
         * 
         * @param groupNode 
         */
        void visitGroupNode(GroupNode *groupNode) {
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
            //send modelview matrix to GPU  
            glm::mat4 mv = modelview.top();
            glUniformMatrix4fv(shaderLocations.getLocation("modelview"), 1, GL_FALSE, glm::value_ptr(mv));
            glm::mat3 normalmatrix = glm::transpose(glm::inverse(glm::mat3(mv)));
            glUniformMatrix3fv(shaderLocations.getLocation("normalmatrix"),1,GL_FALSE,glm::value_ptr(normalmatrix));

            util::Material material = leafNode->getMaterial();
            glUniform3fv(shaderLocations.getLocation("material.ambient"),1,glm::value_ptr(glm::vec3(material.getAmbient())));
            glUniform3fv(shaderLocations.getLocation("material.diffuse"),1,glm::value_ptr(glm::vec3(material.getDiffuse())));
            glUniform3fv(shaderLocations.getLocation("material.specular"),1,glm::value_ptr(glm::vec3(material.getSpecular())));
            glUniform3fv(shaderLocations.getLocation("material.emission"),1,glm::value_ptr(glm::vec3(material.getEmission())));
            glUniform1f(shaderLocations.getLocation("material.shininess"),material.getShininess());

            int samplerLoc = shaderLocations.getLocation("diffuseTex");
            if (samplerLoc>=0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, resolveTexture(leafNode));
                glUniform1i(samplerLoc,0);
            }
            objects[leafNode->getInstanceOf()]->draw();
        }

        /**
         * @brief Multiply the transform to the modelview and recur to child
         * 
         * @param transformNode 
         */
        void visitTransformNode(TransformNode * transformNode) {
            modelview.push(modelview.top());
            modelview.top() = modelview.top() * transformNode->getTransform();
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
        GLuint resolveTexture(const LeafNode *leafNode) {
            string textureName = leafNode->getTextureName();
            if (!textureName.empty()) {
                map<string,GLuint>::iterator it = textureTable.find(textureName);
                if (it!=textureTable.end()) {
                    return it->second;
                }
            }
            return fallbackTexture;
        }

        util::ShaderLocationsVault shaderLocations;
        map<string,util::ObjectInstance *> objects;
        map<string,GLuint>& textureTable;
        GLuint fallbackTexture;

   };
}

#endif
