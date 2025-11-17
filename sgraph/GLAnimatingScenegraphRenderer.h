#ifndef _GLANIMATINGSCENEGRAPHRENDERER_H_
#define _GLANIMATINGSCENEGRAPHRENDERER_H_

#include "GLScenegraphRenderer.h"
#include "KeyframeTransform.h"
#include <ShaderProgram.h>
#include <ShaderLocationsVault.h>
#include "ObjectInstance.h"
#include <stack>
#include <iostream>
using namespace std;

namespace sgraph {
    /**
     * This visitor implements drawing an animating scene graph using OpenGL
     * 
     */
    class GLAnimatingScenegraphRenderer: public GLScenegraphRenderer {
        public:
        /**
         * @brief Construct a new GLScenegraphRenderer object
         * 
         * @param mv a reference to modelview stack that will be used while rendering
         * @param os the map of ObjectInstance objects
         * @param shaderLocations the shader locations for the program used to render
         */
        GLAnimatingScenegraphRenderer(int *tick,stack<glm::mat4>& mv,map<string,util::ObjectInstance *>& os,util::ShaderLocationsVault& shaderLocations) 
            : GLScenegraphRenderer(mv,os,shaderLocations)
            , tick(tick) {
            
        }

        /**
         * @brief Animation node
         * 
         * @param transformNode 
         */
        void visitKeyframeTransformNode(KeyframeTransformNode *node) {
            node->setTransform(*tick);
            
            visitTransformNode(node);
            
        }

    private:
        const int *tick;
        

   };
}

#endif