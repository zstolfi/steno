# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXX         = clang++
LDFLAGS     =
CXXFLAGS    = -std=c++20 -O0 -g -Wall -ferror-limit=24

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
STENO       = ..

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(STENO)

# ~~~~ Rules ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
all : example validate dictionary_open \
reverse_translate number_builder polyhedra \
numbers/numbers_advanced

example : example.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

validate : validate.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

reverse_translate : steno.o steno_parsers.o

dictionary_open : steno.o steno_parsers.o

steno_parsers.o : $(STENO)/steno_parsers.cc $(STENO)/steno_parsers.hh Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

number_builder : number_builder.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

polyhedra : polyhedra.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

steno.o : $(STENO)/steno.cc $(STENO)/steno.hh Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o : %.cc $(wildcard *.hh) Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

numbers/numbers_advanced : numbers/numbers_advanced.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

numbers/%.o : numbers/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ~~~~ Commands ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
clean :
	rm -f *.o example validate dictionary_open \
reverse_translate number_builder polyhedra \
numbers/numbers_advanced
