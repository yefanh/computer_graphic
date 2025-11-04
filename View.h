#ifndef VIEW_H
#define VIEW_H

#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <stack>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//---- Forward declaration to avoid including project-private dependencies in header files ----
namespace util {
    template<typename T> class MatrixStack;
    class ShaderProgram;
    class ShaderLocationsVault;
    class ObjectInstance;
    template<typename VT> class PolygonMesh;
}
namespace sgraph {
    class IScenegraph;
    class GLScenegraphRenderer;
}
class VertexAttrib;

class Callbacks {
public:
    virtual void onkey(int key, int scancode, int action, int mods) = 0;
    virtual void reshape(int width, int height) = 0;
    virtual ~Callbacks() = default;
};

class View {
public:
    View();
    ~View();

    void init(Callbacks *callbacks,
              std::map<std::string, util::PolygonMesh<VertexAttrib>>& meshes);
    void display(sgraph::IScenegraph *scenegraph, int tick);
    bool shouldWindowClose();
    void closeWindow();
    void onResize(int width, int height);

    // ------------ camera control ------------
    void setCameraStationary();
    void setCameraFreeFly();
    void setCameraChopper();
    void setCameraPlane();
    void moveLocal(float rightDelta, float upDelta, float forwardDelta);
    void yawPitch(float yawRad, float pitchRad);
    void debugPrintCamera(const char* tag) const;

private:
    // ------------ window / GL objects ------------
    GLFWwindow *window = nullptr;

    // These objects are included in View.cpp and created with new
    util::ShaderProgram       *program = nullptr;
    util::ShaderLocationsVault*shaderLocations = nullptr;
    std::map<std::string, util::ObjectInstance*> objects;

    // Matrix stack for modelview
    std::stack<glm::mat4> modelview;
    sgraph::GLScenegraphRenderer *renderer  = nullptr;

    glm::mat4 projection{1.0f};
    int   frames = 0;
    double time = 0.0;

    // ------------ camera control ------------
    enum CamMode { Stationary, Free, Chopper, Plane };
    CamMode camMode = Stationary;

    // Fixed camera (Assignment 4.1)
    glm::vec3 initEye    = glm::vec3(180.0f, 120.0f, 260.0f);
    glm::vec3 initCenter = glm::vec3( 80.0f,  20.0f,  20.0f);
    glm::vec3 initUp     = glm::vec3(  0.0f,   1.0f,   0.0f);

    // Free camera (Assignment 4.2)
    glm::vec3 camPos     = initEye;
    glm::vec3 camForward = glm::normalize(initCenter - initEye);
    glm::vec3 camUp      = initUp;
    glm::vec3 camRight   = glm::normalize(glm::cross(camForward, camUp));

    // Pitch accumulation (limit ±89°, avoid flipping)
    float pitchAccumRad = 0.0f;
    const float kMaxPitchRad = glm::radians(89.0f);

    void normalizeCameraBasis();
};

#endif // VIEW_H
