This program implements a simple scene graph, and renders it.

A scene graph has various kinds of nodes:

1. Group node: a logical grouping of other nodes in the graph. It can have zero or more children.

2. Leaf node: this is the only node that contains drawable geometry. It remembers an instance of the object to be drawn, so that the same mesh is not copied over in multiple leaves.

3. Transform node: this represents a single transformation. It has a single child. There are three kinds of specific transformations:
    a. Scale transform
    b. Translate transform
    c. Rotate transform
    

The Scenegraph class implements a scene graph (specifically the IScenegraph abstract class).

To implement operations on a scene graph, a visitor pattern is used. The NodeVisitor abstract class represents the interface of a visitor. An example implementation is provided in the GLScenegraphRenderer class, which renders the scene graph using OpenGL.

Finally, a command-language is implemented that makes it convenient to specify a scene graph. Some commands it supports are:

a. translate variable-name node-name tx ty tz: create a new translate transform node with name "node-name". variable-name is used to refer to this node in the command language

b. group variable-name node-name: self-explanatory

c. add-child abc def: Add the node whose variable-name is "abc" as a child of the node whose variable-name is "def"

d. assign-material leaf material: Assign to the leaf with variable-name "leaf" the material with variable-name "material"

and so on.

Note that single-line comments are allowed in the command language: a line that begins with # is a comment.


Several examples of scene graphs have been given, in the scenegraphmodels folder: 

1. simple.txt: shows a simple scene graph with one box with transformations applied.

2. face-hierarchy-commands.txt: shows a scene graph that creates a clown face with a hat. In comments, the scene graph that is progressively built is shown, for illustration.

3. humanoid-commands.txt: shows a scene graph that creates a humanoid stick figure. Look for comments IN CAPS in the "right arm" part of this file, for an example of how to manipulate this scene graph to change the pose of the humanoid, if need be.

4. face-hierarchy-with-copy-commands.txt: shows an example of a command that allows one to create a copy of part of a scene graph. See towards the end of this file.

5. two-humanoids.txt: shows an example of how one command can be used to import an entire scene graph from another command file. This is helpful when creating a more complicated scene. Instead of writing all the commands in one file, one can create smaller pieces in separate files and then create a "main commands file" that imports them.


Texture usage and attributions
------------------------------

This assignment uses a few small PPM textures, all of which were created specifically for this project and do not come from external copyrighted sources:

- `textures/castle-brick.ppm`: hand-crafted 4×4 red brick checker pattern, written manually as a P3 PPM file in a text editor.
- `textures/tower-stone.ppm`: hand-crafted muted stone tiling pattern, written manually as a P3 PPM file in a text editor.
- `textures/roof-tiles.ppm`: hand-crafted magenta roof-tile pattern, written manually as a P3 PPM file in a text editor.
- `textures/white.ppm`: 1×1 solid white fallback texture, written manually as a P3 PPM file in a text editor.

No third‑party images or websites were used; all textures were authored by the student for this assignment.


Assignment A6 summary
---------------------

a. Feature status

- Basic scenegraph (group / transform / leaf nodes) and command-language loading: working.
- Multiple lights, materials, and keyframe cameras: working.
- Animated airplane with spotlight following the airplane: working.
- Toon shading (press `S` to toggle between Phong+texture and toon-only modes): working.
- Texture mapping using PPM images on boxes, cylinders, and cones with mipmapping: working.
- No known crashes on startup or during normal camera/airplane interaction.

b. Contribution breakdown

- This was an individual assignment; all parts of the implementation (scenegraph extensions, animation, shaders, texture loading, and configuration) were done by Yefan He.

c. Notes for grading

- Tested on macOS with the provided `include/` and `lib/` folders using the supplied Makefile:
    - `make`
    - `./ScenegraphsWithKeyframeCameras`
- Use keys 1–4 to switch cameras; `F`/`B` and the arrow keys to move; `S` to toggle between Phong+texture shading and toon shading.