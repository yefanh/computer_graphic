Assignment 4 – Scene Graph Hogwarts
===================================

Build & Run
-----------
1. `cd /path/to/A4`
2. `make clean && make`
3. `./Scenegraphs code/hogwarts.txt`
     (argument optional; defaults to the Hogwarts file)

Features Implemented
--------------------
- Scene graph printer visitor (`sgraph/ScenegraphPrinter.*`) that outputs the
    tab-and-hyphen layout shown in `face-hierarchy-commands.txt`.
- Hogwarts command file matching the 24-object specification: repeated box set
    for objects 3/4/6/7/9/10/11/14/17/18/20/23/24, repeated tower set for
    objects 1/2/12/13/15/16/21/22, and object 5 as the tallest tower with four
    evenly spaced minarets.
- Two camera modes per rubric: `1` stationary overview, `2` keyboard camera.
    Arrow keys translate in camera space, `F/B` move forward/back, and
    `Shift` + arrows yaw/pitch in place.
- Perspective projection with 60° FOV and resize-safe rendering handled in
    `View::onResize`.
- Commands file supplied on the command line; the Makefile uses relative paths
    so graders need no edits.

Design Notes
------------
- Rendering relies on `sgraph::GLScenegraphRenderer` and a modelview stack.
- Camera basis (`camForward`, `camUp`, `camRight`) stays orthonormal. Movement
    only changes position; rotations only change orientation.
- I left `debugPrintCamera` prints in `Controller.cpp`; they show the camera
    position and forward vector after every arrow / Shift+arrow / F / B press:
    no-Shift arrows change position while keeping the forward vector fixed,
    Shift-modified arrows keep the position fixed while the forward vector
    rotates. Feel free to comment them out once done checking.