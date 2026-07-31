TEST(StenoEnglishCode, Construction) {
	auto const Code = steno::English.Code;
	EXPECT_EQ(Code.name(), "eng");
	EXPECT_EQ(Code.script(), "Latn");
}

#define PHRASE_EQ(Stream, Expected) {                                          \
    g_oss = {};                                                                \
    auto speech = steno::Speech {g_oss, steno::English, steno::Default};       \
    speech.context().as(steno::English)->beginning = true;                     \
    speech << Stream;                                                          \
    EXPECT_EQ(g_oss.str(), Expected);                                          \
}

TEST(StenoEnglishSpeech, OutputStream) {
	PHRASE_EQ("call me Ishmael", "call me Ishmael");
	PHRASE_EQ("some years ago", "some years ago");
	PHRASE_EQ("never mind how long", "never mind how long");
}

TEST(StenoEnglishSpeech, Punctuation) {
	// Comma
	PHRASE_EQ("into the street {,}", "into the street,");
	PHRASE_EQ("street {,} and", "street, and");
	PHRASE_EQ("{,} and methodically", ", and methodically");

	// Period
	PHRASE_EQ("Ishmael {.}", "Ishmael.");
	PHRASE_EQ("Ishmael {.} Some", "Ishmael. Some");
	PHRASE_EQ("{.} some", ". Some");

	// Question mark
	PHRASE_EQ("how then is this {?}", "how then is this?");
	PHRASE_EQ("this {?} are", "this? Are");
	PHRASE_EQ("{?} are the green", "? Are the green");

	// Exclamation point
	PHRASE_EQ("But look {!}", "But look!");
	PHRASE_EQ("look {!} here", "look! Here");
	PHRASE_EQ("{!} here come more", "! Here come more");

	// Semicolon
	PHRASE_EQ("upon his sword {;}", "upon his sword;");
	PHRASE_EQ("sword {;} I", "sword; I");
	PHRASE_EQ("{;} I quietly take", "; I quietly take");

	// Colon
	PHRASE_EQ("something like this {:}", "something like this:");
	PHRASE_EQ("this {:} though", "this: though");
	PHRASE_EQ("{:} Though I cannot", ": Though I cannot");
}

TEST(StenoEnglishSpeech, InvisiblePunctuation) {
	// Combine
	PHRASE_EQ("a{^} gain", "again");
	PHRASE_EQ("a{^} gain {^}st", "against");
	PHRASE_EQ("1 {^}st", "1st");

	// DigitSequence
	PHRASE_EQ("{&2} {&5} {&1}", "251");
	PHRASE_EQ("{&25} {&1}", "251");
	PHRASE_EQ("{&2} {&51}", "251");
	PHRASE_EQ("{&251}", "251");
	PHRASE_EQ("{&B} {&4}", "B4");
	PHRASE_EQ("once {&B} {&4}", "once B4");
	PHRASE_EQ("{&B} {&4} us", "B4 us");
	PHRASE_EQ("{&B} and {&4}", "B and 4");
	PHRASE_EQ("{&4} holy name", "4 holy name");
	PHRASE_EQ("shall always {&B}", "shall always B");

	// Capitalize
	PHRASE_EQ("{-|} november", "November");
	PHRASE_EQ("drizzly {-|} november", "drizzly November");
	PHRASE_EQ("{-|} sabbath afternoon", "Sabbath afternoon");
}

// https://en.wikipedia.org/wiki/English_plurals#Regular_plurals
TEST(StenoEnglishSpeech, RegularPlurals) {
	// Any sibilant
	PHRASE_EQ("kiss {^s}", "kisses");
	PHRASE_EQ("phase {^s}", "phases");
	PHRASE_EQ("dish {^s}", "dishes");
	PHRASE_EQ("massage {^s}", "massages");
	PHRASE_EQ("witch {^s}", "witches");
	PHRASE_EQ("judge {^s}", "judges");

	// Other voiceless consonants
	PHRASE_EQ("lap {^s}", "laps");
	PHRASE_EQ("cat {^s}", "cats");
	PHRASE_EQ("clock {^s}", "clocks");
	PHRASE_EQ("cuff {^s}", "cuffs");
	PHRASE_EQ("death {^s}", "deaths");
	PHRASE_EQ("loch {^s}", "lochs");

	// Other voiced phonemes
	PHRASE_EQ("girl {^s}", "girls");
	PHRASE_EQ("chair {^s}", "chairs");
	PHRASE_EQ("boy {^s}", "boys");

	// Plurals of nouns in -o preceded by a consonant
	PHRASE_EQ("hero {^s}", "heroes");
	PHRASE_EQ("potato {^s}", "potatoes");
//	PHRASE_EQ("volcano {^s}", "volcanoes" or "volcanos");
	PHRASE_EQ("echo {^s}", "echoes");
	PHRASE_EQ("canto {^s}", "cantos");
	PHRASE_EQ("hetero {^s}", "heteros");
	PHRASE_EQ("photo {^s}", "photos");
//	PHRASE_EQ("zero {^s}", "zeros" or "zeroes");
	PHRASE_EQ("piano {^s}", "pianos");
	PHRASE_EQ("portico {^s}", "porticos");
	PHRASE_EQ("pro {^s}", "pros");
	PHRASE_EQ("quarto {^s}", "quartos");
	PHRASE_EQ("kimono {^s}", "kimonos");

	// Plurals of nouns in -y
	PHRASE_EQ("cherry {^s}", "cherries");
	PHRASE_EQ("lady {^s}", "ladies");
	PHRASE_EQ("sky {^s}", "skies");
	PHRASE_EQ("colloquy {^s}", "colloquies");
	PHRASE_EQ("obsequy {^_s", "	obsequies");
	PHRASE_EQ("soliloquy {^s}", "soliloquies");
	PHRASE_EQ("Sicily {^s}", "Sicilies");
	PHRASE_EQ("Scilly {^s}", "Scillies");
//	PHRASE_EQ("Germany {^s}", "Germanys" or "Germanies");
	PHRASE_EQ("lay-by {^s}", "lay-bys");
	PHRASE_EQ("stand-by {^s}", "stand-bys");
	PHRASE_EQ("day {^s}", "days");
	PHRASE_EQ("monkey {^s}", "monkeys");

	// Plurals of nouns in -i
	PHRASE_EQ("alibi {^s}", "alibis");
	PHRASE_EQ("bikini {^s}", "bikinis");
	PHRASE_EQ("Israeli {^s}", "Israelis");
//	PHRASE_EQ("chili {^s}", "chilis" or "chilies");
//	PHRASE_EQ("chilli {^s}", "chillis" or "chillies");
	PHRASE_EQ("alkali {^s}", "alkalies");
}

// Plurals examples not found in the above Wikipedia article
TEST(StenoEnglishSpeech, PluralsOther) {
	PHRASE_EQ("box {^s}", "boxes");
	PHRASE_EQ("quiz {^s}", "quizzes");
}

#undef PHRASE_EQ
