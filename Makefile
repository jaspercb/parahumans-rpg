#OBJS specifies which files to compile as part of the project
OBJS = *.cpp gfx/*.c

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -Wall -O1 -g -std=c++17

#LINKER_FLAGS specifies the libraries we're linking against
ifeq ($(OS),Windows_NT)
LINKER_FLAGS = -LC:\MinGW\lib -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid
CC = g++
OBJ_NAME = client.exe
else
LINKER_FLAGS = -lSDL2main -lSDL2 -lm
CC = g++-7
OBJ_NAME = client
endif

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
