//! [code]

#include <glad/glad.h>
#include "View.h"
#include "Model.h"
#include "Controller.h"

int main(int argc,char *argv[]) {
    Model model;
    View view;
    string sceneFile = "scenegraphmodels/simple.txt";
    if (argc > 1) {
        sceneFile = argv[1];
    }
    Controller controller(model,view,sceneFile);
    controller.run();


}

//! [code]
