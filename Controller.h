#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "View.h"
#include "Model.h"
#include "Callbacks.h"

class Controller: public Callbacks
{
public:
    Controller(Model& m,View& v, string sceneFile);
    ~Controller();
    void run();

    virtual void reshape(int width, int height);
    virtual void dispose();
    virtual void onkey(int key, int scancode, int action, int mods);
    virtual void error_callback(int error, const char* description);
private:
    void initScenegraph(string sceneFile);

    View view;
    Model model;
    bool raytraceRequested = false;
    bool cameraControlMode = false;
    float cameraAngleH = 0.0f;  // Horizontal angle (left/right)
    float cameraAngleV = 40.0f; // Vertical angle (up/down)
    float cameraDistance = 56.57f; // Distance from origin
};

#endif
