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
	// We can't EXPECT_TRUE for the empty stroke, it's treated like (bool)0.
	EXPECT_FALSE(steno::Stroke {"-"}.failed());
	EXPECT_TRUE(steno::Stroke {"#STKPWHRAO*EUFRPBLGTSDZ"}); // All keys
	EXPECT_TRUE(steno::Stroke {"   KP   A    FRP L     "}); // "example"
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
}

TEST(StenoStroke, BadInputString) {
	// Construction from an empty string is probably an error.
	EXPECT_TRUE(steno::Stroke {""}.failed());
	EXPECT_TRUE(steno::Stroke {" "}.failed());
	EXPECT_TRUE(steno::Stroke {"--"}.failed());
	EXPECT_TRUE(steno::Stroke {"##"}.failed());
	// TODO
}

TEST(StenoStroke, KeyAccess) {
	steno::Stroke stroke {"SPROUTS"};
	using enum steno::Key;
	EXPECT_EQ(stroke.get(Num), 0);
	EXPECT_EQ(stroke.get(S_), 1);
	EXPECT_EQ(stroke.get(T_), 0); EXPECT_EQ(stroke.get(K_), 0);
	EXPECT_EQ(stroke.get(P_), 1); EXPECT_EQ(stroke.get(W_), 0);
	EXPECT_EQ(stroke.get(H_), 0); EXPECT_EQ(stroke.get(R_), 1);
	EXPECT_EQ(stroke.get(A ), 0); EXPECT_EQ(stroke.get(O ), 1);
	EXPECT_EQ(stroke.get(x ), 0);
	EXPECT_EQ(stroke.get(E ), 0); EXPECT_EQ(stroke.get(U ), 1);
	EXPECT_EQ(stroke.get(_F), 0); EXPECT_EQ(stroke.get(_R), 0);
	EXPECT_EQ(stroke.get(_P), 0); EXPECT_EQ(stroke.get(_B), 0);
	EXPECT_EQ(stroke.get(_L), 0); EXPECT_EQ(stroke.get(_G), 0);
	EXPECT_EQ(stroke.get(_T), 1); EXPECT_EQ(stroke.get(_S), 1);
	EXPECT_EQ(stroke.get(_D), 0); EXPECT_EQ(stroke.get(_Z), 0);
}

TEST(StenoStroke, KeyUnitAccess) {
	steno::Stroke stroke {"SPROUTS"};
	using namespace steno::KeyUnit;
	EXPECT_EQ(stroke & Num, 0);
	EXPECT_EQ(stroke & S_, 1);
	EXPECT_EQ(stroke & T_, 0); EXPECT_EQ(stroke & K_, 0);
	EXPECT_EQ(stroke & P_, 1); EXPECT_EQ(stroke & W_, 0);
	EXPECT_EQ(stroke & H_, 0); EXPECT_EQ(stroke & R_, 1);
	EXPECT_EQ(stroke & A , 0); EXPECT_EQ(stroke & O , 1);
	EXPECT_EQ(stroke & x , 0);
	EXPECT_EQ(stroke & E , 0); EXPECT_EQ(stroke & U , 1);
	EXPECT_EQ(stroke & _F, 0); EXPECT_EQ(stroke & _R, 0);
	EXPECT_EQ(stroke & _P, 0); EXPECT_EQ(stroke & _B, 0);
	EXPECT_EQ(stroke & _L, 0); EXPECT_EQ(stroke & _G, 0);
	EXPECT_EQ(stroke & _T, 1); EXPECT_EQ(stroke & _S, 1);
	EXPECT_EQ(stroke & _D, 0); EXPECT_EQ(stroke & _Z, 0);
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
	EXPECT_EQ(  leftHand .getBits(), 0b01111111110000000000000000000000);
	EXPECT_EQ((~leftHand).getBits(), 0b10000000001111111111111000000000);
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

#include <iterator>
#include <concepts>
TEST(StenoPhrase, ContainerTypeRequirements) {
	using C  = steno::Phrase;
	using T  = steno::Stroke;
	using I  = steno::Phrase::iterator;
	using IC = steno::Phrase::const_iterator;
	using I_Traits  = std::iterator_traits<I>;
	using IC_Traits = std::iterator_traits<IC>;
	// Container
	EXPECT_TRUE((std::same_as<C::value_type     , T       >));
	EXPECT_TRUE((std::same_as<C::reference      , T&      >));
	EXPECT_TRUE((std::same_as<C::const_reference, T const&>));
	EXPECT_TRUE((std::same_as<C::difference_type, std::ptrdiff_t>));
	EXPECT_TRUE((std::same_as<C::size_type      , std::size_t>));
	// Iterator
	EXPECT_TRUE((std::contiguous_iterator<I>));
	EXPECT_TRUE((std::contiguous_iterator<IC>));
	EXPECT_TRUE((std::convertible_to<I, IC>));
	EXPECT_TRUE((std::same_as<I_Traits::value_type, T>));
	EXPECT_TRUE((std::same_as<IC_Traits::value_type, T>));
	EXPECT_TRUE((std::same_as<I_Traits::difference_type, C::difference_type>));
	EXPECT_TRUE((std::same_as<IC_Traits::difference_type, C::difference_type>));
}
