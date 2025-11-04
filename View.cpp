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
#include "sgraph/KeyframeAnimationNode.h"
#include "sgraph/SGNode.h"
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

void View::display(sgraph::IScenegraph *scenegraph, int tick) {
    program->enable();
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    modelview.push(glm::mat4(1.0));
    glm::mat4 viewMat;
    
    if (camMode == Stationary) {
        viewMat = glm::lookAt(initEye, initCenter, initUp);
    } else if (camMode == Free) {
        viewMat = glm::lookAt(camPos, camPos+camForward, camUp);
    } else if (camMode == Chopper) {
        const glm::vec3 targetCenter = initCenter;
        const glm::vec3 orbitCenter = targetCenter + glm::vec3(60.0f, 0.0f, -40.0f);
        const float orbitRadius = 90.0f;
        const float hoverHeight = 120.0f;
        const float angularSpeed = 0.01f;

        float angle = static_cast<float>(tick) * angularSpeed;
        glm::vec3 offset(std::cos(angle) * orbitRadius, 0.0f, std::sin(angle) * orbitRadius);
        glm::vec3 eye = orbitCenter + offset;
        eye.y = targetCenter.y + hoverHeight + std::sin(angle * 2.5f) * 5.0f;

        glm::vec3 forward = targetCenter - eye;
        if (glm::dot(forward, forward) < 1e-5f) {
            forward = glm::vec3(0.0f, 0.0f, -1.0f);
        } else {
            forward = glm::normalize(forward);
        }

        glm::vec3 up(0.0f, 1.0f, 0.0f);
        if (std::abs(glm::dot(forward, up)) > 0.95f) {
            up = glm::vec3(0.0f, 0.0f, 1.0f);
            if (std::abs(glm::dot(forward, up)) > 0.95f) {
                up = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }

        glm::vec3 right = glm::cross(forward, up);
        if (glm::dot(right, right) < 1e-5f) {
            right = glm::vec3(1.0f, 0.0f, 0.0f);
        } else {
            right = glm::normalize(right);
        }
        glm::vec3 correctedUp = glm::normalize(glm::cross(right, forward));

        viewMat = glm::lookAt(eye, targetCenter, correctedUp);
    } else if (camMode == Plane) {
        // Get plane animation node from scenegraph
        sgraph::SGNode* planeAnimNode = scenegraph->getRoot()->getNode("plane-anim");
        if (planeAnimNode) {
            // Cast to KeyframeAnimationNode to access transform data
            sgraph::KeyframeAnimationNode* keyframeNode = 
                dynamic_cast<sgraph::KeyframeAnimationNode*>(planeAnimNode);
            
            if (keyframeNode && keyframeNode->getKeyframeCount() > 1) {
                size_t count = keyframeNode->getKeyframeCount();
                size_t index = static_cast<size_t>(tick) % count;
                size_t nextIndex = (index + 1) % count;
                size_t prevIndex = (index + count - 1) % count;

                const auto& positions = keyframeNode->getPositions();
                const auto& upVectors = keyframeNode->getUpVectors();

                if (index < positions.size() && nextIndex < positions.size() && prevIndex < positions.size() && index < upVectors.size()) {
                    glm::vec3 planePos = positions[index];

                    glm::vec3 front = positions[nextIndex] - planePos;
                    if (glm::dot(front, front) < 1e-5f) {
                        front = planePos - positions[prevIndex];
                    }
                    if (glm::dot(front, front) < 1e-5f) {
                        front = glm::vec3(0.0f, 0.0f, -1.0f);
                    }
                    front = glm::normalize(front);

                    glm::vec3 up = upVectors[index];
                    if (glm::dot(up, up) < 1e-5f) {
                        up = glm::vec3(0.0f, 1.0f, 0.0f);
                    } else {
                        up = glm::normalize(up);
                    }

                    if (std::abs(glm::dot(front, up)) > 0.95f) {
                        glm::vec3 fallbackUp(0.0f, 1.0f, 0.0f);
                        if (std::abs(glm::dot(front, fallbackUp)) > 0.95f) {
                            fallbackUp = glm::vec3(1.0f, 0.0f, 0.0f);
                        }
                        up = glm::normalize(glm::cross(glm::cross(front, fallbackUp), front));
                    }

                    const float cockpitForwardOffset = 12.0f;
                    const float cockpitUpOffset = 4.0f;
                    const float lookAheadDistance = 80.0f;

                    glm::vec3 camEye = planePos + front * cockpitForwardOffset + up * cockpitUpOffset;
                    glm::vec3 camCenter = camEye + front * lookAheadDistance;
                    glm::vec3 correctedUp = glm::normalize(glm::cross(glm::cross(front, up), front));

                    viewMat = glm::lookAt(camEye, camCenter, correctedUp);
                } else {
                    // Fallback if indices are out of bounds
                    viewMat = glm::lookAt(initEye, initCenter, initUp);
                }
            } else {
                // Fallback to stationary camera if plane node not found or has not enough keyframes
                viewMat = glm::lookAt(initEye, initCenter, initUp);
            }
        } else {
            // Fallback to stationary camera if plane node not found
            viewMat = glm::lookAt(initEye, initCenter, initUp);
        }
    }

    modelview.top() = modelview.top() * viewMat;

    glUniformMatrix4fv(shaderLocations->getLocation("projection"),
                       1, GL_FALSE, glm::value_ptr(projection));

    if (scenegraph && scenegraph->getRoot() && renderer) {
        renderer->setTick(tick);
        scenegraph->getRoot()->accept(renderer);
    }

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

void View::setCameraChopper() {
    camMode = Chopper;
}

void View::setCameraPlane() {
    camMode = Plane;
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
