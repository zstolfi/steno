# ~~~~ Compilers & Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
CXX         = clang++
LDFLAGS     =
CXXFLAGS    = -std=c++20 -O0 -g -Wall -ferror-limit=24

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

$(STENO)/steno.hh : $(STENO)/steno_languages.hh

test : test.o steno.o steno_languages.o gtest_main.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o : test.cc $(STENO)/steno.cc $(STENO)/steno.hh English.cc $(GTEST_INC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

steno.o : $(STENO)/steno.cc $(STENO)/steno_languages.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

steno_languages.o : $(STENO)/steno_languages.cc $(STENO)/steno_languages.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

gtest-all.o : $(GTEST)/src/gtest-all.cc $(GTEST_SRCS)
	$(CXX) $(CXXFLAGS) -I$(GTEST) -c $< -o $@

gtest_main.o : $(GTEST)/src/gtest_main.cc $(GTEST_SRCS)
	$(CXX) $(CXXFLAGS) -I$(GTEST) -c $< -o $@

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# ~~~~ Commands ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
clean :
	rm -f *.o *.a test
