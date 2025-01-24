#include "steno.hh"
#include <iostream>

int main() {
	steno::Strokes t = {"OE/WUPB/TWO/THRE"};

	std::cout << ".........ZDSTGLBPRFUE*OARHWPKTS.\n";
	std::cout << t[0].keys.bits << "\n";
	std::cout << t[1].keys.bits << "\n";
	std::cout << t[2].keys.bits << "\n";
	std::cout << t[3].keys.bits << "\n";
}
