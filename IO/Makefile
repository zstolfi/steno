CXX       = clang++
CXXFLAGS  = -std=c++20 -O1 -Wall
OBJECTS   = /mnt/c/source/steno/steno.o
CXXFLAGS += -I/usr/local/boost_1_87_0 -I/mnt/c/source/steno
LDFLAGS   = 

all : JSON_import

%.o : %.cc $(wildcard *.hh) $(wildcard *.inc) Makefile
	$(CXX) $< -c -o $@ $(CXXFLAGS)

JSON_import : JSON_import.o $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

clean :
	rm -f *.o
