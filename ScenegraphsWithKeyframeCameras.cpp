//! [code]

#include <glad/glad.h>
#include "View.h"
#include "Model.h"
#include "Controller.h"
#include "ObjImporter.h"
#include "ObjExporter.h"
#include "PolygonMesh.h"

int main(int argc,char *argv[]) {

    Model model;
    View view;
    Controller controller(model,view);
    controller.run();

}

//! [code]
