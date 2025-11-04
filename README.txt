Assignment 5 – Scene Graph Animation & Multiple Cameras
========================================================

Prerequisites
-------------
This assignment is self-contained with all required dependencies included:
    A5/
    ├── include/        (glad, utility headers: ShaderProgram, PolygonMesh, etc.)
    ├── lib/            (libglad.a)
    ├── sgraph/         (scene graph implementation)
    ├── code/           (scene files and keyframe data)
    └── ...

External dependencies (GLFW, GLM) are assumed to be installed system-wide
(e.g., via Homebrew on macOS). The Makefile will automatically find them
in /opt/homebrew/include and /opt/homebrew/lib.

Build & Run
-----------
1. `cd /path/to/A5`
2. `make clean && make`
3. `./Scenegraphs code/hogwarts-plane.txt`
     (argument optional; defaults to hogwarts-plane.txt)

Features Implemented
--------------------

✅ 2.1 Keyframe Animation (KeyframeAnimationNode)
   - Implemented `sgraph/KeyframeAnimationNode.*` extending AnimationNode
   - Loads keyframe data from file (position + up vector per frame)
   - Computes transformation matrix each tick based on current keyframe index
   - Handles forward direction calculation using next keyframe position
   - Includes robust degenerate case handling (coincident points, parallel vectors)
   - Plane model animates smoothly along the flight path

✅ 2.2 Animated Scene File (hogwarts-plane.txt)
   - Created `code/hogwarts-plane.txt` with plane animation node
   - Plane instance added to scene graph with appropriate scale (40x40x40)
   - Keyframe transform node "plane-anim" loads from `code/aeroplane-path.txt`
   - Plane flies around Hogwarts castle in a smooth curved path

✅ 2.3 Animation Loop
   - `Controller::run()` maintains `tickCount` that increments each frame
   - Passes tick to `View::display()` which forwards to renderer
   - `GLScenegraphRenderer::setTick()` updates animation nodes
   - Animation loops seamlessly (modulo keyframe count)
   - Tick counter wraps at INT_MAX to prevent overflow

✅ 2.4 Plane Camera (First-Person View)
   - Camera positioned at cockpit location (forward offset: 12, up offset: 4)
   - Points in plane's forward direction (computed from keyframe path)
   - Up vector consistent with keyframe data (Gram-Schmidt orthogonalization)
   - Properly follows plane orientation including banking/rolling
   - Activated by pressing key `4`

✅ 2.5 Multiple Cameras (4 Camera Modes)
   - Key `1`: Stationary camera - fixed overview position (180, 120, 260)
              looking at castle center (80, 20, 20)
   - Key `2`: Free-fly keyboard camera - user-controlled with arrow keys
              (no Shift: translate, Shift+arrows: look around, F/B: forward/back)
   - Key `3`: Chopper camera - circular orbit around castle with offset center
              (orbit radius: 90, hover height: 120 with slight sine wave variation)
              Automatically rotates at constant angular speed (0.01 rad/frame)
   - Key `4`: Plane camera - first-person view from cockpit as described in 2.4

✅ 3.1 Command-Line Argument
   - Program accepts scene file path as command-line argument
   - Example: `./Scenegraphs scenegraphmodels/humanoid-commands.txt`
   - Defaults to `code/hogwarts-plane.txt` if no argument provided
   - No code recompilation needed to load different files

✅ 3.2 Fixed Camera Position
   - Stationary camera (key `1`) positioned at (180, 120, 260)
   - Looks toward castle center at (80, 20, 20)
   - Provides excellent overview of entire Hogwarts scene
   - Shows plane flying around castle from good viewing angle

✅ 3.3 Perspective Projection
   - Uses `glm::perspective()` with 60° field-of-view
   - Near clipping plane: 0.1, far clipping plane: 10000
   - Suitable FOV for both close-up and wide scene viewing
   - Supports large scene depth range

✅ 3.4 Aspect Ratio Preservation on Resize
   - `View::onResize()` updates viewport with `glViewport(0, 0, w, h)`
   - Projection matrix recalculated with new aspect ratio `(float)w/h`
   - No stretching or squeezing of rendered content
   - Blank areas appear naturally when aspect ratio changes

Design Notes
------------

Architecture:
- Scene graph structure maintained from Assignment 4
- `KeyframeAnimationNode` extends `TransformNode` via `AnimationNode` base
- Animation updates happen in rendering pass via visitor pattern
- Tick-based animation allows smooth playback at any frame rate

Keyframe Animation:
- Each keyframe stores: position (vec3) + up vector (vec3)
- Forward direction computed from current→next position difference
- Fallback to previous frame if consecutive positions are identical
- Right vector: cross(forward, up) with degenerate case handling
- Final transform matrix built from orthonormalized basis + position

Camera System:
- Four distinct camera modes with different control mechanisms
- Stationary: fixed eye/center/up (simple lookAt)
- Free-fly: maintains orthonormal basis (camForward, camUp, camRight)
            with pitch clamping (±89°) to prevent gimbal lock
- Chopper: computed eye position from time-based circular orbit
           always looks at fixed target center
- Plane: eye position = plane position + offset in plane's local frame
         look direction = plane's forward vector
         up vector = plane's up vector (orthogonalized)

Camera Transitions:
- Switching to Free-fly (key 2) resets camera to initial stationary position
- Switching to Chopper/Plane uses current tick for smooth continuation
- Debug prints show camera state after movements (can be disabled)

Known Behaviors:
- `debugPrintCamera()` prints camera position/forward after key presses
  in Free-fly mode - useful for debugging, can be commented out
- Chopper camera orbits with slight up/down oscillation (sine wave)
  for more dynamic aerial cinematography effect
- Plane camera may show small portion of plane model depending on plane
  mesh geometry and cockpit offset values

Testing:
1. Build and run with default file: `make clean && make && ./Scenegraphs`
2. Press `1` - see stationary view with plane flying
3. Press `2` - control camera with arrows/F/B, Shift+arrows to look
4. Press `3` - watch helicopter-style orbital view
5. Press `4` - experience first-person plane cockpit view
6. Resize window - verify no distortion
7. Try other scene files: `./Scenegraphs scenegraphmodels/humanoid-commands.txt`