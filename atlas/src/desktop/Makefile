# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CC          = ckang
CXX         = clang++
EMFLAGS     = -fsanitize=undefined
CXXFLAGS    = -std=c++20 -Wall -O0 -g -ferror-limit=30
LDFLAGS     = -lSDL3 -lGL

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
IMGUI       = ../imgui
TARGET      = atlas

# ~~~~ Project Sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
SOURCES     = ../main.cc
SOURCES    += $(IMGUI)/imgui.cpp $(IMGUI)/imgui_demo.cpp
SOURCES    += $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp $(IMGUI)/imgui_widgets.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_sdl3.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_opengl3.cpp
OBJECTS     = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(IMGUI) -I$(IMGUI)/backends
LDFLAGS    += 

# ~~~~ Rules ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
all : $(TARGET)

main.o : ../window.hh

$(TARGET): $(OBJECTS) Makefile
	$(CXX) $(EMFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

%.o : ../%.cc
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(IMGUI)/%.cpp
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(IMGUI)/backends/%.cpp
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

# ~~~~ Commands ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
clean :
	rm -rf $(OBJECTS)
