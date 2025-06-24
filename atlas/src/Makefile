# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CC          = emcc
CXX         = em++
EMFLAGS     = -sUSE_SDL=3
CXXFLAGS    = -std=c++20 -Wall -O1 -ferror-limit=30
LDFLAGS     = --shell-file shell.html -sALLOW_MEMORY_GROWTH=1
FUNCTIONS   = _main,_malloc,_free,_setDragOver

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
STENO       = ../..
IMGUI       = $(STENO)/.include/imgui
BOOST       = $(STENO)/.include/boost
STB         = $(STENO)/.include/stb
BOOST_LIBS  = parser
WWW         = ..
TARGET      = $(WWW)/index.html

# ~~~~ Project Sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
SOURCES     = main.cc $(STENO)/steno.cc $(STENO)/steno_parsers.cc
SOURCES    += $(IMGUI)/imgui.cpp $(IMGUI)/imgui_demo.cpp
SOURCES    += $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp $(IMGUI)/imgui_widgets.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_sdl3.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_opengl3.cpp
OBJECTS     = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(IMGUI) -I$(IMGUI)/backends -I$(STENO) -I$(STB)
CXXFLAGS   += $(BOOST_LIBS:%=-I$(BOOST)/%/include)
LDFLAGS    += -sEXPORTED_FUNCTIONS=$(FUNCTIONS)
LDFLAGS    += -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,FS

# ~~~~ Rules ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
all : $(TARGET)

main.o : $(wildcard *.hh)

$(TARGET): $(OBJECTS) shell.html Makefile
	$(CXX) $(EMFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.cc
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(STENO)/%.cc $(STENO)/steno.hh
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(IMGUI)/%.cpp
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : $(IMGUI)/backends/%.cpp
	$(CXX) $(EMFLAGS) $(CXXFLAGS) -c $< -o $@

# ~~~~ Commands ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
serve : all
	python3 -m http.server -d $(WWW)

clean :
	rm -rf $(OBJECTS)
