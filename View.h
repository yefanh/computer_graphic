#ifndef __VIEW_H__
#define __VIEW_H__

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ShaderProgram.h>
#include "sgraph/SGNodeVisitor.h"
#include "ObjectInstance.h"
#include "PolygonMesh.h"
#include "TextureImage.h"
#include "VertexAttrib.h"
#include "Callbacks.h"
#include "sgraph/IScenegraph.h"

#include <stack>
using namespace std;


class View
{
public:
    View();
    ~View();
    void init(Callbacks *callbacks,map<string,util::PolygonMesh<VertexAttrib>>& meshes,map<string,util::TextureImage*>& textures);
    void display(sgraph::IScenegraph *scenegraph);
    bool shouldWindowClose();
    void closeWindow();
    
    /**
     * @brief Start ray tracing and output result to a PPM image file
     * @param scenegraph The scene graph to ray trace
     */
    void raytrace(sgraph::IScenegraph *scenegraph);
    
    /**
     * @brief Get the current modelview matrix (for ray tracing camera setup)
     * @return The modelview matrix stack
     */
    stack<glm::mat4>& getModelview() { return modelview; }
    
    /**
     * @brief Get the window width
     */
    int getWindowWidth();
    
    /**
     * @brief Get the window height
     */
    int getWindowHeight();

private: 

    GLFWwindow* window;
    util::ShaderProgram program;
    util::ShaderLocationsVault shaderLocations;
    map<string,util::ObjectInstance *> objects;
    map<string,GLuint> textureIds;
    glm::mat4 projection;
    stack<glm::mat4> modelview;
    sgraph::SGNodeVisitor *renderer;
    int frames;
    double time;
};

#endif