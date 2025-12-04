#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "View.h"
#include "Model.h"
#include "Callbacks.h"

class Controller: public Callbacks
{
public:
    Controller(Model& m,View& v);
    ~Controller();
    void run();

    virtual void reshape(int width, int height);
    virtual void dispose();
    virtual void onkey(int key, int scancode, int action, int mods);
    virtual void error_callback(int error, const char* description);
private:
    void initScenegraph();

    View view;
    Model model;
};

#endif