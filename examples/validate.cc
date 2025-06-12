#include "../steno.hh"
#include <iostream>

int main() {
	std::cout << "# ''''''' Program Begin ''''''' #\n";
	for (std::string line; std::getline(std::cin, line);) {
		const steno::Stroke stroke {line};
		if (stroke.failed()) std::cout << line << "\tREJECT!\n";
		else {
			std::cout << line << "\tACCEPT!\t";
			std::cout << "|" << steno::toString(stroke) << "|\t";
			std::cout <<  stroke.bits << "\n";
		}
	}
	std::cout << "# ........ Program End ........ #\n";
}
