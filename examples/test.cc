#include "steno.hh"
#include <gtest/gtest.h>

/* ~~ Stroke Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Modeled after https://cppreference.com/w/cpp/utility/bitset

TEST(StenoStroke, EmptyConstruction) {
	steno::Stroke stroke;
	EXPECT_EQ(stroke, steno::NoStroke);
	EXPECT_EQ(steno::Stroke (), steno::NoStroke);
	EXPECT_EQ(steno::Stroke {}, steno::NoStroke);
	EXPECT_EQ(steno::Stroke {"-"}, steno::NoStroke);
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
	// Compound
	EXPECT_TRUE(steno::Stroke {"12HOURS"});
}

TEST(StenoStroke, BadInputString) {
	// Construction from an empty string is probably an error.
	EXPECT_TRUE(steno::Stroke {""}.failed());
	EXPECT_TRUE(steno::Stroke {" "}.failed());
	EXPECT_TRUE(steno::Stroke {"--"}.failed());
	EXPECT_TRUE(steno::Stroke {"##"}.failed());
	EXPECT_TRUE(steno::Stroke {"SS"}.failed());
	EXPECT_TRUE(steno::Stroke {"TT"}.failed());
	EXPECT_TRUE(steno::Stroke {"RR"}.failed());
	// TODO
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
	EXPECT_EQ(  leftHand .getBits(), 0b01111111110000000000000'000000000);
	EXPECT_EQ((~leftHand).getBits(), 0b10000000001111111111111'000000000);
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

/* ~~ Phrase Tests ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Modeled after https://en.cppreference.com/w/cpp/named_req/SequenceContainer

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

// Double parentheses required so our '<' isn't parsed as a less-than.
#define EXPECT_SAME_TYPE(T, U) EXPECT_TRUE((std::same_as<T, U>))
#define EXPECT_CONCEPT(C, ...) EXPECT_TRUE((C<__VA_ARGS__>))

#include <iterator>
#include <concepts>
TEST(StenoPhrase, ContainerTypes) {
	using C  = steno::Phrase;
	using T  = steno::Stroke;
	using I  = steno::Phrase::iterator;
	using IC = steno::Phrase::const_iterator;
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

#define EXPECT_EXPRESSION(Expression, Type, ... ) {                            \
	EXPECT_SAME_TYPE(Type, decltype(Expression));                              \
	(void) (Expression);                                                       \
	__VA_ARGS__; /*PostCondition*/                                             \
}

#pragma clang diagnostic push
TEST(StenoPhrase, ContainerExpressions) {
	using C = steno::Phrase;
	auto v = C {"STEPB/OE"};
	auto lhs = C {};
#	pragma clang diagnostic ignored "-Wvexing-parse"
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
		auto i = v.begin();
		auto j = v.end();
		EXPECT_EXPRESSION(i <=> j, std::strong_ordering);
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
	}
	EXPECT_EXPRESSION(v.size()    , C::size_type);
	EXPECT_EXPRESSION(v.max_size(), C::size_type);
	EXPECT_EXPRESSION(v.empty()   , bool        );
}
#pragma clang diagnostic pop

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
