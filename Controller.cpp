#include "Controller.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <limits>

#include <glm/glm.hpp>           // for glm::radians
#include <GLFW/glfw3.h>

#include "sgraph/IScenegraph.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/ScaleTransform.h"
#include "sgraph/ScenegraphExporter.h"
#include "sgraph/ScenegraphImporter.h"
#include "sgraph/ScenegraphPrinter.h"

using namespace sgraph;

Controller::Controller(Model& m, View& v, const std::string& commandsFilePath)
    : view(v), model(m), commandsPath(commandsFilePath)
{
    initScenegraph();
}

void Controller::initScenegraph() {
    // Prefer CLI-provided file; else default
    std::string path = commandsPath.empty()
        ? std::string("code/hogwarts.txt")
        : commandsPath;

    std::ifstream inFile(path);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open commands file: " << path << "\n";
        throw std::runtime_error("Cannot open commands file");
    }

    sgraph::ScenegraphImporter importer;
    IScenegraph *scenegraph = importer.parse(inFile);
    model.setScenegraph(scenegraph);

    std::cout << "Scenegraph made from: " << path << std::endl;
    if (scenegraph && scenegraph->getRoot()) {
        std::cout << "scenegraph (final):" << std::endl;
        sgraph::ScenegraphPrinter printer(std::cout);
        scenegraph->getRoot()->accept(&printer);
    }
}

Controller::~Controller() {}

void Controller::run() {
    IScenegraph *scenegraph = model.getScenegraph();
    auto meshes = scenegraph->getMeshes();
    view.init(this, meshes);

    while (!view.shouldWindowClose()) {
        view.display(scenegraph, tickCount);
        tickCount = (tickCount == std::numeric_limits<int>::max()) ? 0 : tickCount + 1;
    }
    view.closeWindow();
    exit(EXIT_SUCCESS);
}

void Controller::onkey(int key, int scancode, int action, int mods)
{
    // Keep Shift state ourselves, so key repeats never lose it
    if (key==GLFW_KEY_LEFT_SHIFT || key==GLFW_KEY_RIGHT_SHIFT) {
        if (action==GLFW_PRESS)   shiftDown = true;
        if (action==GLFW_RELEASE) shiftDown = false;
        return;
    }
    if (!(action==GLFW_PRESS || action==GLFW_REPEAT)) return;

    const float moveStep  = 10.0f;
    const float angleStep = glm::radians(10.0f); // visually clear per-hit rotation
    bool shift = shiftDown || (mods & GLFW_MOD_SHIFT);

    switch (key) {
        case GLFW_KEY_1:
            view.setCameraStationary();
            break;

        case GLFW_KEY_2:
            view.setCameraFreeFly();
            std::cout << "[Camera] Free-fly: arrows=strafe/up/down, "
                         "SHIFT+arrows=look, F/B=forward/back\n";
            break;

        // --- WITHOUT SHIFT: translate; WITH SHIFT: rotate in place ---
        case GLFW_KEY_LEFT:
            if (shift) {
                view.yawPitch(-angleStep, 0.0f);        // look left (in place)
                view.debugPrintCamera("Shift+Left (yaw left)");
            } else {
                view.moveLocal(-moveStep, 0.0f, 0.0f);  // strafe left
                view.debugPrintCamera("Left (strafe left)");
            }
            break;

        case GLFW_KEY_RIGHT:
            if (shift) {
                view.yawPitch(+angleStep, 0.0f);        // look right (in place)
                view.debugPrintCamera("Shift+Right (yaw right)");
            } else {
                view.moveLocal(+moveStep, 0.0f, 0.0f);  // strafe right
                view.debugPrintCamera("Right (strafe right)");
            }
            break;

        case GLFW_KEY_UP:
            if (shift) {
                view.yawPitch(0.0f, -angleStep);        // look up (in place)
                view.debugPrintCamera("Shift+Up (pitch up)");
            } else {
                view.moveLocal(0.0f, +moveStep, 0.0f);  // move up (no gaze change)
                view.debugPrintCamera("Up (move up)");
            }
            break;

        case GLFW_KEY_DOWN:
            if (shift) {
                view.yawPitch(0.0f, +angleStep);        // look down (in place)
                view.debugPrintCamera("Shift+Down (pitch down)");
            } else {
                view.moveLocal(0.0f, -moveStep, 0.0f);  // move down (no gaze change)
                view.debugPrintCamera("Down (move down)");
            }
            break;

        case GLFW_KEY_F:
            if (!shift) {
                view.moveLocal(0.0f, 0.0f, +moveStep); // forward
                view.debugPrintCamera("F (move forward)");
            }
            break;

        case GLFW_KEY_B:
            if (!shift) {
                view.moveLocal(0.0f, 0.0f, -moveStep); // backward
                view.debugPrintCamera("B (move backward)");
            }
            break;

        default:
            break;
    }
}

void Controller::reshape(int width, int height) {
    std::cout << "Window reshaped to width=" << width
              << " and height=" << height << std::endl;
    view.onResize(width, height);
}

void Controller::dispose() {
    view.closeWindow();
}

void Controller::error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}
