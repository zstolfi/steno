#include "steno.hh"
#include <iostream>
#include <string>
#include <array>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const auto N_Start = std::array<steno::Brief, 20> {{
	steno::NoBrief,
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
	{"0~", {"S K WH   -            "}},
	steno::NoBrief,
	{"2~", {" T PW    -            "}},
	{"3~", {" T   HR  -            "}},
	{"4~", {" T P  R  -            "}},
	{"5~", {" TK W    -            "}},
	{"6~", {"S K      -            "}},
	{"7~", {"S     R  -            "}},
	{"8~", {"  K W R  -            "}},
	{"9~", {" T P H   *            "}}
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

auto Num(unsigned x, bool recursed=false) -> steno::Brief {
	if (x < 20 && !(x < 10 && recursed)) return N_Start[x];
	if (x < 100) return N_Tens[x/10] + N_Ones[x%10];
	if (x < 1000) {
		if (x%100 == 0) return Num(x/100) | Hundred_N;
		return (x%100 == 0)
		?	Num(x/100) | Hundred_N
		:	Num(x/100) | steno::Glue + Num(x%100, true);
	}
	return steno::NoBrief;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

auto WrittenOut(steno::Brief b) -> steno::Brief {
	if (b.text.size() == 1)
	if (char c = b.text[0]; '0' <= c&&c <= '9') {
		b.text = std::array {
			"zero", "one", "two", "three", "four", "five",
			"six", "seven", "eight", "nine", "ten"
		} [c-'0'];
		return b;
	}
	return steno::NoBrief;
};

auto Dollars(steno::Brief magnitude) {
	return [magnitude](steno::Brief b) {
		return "$~" + b | magnitude + steno::Stroke {"-DZ"};
	};
};

auto AsDollars(steno::Brief b) -> steno::Brief {
	return "$~" + b | steno::Stroke{"-DZ"};
};

auto AsCents(steno::Brief b) -> steno::Brief {
	while (b.text.size() < 3) b.text = '0' + b.text; 
	b.text.insert(b.text.end()-2, '.');
	return "$~" + b | steno::Stroke{"-S"};
};

auto OneDollarCents(steno::Brief b) -> steno::Brief {
	while (b.text.size() < 2) b.text = '0' + b.text;
	return steno::Brief {"$1.~", {"TKHRAR"}} | b;
}

auto aDollarCents(steno::Brief b) -> steno::Brief {
	return steno::Stroke {"AEU"} | OneDollarCents(b);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main() {
	steno::Dictionary numbersDict {};

	for (unsigned i=0; i<=999; i++) {
		/*0*/
		if (i == 0) numbersDict.add({"0", {"OE"}});
		/*zero*/
		if (i == 0) numbersDict.add({"zero", {"STKPWRHRAOER"}});
		/*1 2 3*/
		if (1 <= i&&i <= 10) numbersDict.add(Num(i) + steno::Stroke {"*"});
		/*one two three*/
		if (1 <= i&&i <= 10) numbersDict.add(Num(i) | WrittenOut);
		/*00 01 02 03*/
		if (0 <= i&&i <= 9) numbersDict.add(Num(i, true));
		/*123*/
		if (11 <= i&&i <= 999) numbersDict.add(Num(i));
		/*123,000*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Thousand_N);
		/*$123,000*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Dollars(Thousand_N));
		/*$123 million*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Dollars(Million));
		/*$123 billion*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Dollars(Billion));
		/*$123 trillion*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Dollars(Trillion));

		/*$123*/
		if (1 <= i&&i <= 999) numbersDict.add(Num(i) | AsDollars);
		/*$1.23*/
		if (1 <= i&&i <= 999 && i%100) numbersDict.add(Num(i) | AsCents);
		/*$1.02*/
		if (1 <= i&&i <= 9)  numbersDict.add(Num(i, true) | OneDollarCents);
		if (1 <= i&&i <= 9)  numbersDict.add(Num(i, true) | aDollarCents);
		/*$1.23*/
		if (1 <= i&&i <= 99)  numbersDict.add(Num(i) | OneDollarCents);
		if (1 <= i&&i <= 99)  numbersDict.add(Num(i) | aDollarCents);
	}

//	numbersDict.add(Num(1) | Hundred_N);
	numbersDict.genContraction({"WUPB/HUPB"}, {"WHUPB"});
	for (const auto& entry : numbersDict.getEntries()) {
		std::cout << "|" << steno::toString(entry.first) << "| == " << entry.second.text << "\n";
	}
}
