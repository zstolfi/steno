#include "../steno.hh"
#include <iostream>
#include <cassert>

template <class T> bool valid(T t) { return !t.failed(); }

int main() {
	std::cout << "# ''''''' Program Begin ''''''' #\n";
	std::cout << "String constructors ... ";
	assert(valid(steno::Stroke {"KPAFRPL"})); // The word "example"
	assert(valid(steno::Strokes {"EBGS/APL/P-L"})); // "example" long-handed
	assert(valid(steno::Brief {"stenography", {"STAOPB/TKPWRAEF"}}));
	std::cout << "passed!\n";

	std::cout << "Empty stroke constructors ... ";
	assert(valid(steno::Stroke {"-"}));
	assert(valid(steno::Stroke {" "}));
	assert(valid(steno::Stroke {""}));
	std::cout << "passed!\n";

	std::cout << "Number zero constructors ... ";
	assert(valid(steno::Stroke {" O"})); // ── Without number bar
	assert(valid(steno::Stroke {" 0"})); // ┐
	assert(valid(steno::Stroke {"#O"})); // ├─ With number bar
	assert(valid(steno::Stroke {"#0"})); // ┘
	std::cout << "passed!\n";

	const steno::Stroke example {"KPAFRPL"};
	const steno::Stroke AllKeys {steno::FromBits, 0xFFFFFFFF};
	std::cout << "Steno order: |" << steno::toString(AllKeys) << "|\n";
	std::cout << "'example':   |" << steno::toString(example) << "|\n";
	std::cout << "# ........ Program End ........ #\n";
}
