OBJS = Scenegraphs.o View.o Controller.o Model.o KDTree.o KDAbstractNode.o KDInternalNode.o KDLeafNode.o
INCLUDES = -I../include
LIBS = -L../lib
LDFLAGS = -lglad -lglfw3
CFLAGS = -g -std=c++11
PROGRAM = ScenegraphsLightsTextures


ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    LDFLAGS += -lopengl32 -lgdi32
    PROGRAM :=$(addsuffix .exe,$(PROGRAM))
	COMPILER = g++
else ifeq ($(shell uname -s),Darwin)     # is MACOSX
    LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit
	COMPILER = clang++
endif

ScenegraphsLightsTextures: $(OBJS)
	$(COMPILER) -o $(PROGRAM) $(OBJS) $(LIBS) $(LDFLAGS)

Scenegraphs.o: Scenegraphs.cpp
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Scenegraphs.cpp

View.o: View.cpp View.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c View.cpp	

Controller.o: Controller.cpp Controller.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Controller.cpp	

Model.o: Model.cpp Model.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Model.cpp

KDTree.o: KDTree.cpp KDTree.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c KDTree.cpp

KDAbstractNode.o: KDAbstractNode.cpp KDAbstractNode.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c KDAbstractNode.cpp

KDInternalNode.o: KDInternalNode.cpp KDInternalNode.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c KDInternalNode.cpp

KDLeafNode.o: KDLeafNode.cpp KDLeafNode.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c KDLeafNode.cpp		
	
RM = rm	-f
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    RM := del
endif

clean: 
	$(RM) $(OBJS) $(PROGRAM)
    