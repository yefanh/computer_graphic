OBJS = Scenegraphs.o View.o Controller.o Model.o sgraph/ScenegraphPrinter.o sgraph/KeyframeAnimationNode.o

# Resolve shared resource paths using relatives so graders can build without edits.
HW_ROOT_REL            := ..
COURSE_ROOT_REL        := ../..
A1_DEPS_INCLUDE_DIR    := $(HW_ROOT_REL)/a1/deps/include
HW_INCLUDE_DIR         := $(HW_ROOT_REL)/include
DEMOS_GLFW_INCLUDE_DIR := $(COURSE_ROOT_REL)/Introduction\ to\ OpenGL/demos-glfw/include
HW_LIB_DIR             := $(HW_ROOT_REL)/lib

# Course headers (glad, KHR) + professor demos-glfw include + Homebrew include
# Prefer assignment/local copies; fall back to system Homebrew path if available
INCLUDES = -I. \
	-I$(A1_DEPS_INCLUDE_DIR) \
	-I$(DEMOS_GLFW_INCLUDE_DIR) \
	-I/opt/homebrew/include

# Course lib (libglad.a) + Homebrew lib (libglfw)
LIBS = -L$(HW_LIB_DIR) -L/opt/homebrew/lib
LDFLAGS = -lglad -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
CFLAGS = -g -std=c++11
PROGRAM = Scenegraphs


ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    LDFLAGS += -lopengl32 -lgdi32
    PROGRAM :=$(addsuffix .exe,$(PROGRAM))
	COMPILER = g++
else ifeq ($(shell uname -s),Darwin)     # is MACOSX
    LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit
	COMPILER = clang++
endif

Scenegraphs: $(OBJS)
	$(COMPILER) -o $(PROGRAM) $(OBJS) $(LIBS) $(LDFLAGS)

.PHONY: run
run: $(PROGRAM)
	./$(PROGRAM)

Scenegraphs.o: Scenegraphs.cpp
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Scenegraphs.cpp

View.o: View.cpp View.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c View.cpp	

Controller.o: Controller.cpp Controller.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Controller.cpp	

Model.o: Model.cpp Model.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Model.cpp		

# Build object for ScenegraphPrinter visitor
sgraph/ScenegraphPrinter.o: sgraph/ScenegraphPrinter.cpp sgraph/ScenegraphPrinter.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c sgraph/ScenegraphPrinter.cpp -o sgraph/ScenegraphPrinter.o

sgraph/KeyframeAnimationNode.o: sgraph/KeyframeAnimationNode.cpp sgraph/KeyframeAnimationNode.h sgraph/AnimationNode.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c sgraph/KeyframeAnimationNode.cpp -o sgraph/KeyframeAnimationNode.o
	
RM = rm	-f
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    RM := del
endif

clean: 
	$(RM) $(OBJS) $(PROGRAM)
    