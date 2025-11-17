#include "View.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sgraph/GLAnimatingScenegraphRenderer.h"
#include "VertexAttrib.h"

constexpr int View::MAX_LIGHTS;

View::View() {
    tick = 0;
    currentCamera = STATIONARY;
    kbd_camera.eye = glm::vec4(200.0f,200.0f,200.0f,1.0f);
    kbd_camera.behind = glm::vec4(0.0f,0.0f,1.0f,0.0f);
    kbd_camera.up = glm::vec4(0.0f,1.0f,0.0f,0.0f);
    kbd_camera.right = glm::vec4(1.0f,0.0f,0.0f,0.0f);
    kbd_camera.move_speed = 5;
    kbd_camera.turn_speed = 5;
    useToonShading = false;
}

View::~View(){

}

void View::init(Callbacks *callbacks,map<string,util::PolygonMesh<VertexAttrib>>& meshes) 
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 800, "Hello GLFW: Per-vertex coloring", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
     glfwSetWindowUserPointer(window, (void *)callbacks);

    //using C++ functions as callbacks to a C-style library
    glfwSetKeyCallback(window, 
    [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->onkey(key,scancode,action,mods);
    });
    
    glfwSetWindowSizeCallback(window, 
    [](GLFWwindow* window, int width,int height)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->reshape(width,height);
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // create the shader program
    program.createProgram(string("shaders/default.vert"),
                          string("shaders/default.frag"));
    // assuming it got created, get all the shader variables that it uses
    // so we can initialize them at some point
    // enable the shader program
    program.enable();
    shaderLocations = program.getAllShaderVariables();

    
    /* In the mesh, we have some attributes for each vertex. In the shader
     * we have variables for each vertex attribute. We have to provide a mapping
     * between attribute name in the mesh and corresponding shader variable
     name.
     *
     * This will allow us to use PolygonMesh with any shader program, without
     * assuming that the attribute names in the mesh and the names of
     * shader variables will be the same.

       We create such a shader variable -> vertex attribute mapping now
     */
    map<string, string> shaderVarsToVertexAttribs;

    shaderVarsToVertexAttribs["vPosition"] = "position";
    shaderVarsToVertexAttribs["vNormal"] = "normal";
    
    
    for (typename map<string,util::PolygonMesh<VertexAttrib> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++) {
        util::ObjectInstance * obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(shaderLocations,shaderVarsToVertexAttribs,it->second);
        objects[it->first] = obj;
    }
    
	int window_width,window_height;
    glfwGetFramebufferSize(window,&window_width,&window_height);

    //prepare the projection matrix for perspective projection
	projection = glm::perspective(glm::radians(60.0f),(float)window_width/window_height,0.1f,10000.0f);
    glViewport(0, 0, window_width,window_height);

    frames = 0;
    time = glfwGetTime();

    renderer = new sgraph::GLAnimatingScenegraphRenderer(&tick,modelview,objects,shaderLocations);
    
}

void View::setCamera(Camera new_camera_setting) {
    currentCamera = new_camera_setting;
}

void View::moveUp() {
    kbd_camera.eye = kbd_camera.eye + kbd_camera.move_speed * kbd_camera.up;
}
    
void View::moveDown() {
    kbd_camera.eye = kbd_camera.eye - kbd_camera.move_speed * kbd_camera.up;
}
    
void View::moveLeft() {
    kbd_camera.eye = kbd_camera.eye - kbd_camera.move_speed * kbd_camera.right;
}
    
void View::moveRight() {
    kbd_camera.eye = kbd_camera.eye + kbd_camera.move_speed * kbd_camera.right;
}

void View::moveForward() {
    kbd_camera.eye = kbd_camera.eye - kbd_camera.move_speed * kbd_camera.behind;
}

void View::moveBack() {
    kbd_camera.eye = kbd_camera.eye + kbd_camera.move_speed * kbd_camera.behind;
}
    
void View::turnUp() {
    glm::mat4 transform = glm::rotate(glm::mat4(1.0),glm::radians(kbd_camera.turn_speed),kbd_camera.right.xyz());
    kbd_camera.up =  transform * kbd_camera.up;
    kbd_camera.behind = transform * kbd_camera.behind;
}
    
void View::turnDown() {
glm::mat4 transform = glm::rotate(glm::mat4(1.0),glm::radians(-kbd_camera.turn_speed),kbd_camera.right.xyz());
    kbd_camera.up =  transform * kbd_camera.up;
    kbd_camera.behind = transform * kbd_camera.behind;
}
    
void View::turnLeft() {
    glm::mat4 transform = glm::rotate(glm::mat4(1.0),glm::radians(kbd_camera.turn_speed),kbd_camera.up.xyz());
    kbd_camera.right =  transform * kbd_camera.right;
    kbd_camera.behind = transform * kbd_camera.behind;
}

