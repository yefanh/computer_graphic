#include "Model.h"
#include <GLFW/glfw3.h>


Model::Model() {

}

Model::~Model() {
    if (scenegraph) {
        delete scenegraph;
    }
}





sgraph::IScenegraph *Model::getScenegraph() {
    return this->scenegraph;
}

void Model::setScenegraph(sgraph::IScenegraph *scenegraph) {
    this->scenegraph = scenegraph;
}



