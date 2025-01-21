#include "steno.hh"
#include <algorithm>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(std::string s) {
	const std::string Whitespace = " \t";
	const std::string Left   = "STKPWHR";
	const std::string Middle = "AO*EU";
	const std::string Right  = "FRPBLGTSDZ";
	const std::string Misc   = "-";
	auto in = [](std::string set) {
		return [set](char c) { return set.find(c) != set.npos; };
	};

	// Remove whitespace.
	s.erase(
		std::remove_if(s.begin(), s.end(), in(Whitespace)),
		s.end()
	);

	// Sinple charset test:
	if (!std::all_of(s.begin(), s.end(), in(Left+Middle+Right+Misc))) {
		failConstruction(); return;
	}

	// Find unique middle secion (vowels).
	unsigned v0 {}, v1 {};
	if (auto dash = s.find('-'); dash != s.npos) {
		v0 = dash;
		v1 = dash+1;
	}
	else if (auto i = s.find_first_of(Middle); i != s.npos) {
		auto j = s.find_first_not_of(Middle, i);
		v0 = i;
		v1 = (j != s.npos) ? j : s.size();
	}
	// Fail if there is not a middle section.
	else { failConstruction(); return; }

	// Split 's' into substrings.
	const std::string sL = s.substr(0, v0);
	const std::string sM = s.substr(v0, v1-v0);
	const std::string sR = s.substr(v1, s.size()-v1);

	// More checks.
	if (!std::all_of(sL.begin(), sL.end(), in(Left  ))
	||  !std::all_of(sM.begin(), sM.end(), in(Middle))
	||  !std::all_of(sR.begin(), sR.end(), in(Right ))) {
		failConstruction(); return;
	}

	// Assign.
	for (char c : sL) keys.bits[Left  .find(c) +  1] = true;
	for (char c : sM) keys.bits[Middle.find(c) +  8] = true;
	for (char c : sR) keys.bits[Right .find(c) + 13] = true;
}

void Stroke::failConstruction() { keys.FailedCostruction = true; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
