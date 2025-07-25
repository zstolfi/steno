# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CC          = emcc
CXX         = em++
EMFLAGS     = -sUSE_SDL=3
CXXFLAGS    = -std=c++20 -Wall -O3 -ferror-limit=20
LDFLAGS     = --shell-file shell.html -sMAX_WEBGL_VERSION=2 -sASYNCIFY
FUNCTIONS   = _setDragOver,_showDebug
METHODS     = FS
BOOST_LIBS  = parser

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
STENO       = ../..
IMGUI       = $(STENO)/.include/imgui
BOOST       = $(STENO)/.include/boost
STB         = $(STENO)/.include/stb
ASSETS      = assets
WWW         = ..
TARGET      = $(WWW)/index.html

# ~~~~ Project Sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
SOURCES     = main.cc $(STENO)/steno.cc $(STENO)/steno_parsers.cc
SOURCES    += $(IMGUI)/imgui.cpp $(IMGUI)/imgui_demo.cpp
SOURCES    += $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp $(IMGUI)/imgui_widgets.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_sdl3.cpp
SOURCES    += $(IMGUI)/backends/imgui_impl_opengl3.cpp
OBJECTS     = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

# Makefile literals
empty =
space = $(empty) $(empty)
comma = ,

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(IMGUI) -I$(IMGUI)/backends -I$(STENO) -I$(STB)
CXXFLAGS   += $(BOOST_LIBS:%=-I$(BOOST)/%/include)
LDFLAGS    +=  -sALLOW_MEMORY_GROWTH=1
LDFLAGS    += -sEXPORTED_FUNCTIONS=_main,_malloc,_free$(if $(FUNCTIONS),$(comma)$(FUNCTIONS))
LDFLAGS    += -sEXPORTED_RUNTIME_METHODS=ccall,cwrap$(if $(METHODS),$(comma)$(METHODS))
LDFLAGS    +=  --embed-file $(ASSETS)

# ~~~~ Rules ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
all : $(TARGET)

main.o : $(wildcard *.hh)

$(TARGET): $(OBJECTS) shell.html $(wildcard $(ASSETS)/*) Makefile
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
