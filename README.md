# Project Overview
Ray tracer + OpenGL scene graph. Shadows, reflections, texture mapping (spheres/boxes), refraction, and a creative Christmas scene.

# How to Build & Run
```
make clean && make
# run any scene file, e.g.
./ScenegraphsLightsTextures scenegraphmodels/simple_refraction_and_final_demo.txt
```
Controls (window):
- `S` : run ray tracer → writes `raytraced_output.ppm`
- Close window to exit
- (Camera adjust: key callbacks already wired; you can move/rotate as in prior assignments.)

# Scene → Image Mapping (submission/)
- `simple_shadow_demo.txt` → `shadow demo - 1.1.png` (shadows)
- `simple_reflection_demo.txt` → `reflection demo - 1.2.png` (reflections)
- `simple_texture_mapping_demo.txt` → `texture mapping demo - 2.1 extra credit.png` (textures on box+sphere)
- `simple_refraction_and_final_demo.txt` → `refraction demo - 2.2 extra credit.png`
- `simple_refraction_and_final_demo.txt` → `final rendering - ray tracing.png` (best raytraced; shows shadows/reflect/refraction/texture)
- `creative_ec_scene.txt` → `extra credit for 2.3.png` creative 2.3 extra credit scene (render a new 800x800 raytrace + matching OpenGL screenshot)

If you want a fresh final pair: run `./ScenegraphsLightsTextures scenegraphmodels/simple_refraction_and_final_demo.txt`, press `S` for raytrace, then capture the same view in OpenGL for the matching shot.

# Files of Interest
- Ray tracer core: `sgraph/RaycastRenderer.h` (shadow rays, reflection/refraction, texture sampling)
- Material parsing: `sgraph/ScenegraphImporter.h`
- Texture UV for box/sphere: `KDAbstractNode::testTriangles`
- Scenes: `scenegraphmodels/*.txt`

# Camera controls (during OpenGL view if you need to adjust the view)
- `2` : toggle camera control mode
  - Arrow keys: rotate camera (left/right/up/down)
- `F` / `B` : move camera forward/back along current gaze direction
- Default view looks at scene origin; adjust as needed before pressing `S` to raytrace.

# Notes
- All required features are demonstrated in the PNGs under `submission/`.
- Extra credit: textures (2.1), refraction (2.2), creative scene (2.3) have dedicated scenes/images as listed above.
