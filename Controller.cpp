#include "Controller.h"
#include "sgraph/IScenegraph.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/ScaleTransform.h"
#include "ObjImporter.h"
#include <GLFW/glfw3.h>
using namespace sgraph;
#include <iostream>
using namespace std;

#include "sgraph/ScenegraphExporter.h"
#include "sgraph/ScenegraphImporter.h"

Controller::Controller(Model& m,View& v, string sceneFile) {
    model = m;
    view = v;

    initScenegraph(sceneFile);
}

void Controller::initScenegraph(string sceneFile) {

     
    
    //read in the file of commands
    ifstream inFile(sceneFile);
    //ifstream inFile("tryout.txt");
    sgraph::ScenegraphImporter importer;
    

    IScenegraph *scenegraph = importer.parse(inFile);
    //scenegraph->setMeshes(meshes);
    model.setScenegraph(scenegraph);
    cout <<"Scenegraph made" << endl;   

}

Controller::~Controller()
{
    
}

void Controller::run()
{
    sgraph::IScenegraph * scenegraph = model.getScenegraph();
    map<string,util::PolygonMesh<VertexAttrib> > meshes = scenegraph->getMeshes();
    map<string,util::TextureImage *> textures = scenegraph->getTextures();
    view.init(this,meshes,textures);
    while (!view.shouldWindowClose()) {
        if (raytraceRequested) {
            view.raytrace(scenegraph);
            raytraceRequested = false;
        } else {
            view.display(scenegraph);
        }
    }
    view.closeWindow();
    exit(EXIT_SUCCESS);
}

void Controller::onkey(int key, int scancode, int action, int mods)
{
    // Only respond to key press, not release
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        cout << (char)key << " pressed" << endl;
        
        // 'S' key triggers ray tracing mode (outputs image instead of screen)
        if (key == 'S' || key == 's') {
            cout << "Switching to ray tracing mode..." << endl;
            raytraceRequested = true;
        }
    }
}

void Controller::reshape(int width, int height) 
{
    //cout <<"Window reshaped to width=" << width << " and height=" << height << endl;
    glViewport(0, 0, width, height);
}

void Controller::dispose()
{
    view.closeWindow();
}

void Controller::error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
