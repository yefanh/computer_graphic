OBJS = Scenegraphs.o View.o Controller.o Model.o sgraph/ScenegraphPrinter.o sgraph/KeyframeAnimationNode.o

# Use local include/ and lib/ directories for all dependencies
# This makes the assignment self-contained and portable
LOCAL_INCLUDE_DIR := ./include
LOCAL_LIB_DIR     := ./lib

# Include paths: local headers first, then system libraries (GLFW/GLM via Homebrew)
INCLUDES = -I. \
	-I$(LOCAL_INCLUDE_DIR) \
	-I/opt/homebrew/include

# Library paths: local libglad.a first, then system libraries
LIBS = -L$(LOCAL_LIB_DIR) -L/opt/homebrew/lib
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
    