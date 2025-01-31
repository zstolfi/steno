#include "steno.hh"
#include <iostream>

int main() {
	steno::Strokes t = {"OE/WUPB/TWO/THRE"};

	std::cout << ".........ZDSTGLBPRFUE*OARHWPKTS.\n";
	std::cout << t[0].bits << "\n";
	std::cout << t[1].bits << "\n";
	std::cout << t[2].bits << "\n";
	std::cout << t[3].bits << "\n";
}
