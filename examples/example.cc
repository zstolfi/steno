#include "../steno.hh"
#include <iostream>
#include <cassert>

template <class T> bool valid(T t) { return !t.failed(); }

int main() {
	std::cout << "# ''''''' Program Begin ''''''' #\n";
	assert(valid(steno::Stroke {"KPAFRPL"})); // The word "example"
	assert(valid(steno::Strokes {"EBGS/APL/P-L"})); // "example" long-handed
	assert(valid(steno::Brief {"stenography", {"STAOPB/TKPWRAEF"}}));

	assert(    valid(steno::Stroke {"-"})); // The empty stroke
//	assert(not valid(steno::Stroke {" "}));
//	assert(not valid(steno::Stroke {""}));

	assert(valid(steno::Stroke {" O"})); // ── Without number bar
	assert(valid(steno::Stroke {" 0"})); // ┐
	assert(valid(steno::Stroke {"#O"})); // ├─ With number bar
	assert(valid(steno::Stroke {"#0"})); // ┘
	std::cout << "# ........ Program End ........ #\n";
}
