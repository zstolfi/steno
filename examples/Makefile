CXX       = clang++
CXXFLAGS  = -std=c++20 -O1 -Wall
LDFLAGS   = 

all : example #number_builder

example : example.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

number_builder : number_builder.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

steno.o : ../steno.cc ../steno.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o : %.cc $(wildcard *.hh) Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -f *.o
