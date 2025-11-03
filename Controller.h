#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include "Model.h"
#include "View.h"

class Controller : public Callbacks {
public:
    Controller(Model& m, View& v, const std::string& commandsFilePath);
    ~Controller();

    void run();

    // Callbacks
    void onkey(int key, int scancode, int action, int mods) override;
    void reshape(int width, int height) override;

    void dispose();
    static void error_callback(int error, const char* description);

private:
    void initScenegraph();

    View&  view;
    Model& model;
    std::string commandsPath;

    // Track Shift state so repeats never lose the modifier
    bool shiftDown = false;
    int tickCount = 0;
};

#endif // CONTROLLER_H
