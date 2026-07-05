# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXX         = clang++
LDFLAGS     =
CXXFLAGS    = -std=c++20 -Wall -ferror-limit=24

# ~~~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
STENO       = ..
GTEST       = $(STENO)/.include/googletest/googletest
GTEST_INC   = $(GTEST)/include/gtest/*.h $(GTEST)/include/gtest/internal/*.h
GTEST_SRCS  = $(GTEST)/src/*.cc $(GTEST)/src/*.h $(GTEST_INC)

# ~~~~ Flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXXFLAGS   += -I$(STENO)
CXXFLAGS   += -isystem $(GTEST)/include

# ~~~~ Rules ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

all : test

test : test.o steno.o steno_parsers.o gtest_main.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o : test.cc $(STENO)/steno.cc $(STENO)/steno.hh $(GTEST_INC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

steno.o : $(STENO)/steno.cc $(STENO)/steno.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

steno_parsers.o : $(STENO)/steno_parsers.cc $(STENO)/steno_parsers.hh Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

gtest-all.o : $(GTEST)/src/gtest-all.cc $(GTEST_SRCS)
	$(CXX) $(CXXFLAGS) -I$(GTEST) -c $< -o $@

gtest_main.o : $(GTEST)/src/gtest_main.cc $(GTEST_SRCS)
	$(CXX) $(CXXFLAGS) -I$(GTEST) -c $< -o $@

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^
