#include "Controller.h"
#include "sgraph/IScenegraph.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/ScaleTransform.h"
#include "ObjImporter.h"
using namespace sgraph;
#include <iostream>
using namespace std;

#include "sgraph/ScenegraphExporter.h"
#include "sgraph/ScenegraphImporter.h"

Controller::Controller(Model& m,View& v) {
    model = m;
    view = v;

    initScenegraph();
}

void Controller::initScenegraph() {
    //read in the file of commands
    // Use the Hogwarts scene with the airplane as the main A6 scene
    ifstream inFile("scenegraphmodels/hogwarts-with-plane.txt");
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
    map<string,string> texturePaths = scenegraph->getTexturePaths();
    view.init(this,meshes,texturePaths);
    while (!view.shouldWindowClose()) {
        view.display(scenegraph);
    }
    view.closeWindow();
    exit(EXIT_SUCCESS);
}

void Controller::onkey(int key, int scancode, int action, int mods)
{
    cout << (char)key << " pressed" << endl;
    switch(key) {
        case GLFW_KEY_1:
            view.setCamera(STATIONARY);
            break;
        case GLFW_KEY_2:
            view.setCamera(KEYBOARD);
            break;
        case GLFW_KEY_3:
            view.setCamera(CHOPPER);
            break;
        case GLFW_KEY_4:
            view.setCamera(FPS);
            break;
        case GLFW_KEY_S:
            view.toggleShading();
            break;
        case GLFW_KEY_LEFT:
            if (mods & GLFW_MOD_SHIFT) {
                cout << "Turn left" << endl;
                view.turnLeft();
            }
            else {
                cout << "Move left" << endl;
                view.moveLeft();
            }
            break;
        case GLFW_KEY_RIGHT:
            if (mods & GLFW_MOD_SHIFT) {
                cout << "Turn right" << endl;
                view.turnRight();
            }
            else {
                cout << "Move right" << endl;
                view.moveRight();
            }
            break;
        case GLFW_KEY_UP:
            if (mods & GLFW_MOD_SHIFT) {
                cout << "Turn up" << endl;
                view.turnUp();
            }
            else {
                cout << "Move up" << endl;
                view.moveUp();
            }
            break;
        case GLFW_KEY_DOWN:
            if (mods & GLFW_MOD_SHIFT) {
                cout << "Turn down" << endl;
                view.turnDown();
            }
            else {
                cout << "Move down" << endl;
                view.moveDown();
            }
            break;
        case GLFW_KEY_F:
            cout << "Move forward" << endl;
            view.moveForward();
            break;
        case GLFW_KEY_B:
            cout << "Move backward" << endl;
            view.moveBack();
            break;


    }
}

void Controller::reshape(int width, int height) 
{
    cout <<"Window reshaped to width=" << width << " and height=" << height << endl;
    view.resize(width,height);
}

void Controller::dispose()
{
    view.closeWindow();
}

void Controller::error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
