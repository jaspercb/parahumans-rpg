CC := g++-7
# CC = clang --analyze

SRCDIR := src
BUILDDIR := build
TARGET := bin/client

ccred=$(shell echo "\033[0;31m")
ccbold=$(shell echo "\033[1m")
ccend=$(shell echo "\033[0m")

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT) -not -name "*main.cpp")
OBJECTS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -std=c++17 -g -O1 -Wall
LIB := -lSDL2
TESTLIBS :=  -lgtest -lgtest_main -lpthread
INC := -I include -I lib

$(TARGET): $(OBJECTS)
	@echo "    Linking... $(ccbold)$(TARGET)$(ccend)"; $(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB);

bin/client: $(OBJECTS)
	@echo "    Linking... $(ccbold)$(TARGET)$(ccend)"; $(CC) $(CFLAGS) $^ build/main.o -o bin/client $(LIB);

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR)/gfx
	@echo "    Compiling  $(ccbold)$<$(ccend)"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<;

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

# Tests
test: $(OBJECTS)
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $^ -o bin/tester $(LIB) $(TESTLIBS); ./bin/tester
