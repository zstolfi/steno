#include "../steno.hh"
#include <iostream>
#include <string>
#include <array>
#include <sstream>

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

const auto Hundred  = steno::Brief {"~00"     , {"     H     U  PB      "}};
const auto Thousand = steno::Brief {"~,000"   , {" T   H  O  U          "}};
const auto Million  = steno::Brief {"million" , {"   P H    EU    L     "}};
const auto Billion  = steno::Brief {"billion" , {"   PWHR O     PB      "}};
const auto Trillion = steno::Brief {"trillion", {" T    R   EU    L     "}};

const auto WrittenNumbers = std::array {
	"zero", "one", "two", "three", "four",
	"five", "six", "seven", "eight", "nine", "ten"
};

const auto Decimal = steno::Brief {"~.~", {" R- R"}};
const auto Colon   = steno::Brief {"~:~", {"HR-FR"}};
const auto AM      = steno::Brief {"a.m.", {"A*PL"}};
const auto PM      = steno::Brief {"p.m.", {"P*PL"}};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

auto Num(unsigned x, bool recursed=false) -> steno::Brief {
	if (x < 20 && !(x < 10 && recursed)) return N_Start[x];
	if (x < 100) return N_Tens[x/10] + N_Ones[x%10];
	if (x < 1000) {
		return (x%100 == 0)
		?	Num(x/100) | Hundred
		:	Num(x/100) | steno::Glue + Num(x%100, true);
	}
	return steno::NoBrief;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

auto WrittenOut(steno::Brief b) -> steno::Brief {
	unsigned n {};
	std::istringstream {b.text} >> n;
	if (n < WrittenNumbers.size()) b.text = WrittenNumbers[n];
	return b;
};

auto Pad(unsigned len) {
	return [len](steno::Brief b) {
		while (b.text.size() < len) b.text = '0' + b.text;
		return b;
	};
}

auto Dollars(steno::Brief b) {
	return "$~" + b | steno::Stroke {"-DZ"};
};

auto Cents(steno::Brief b) -> steno::Brief {
	b += Pad(3);
	b.text.insert(b.text.end()-2, '.');
	return "$~" + b | steno::Stroke{"-S"};
};

auto OneDollarCents(steno::Brief b) -> steno::Brief {
	return steno::Brief {"$1.~", {"TKHRAR"}} | b + Pad(2);
}

auto aDollarCents(steno::Brief b) -> steno::Brief {
	return steno::Stroke {"AEU"} | OneDollarCents(b);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main() {
	steno::Dictionary numbersDict {};

	for (unsigned i=0; i<=12'59; i++) {
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
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Thousand);
		/*$1200*/
		if (10 <= i&&i <= 99 && i%10) numbersDict.add(Num(i) | Hundred | Dollars);
		/*$123,000*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Thousand | Dollars);
		/*$123 million*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Million | Dollars);
		/*$123 billion*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Billion | Dollars);
		/*$123 trillion*/
		if (1 <= i&&i <= 100) numbersDict.add(Num(i) | Trillion | Dollars);

		/*,123*/
		if (1 <= i&&i <= 999) numbersDict.add(steno::Brief {"~,~", {"THOU"}} | Num(i) + Pad(3));
		/*$123*/
		if (1 <= i&&i <= 999) numbersDict.add(Num(i) | Dollars);
		/*$1.23*/
		if (1 <= i&&i <= 999 && i%100) numbersDict.add(Num(i) | Cents);
		/*$1.02*/
		if (1 <= i&&i <= 9)  numbersDict.add(Num(i, true) | OneDollarCents);
		if (1 <= i&&i <= 9)  numbersDict.add(Num(i, true) | aDollarCents);
		/*$1.23*/
		if (1 <= i&&i <= 99)  numbersDict.add(Num(i) | OneDollarCents);
		if (1 <= i&&i <= 99)  numbersDict.add(Num(i) | aDollarCents);

		if (auto h=i/100, m=i%100; 1 <= h&&h <= 12 && 00 <= m&&m <= 59) {
			/*1:23*/
			numbersDict.add(Num(h) | Colon | Num(m) + Pad(2));
			/*1:23 a.m.*/
			numbersDict.add(Num(h) + "~:~" | Num(m) + Pad(2) | AM);
			/*1:23 p.m.*/
			numbersDict.add(Num(h) + "~:~" | Num(m) + Pad(2) | PM);
		}
	}

	numbersDict.genContraction({"WUPB/HUPB"}, {"WHUPB"});
	numbersDict.genContraction({"HUPB/-DZ"}, {"HUPBDZ"});
	numbersDict.genContraction({"THOU/-DZ"}, {"THOUDZ"});
	numbersDict.genContraction({"PHEUL/-DZ"}, {"PHEULDZ"});
	numbersDict.genContraction({"PWHROPB/-DZ"}, {"PWHR-PBDZ"}); // Drop the O to avoid "blonds".
	numbersDict.genContraction({"TREUL/-DZ"}, {"TREULDZ"});
	numbersDict.genContraction({"WUPB/HUPB/-DZ"}, {"WHUPBDZ"});
	
	for (const auto& entry : numbersDict.getEntries()) {
		std::cout << "|" << steno::toString(entry.first) << "| == " << entry.second.text << "\n";
	}
}