void View::turnRight() {
    glm::mat4 transform = glm::rotate(glm::mat4(1.0),glm::radians(-kbd_camera.turn_speed),kbd_camera.up.xyz());
    kbd_camera.right =  transform * kbd_camera.right;
    kbd_camera.behind = transform * kbd_camera.behind;
}

void View::toggleShading() {
    useToonShading = !useToonShading;
}

void View::uploadLights(const vector<util::Light>& lights) {
    int count = std::min(static_cast<int>(lights.size()), MAX_LIGHTS);
    auto getLocation = [&](const string& name) -> int {
        int loc = shaderLocations.getLocation(name);
        if (loc < 0) {
            loc = glGetUniformLocation(program.getProgram(),name.c_str());
        }
        return loc;
    };
    glUniform1i(getLocation("numLights"),count);
    for (int i=0;i<count;i++) {
        string base = string("lights[")+to_string(i)+"]";
        glm::vec3 ambient = lights[i].getAmbient();
        glm::vec3 diffuse = lights[i].getDiffuse();
        glm::vec3 specular = lights[i].getSpecular();
        glm::vec4 position = lights[i].getPosition();
        glm::vec4 spotDir = lights[i].getSpotDirection();
        glUniform3fv(getLocation(base+".ambient"),1,glm::value_ptr(ambient));
        glUniform3fv(getLocation(base+".diffuse"),1,glm::value_ptr(diffuse));
        glUniform3fv(getLocation(base+".specular"),1,glm::value_ptr(specular));
        glUniform4fv(getLocation(base+".position"),1,glm::value_ptr(position));
        glUniform4fv(getLocation(base+".spotDirection"),1,glm::value_ptr(spotDir));
        glUniform1f(getLocation(base+".spotCutoff"),lights[i].getSpotCutoff());
    }
}


void View::display(sgraph::IScenegraph *scenegraph) {
    int CHOPPER_CAM_RADIUS = 300;
    float speed_chopper = 0.25f;
    tick +=2;

  //  std::cout << "Tick: " << tick << endl;
    program.enable();
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT_FACE);

    

    modelview.push(glm::mat4(1.0));
  //  modelview.top() = modelview.top() * glm::lookAt(glm::vec3(0.0f,80.0f,150.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
    if (currentCamera == STATIONARY) {
        modelview.top() = modelview.top() * glm::lookAt(glm::vec3(200.0f,200.0f,200.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
    }
    else if (currentCamera == KEYBOARD) {
        modelview.top() = modelview.top() * glm::lookAt(kbd_camera.eye.xyz(),kbd_camera.eye.xyz()-100.0f*kbd_camera.behind.xyz(),kbd_camera.up.xyz());
    }
    else if (currentCamera == FPS) {
        map<string,sgraph::SGNode *> nodes = scenegraph->getNodes();
        sgraph::SGNode *fpsTemp = nodes["k-plane"];
        sgraph::TransformNode *fpsNode = dynamic_cast<sgraph::TransformNode *>(fpsTemp);
        if (fpsNode!=NULL) {
            glm::mat4 transform = fpsNode->getTransform();
            modelview.top() = modelview.top() * glm::lookAt(glm::vec3(0,0,0),glm::vec3(0,0,-10),glm::vec3(0,1,0))
                                              * glm::inverse(transform);
        }
    }
    else if (currentCamera == CHOPPER) {
       modelview.top() = modelview.top() * glm::lookAt(glm::vec3(75.0f + CHOPPER_CAM_RADIUS*cos(glm::radians(speed_chopper*(float)tick)),200.0f,75.0f+CHOPPER_CAM_RADIUS*sin(speed_chopper*glm::radians((float)tick))),glm::vec3(75.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)); 
    }
    vector<util::Light> lights = scenegraph->getLightsInViewCoordinates(modelview.top());
    uploadLights(lights);

    glUniform1i(shaderLocations.getLocation("toonMode"),useToonShading ? 1 : 0);

    //send projection matrix to GPU    
    glUniformMatrix4fv(shaderLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    

    //draw scene graph here
    scenegraph->getRoot()->accept(renderer);

    
    
    modelview.pop();
    glFlush();
    program.disable();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    frames++;
    double currenttime = glfwGetTime();
    if ((currenttime-time)>1.0) {
        printf("Framerate: %2.0f\r",frames/(currenttime-time));
        frames = 0;
        time = currenttime;
    }
    

}

bool View::shouldWindowClose() {
    return glfwWindowShouldClose(window);
}



void View::closeWindow() {
    for (map<string,util::ObjectInstance *>::iterator it=objects.begin();
           it!=objects.end();
           it++) {
          it->second->cleanup();
          delete it->second;
    } 
    glfwDestroyWindow(window);

    glfwTerminate();
}





