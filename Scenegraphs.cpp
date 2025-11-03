//! [code]

#include <glad/glad.h>
#include "View.h"
#include "Model.h"
#include "Controller.h"

int main(int argc,char *argv[]) {
    Model model;
    View view;
    // Accept commands file from CLI: ./Scenegraphs <path-to-commands.txt>
    std::string commandsFile = (argc > 1) ? std::string(argv[1]) : std::string("");
    Controller controller(model,view,commandsFile);
    controller.run();


}

//! [code]
