//! [code]

#include <glad/glad.h>
#include "View.h"
#include "Model.h"
#include "Controller.h"

int main(int argc,char *argv[]) {
    Model model;
    View view;
    Controller controller(model,view);
    controller.run();


}

//! [code]
