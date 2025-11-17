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
#include "VertexAttrib.h"
#include "Callbacks.h"
#include "sgraph/IScenegraph.h"
#include <Light.h>

#include <stack>
#include <vector>
using namespace std;

enum Camera {STATIONARY,KEYBOARD,FPS,CHOPPER};

class View
{
public:
    class KeyboardCamera {
    public:
        glm::vec4 eye;
        glm::vec4 behind,up,right;
        float move_speed,turn_speed;
    };
    View();
    ~View();
    void init(Callbacks* callbacks,map<string,util::PolygonMesh<VertexAttrib>>& meshes);
    void display(sgraph::IScenegraph *scenegraph);
    bool shouldWindowClose();
    void closeWindow();
    void setCamera(Camera new_setting);
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void moveForward();
    void moveBack();
    void turnUp();
    void turnDown();
    void turnLeft();
    void turnRight();
    void toggleShading();

private: 
    Camera currentCamera;
    KeyboardCamera kbd_camera;
    GLFWwindow* window;
    util::ShaderProgram program;
    util::ShaderLocationsVault shaderLocations;
    map<string,util::ObjectInstance *> objects;
    glm::mat4 projection;
    stack<glm::mat4> modelview;
    sgraph::SGNodeVisitor *renderer;
    int frames;
    double time;
    int tick;
    bool useToonShading;
    static constexpr int MAX_LIGHTS = 10;

    void uploadLights(const vector<util::Light>& lights);
};

#endif
