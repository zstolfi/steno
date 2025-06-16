CXX       = clang++
CXXFLAGS  = -std=c++20 -O1 -Wall -ferror-limit=10
CXXFLAGS += -I../.include/boost/parser/include
LDFLAGS   = 

all : example validate #number_builder

example : example.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

validate : validate.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

parser_test : steno.o steno_parsers.o

steno_parsers.o : ../steno_parsers.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

number_builder : number_builder.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

steno.o : ../steno.cc ../steno.hh Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o : %.cc $(wildcard *.hh) Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -f *.o
