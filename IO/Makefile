CXX       = clang++
CXXFLAGS  = -std=c++20 -O1 -Wall
LDFLAGS   = 

STENO     = /mnt/c/source/steno
BOOST     = /usr/local/boost_1_87_0

CXXFLAGS += -I$(BOOST) -I$(STENO)

all : JSON_import

JSON_import : JSON_import.o steno.o
	$(CXX) $(LDFLAGS) $^ -o $@

steno.o : $(STENO)/steno.cc $(STENO)/steno.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o : %.cc $(wildcard *.hh) Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -f *.o
