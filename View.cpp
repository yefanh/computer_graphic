#include "View.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sgraph/GLScenegraphRenderer.h"
#include "VertexAttrib.h"
#include "sgraph/LightGatherer.h"

View::View() {

}

View::~View(){

}

void View::init(Callbacks *callbacks,map<string,util::PolygonMesh<VertexAttrib>>& meshes,map<string,util::TextureImage*>& textures) 
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 800, "Lights and Textures in a Scenegraph", NULL, NULL);
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
    program.createProgram(string("shaders/lights-textures.vert"),
                          string("shaders/lights-textures.frag"));
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
    shaderVarsToVertexAttribs["vTexCoord"] = "texcoord";
    
    //objects
    for (typename map<string,util::PolygonMesh<VertexAttrib> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++) {
        util::ObjectInstance * obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(shaderLocations,shaderVarsToVertexAttribs,it->second);
        objects[it->first] = obj;
    }

    //textures
    for (typename map<string,util::TextureImage*>::iterator it=textures.begin();
           it!=textures.end();
           it++) {
        GLuint textureId;
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D,textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //if the s-coordinate goes outside (0,1), repeat it
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //if the t-coordinate goes outside (0,1), repeat it
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, it->second->getWidth(),it->second->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,it->second->getImage());
        glGenerateMipmap(GL_TEXTURE_2D);
        textureIds[it->first] = textureId;
        
    } 
    
	int window_width,window_height;
    glfwGetFramebufferSize(window,&window_width,&window_height);

    //prepare the projection matrix for perspective projection
	projection = glm::perspective(glm::radians(60.0f),(float)window_width/window_height,0.1f,10000.0f);
    glViewport(0, 0, window_width,window_height);

    frames = 0;
    time = glfwGetTime();

    renderer = new sgraph::GLScenegraphRenderer(modelview,objects,textureIds,shaderLocations);
    
}




void View::display(sgraph::IScenegraph *scenegraph) {
    
    program.enable();
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT_FACE);

    
    
    modelview.push(glm::mat4(1.0));
    modelview.top() = modelview.top() * glm::lookAt(glm::vec3(0.0f,40.0f,40.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
    //send projection matrix to GPU    
    glUniformMatrix4fv(shaderLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    //get all the lights in view space
    sgraph::LightGatherer * gatherer = new sgraph::LightGatherer(modelview);
    scenegraph->getRoot()->accept(gatherer);
    vector<util::Light> lightsInViewSpace = gatherer->getLightsInViewSpace();

    //send them down to the gpu

    //pass light color properties to shader
    glUniform1i(shaderLocations.getLocation("numLights"),lightsInViewSpace.size());
    

    for (int i = 0; i < lightsInViewSpace.size(); i++) {
        stringstream name;

        name << "light[" << i << "]";
        glUniform3fv(shaderLocations.getLocation(name.str()+".ambient"), 1, glm::value_ptr(lightsInViewSpace[i].getAmbient()));
        glUniform3fv(shaderLocations.getLocation(name.str()+".diffuse"), 1, glm::value_ptr(lightsInViewSpace[i].getDiffuse()));
        glUniform3fv(shaderLocations.getLocation(name.str()+".specular"), 1,glm::value_ptr(lightsInViewSpace[i].getSpecular()));
        glUniform4fv(shaderLocations.getLocation(name.str()+".position"), 1,glm::value_ptr(lightsInViewSpace[i].getPosition()));

        glUniform4fv(shaderLocations.getLocation(name.str()+".spotdirection"), 1,glm::value_ptr(lightsInViewSpace[i].getSpotDirection()));
        glUniform1f(shaderLocations.getLocation(name.str()+".cosSpotCutoff"), glm::cos(glm::radians(lightsInViewSpace[i].getSpotCutoff())));
    }

    //enable texture mapping
    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    //tell the shader to look for GL_TEXTURE"0"
    glUniform1i(shaderLocations.getLocation("image"), 0);


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

    for (typename map<string,GLuint>::iterator it=textureIds.begin();
           it!=textureIds.end();
           it++) { 
        glDeleteTextures(1,&it->second);
    }
    glfwDestroyWindow(window);

    glfwTerminate();
}





