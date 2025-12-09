Lights, textures, and ray tracing with a scene graph.

Build
-----
make clean && make

Run
---
./ScenegraphsLightsTextures <scene file>
Examples:
 - ./ScenegraphsLightsTextures scenegraphmodels/simple.txt   (spotlight test)
 - ./ScenegraphsLightsTextures scenegraphmodels/single-sphere.txt
 - ./ScenegraphsLightsTextures scenegraphmodels/box.txt
 - ./ScenegraphsLightsTextures scenegraphmodels/creative-scene.txt （extra credit）

Controls
--------
S : Run the ray tracer for the current scene and write raytraced_output.ppm
V : (already wired) prints keypress; OpenGL view continues to render
Close window to exit.

Ray tracing notes
-----------------
- Output image is PPM at up to 800x800 using the current window’s logical size
  (smaller windows run faster; set window ~800x800 for final outputs).
- Camera matches the OpenGL view (lookAt in View.cpp).
- Lighting matches the shader (ambient/diffuse/specular, point/directional/spot),
  textures are ignored for ray tracing per assignment requirements.

=============================================================================
SUBMISSION FILES
=============================================================================

All submission images are located in the "submission/" folder:

| File                      | Description                                    |
|---------------------------|------------------------------------------------|
| 1_final_raytraced.png     | Final ray traced image (800x800)               |
| 2_opengl_rendering.png    | OpenGL rendering of the same scene             |
| 3_extra_credit.png        | Extra credit: creative scene with custom model |

Scene files used:
- scenegraphmodels/simple.txt          -> Used for images 1 & 2
- scenegraphmodels/creative-scene.txt  -> Used for image 3 (extra credit)

IMAGE REQUIREMENTS CHECKLIST:
-----------------------------
The final raytraced image (1_final_raytraced.png) demonstrates ALL of the following:

✓ Ray tracing works for SPHERES  -> one-sphere in simple.txt
✓ Ray tracing works for BOXES    -> one-box and two-box (ground) in simple.txt  
✓ Lighting works with SPOTLIGHT  -> two-light has spot-direction and spot-angle

=============================================================================
EXTRA CREDIT
=============================================================================

Custom 3D Model: models/house.obj
- A custom house mesh with walls and a triangular roof
- Contains 10+ vertices and normals (satisfies complex mesh requirement)
- Used in creative-scene.txt which includes 17 objects:
  * 2 houses (using custom house.obj model)
  * 2 trees (trunk + cone leaves each)
  * 1 sun (emissive sphere)
  * 5 flowers (red and yellow spheres)
  * 2 decorative stones
  * 1 bush
  * 1 ground plane
  * 1 stone path