#include "View.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 只在实现文件里包含工程依赖
#include "ShaderProgram.h"
#include "ShaderLocationsVault.h"
#include "ObjectInstance.h"
#include "sgraph/GLScenegraphRenderer.h"   // will indirectly include the MatrixStack definition
#include "VertexAttrib.h"

View::View() { camMode = Stationary; }
View::~View(){}

void View::init(Callbacks *callbacks,
                std::map<std::string, util::PolygonMesh<VertexAttrib>>& meshes)
{
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 800, "Hello GLFW: Per-vertex coloring", NULL, NULL);
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
    glfwSetWindowUserPointer(window, (void *)callbacks);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetKeyCallback(window,
    [](GLFWwindow* w, int key, int sc, int action, int mods)
    {
        if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS ||
            glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            mods |= GLFW_MOD_SHIFT;
        }
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(w))
            ->onkey(key,sc,action,mods);
    });
    glfwSetWindowSizeCallback(window,
    [](GLFWwindow* w, int width,int height)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(w))->reshape(width,height);
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // Shader program
    program = new util::ShaderProgram();
    program->createProgram("shaders/default.vert","shaders/default.frag");
    program->enable();

    auto vault = program->getAllShaderVariables();
    shaderLocations = new util::ShaderLocationsVault(vault);

    // Mesh attribute mapping
    std::map<std::string, std::string> shaderVarsToVertexAttribs;
    shaderVarsToVertexAttribs["vPosition"] = "position";

    for (auto &kv : meshes) {
        auto *obj = new util::ObjectInstance(kv.first);
        obj->initPolygonMesh(*shaderLocations, shaderVarsToVertexAttribs, kv.second);
        objects[kv.first] = obj;
    }

    // Matrix stack
    while (!modelview.empty()) modelview.pop();

    int ww, hh; glfwGetFramebufferSize(window,&ww,&hh);
    onResize(ww,hh);

    frames = 0;
    time = glfwGetTime();

    renderer = new sgraph::GLScenegraphRenderer(modelview, objects, *shaderLocations);

    // Free camera starts from a fixed camera position/orientation
    camPos        = initEye;
    camForward    = glm::normalize(initCenter - initEye);
    camUp         = initUp;
    pitchAccumRad = 0.0f;
    normalizeCameraBasis();
}

void View::display(sgraph::IScenegraph *scenegraph) {
    program->enable();
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    modelview.push(glm::mat4(1.0));
    glm::mat4 viewMat =
        (camMode==Stationary)
        ? glm::lookAt(initEye, initCenter, initUp)
        : glm::lookAt(camPos, camPos+camForward, camUp);

    modelview.top() = modelview.top() * viewMat;

    glUniformMatrix4fv(shaderLocations->getLocation("projection"),
                       1, GL_FALSE, glm::value_ptr(projection));

    scenegraph->getRoot()->accept(renderer);

    modelview.pop();
    glFlush();
    program->disable();

    glfwSwapBuffers(window);
    glfwPollEvents();
    frames++;
    double t = glfwGetTime();
    if ((t-time)>1.0) {
        printf("Framerate: %2.0f\r",frames/(t-time));
        frames = 0;
        time = t;
    }
}

bool View::shouldWindowClose() { return glfwWindowShouldClose(window); }

void View::closeWindow() {
    for (auto &kv : objects) { kv.second->cleanup(); delete kv.second; }
    objects.clear();

    delete renderer;        renderer = nullptr;
    while (!modelview.empty()) modelview.pop();
    delete shaderLocations; shaderLocations = nullptr;
    delete program;         program = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();
}

void View::onResize(int w, int h) {
    if (h<=0) h = 1;
    glViewport(0, 0, w,h);
    projection = glm::perspective(glm::radians(60.0f), (float)w/h, 0.1f, 10000.0f);
}

void View::setCameraStationary() { camMode = Stationary; }

void View::setCameraFreeFly() {
    camMode      = Free;
    camPos       = initEye;
    camForward   = glm::normalize(initCenter - initEye);
    camUp        = initUp;
    pitchAccumRad = 0.0f;
    normalizeCameraBasis();
}

void View::moveLocal(float rightDelta, float upDelta, float forwardDelta) {
    if (camMode!=Free) return;
    camPos += camRight * rightDelta + camUp * upDelta + camForward * forwardDelta;
}

void View::yawPitch(float yawRad, float pitchRad) {
    if (camMode!=Free) return;

    // --- yaw ---
    if (yawRad != 0.0f) {
        glm::mat4 Ryaw = glm::rotate(glm::mat4(1.0f), yawRad, camUp);
        camForward = glm::normalize(glm::vec3(Ryaw * glm::vec4(camForward, 0.0f)));
        camRight   = glm::normalize(glm::vec3(Ryaw * glm::vec4(camRight,   0.0f)));
    }

    // --- pitch ---
    if (pitchRad != 0.0f) {
        float newPitch = glm::clamp(pitchAccumRad + pitchRad, -kMaxPitchRad, +kMaxPitchRad);
        float apply    = newPitch - pitchAccumRad;
        if (apply != 0.0f) {
            glm::mat4 Rpitch = glm::rotate(glm::mat4(1.0f), apply, camRight);
            camForward = glm::normalize(glm::vec3(Rpitch * glm::vec4(camForward, 0.0f)));
            camUp      = glm::normalize(glm::vec3(Rpitch * glm::vec4(camUp,      0.0f)));
            pitchAccumRad = newPitch;
        }
    }

    normalizeCameraBasis();
}

void View::normalizeCameraBasis() {
    camForward = glm::normalize(camForward);

    glm::vec3 right = glm::cross(camForward, camUp);
    if (glm::dot(right, right) < 1e-6f) {
        // Forward nearly parallel to up, snap up to a safe axis then rebuild
        glm::vec3 fallbackUp = std::abs(camForward.y) < 0.9f
            ? glm::vec3(0.0f, 1.0f, 0.0f)
            : glm::vec3(1.0f, 0.0f, 0.0f);
        right = glm::cross(camForward, fallbackUp);
    }

    camRight = glm::normalize(right);
    camUp    = glm::normalize(glm::cross(camRight, camForward));
}

void View::debugPrintCamera(const char* tag) const {
    std::cout << "[CameraDebug] " << tag
              << " | pos=(" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")"
              << " forward=(" << camForward.x << ", " << camForward.y << ", " << camForward.z << ")"
              << std::endl;
}
