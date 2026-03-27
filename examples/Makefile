# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXX         = clang++
CXXFLAGS    = -std=c++20 -O1 -Wall -ferror-limit=10
LDFLAGS     = 

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
STENO       = ..
BOOST       = $(STENO)/.include/boost
GTEST       = $(STENO)/.include/googletest/googletest

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(STENO) -I$(BOOST)/parser/include
CXXFLAGS   += -isystem $(GTEST)/include

# ~~~~ Rules ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
all : test example validate reverse_translate \
number_builder polyhedra \
numbers/numbers_advanced

example : example.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

validate : validate.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

reverse_translate : steno.o steno_parsers.o

steno_parsers.o : $(STENO)/steno_parsers.cc
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

# ~~~~ Google Test Specific ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
GTEST_INC   = $(GTEST)/include/gtest/*.h $(GTEST)/include/gtest/internal/*.h
GTEST_SRCS  = $(GTEST)/src/*.cc $(GTEST)/src/*.h $(GTEST_INC)

gtest-all.o : $(GTEST)/src/gtest-all.cc $(GTEST_SRCS)
	$(CXX) $(CXXFLAGS) -I$(GTEST) -c $< -o $@

gtest_main.o : $(GTEST)/src/gtest_main.cc $(GTEST_SRCS)
	$(CXX) $(CXXFLAGS) -I$(GTEST) -c $< -o $@

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

test : test.o steno.o gtest_main.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o : test.cc $(STENO)/steno.cc $(STENO)/steno.hh $(GTEST_INC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ~~~~ Commands ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
clean :
	rm -f *.o *.a test example validate number_builder
