CXX       = clang++
CXXFLAGS  = -std=c++20 -O1 -Wall
LDFLAGS   = 

all : example number_builder

example : example.o ../steno.o
	$(CXX) $^ -o $@ $(LDFLAGS)

number_builder : number_builder.o ../steno.o
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o : %.cc *.hh Makefile
	$(CXX) $< -c -o $@ $(CXXFLAGS)

clean :
	rm -f *.o
