#include "steno.hh"
#include "steno_parsers.hh"
#include <iostream>
#include <fstream>

int main(int argc, char const* argv[]) {
	std::vector<std::string> paths {argv+1, argv+argc};
	if (paths.empty()) std::cerr << "No dictionaries provided.\n";
	else for (auto path : paths) {
		if (std::ifstream file {path}) {
			if (auto dict = steno::parseDictionary(file)) {
				std::cout << path << " " << dict->size() << " entries.\n";
				for (steno::Brief b : *dict) {
					std::cout << "|" << steno::Wide << b.phrase() << "|\t";
					std::cout << b.text() << "\n";
				}
			}
			else std::cerr << "Unable to parse dictionary " << path << ".\n";
		}
		else std::cerr << "Unable to open dictionary" << path << ".\n";
	}
}
