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

/**
 * @brief Controller constructor - initializes scene graph.
 * 
 * @param m Reference to Model (holds scene graph data)
 * @param v Reference to View (manages rendering)
 * @param commandsFilePath Path to scene graph commands file (Assignment 5 requirement 3.1)
 */
Controller::Controller(Model& m, View& v, const std::string& commandsFilePath)
    : view(v), model(m), commandsPath(commandsFilePath)
{
    initScenegraph();
}

/**
 * @brief Load and parse scene graph from commands file.
 * 
 * Supports command-line file specification (Assignment 5 requirement 3.1).
 * Defaults to "code/hogwarts-plane.txt" if no file specified.
 */
void Controller::initScenegraph() {
    // Use CLI-provided file if available, otherwise use default
    std::string path = commandsPath.empty()
        ? std::string("code/hogwarts-plane.txt")
        : commandsPath;

    std::ifstream inFile(path);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open commands file: " << path << "\n";
        throw std::runtime_error("Cannot open commands file");
    }

    // Parse scene graph from file
    sgraph::ScenegraphImporter importer;
    IScenegraph *scenegraph = importer.parse(inFile);
    model.setScenegraph(scenegraph);

    // Print scene graph structure for debugging
    std::cout << "Scenegraph made from: " << path << std::endl;
    if (scenegraph && scenegraph->getRoot()) {
        std::cout << "scenegraph (final):" << std::endl;
        sgraph::ScenegraphPrinter printer(std::cout);
        scenegraph->getRoot()->accept(&printer);
    }
}

Controller::~Controller() {}

/**
 * @brief Main rendering loop with animation tick counter.
 * 
 * Implements tick-based animation (Assignment 5 requirement 2.3):
 * - tickCount increments each frame
 * - Passed to View::display() → renderer → animation nodes
 * - Wraps at INT_MAX to prevent overflow
 */
void Controller::run() {
    IScenegraph *scenegraph = model.getScenegraph();
    auto meshes = scenegraph->getMeshes();
    view.init(this, meshes);

    // Main render loop
    while (!view.shouldWindowClose()) {
        // Render frame with current tick for animation
        view.display(scenegraph, tickCount);
        // Increment tick with wraparound to prevent overflow
        tickCount = (tickCount == std::numeric_limits<int>::max()) ? 0 : tickCount + 1;
    }
    view.closeWindow();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Keyboard event handler for camera control and navigation.
 * 
 * Camera Switching (Assignment 5 requirement 2.5):
 * - Key 1: Stationary camera
 * - Key 2: Free-fly camera
 * - Key 3: Chopper (circular orbit) camera
 * - Key 4: Plane (first-person) camera
 * 
 * Free-fly Camera Controls (Assignment 4.2):
 * - Arrow keys: Strafe left/right, move up/down
 * - Shift + arrows: Look left/right (yaw), look up/down (pitch)
 * - F/B keys: Move forward/backward
 * 
 * @param key GLFW key code
 * @param scancode System-specific scancode
 * @param action GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT
 * @param mods Modifier key flags (Shift, Ctrl, Alt)
 */
void Controller::onkey(int key, int scancode, int action, int mods)
{
    // Track Shift key state manually for reliable repeat behavior
    if (key==GLFW_KEY_LEFT_SHIFT || key==GLFW_KEY_RIGHT_SHIFT) {
        if (action==GLFW_PRESS)   shiftDown = true;
        if (action==GLFW_RELEASE) shiftDown = false;
        return;
    }
    // Only handle press and repeat events
    if (!(action==GLFW_PRESS || action==GLFW_REPEAT)) return;

    const float moveStep  = 10.0f;               // Translation distance per key press
    const float angleStep = glm::radians(10.0f); // Rotation angle per key press
    bool shift = shiftDown || (mods & GLFW_MOD_SHIFT);

    switch (key) {
        // ========== Camera Mode Selection (Assignment 5 requirement 2.5) ==========
        
        case GLFW_KEY_1:
            view.setCameraStationary();
            break;

        case GLFW_KEY_2:
            view.setCameraFreeFly();
            std::cout << "[Camera] Free-fly: arrows=strafe/up/down, "
                         "SHIFT+arrows=look, F/B=forward/back\n";
            break;

        case GLFW_KEY_3:
            view.setCameraChopper();
            std::cout << "[Camera] Chopper view: circling the castle.\n";
            break;

        case GLFW_KEY_4:
            view.setCameraPlane();
            std::cout << "[Camera] Plane view: flying with the plane!\n";
            break;

        // ========== Free-fly Camera Navigation ==========
        // WITHOUT SHIFT: translate camera position
        // WITH SHIFT: rotate camera (look around)
        
        case GLFW_KEY_LEFT:
            if (shift) {
                view.yawPitch(-angleStep, 0.0f);        // Look left (yaw)
                view.debugPrintCamera("Shift+Left (yaw left)");
            } else {
                view.moveLocal(-moveStep, 0.0f, 0.0f);  // Strafe left
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
