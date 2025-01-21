#include <iostream>
#include <bitset>
#include <string>

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

	Stroke(std::string s);

private:
	void failConstruction();
};

constexpr auto NoStroke = Stroke {};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
