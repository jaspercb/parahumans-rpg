CC := g++-7
# CC = clang --analyze

SRCDIR := src
TESTDIR := test
BUILDDIR := build
TARGET := bin/client
TEST_TARGET := bin/tester

ccred=$(shell echo "\033[0;31m")
ccbold=$(shell echo "\033[1m")
ccend=$(shell echo "\033[0m")

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT) -not -name "*main.cpp")
OBJECTS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(SOURCES:.$(SRCEXT)=.o))
TESTSOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT) -not -name "*tester.cpp")
TESTOBJECTS := $(patsubst $(TESTDIR)/%, $(BUILDDIR)/test/%, $(TESTSOURCES:.$(SRCEXT)=.o))
CFLAGS := -std=c++17 -g -O3 -Wall -Wno-narrowing -Wno-unused -Wno-reorder
LIB := -lSDL2
TESTLIBS :=  -lgtest -lgtest_main -lpthread
INC := -I include -I lib

bin/client: $(OBJECTS) build/main.o
	@echo "    Linking... $(ccbold)$(TARGET)$(ccend)"; $(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB);

run: bin/client
	./bin/client

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR)/gfx
	@mkdir -p $(BUILDDIR)/FastNoise
	@echo "    Compiling  $(ccbold)$<$(ccend)"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<;

$(BUILDDIR)/test/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR)/test
	@echo "    Compiling  $(ccbold)$<$(ccend)"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<;

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

# Tests
test: $(OBJECTS) $(TESTOBJECTS)
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $^ -o $(TEST_TARGET) $(LIB) $(TESTLIBS); $(TEST_TARGET)
