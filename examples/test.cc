#include "steno.hh"
#include <gtest/gtest.h>

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
