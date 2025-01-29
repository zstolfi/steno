#include "steno.hh"
#include <iostream>
#include <string>
#include <array>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const auto N_Start = std::array<steno::Brief, 20> {{
	{ "0", {"        O E           "}},
	{ "1", {"    W      U  PB      "}},
	{ "2", {" T  W   O             "}},
	{ "3", {" T   HR   E           "}},
	{ "4", {" T P    O E  R        "}},
	{ "5", {" T P      EUF         "}},
	{ "6", {"S         EU   B G S  "}},
	{ "7", {"S         E F         "}},
	{ "8", {"       A  E       T   "}},
	{ "9", {" T P H    EU  PB      "}},
	{"10", {" T        E   PB      "}},
	{"11", {"     HR   E F         "}},
	{"12", {" T  W     E F   L     "}},
	{"13", {" T   H    EU R    T   "}},
	{"14", {" T P    O E  R    T   "}},
	{"15", {" T P      EUF     T   "}},
	{"16", {"S         EU   B GT   "}},
	{"17", {"S         E F     T   "}},
	{"18", {"  K W RA  E       T   "}},
	{"19", {" T P H A  E       T   "}}
}};

const auto N_Tens = std::array<steno::Brief, 10> {{
	{}, {},
	{"2~", {" T PW    -            "}},
	{"3~", {" T   HR  -            "}},
	{"4~", {" T P  R  -            "}},
	{"5~", {" TK W    -            "}},
	{"6~", {"S K      -            "}},
	{"7~", {"S     R  -            "}},
	{"8~", {"  K W R  -            "}},
	{"9~", {" T P H   -            "}}
}};

const auto N_Ones = std::array<steno::Brief, 10> {{
	{"~0", {"       AO EU          "}},
	{"~1", {"           U  PB      "}},
	{"~2", {"       AO             "}},
	{"~3", {"       A  E           "}},
	{"~4", {"        O E  R        "}},
	{"~5", {"          EUF         "}},
	{"~6", {"          EU   B G    "}},
	{"~7", {"          E F P       "}},
	{"~8", {"       A  EU          "}},
	{"~9", {"       AO EU  PB      "}}
}};

const auto Hundred    = steno::Brief {"hundred" , {"     H     U  PB      "}};
const auto Hundred_N  = steno::Brief {"~00"     , {"     H     U  PB      "}};
const auto Thousand   = steno::Brief {"thousand", {" T   H  O  U          "}};
const auto Thousand_N = steno::Brief {"~,000"   , {" T   H  O  U          "}};
const auto Million    = steno::Brief {"million" , {"   P H    EU    L     "}};
const auto Billion    = steno::Brief {"billion" , {"   PWHR O     PB      "}};
const auto Trillion   = steno::Brief {"trillion", {" T    R   EU    L     "}};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

steno::Dictionary numbersDict {};

auto Num(unsigned x, bool recursed=false) -> steno::Brief {
	if (x == 0 && recursed) return steno::NoBrief;
	if (x < 20) return N_Start[x];
	if (x < 100) return N_Tens[x/10] + N_Ones[x%10];
	if (x < 1000) {
		if (x%100 == 0) return Num(x/100) | Hundred_N;
		return x%100 < 10
		?	Num(x/100) | steno::Brief {"~0~", Hundred_N} | Num(x%100)
		:	Num(x/100) | Num(x%100);
	}
	return steno::NoBrief;
}

int main() {
	for (unsigned i=0; i<1000; i++) {
		auto brief = Num(i);
		std::cout << brief.text << " := \t|"<< toString(brief.strokes) << " |\n";
		numbersDict.entries.push_back(brief);
	}

	numbersDict.entries.insert(numbersDict.entries.end(), {
		Hundred, Thousand, Million, Billion, Trillion
	});
}
