#include "steno.hh"
#include <gtest/gtest.h>
#include <iterator>
#include <concepts>

// Double parentheses required so our '<' isn't parsed as a less-than.
#define EXPECT_SAME_TYPE(T, U) EXPECT_TRUE((std::same_as<T, U>))
#define EXPECT_CONCEPT(C, ...) EXPECT_TRUE((C<__VA_ARGS__>))
#define EXPECT_EXPRESSION(Expression, Type, ... ) {                            \
	EXPECT_SAME_TYPE(Type, decltype(Expression));                              \
	(void) (Expression);                                                       \
	__VA_ARGS__; /*PostCondition*/                                             \
}

/* ~~ Key Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(StenoKey, Addition) {
	steno::Stroke stroke {"PAT"};
	using enum steno::Key;
	EXPECT_EQ(stroke +  _S, steno::Stroke {"PATS"});
	EXPECT_EQ(stroke += _S, steno::Stroke {"PATS"});
	EXPECT_EQ(stroke -  _S, steno::Stroke {"PAT"});
	EXPECT_EQ(stroke -= _S, steno::Stroke {"PAT"});
	EXPECT_EQ(S_ + P_ + R_ + O + U + _T + _S, steno::Stroke {"SPROUTS"});
}

TEST(StenoKey, ToString) {
	EXPECT_EQ(steno::toString(steno::Key::Num), "#");
	EXPECT_EQ(steno::toString(steno::Key::x  ), "*");

	EXPECT_EQ(steno::toString(steno::Key::K_), "K");
	EXPECT_EQ(steno::toString(steno::Key::A ), "A");
	EXPECT_EQ(steno::toString(steno::Key::O ), "O");
	EXPECT_EQ(steno::toString(steno::Key::E ), "E");
	EXPECT_EQ(steno::toString(steno::Key::_Z), "Z");

	EXPECT_EQ(steno::toString(steno::Key::S_), "S");
	EXPECT_EQ(steno::toString(steno::Key::_S), "S");
}

/* ~~ Stroke Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Modeled after https://cppreference.com/w/cpp/utility/bitset

TEST(StenoStroke, EmptyConstruction) {
	steno::Stroke stroke;
	EXPECT_EQ(stroke, steno::NoStroke);
	EXPECT_EQ(steno::Stroke (), steno::NoStroke);
	EXPECT_EQ(steno::Stroke {}, steno::NoStroke);
	EXPECT_EQ(steno::Stroke {"-"}, steno::NoStroke);
	EXPECT_TRUE(steno::Stroke {""}.failed());
	EXPECT_TRUE(steno::Stroke {" "}.failed());
}

TEST(StenoStroke, GoodInputString) {
	// We can't EXPECT_TRUE for the empty stroke, it's treated like 0.
	EXPECT_FALSE(steno::Stroke {"-"});
	EXPECT_FALSE(steno::Stroke {"-"}.failed());
	EXPECT_TRUE(steno::Stroke {"#STKPWHRAO*EUFRPBLGTSDZ"}); // All keys
	EXPECT_TRUE(steno::Stroke {" STKPWHR  -            "}); // Left
	EXPECT_TRUE(steno::Stroke {"        AO*EU          "}); // Middle
	EXPECT_TRUE(steno::Stroke {"          -  FRPBLGTSDZ"}); // Right
	EXPECT_TRUE(steno::Stroke {" S                     "}); // ┬─ S key only
	EXPECT_TRUE(steno::Stroke {" S        -            "}); // ┘
	EXPECT_TRUE(steno::Stroke {"#                      "}); // ┬─ Num. bar only
	EXPECT_TRUE(steno::Stroke {"#         -            "}); // ┘
	EXPECT_TRUE(steno::Stroke {"          *            "}); // ── Asterisk only
	EXPECT_TRUE(steno::Stroke {" S  P  R O  U      TS  "}); // "sprouts"
	EXPECT_TRUE(steno::Stroke {"        A       B      "}); // A B
	EXPECT_TRUE(steno::Stroke {"   K   R  -          D "}); // C D
	EXPECT_TRUE(steno::Stroke {"           E F    G    "}); // E F G
	EXPECT_TRUE(steno::Stroke {"      H    EU  PBLG    "}); // H I J
	EXPECT_TRUE(steno::Stroke {"   K  HR  -    P L     "}); // K L M
	EXPECT_TRUE(steno::Stroke {"  T P H  O     P       "}); // N O P
	EXPECT_TRUE(steno::Stroke {"   K W    -   R     S  "}); // Q R S
	EXPECT_TRUE(steno::Stroke {"  T       * UF         "}); // T U V
	EXPECT_TRUE(steno::Stroke {"     W    -     B G S  "}); // W X
	EXPECT_TRUE(steno::Stroke {"   K W R  -           Z"}); // Y Z
	// Numbers
	EXPECT_TRUE(steno::Stroke {"1"});
	EXPECT_TRUE(steno::Stroke {"2"});
	EXPECT_TRUE(steno::Stroke {"3"});
	EXPECT_TRUE(steno::Stroke {"4"});
	EXPECT_TRUE(steno::Stroke {"5"});
	EXPECT_TRUE(steno::Stroke {"0"});
	EXPECT_TRUE(steno::Stroke {"6"});
	EXPECT_TRUE(steno::Stroke {"7"});
	EXPECT_TRUE(steno::Stroke {"8"});
	EXPECT_TRUE(steno::Stroke {"9"});
	// Optional hyphen for purely numeric input
	EXPECT_TRUE(steno::Stroke {"1-"});
	EXPECT_TRUE(steno::Stroke {"2-"});
	EXPECT_TRUE(steno::Stroke {"3-"});
	EXPECT_TRUE(steno::Stroke {"4-"});
	EXPECT_TRUE(steno::Stroke {"-6"});
	EXPECT_TRUE(steno::Stroke {"-7"});
	EXPECT_TRUE(steno::Stroke {"-8"});
	EXPECT_TRUE(steno::Stroke {"-9"});
	EXPECT_TRUE(steno::Stroke {"1234-6789"});
	// A hash is always allowed at the start of input
	EXPECT_TRUE(steno::Stroke {"#1"});
	EXPECT_TRUE(steno::Stroke {"#2"});
	EXPECT_TRUE(steno::Stroke {"#3"});
	EXPECT_TRUE(steno::Stroke {"#4"});
	EXPECT_TRUE(steno::Stroke {"#5"});
	EXPECT_TRUE(steno::Stroke {"#0"});
	EXPECT_TRUE(steno::Stroke {"#6"});
	EXPECT_TRUE(steno::Stroke {"#7"});
	EXPECT_TRUE(steno::Stroke {"#8"});
	EXPECT_TRUE(steno::Stroke {"#9"});
	EXPECT_TRUE(steno::Stroke {"#1-"});
	EXPECT_TRUE(steno::Stroke {"#2-"});
	EXPECT_TRUE(steno::Stroke {"#3-"});
	EXPECT_TRUE(steno::Stroke {"#4-"});
	EXPECT_TRUE(steno::Stroke {"#-6"});
	EXPECT_TRUE(steno::Stroke {"#-7"});
	EXPECT_TRUE(steno::Stroke {"#-8"});
	EXPECT_TRUE(steno::Stroke {"#-9"});
	// Compound
	EXPECT_TRUE(steno::Stroke {"12HOURS"});
	EXPECT_TRUE(steno::Stroke {"#12HOURS"});
	EXPECT_TRUE(steno::Stroke {"1-TSDZ"});
	EXPECT_TRUE(steno::Stroke {"#1-TSDZ"});
}

TEST(StenoStroke, BadInputString) {
	// Construction from an empty string is probably an error.
	EXPECT_FALSE(steno::Stroke {"--"});
	EXPECT_FALSE(steno::Stroke {"##"});
	EXPECT_FALSE(steno::Stroke {"SS"});
	EXPECT_FALSE(steno::Stroke {"TT"});
	EXPECT_FALSE(steno::Stroke {"RR"});
	EXPECT_FALSE(steno::Stroke {"1TSDZ"});
	EXPECT_FALSE(steno::Stroke {"#1TSDZ"});
	// TODO
}

TEST(StenoStroke, Getters) {
	steno::Stroke stroke {"SPROUTS"};
	EXPECT_EQ(stroke.raw(), 0b01001001010010000001100'000000000);
}

TEST(StenoStroke, RangeFor) {
	steno::Stroke stroke {"SPROUTS"};
	steno::Stroke copy {};
	for (steno::Key k : stroke) copy += k;
	EXPECT_EQ(stroke, copy);
}

TEST(StenoStroke, KeyAccessGet) {
	steno::Stroke stroke {"SPROUTS"};
	using enum steno::Key;
	EXPECT_EQ(
	 	stroke.get(Num) << 22
	|	stroke.get(S_) << 21
	|	stroke.get(T_) << 20    |    stroke.get(K_) << 19
	|	stroke.get(P_) << 18    |    stroke.get(W_) << 17
	|	stroke.get(H_) << 16    |    stroke.get(R_) << 15
	|	stroke.get(A ) << 14    |    stroke.get(O ) << 13
	|	stroke.get(x ) << 12
	|	stroke.get(E ) << 11    |    stroke.get(U ) << 10
	|	stroke.get(_F) <<  9    |    stroke.get(_R) <<  8
	|	stroke.get(_P) <<  7    |    stroke.get(_B) <<  6
	|	stroke.get(_L) <<  5    |    stroke.get(_G) <<  4
	|	stroke.get(_T) <<  3    |    stroke.get(_S) <<  2
	|	stroke.get(_D) <<  1    |    stroke.get(_Z) <<  0,
	//#STKPWHRAO*EUFRPBLGTSDZ
	// S  P  R O  U      TS  
	0b01001001010010000001100);
}

TEST(StenoStroke, KeyAccessBitAnd) {
	steno::Stroke stroke {"SPROUTS"};
	using enum steno::Key;
	EXPECT_EQ(
	 	(stroke & Num) << 22
	|	(stroke & S_) << 21
	|	(stroke & T_) << 20    |    (stroke & K_) << 19
	|	(stroke & P_) << 18    |    (stroke & W_) << 17
	|	(stroke & H_) << 16    |    (stroke & R_) << 15
	|	(stroke & A ) << 14    |    (stroke & O ) << 13
	|	(stroke & x ) << 12
	|	(stroke & E ) << 11    |    (stroke & U ) << 10
	|	(stroke & _F) <<  9    |    (stroke & _R) <<  8
	|	(stroke & _P) <<  7    |    (stroke & _B) <<  6
	|	(stroke & _L) <<  5    |    (stroke & _G) <<  4
	|	(stroke & _T) <<  3    |    (stroke & _S) <<  2
	|	(stroke & _D) <<  1    |    (stroke & _Z) <<  0,
	//#STKPWHRAO*EUFRPBLGTSDZ
	// S  P  R O  U      TS  
	0b01001001010010000001100);
}

TEST(StenoStroke, KeyAccessSubscript) {
	steno::Stroke stroke {"SPROUTS"};
	using enum steno::Key;
	EXPECT_EQ(
	 	stroke[Num] << 22
	|	stroke[S_] << 21
	|	stroke[T_] << 20    |    stroke[K_] << 19
	|	stroke[P_] << 18    |    stroke[W_] << 17
	|	stroke[H_] << 16    |    stroke[R_] << 15
	|	stroke[A ] << 14    |    stroke[O ] << 13
	|	stroke[x ] << 12
	|	stroke[E ] << 11    |    stroke[U ] << 10
	|	stroke[_F] <<  9    |    stroke[_R] <<  8
	|	stroke[_P] <<  7    |    stroke[_B] <<  6
	|	stroke[_L] <<  5    |    stroke[_G] <<  4
	|	stroke[_T] <<  3    |    stroke[_S] <<  2
	|	stroke[_D] <<  1    |    stroke[_Z] <<  0,
	//#STKPWHRAO*EUFRPBLGTSDZ
	// S  P  R O  U      TS  
	0b01001001010010000001100);
}

TEST(StenoStroke, KeyModify) {
	steno::Stroke stroke {};
	using enum steno::Key;
	stroke.set(S_).set(P_).set(R_).set(O).set(U).set(_T).set(_S);
	EXPECT_EQ(stroke, steno::Stroke {"SPROUTS"});
}

TEST(StenoStroke, SubscriptModify) {
	steno::Stroke stroke {};
	using enum steno::Key;
	stroke[S_] = stroke[P_] = stroke[R_] = stroke[O]
	= stroke[U] = stroke[_T] = stroke[_S] = true;
	EXPECT_EQ(stroke, steno::Stroke {"SPROUTS"});
}

TEST(StenoStroke, UnaryNegate) {
	steno::Stroke leftHand {"STKPWHRAO"};
	EXPECT_EQ(  leftHand .raw(), 0b01111111110000000000000'000000000);
	EXPECT_EQ((~leftHand).raw(), 0b10000000001111111111111'000000000);
}

TEST(StenoStroke, Addition) {
	auto two               = steno::Stroke {" T  W   O             "};
	auto dollars           = steno::Stroke {"         -          DZ"};
	EXPECT_EQ(two + dollars, steno::Stroke {" T  W   O           DZ"});

	auto one               = steno::Stroke {"    W      U  PB      "};
	auto hundred           = steno::Stroke {"     H     U  PB      "};
	EXPECT_EQ(one + hundred, steno::Stroke {"    WH     U  PB      "});

	EXPECT_EQ( steno::NoStroke +  steno::NoStroke,  steno::NoStroke);
	EXPECT_EQ( steno::NoStroke + ~steno::NoStroke, ~steno::NoStroke);
	EXPECT_EQ(~steno::NoStroke +  steno::NoStroke, ~steno::NoStroke);
	EXPECT_EQ(~steno::NoStroke + ~steno::NoStroke, ~steno::NoStroke);

	steno::Stroke test {};
	EXPECT_EQ(test += steno::Stroke {"-" }, steno::Stroke {"-"});
	EXPECT_EQ(test += steno::Stroke {"T-"}, steno::Stroke {"T-"});
	EXPECT_EQ(test += steno::Stroke {"E" }, steno::Stroke {"TE"});
	EXPECT_EQ(test += steno::Stroke {"-S"}, steno::Stroke {"TES"});
	EXPECT_EQ(test += steno::Stroke {"*" }, steno::Stroke {"T*ES"});

	// TODO: Chaining
}

TEST(StenoStroke, Subtraction) {
	steno::Stroke stroke {"SPROUTS"};
	EXPECT_EQ(stroke - steno::Stroke {"#"}, stroke);
	EXPECT_EQ(stroke - steno::Stroke {"-"}, steno::Stroke {"SPROUTS"});
	EXPECT_EQ(stroke - steno::Stroke {"-S"}, steno::Stroke {"SPROUT"});
	EXPECT_EQ(stroke - steno::Stroke {"SPR-S"}, steno::Stroke {"OUT"});

	steno::Stroke test {"T*ES"};
	EXPECT_EQ(test -= steno::Stroke {"-" }, steno::Stroke {"T*ES"});
	EXPECT_EQ(test -= steno::Stroke {"T-"}, steno::Stroke {"*ES"});
	EXPECT_EQ(test -= steno::Stroke {"E" }, steno::Stroke {"*S"});
	EXPECT_EQ(test -= steno::Stroke {"-S"}, steno::Stroke {"*"});
	EXPECT_EQ(test -= steno::Stroke {"*" }, steno::Stroke {"-"});
}

TEST(StenoStroke, BitAnd) {
	steno::Stroke stroke {"SPROUTS"};
	steno::Stroke const LeftHand  {"STKPWHRAO"};
	steno::Stroke const RightHand {"*EUFRPBLGTSDZ"};
	EXPECT_EQ(stroke & LeftHand , steno::Stroke {"SPRO"});
	EXPECT_EQ(stroke & RightHand, steno::Stroke {"UTS"});

	steno::Stroke shouldYouHave {"SHAO*UF"};
	steno::Stroke const Vowel {"AOEU"};
	steno::Stroke const Consonant {"STKPWHR*FRPBLGTSDZ"};
	EXPECT_EQ(shouldYouHave & LeftHand             , steno::Stroke{"SHAO"});
	EXPECT_EQ(shouldYouHave & RightHand & Vowel    , steno::Stroke{"U"});
	EXPECT_EQ(shouldYouHave & RightHand & Consonant, steno::Stroke{"*F"});

	EXPECT_EQ(stroke &= ~steno::NoStroke, stroke);
	EXPECT_EQ(stroke &= steno::NoStroke, steno::NoStroke);
	EXPECT_EQ(stroke, steno::NoStroke);
}

TEST(StenoStroke, BitXor) {
	steno::Stroke stroke1 {"SPROUTS"};
	steno::Stroke stroke2 {"KPAFRPL"};
	EXPECT_EQ(stroke1 ^ stroke1, steno::NoStroke);
	EXPECT_EQ(stroke2 ^ stroke2, steno::NoStroke);
	EXPECT_EQ(stroke1 ^ steno::NoStroke, stroke1);
	EXPECT_EQ(stroke1 ^ stroke2, stroke2 ^ stroke1);

	steno::Stroke const rat  {"RA  T"};
	steno::Stroke const rate {"RAEUT"};
	steno::Stroke stroke {rat};
	EXPECT_EQ(stroke ^= steno::Stroke {"EU"}, rate);
	EXPECT_EQ(stroke ^= steno::Stroke {"EU"}, rat);
}

#include <map>
#include <unordered_map>
TEST(StenoStroke, UseWithMaps) {
	steno::Stroke const array[] = {{"A"}, {"A"}, {"-B"}, {"A"}};

	std::map<steno::Stroke, int> count;
	for (auto stroke : array) count[stroke]++;
	EXPECT_EQ(count[{"A" }], 3);
	EXPECT_EQ(count[{"-B"}], 1);
	EXPECT_EQ(count[{"KR"}], 0);

	std::unordered_map<steno::Stroke, bool> seen;
	for (auto stroke : array) seen[stroke] = true;
	EXPECT_EQ(seen[{"A" }], true);
	EXPECT_EQ(seen[{"-B"}], true);
	EXPECT_EQ(seen[{"KR"}], false);
}

#include <sstream>
TEST(StenoStroke, ToString) {
	steno::Stroke array[] = {
		{"         *EU          "},
		{"                8     "},
		{"S          U  P L     "},
		{"   P      EU          "},
		{"S KP                  "},
		{"S  P  R  O U      TS  "},
	};
	EXPECT_EQ(steno::toString(array[0]), "*EU");     // I
	EXPECT_EQ(steno::toString(array[1]), "8");       // 8
	EXPECT_EQ(steno::toString(array[2]), "SUPL");    // sum
	EXPECT_EQ(steno::toString(array[3]), "PEU");     // pi
	EXPECT_EQ(steno::toString(array[4]), "SKP-");    // and
	EXPECT_EQ(steno::toString(array[5]), "SPROUTS"); // sprouts

	steno::Stroke stroke {"12HOURS"};
	using enum steno::Format;
	EXPECT_EQ(steno::toString(stroke, Packed|Alpha), "#STHOURS");
	EXPECT_EQ(steno::toString(stroke, Numeric|Wide), " 12   4  0  U R     S  ");
	
	std::stringstream ss {};
	ss << Alpha << Wide << stroke;
	EXPECT_EQ(ss.str(), "#ST   H  O  U R     S  ");

	ss = std::stringstream {};
	ss << Packed << Numeric << stroke;
	EXPECT_EQ(ss.str(), "1240URS");
}

/* ~~ Phrase Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Modeled after https://en.cppreference.com/w/cpp/named_req/SequenceContainer

TEST(StenoPhrase, StrokeModify) {
	steno::Phrase p1 {"PHAOUT/ABL"};
	EXPECT_EQ(p1[0], steno::Stroke {"   P H AO  U      T   "});
	EXPECT_EQ(p1[1], steno::Stroke {"       A       BL     "});

	p1[1] = {"AEUTD"};
	EXPECT_EQ(p1[0], steno::Stroke {"   P H AO  U      T   "});
	EXPECT_EQ(p1[1], steno::Stroke {"       A  EU      T D "});

	steno::Phrase const p2 {"KOPB/STAPBT"};
	EXPECT_EQ(p2[0], steno::Stroke {"  K     O     PB      "});
	EXPECT_EQ(p2[1], steno::Stroke {"ST     A      PB  T   "});
}

TEST(StenoPhrase, Concatenation) {
	using S = steno::Stroke;
	using P = steno::Phrase;
	EXPECT_EQ(S("1") | S("2") | S("3"), P("1/2/3"));
	EXPECT_EQ(P("1") | P("2") | P("3"), P("1/2/3"));
	EXPECT_EQ(P("1   /    2") | S("3"), P("1/2/3"));
	EXPECT_EQ(S("1") | P("2   /    3"), P("1/2/3"));
}

#include <map>
#include <unordered_map>
TEST(StenoPhrase, UseWithMaps) {
	steno::Phrase const array[] = {{"1"}, {"1"}, {"2"}, {"1"}};

	std::map<steno::Phrase, int> count;
	for (auto phrase : array) count[phrase]++;
	EXPECT_EQ(count[{"1"}], 3);
	EXPECT_EQ(count[{"2"}], 1);
	EXPECT_EQ(count[{"3"}], 0);

	std::unordered_map<steno::Phrase, bool> seen;
	for (auto phrase : array) seen[phrase] = true;
	EXPECT_EQ(seen[{"1"}], true);
	EXPECT_EQ(seen[{"2"}], true);
	EXPECT_EQ(seen[{"3"}], false);
}

TEST(StenoPhrase, ToString) {
	steno::Phrase phrase {"#O/#S-/#T-/#P-/#H-/#A/#-F/#-P/#-L/#-T"};
	EXPECT_EQ(steno::toString(phrase), "0/1/2/3/4/5/6/7/8/9");
}

TEST(StenoPhrase, ContainerTypes) {
	using C  = steno::Phrase;
	using T  = steno::Stroke;
	using I  = C::iterator;
	using IC = C::const_iterator;
	using I_Traits  = std::iterator_traits<I>;
	using IC_Traits = std::iterator_traits<IC>;
	// Container
	EXPECT_SAME_TYPE(C::value_type     , T             );
	EXPECT_SAME_TYPE(C::reference      , T&            );
	EXPECT_SAME_TYPE(C::const_reference, T const&      );
	EXPECT_SAME_TYPE(C::difference_type, std::ptrdiff_t);
	EXPECT_SAME_TYPE(C::size_type      , std::size_t   );
	// Iterator
	EXPECT_CONCEPT(std::contiguous_iterator, I);
	EXPECT_CONCEPT(std::contiguous_iterator, IC);
	EXPECT_CONCEPT(std::convertible_to, I, IC);
	EXPECT_SAME_TYPE(I_Traits::value_type, T);
	EXPECT_SAME_TYPE(IC_Traits::value_type, T);
	EXPECT_SAME_TYPE(I_Traits::difference_type, C::difference_type);
	EXPECT_SAME_TYPE(IC_Traits::difference_type, C::difference_type);
}

TEST(StenoPhrase, ContainerStatements) {
	using C = steno::Phrase;
	{
		C a;
		C b = C();
		EXPECT_TRUE(a.empty());
		EXPECT_TRUE(b.empty());
		EXPECT_EQ(a, steno::NoPhrase);
		EXPECT_EQ(b, steno::NoPhrase);
	} {
		auto v = C {"STEPB/OE"};
		C a(v);
		C b = C(v);
		EXPECT_EQ(a, v);
		EXPECT_EQ(b, v);
	}
}

TEST(StenoPhrase, ContainerExpressions) {
	using C = steno::Phrase;
	auto v = C {"STEPB/OE"};
	auto lhs = C {};
	EXPECT_EXPRESSION(C()    , C , EXPECT_TRUE(C().empty()));
	EXPECT_EXPRESSION(C(v)   , C , EXPECT_EQ(C(v), v)      );
	EXPECT_EXPRESSION(lhs = v, C&, EXPECT_EQ(lhs, v)       );

	auto       mv = C {"PHAOUT/ABL"};
	auto const cv = C {"KOPB/STAPBT"};
	EXPECT_EXPRESSION(mv.begin(), C::iterator      );
	EXPECT_EXPRESSION(cv.begin(), C::const_iterator);
	EXPECT_EXPRESSION(mv.end()  , C::iterator      );
	EXPECT_EXPRESSION(cv.end()  , C::const_iterator);
	EXPECT_EXPRESSION(v.cbegin(), C::const_iterator);
	EXPECT_EXPRESSION(v.cend()  , C::const_iterator);
	{
		auto u = C {"U"};
		auto v = C {"SR"};
		EXPECT_EXPRESSION(u == v, bool);
		EXPECT_EXPRESSION(u != v, bool);

		auto const one = steno::Phrase {"WUPB"};
		auto const two = steno::Phrase {"TWO"};
		auto lhs = one;
		auto rhs = two;
		EXPECT_EXPRESSION(lhs.swap(rhs), void,
			EXPECT_EQ(lhs, two); EXPECT_EQ(rhs, one)
		);
		EXPECT_EXPRESSION(std::swap(lhs, rhs), void,
			EXPECT_EQ(lhs, one); EXPECT_EQ(rhs, two)
		);
		// std::erase[_if] will not work, instead we provide global functions.
		EXPECT_EXPRESSION(erase(v, steno::NoStroke), C::size_type);
		EXPECT_EXPRESSION(erase_if(v, [] ( ... ) { return 1; }), C::size_type);
	}
	EXPECT_EXPRESSION(v.size()    , C::size_type);
	EXPECT_EXPRESSION(v.max_size(), C::size_type);
	EXPECT_EXPRESSION(v.empty()   , bool        );
}

#include <list>
TEST(StenoPhrase, SequenceStatements) {
	using C = steno::Phrase;
	for (C::size_type n : {0, 1, 10, 10000}) {
		auto const t = C::value_type {"PWHRA*PBG"};
		C c(n, t);
		EXPECT_EQ(std::distance(c.begin(), c.end()), n);
	} {
		std::list<steno::Stroke> const foreign {
			{" TK       EUFR   G    "}, // "differing"
			{"          EU      T   "}, // "iterator"
			{" T     AO EU  P    S  "}, // "types"
		};
		auto i = foreign.begin();
		auto j = foreign.end();
		C c(i, j);
		EXPECT_EQ(std::distance(c.begin(), c.end()), std::distance(i, j));
	}
}

TEST(StenoPhrase, SequenceExpressions) {
	using C = steno::Phrase;
	auto v = C {"STEPB/OE/TPRAEUZ"};
	auto const cv = C {"KO*PBS/TPRAEUZ"};
	auto i = cv.begin();
	auto j = cv.end();
	auto il = std::initializer_list<C::value_type> {{"1"}, {"2"}, {"3"}};
	auto n = C::size_type {2};
	auto t = C::value_type {"STROEBG"};
	EXPECT_EXPRESSION(C(il) , C , EXPECT_EQ(C(il), C(il.begin(), il.end())));
	EXPECT_EXPRESSION(v = il, C&, EXPECT_EQ(v, C(il)));
	C::const_iterator p, q, q1, q2;
	auto pq = [&] { q = v.begin(); p = v.end(); q1 = q; q2 = q+1; };
	pq(); EXPECT_EXPRESSION(v.emplace(p, "123"), C::iterator);
	pq(); EXPECT_EXPRESSION(v.insert(p, t)     , C::iterator);
	pq(); EXPECT_EXPRESSION(v.insert(p, n, t)  , C::iterator);
	pq(); EXPECT_EXPRESSION(v.insert(p, i, j)  , C::iterator);
	pq(); EXPECT_EXPRESSION(v.insert(p, il)    , C::iterator);
	pq(); EXPECT_EXPRESSION(v.erase(q)         , C::iterator);
	pq(); EXPECT_EXPRESSION(v.erase(q1, q2)    , C::iterator);
	EXPECT_EXPRESSION(i <=> j, std::strong_ordering);
	EXPECT_EXPRESSION(v.clear()     , void, EXPECT_TRUE(v.empty()   ));
	EXPECT_EXPRESSION(v.assign(i, j), void, EXPECT_TRUE(v == C(i, j)));
	EXPECT_EXPRESSION(v.assign(il)  , void, EXPECT_TRUE(v == C(il)  ));
	EXPECT_EXPRESSION(v.assign(n, t), void, EXPECT_TRUE(v == C(n, t)));
	// Vector specific
	EXPECT_EXPRESSION(v .front(), C::reference);
	EXPECT_EXPRESSION(cv.front(), C::const_reference);
	EXPECT_EXPRESSION(v .back() , C::reference);
	EXPECT_EXPRESSION(cv.back() , C::const_reference);
	EXPECT_EXPRESSION(v.emplace_back("123"), void);
	EXPECT_EXPRESSION(v.push_back(t)       , void);
	EXPECT_EXPRESSION(v.pop_back()         , void);
	EXPECT_EXPRESSION(v [n], C::reference);
	EXPECT_EXPRESSION(cv[n], C::const_reference);
	EXPECT_EXPRESSION(v .at(C::size_type {0}), C::reference);
	EXPECT_EXPRESSION(cv.at(C::size_type {0}), C::const_reference);
	EXPECT_THROW(v .at(v .size()), std::out_of_range);
	EXPECT_THROW(cv.at(cv.size()), std::out_of_range);
}

/* ~~ Brief Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(StenoBrief, Construction) {
	steno::Brief apple1 {{"AP/EL"}, "apple"};
	EXPECT_EQ(apple1.phrase(), steno::Phrase {"AP/EL"});
	EXPECT_EQ(apple1.text(), "apple");

	steno::Brief apple2 {apple1, "Apple ]["};
	EXPECT_EQ(apple2.phrase(), steno::Phrase {"AP/EL"});
	EXPECT_EQ(apple2.text(), "Apple ][");
}

TEST(StenoBrief, Getters) {
	steno::Brief brief {{"1/2"}, "one, two"};
	EXPECT_EQ(brief.phrase(), steno::Phrase {"1/2"});
	EXPECT_EQ(brief.text(), "one, two");

	brief.phrase() |= steno::Stroke {"3"};
	brief.text() += ", three";
	EXPECT_EQ(brief.phrase(), steno::Phrase {"1/2/3"});
	EXPECT_EQ(brief.text(), "one, two, three");

	brief |= steno::Brief {{"4"}, ", four"};
//	brief |= steno::Brief {{"4"}, "{,}four"};
	EXPECT_EQ(brief.phrase(), steno::Phrase {"1/2/3/4"});
	EXPECT_EQ(brief.text(), "one, two, three, four");

	using enum steno::Key;
	steno::Brief const ab {{"A/-B"}, "\tayy bee\t"};
	EXPECT_TRUE(ab.phrase()[0][A]);
	EXPECT_TRUE(ab.phrase()[1][_B]);
	EXPECT_NE(ab.text(), "ayy bee");
}

TEST(StenoBrief, StructuredBinding) {
	steno::Brief brief {{"AP/EL"}, "apple"};
	  [[maybe_unused]] auto        [strokes, text] = brief;  
	{ [[maybe_unused]] auto      & [strokes, text] = brief; }
	{ [[maybe_unused]] auto const  [strokes, text] = brief; }
	{ [[maybe_unused]] auto const& [strokes, text] = brief; }
	EXPECT_EQ(strokes.size(), 2);
	EXPECT_EQ(strokes[0], steno::Stroke {"AP"});
	EXPECT_EQ(strokes[1], steno::Stroke {"EL"});
	EXPECT_EQ(text, "apple");
}

/* ~~ Brief Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Modeled after https://en.cppreference.com/w/cpp/named_req/AssociativeContainer

TEST(StenoDictionary, ContainerTypes) {
	using C  = steno::Dictionary;
	using T  = std::map<steno::Phrase, std::string>::value_type; // !
	using I  = C::iterator;
	using IC = C::const_iterator;
	using I_Traits  = std::iterator_traits<I>;
	using IC_Traits = std::iterator_traits<IC>;
	// Container
	EXPECT_SAME_TYPE(C::value_type     , T             );
	EXPECT_SAME_TYPE(C::reference      , T&            );
	EXPECT_SAME_TYPE(C::const_reference, T const&      );
	EXPECT_SAME_TYPE(C::difference_type, std::ptrdiff_t);
	EXPECT_SAME_TYPE(C::size_type      , std::size_t   );
	// Iterator
	EXPECT_CONCEPT(std::forward_iterator, I);
	EXPECT_CONCEPT(std::forward_iterator, IC);
	EXPECT_CONCEPT(std::convertible_to, I, IC);
	EXPECT_SAME_TYPE(I_Traits::value_type, T);
	EXPECT_SAME_TYPE(IC_Traits::value_type, T);
	EXPECT_SAME_TYPE(I_Traits::difference_type, C::difference_type);
	EXPECT_SAME_TYPE(IC_Traits::difference_type, C::difference_type);
}

TEST(StenoDictionary, ContainerStatements) {
	using C = steno::Dictionary;
	{
		C a;
		C b = C();
		EXPECT_TRUE(a.empty());
		EXPECT_TRUE(b.empty());
	} {
		auto v = C {{{"1"}, "one"}, {{"2"}, "two"}};
		C a(v);
		C b = C(v);
		EXPECT_EQ(a, v);
		EXPECT_EQ(b, v);
	}
}

TEST(StenoDictionary, ContainerExpressions) {
	using C = steno::Dictionary;
	auto v = C {{{"1"}, "one"}, {{"2"}, "two"}};
	auto lhs = C {};
	EXPECT_EXPRESSION(C()    , C , EXPECT_TRUE(C().empty()));
	EXPECT_EXPRESSION(C(v)   , C , EXPECT_EQ(C(v), v)      );
	EXPECT_EXPRESSION(lhs = v, C&, EXPECT_EQ(lhs, v)       );

	auto       mv = C {{{"PH"}, "M"}, {{"SR"}, "V"}};
	auto const cv = C {{{"KR"}, "C"}, {{"SR"}, "V"}};
	EXPECT_EXPRESSION(mv.begin(), C::iterator      );
	EXPECT_EXPRESSION(cv.begin(), C::const_iterator);
	EXPECT_EXPRESSION(mv.end()  , C::iterator      );
	EXPECT_EXPRESSION(cv.end()  , C::const_iterator);
	EXPECT_EXPRESSION(v.cbegin(), C::const_iterator);
	EXPECT_EXPRESSION(v.cend()  , C::const_iterator);
	{
		auto u = C {{{"1*"}, "first"}};
		auto v = C {{{"2*"}, "second"}};
		EXPECT_EXPRESSION(u == v, bool);
		EXPECT_EXPRESSION(u != v, bool);

		auto const ayy = steno::Dictionary {{{" A "}, "a"}};
		auto const bee = steno::Dictionary {{{"-B "}, "b"}};
		auto lhs = ayy;
		auto rhs = bee;
		EXPECT_EXPRESSION(lhs.swap(rhs), void,
			EXPECT_EQ(lhs, bee); EXPECT_EQ(rhs, ayy)
		);
		EXPECT_EXPRESSION(std::swap(lhs, rhs), void,
			EXPECT_EQ(lhs, ayy); EXPECT_EQ(rhs, bee)
		);
	}
	EXPECT_EXPRESSION(v.size()    , C::size_type);
	EXPECT_EXPRESSION(v.max_size(), C::size_type);
	EXPECT_EXPRESSION(v.empty()   , bool        );
}
