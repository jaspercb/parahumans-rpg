#OBJS specifies which files to compile as part of the project
OBJS = *.cpp gfx/*.c

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -Wall -O1 -g -std=c++17

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -LC:\MinGW\lib -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = client.exe

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
