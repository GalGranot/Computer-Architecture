all: bp_main

# Environment for C 
CC = gcc
CFLAGS = -std=c99 -Wall

# Environment for C++ 
CXX = g++
CXXFLAGS = -std=c++11 -Wall

SRC_BP = $(wildcard bp.c bp.cpp)
SRC_GIVEN = bp_main.c
EXTRA_DEPS = bp_api.h bp_classes.h

SRC_CLASSES = bp_classes.cpp

OBJ_GIVEN = $(patsubst %.c,%.o,$(SRC_GIVEN))
OBJ_CLASSES = $(patsubst %.cpp,%.o,$(SRC_CLASSES))
OBJ_BP = bp.o
OBJ = $(OBJ_GIVEN) $(OBJ_BP) $(OBJ_CLASSES)

ifeq ($(SRC_BP),bp.c)
bp_main: $(OBJ)
	$(CC) -o $@ $(OBJ) -lm

bp.o: bp.c
	$(CC) -c $(CFLAGS) -o $@ $^ -lm

else
bp_main: $(OBJ)
	$(CXX) -o $@ $(OBJ)

bp.o: bp.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^ -lm

bp_classes.o: bp_classes.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

endif

$(OBJ_GIVEN): %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^ -lm

.PHONY: clean
clean:
	rm -f bp_main $(OBJ) $(OBJ_CLASSES)
