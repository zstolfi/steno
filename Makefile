CXX       = clang++
CXXFLAGS  = -std=c++20 -O1 -Wall
LDFLAGS   = 

all : steno.o

%.o : %.cc *.hh Makefile
	$(CXX) $< -c -o $@ $(CXXFLAGS)

clean :
	rm -f *.o
