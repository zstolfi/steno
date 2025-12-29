#include "steno.hh"
#include <format>
using namespace steno;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Brief const And      {{"S KP     -            "}, "and"      };
Brief const Hundred  {{"     H     U  PB      "}, "00"       };
Brief const Thousand {{" T   H  O  U          "}, ",000"     };
Brief const Million  {{"   P H    EU    L     "}, " million" };
Brief const Billion  {{"   PWHR O     PB      "}, " billion" };
Brief const Trillion {{" T    R   EU    L     "}, " trillion"};

Stroke const N_Start[20] = {
	/* 0*/ {"        O E           "},
	/* 1*/ {"    W      U  PB      "},
	/* 2*/ {" T  W   O             "},
	/* 3*/ {" T   HRAO E           "},
	/* 4*/ {" T P    O E  R        "},
	/* 5*/ {" T P      EUF         "},
	/* 6*/ {"S         EU   B G S  "},
	/* 7*/ {"S         E F         "},
	/* 8*/ {"       A  E       T   "},
	/* 9*/ {" T P H    EU  PB      "},
	/*10*/ {" T        E   PB      "},
	/*11*/ {"     HR   E F         "},
	/*12*/ {" T  W     E F   L     "},
	/*13*/ {" T   H    EU R    T   "},
	/*14*/ {" T P    O E  R    T   "},
	/*15*/ {" T P      EUF     T   "},
	/*16*/ {"S         EU   B GT   "},
	/*17*/ {"S         E F     T   "},
	/*18*/ {"  K W RA  E       T   "},
	/*19*/ {" T P H A  E       T   "},
};

Stroke const N_Tens[10] = {
	/*0~*/ And.phrase()[0],
	/*1~*/ NoStroke,
	/*2~*/ {" T PW    -            "},
	/*3~*/ {" T   HR  -            "},
	/*4~*/ {" T P  R  -            "},
	/*5~*/ {" TK W    -            "},
	/*6~*/ {"S K      -            "},
	/*7~*/ {"S     R  -            "},
	/*8~*/ {"  K W R  -            "},
	/*9~*/ {" T P H   *            "},
};

Stroke const N_Ones[10] = {
	/*~0*/ {"       AO EU          "},
	/*~1*/ {"           U  PB      "},
	/*~2*/ {"       AO             "},
	/*~3*/ {"       A  E           "},
	/*~4*/ {"        O E  R        "},
	/*~5*/ {"          EUF         "},
	/*~6*/ {"          EU   B G    "},
	/*~7*/ {"          E F P       "},
	/*~8*/ {"       A  EU          "},
	/*~9*/ {"       AO EU  PB      "},
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Brief Num(unsigned x, bool recursed=false) {
	auto const asString = std::format("{}", x);
	if (x < 20 && !(x < 10 && recursed)) return {N_Start[x], asString};
	if (x < 100) return {N_Tens[x/10] + N_Ones[x%10], asString};
	if (x < 1000) {
		return (x%100 == 0)
		?	Num(x/100) | Hundred
		:	Num(x/100) | Num(x%100, true);
	}
	return NoBrief;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main() {
	Dictionary numbers {};
	for (int i=0; i<=2359; i++) {
		int const x = i;
		int const h = i/100, m = i%100;
//		if (x <= 999) numbers.insert(Num(x));
		if ((1 <= h&&h <= 12) && (0 <= m&&m <= 59)) {
			Brief time = Num(h) | Num(m, true);
			time.phrase()[1].set(Key::x);
			time.text() = std::format("{}:{:0>2}", h, m);
			numbers.insert(time);
		}
	}
	for (auto const& entry : numbers) {
		std::cout << entry.phrase() << "  =  " << entry.text() << "\n";
	}
}
