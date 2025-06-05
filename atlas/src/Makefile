# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CC          = emcc
CXX         = em++
EMFLAGS     = -sUSE_SDL=3 -fsanitize=undefined
CXXFLAGS    = -std=c++20 -Wall -O1 -ferror-limit=30
LDFLAGS     = --shell-file shell.html -sALLOW_MEMORY_GROWTH=1
FUNCTIONS   = _main,_setDragOver,_receiveFile

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
IMGUI       = imgui
WEB_DIR     = ..
TARGET      = $(WEB_DIR)/index.html

# ~~~~ Project Sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
SOURCES     = main.cc
SOURCES    += $(IMGUI)/imgui.cpp $(IMGUI)/imgui_demo.cpp
SOURCES    += $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp $(IMGUI)/imgui_widgets.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_sdl3.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_opengl3.cpp
OBJECTS     = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(IMGUI) -I$(IMGUI)/backends
LDFLAGS    += -sEXPORTED_FUNCTIONS=$(FUNCTIONS)
LDFLAGS    += -sEXPORTED_RUNTIME_METHODS=ccall,cwrap

all : $(TARGET)

$(TARGET): $(OBJECTS) shell.html Makefile
	$(CXX) $(EMFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

main.o : window.hh

%.o : %.cc
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(IMGUI)/%.cpp
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(IMGUI)/backends/%.cpp
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

serve: all
	python3 -m http.server -d $(WEB_DIR)

clean:
	rm -rf $(OBJECTS)
