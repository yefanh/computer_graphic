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

/**
 * @brief Callback interface for handling window events.
 * 
 * Implemented by Controller to handle keyboard input and window resize events.
 */
class Callbacks {
public:
    virtual void onkey(int key, int scancode, int action, int mods) = 0;
    virtual void reshape(int width, int height) = 0;
    virtual ~Callbacks() = default;
};

/**
 * @brief Main view/rendering class for Assignment 5.
 * 
 * Manages OpenGL window, rendering pipeline, and camera system.
 * Supports 4 camera modes as required by Assignment 5:
 * 
 * Camera Modes (Assignment 5 requirement 2.5):
 * 1. Stationary (key 1): Fixed overview camera from Assignment 4
 * 2. Free-fly (key 2): Keyboard-controlled camera from Assignment 4
 * 3. Chopper (key 3): Circular orbit camera with offset center
 * 4. Plane (key 4): First-person view from plane cockpit
 * 
 * Rendering Features:
 * - Perspective projection with 60° FOV (requirement 3.3)
 * - Aspect ratio preservation on resize (requirement 3.4)
 * - Tick-based animation rendering (requirement 2.3)
 * 
 * @see Controller for keyboard handling
 * @see GLScenegraphRenderer for scene graph traversal
 */
class View {
public:
    View();
    ~View();

    /**
     * @brief Initialize OpenGL window and rendering context.
     * @param callbacks Pointer to callback handler (usually Controller)
     * @param meshes Map of mesh name → polygon mesh data
     */
    void init(Callbacks *callbacks,
              std::map<std::string, util::PolygonMesh<VertexAttrib>>& meshes);
    
    /**
     * @brief Render one frame of the scene.
     * @param scenegraph Scene graph to render
     * @param tick Current animation tick for keyframe animation
     */
    void display(sgraph::IScenegraph *scenegraph, int tick);
    
    bool shouldWindowClose();
    void closeWindow();
    
    /**
     * @brief Handle window resize event.
     * 
     * Updates viewport and recalculates projection matrix with new aspect ratio
     * to prevent stretching/squeezing (Assignment 5 requirement 3.4).
     * 
     * @param width New window width in pixels
     * @param height New window height in pixels
     */
    void onResize(int width, int height);

    // ------------ Camera Control (Assignment 5 requirement 2.5) ------------
    
    /**
     * @brief Switch to stationary camera (key 1).
     * Fixed overview position from Assignment 4.
     */
    void setCameraStationary();
    
    /**
     * @brief Switch to free-fly camera (key 2).
     * Keyboard-controlled camera from Assignment 4. Resets to initial position.
     */
    void setCameraFreeFly();
    
    /**
     * @brief Switch to chopper camera (key 3).
     * Circular orbit around castle with offset center and slight altitude variation.
     */
    void setCameraChopper();
    
    /**
     * @brief Switch to plane camera (key 4).
     * First-person view from plane cockpit (Assignment 5 requirement 2.4).
     */
    void setCameraPlane();
    
    /**
     * @brief Move camera in local coordinate system (Free-fly mode only).
     * @param rightDelta Movement along camera's right axis
     * @param upDelta Movement along camera's up axis
     * @param forwardDelta Movement along camera's forward axis
     */
    void moveLocal(float rightDelta, float upDelta, float forwardDelta);
    
    /**
     * @brief Rotate camera (Free-fly mode only).
     * @param yawRad Rotation around up axis (radians, positive = right)
     * @param pitchRad Rotation around right axis (radians, positive = down, clamped to ±89°)
     */
    void yawPitch(float yawRad, float pitchRad);
    
    /**
     * @brief Print camera state to console (debug utility).
     * @param tag Label to identify this debug print
     */
    void debugPrintCamera(const char* tag) const;

private:
    // ------------ Window / GL Objects ------------
    GLFWwindow *window = nullptr;

    // These objects are created in View.cpp and managed with new/delete
    util::ShaderProgram       *program = nullptr;
    util::ShaderLocationsVault*shaderLocations = nullptr;
    std::map<std::string, util::ObjectInstance*> objects;

    // Matrix stack for modelview transformations
    std::stack<glm::mat4> modelview;
    sgraph::GLScenegraphRenderer *renderer  = nullptr;

    glm::mat4 projection{1.0f};  ///< Perspective projection matrix
    int   frames = 0;             ///< Frame counter for FPS calculation
    double time = 0.0;            ///< Last FPS update time

    // ------------ Camera System (Assignment 5 requirement 2.5) ------------
    
    /**
     * @brief Camera mode enumeration.
     * 
     * Stationary: Fixed position/orientation (Assignment 4.1)
     * Free: User-controlled with keyboard (Assignment 4.2)
     * Chopper: Circular orbit around scene (Assignment 5 requirement 2.5 #3)
     * Plane: First-person from plane cockpit (Assignment 5 requirement 2.4)
     */
    enum CamMode { Stationary, Free, Chopper, Plane };
    CamMode camMode = Stationary;

    // Fixed camera parameters (Assignment 4.1, used by Stationary and Chopper modes)
    glm::vec3 initEye    = glm::vec3(180.0f, 120.0f, 260.0f);  ///< Stationary camera position
    glm::vec3 initCenter = glm::vec3( 80.0f,  20.0f,  20.0f);  ///< Look-at point for stationary
    glm::vec3 initUp     = glm::vec3(  0.0f,   1.0f,   0.0f);  ///< Up vector

    // Free-fly camera state (Assignment 4.2)
    glm::vec3 camPos     = initEye;                                  ///< Current camera position
    glm::vec3 camForward = glm::normalize(initCenter - initEye);     ///< Forward direction (normalized)
    glm::vec3 camUp      = initUp;                                   ///< Up direction (normalized)
    glm::vec3 camRight   = glm::normalize(glm::cross(camForward, camUp)); ///< Right direction (normalized)

    // Pitch accumulation for free-fly camera (limits to ±89° to prevent gimbal lock)
    float pitchAccumRad = 0.0f;                      ///< Accumulated pitch angle in radians
    const float kMaxPitchRad = glm::radians(89.0f);  ///< Maximum pitch (avoids flipping)

    /**
     * @brief Ensure camera basis vectors remain orthonormal.
     * 
     * Re-orthogonalizes the forward/up/right basis using Gram-Schmidt.
     * Called after camera rotations in Free-fly mode.
     */
    void normalizeCameraBasis();
};

#endif // VIEW_H
