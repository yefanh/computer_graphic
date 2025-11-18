#include "View.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <limits>
#include <set>
#include <iostream>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sgraph/GLAnimatingScenegraphRenderer.h"
#include "VertexAttrib.h"

constexpr int View::MAX_LIGHTS;

bool View::loadPPM(const string& filename,vector<unsigned char>& pixels,int& width,int& height) {
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cerr << "Failed to open texture: " << filename << endl;
        return false;
    }

    auto readToken = [&](string& out) -> bool {
        while (file >> out) {
            if (!out.empty() && out[0]=='#') {
                file.ignore(numeric_limits<streamsize>::max(),'\n');
                continue;
            }
            return true;
        }
        return false;
    };

    string token;
    if (!readToken(token) || token != "P3") {
        cerr << "Unsupported PPM format in " << filename << endl;
        return false;
    }
    if (!readToken(token)) return false;
    width = stoi(token);
    if (!readToken(token)) return false;
    height = stoi(token);
    if (!readToken(token)) return false;
    int maxVal = stoi(token);
    if (maxVal <=0 || maxVal > 255) {
        cerr << "Invalid max value in PPM: " << filename << endl;
        return false;
    }

    pixels.resize(width*height*3);
    for (int i=0;i<width*height*3;i++) {
        if (!readToken(token)) {
            cerr << "Unexpected EOF while reading PPM: " << filename << endl;
            return false;
        }
    int value = stoi(token);
    value = glm::clamp(value,0,255);
        pixels[i] = static_cast<unsigned char>(value);
    }
    return true;
}

GLuint View::createTextureFromImage(const vector<unsigned char>& pixels,int width,int height) {
    if (pixels.empty() || width<=0 || height<=0) {
        return 0;
    }
    GLuint texId = 0;
    glGenTextures(1,&texId);
    glBindTexture(GL_TEXTURE_2D,texId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,0);
    return texId;
}

GLuint View::createSolidTexture(unsigned char r,unsigned char g,unsigned char b) {
    vector<unsigned char> data = {r,g,b};
    return createTextureFromImage(data,1,1);
}

void View::loadTextures(const map<string,string>& texturePaths) {
    textures.clear();
    if (defaultTexture!=0) {
        glDeleteTextures(1,&defaultTexture);
        defaultTexture = 0;
    }
    defaultTexture = createSolidTexture(255,255,255);
    bool whiteProvided = false;

    for (map<string,string>::const_iterator it=texturePaths.begin(); it!=texturePaths.end(); ++it) {
        vector<unsigned char> pixels;
        int width=0,height=0;
        if (loadPPM(it->second,pixels,width,height)) {
            GLuint texId = createTextureFromImage(pixels,width,height);
            if (texId!=0) {
                textures[it->first] = texId;
                if (it->first == "white") {
                    whiteProvided = true;
                }
                continue;
            }
        }
        cerr << "Falling back to white texture for " << it->first << endl;
    }

    if (!whiteProvided && defaultTexture!=0) {
        textures["white"] = defaultTexture;
    }
}

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
    defaultTexture = 0;
    renderer = NULL;
    toonRenderer = NULL;
}

View::~View(){

}

void View::init(Callbacks *callbacks,
                map<string,util::PolygonMesh<VertexAttrib>>& meshes,
                const map<string,string>& texturePaths) 
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

    // create the Phong shader program
    program.createProgram(string("shaders/default.vert"),
                          string("shaders/default.frag"));
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
    shaderVarsToVertexAttribs["vTexCoord"] = "texcoord";
    
    for (typename map<string,util::PolygonMesh<VertexAttrib> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++) {
        util::ObjectInstance * obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(shaderLocations,shaderVarsToVertexAttribs,it->second);
        objects[it->first] = obj;
    }

    // create the toon shader program (separate shader as per assignment)
    toonProgram.createProgram(string("shaders/default.vert"),
                              string("shaders/toon.frag"));
    toonProgram.enable();
    toonShaderLocations = toonProgram.getAllShaderVariables();

    for (typename map<string,util::PolygonMesh<VertexAttrib> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++) {
        util::ObjectInstance * obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(toonShaderLocations,shaderVarsToVertexAttribs,it->second);
        toonObjects[it->first] = obj;
    }

    loadTextures(texturePaths);
    
	int window_width,window_height;
    glfwGetFramebufferSize(window,&window_width,&window_height);

    //prepare the projection matrix for perspective projection
	projection = glm::perspective(glm::radians(60.0f),(float)window_width/window_height,0.1f,10000.0f);
    glViewport(0, 0, window_width,window_height);

    frames = 0;
    time = glfwGetTime();

    renderer = new sgraph::GLAnimatingScenegraphRenderer(&tick,modelview,objects,shaderLocations,textures,defaultTexture);
    toonRenderer = new sgraph::GLAnimatingScenegraphRenderer(&tick,modelview,toonObjects,toonShaderLocations,textures,defaultTexture);
    
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

void View::resize(int width,int height) {
    if (height==0) height = 1;
    glViewport(0,0,width,height);
    projection = glm::perspective(glm::radians(60.0f),
                                  (float)width/height,
                                  0.1f,
                                  10000.0f);
}

void View::uploadLights(const vector<util::Light>& lights,
                        util::ShaderProgram& activeProgram,
                        util::ShaderLocationsVault& activeLocations) {
    int count = std::min(static_cast<int>(lights.size()), MAX_LIGHTS);
    auto getLocation = [&](const string& name) -> int {
        int loc = activeLocations.getLocation(name);
        if (loc < 0) {
            loc = glGetUniformLocation(activeProgram.getProgram(),name.c_str());
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
    util::ShaderProgram& activeProgram = useToonShading ? toonProgram : program;
    util::ShaderLocationsVault& activeLocations = useToonShading ? toonShaderLocations : shaderLocations;
    sgraph::SGNodeVisitor *activeRenderer = useToonShading ? toonRenderer : renderer;

    activeProgram.enable();
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
    uploadLights(lights,activeProgram,activeLocations);

    //send projection matrix to GPU    
    glUniformMatrix4fv(activeLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    

    //draw scene graph here
    scenegraph->getRoot()->accept(activeRenderer);

    
    
    modelview.pop();
    glFlush();
    activeProgram.disable();
    
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
    for (map<string,util::ObjectInstance *>::iterator it=toonObjects.begin();
           it!=toonObjects.end();
           it++) {
          it->second->cleanup();
          delete it->second;
    }
    objects.clear();
    toonObjects.clear();

    if (renderer!=NULL) {
        delete renderer;
        renderer = NULL;
    }
    if (toonRenderer!=NULL) {
        delete toonRenderer;
        toonRenderer = NULL;
    }

    set<GLuint> textureIds;
    if (defaultTexture!=0) {
        textureIds.insert(defaultTexture);
    }
    for (map<string,GLuint>::iterator it=textures.begin(); it!=textures.end(); ++it) {
        textureIds.insert(it->second);
    }
    if (!textureIds.empty()) {
        vector<GLuint> toDelete(textureIds.begin(), textureIds.end());
        glDeleteTextures(static_cast<GLsizei>(toDelete.size()), toDelete.data());
    }
    textures.clear();
    defaultTexture = 0;
    glfwDestroyWindow(window);

    glfwTerminate();
}





