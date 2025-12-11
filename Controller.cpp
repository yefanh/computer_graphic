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
        // '2' key toggles camera control mode
        else if (key == '2') {
            cameraControlMode = !cameraControlMode;
            if (cameraControlMode) {
                cout << "Camera control mode ON. Use arrow keys to rotate camera." << endl;
                cout << "  Up/Down: Vertical angle, Left/Right: Horizontal angle" << endl;
                cout << "  Press '2' again to exit." << endl;
            } else {
                cout << "Camera control mode OFF." << endl;
            }
        }
        
        // F/B keys move camera forward/backward along gaze direction (independent of control mode)
        if (key == 'F' || key == 'f') {
            float step = 2.0f;
            cameraDistance = view.moveCameraAlongGaze(step);
        }
        else if (key == 'B' || key == 'b') {
            float step = -2.0f;
            cameraDistance = view.moveCameraAlongGaze(step);
        }

        // Arrow keys control camera when in camera control mode
        if (cameraControlMode) {
            bool updated = false;
            if (key == GLFW_KEY_LEFT) {
                cameraAngleH -= 5.0f;
                updated = true;
            } else if (key == GLFW_KEY_RIGHT) {
                cameraAngleH += 5.0f;
                updated = true;
            } else if (key == GLFW_KEY_UP) {
                cameraAngleV += 5.0f;
                if (cameraAngleV > 89.0f) cameraAngleV = 89.0f;
                updated = true;
            } else if (key == GLFW_KEY_DOWN) {
                cameraAngleV -= 5.0f;
                if (cameraAngleV < 1.0f) cameraAngleV = 1.0f;
                updated = true;
            }
            
            if (updated) {
                cout << "Camera: H=" << cameraAngleH << "° V=" << cameraAngleV << "°" << endl;
                view.setCameraAngles(cameraAngleH, cameraAngleV, cameraDistance);
            }
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
