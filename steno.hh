#include <iostream>
#include <bitset>
#include <string>
#include <algorithm>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Stroke {
	union {
		std::bitset<32> bits {};
		struct {
			/* Number Bar: */
			unsigned Num:1;
			/* Initial Consonants: */
			unsigned S_:1, T_:1, K_:1, P_:1, W_:1, H_:1, R_:1;
			/* Vowels: */
			unsigned A:1, O:1, x:1, E:1, U:1;
			/* Final Consonants: */
			unsigned _F:1, _R:1, _P:1, _B:1, _L:1, _G:1, _T:1, _S:1, _D:1, _Z:1;
			/* Flags: */
			unsigned Mark:1, OpenLeft:1, OpenRight:1;
			unsigned /*padding*/:5, FailedCostruction:1;
		};
	} keys;

public:
	constexpr Stroke() {};

	Stroke(std::string s) {
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

private:
	void failConstruction() { keys.FailedCostruction = true; }
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

constexpr auto NoStroke = Stroke {};

} // namespace steno
