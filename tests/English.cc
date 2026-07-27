/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(StenoEnglishCode, Construction) {
	auto const Code = steno::English.Code;
	EXPECT_EQ(Code.name(), "eng");
	EXPECT_EQ(Code.script(), "Latn");
}

#define SPEECH_SETUP(Langage)                                                  \
    g_oss = {};                                                                \
    auto speech = steno::Speech {g_oss};                                       \
    auto CaseLanguage = Langage

#define PHRASE_EQ(Stream, Result)                                              \
    g_oss = {}, speech = {g_oss, CaseLanguage, steno::Default};                \
    speech.context().as(CaseLanguage)->beginning = true;                       \
    speech << Stream;                                                          \
    EXPECT_EQ(g_oss.str(), Result)

TEST(StenoEnglishPhrase, OutputStream) {
	SPEECH_SETUP(steno::English);

	PHRASE_EQ("call"<<"me"<<"Ishmael", "call me Ishmael");
	PHRASE_EQ("some"<<"years"<<"ago", "some years ago");
	PHRASE_EQ("never"<<"mind"<<"how"<<"long", "never mind how long");
}

TEST(StenoEnglishPhrase, Punctuation) {
	SPEECH_SETUP(steno::English);

	// Comma
	PHRASE_EQ("into"<<"the"<<"street"<<"{,}", "into the street,");
	PHRASE_EQ("street"<<"{,}"<<"and", "street, and");
	PHRASE_EQ("{,}"<<"and"<<"methodically", ", and methodically");

	// Period
	PHRASE_EQ("Ishmael"<<"{.}", "Ishmael.");
	PHRASE_EQ("Ishmael"<<"{.}"<<"Some", "Ishmael. Some");
	PHRASE_EQ("{.}"<<"some", ". Some");

	// Question mark
	PHRASE_EQ("how"<<"then"<<"is"<<"this"<<"{?}", "how then is this?");
	PHRASE_EQ("this"<<"{?}"<<"are", "this? Are");
	PHRASE_EQ("{?}"<<"are"<<"the"<<"green", "? Are the green");

	// Exclamation point
	PHRASE_EQ("But"<<"look"<<"{!}", "But look!");
	PHRASE_EQ("look"<<"{!}"<<"here", "look! Here");
	PHRASE_EQ("{!}"<<"here"<<"come"<<"more", "! Here come more");

	// Semicolon
	PHRASE_EQ("upon"<<"his"<<"sword"<<"{;}", "upon his sword;");
	PHRASE_EQ("sword"<<"{;}"<<"I", "sword; I");
	PHRASE_EQ("{;}"<<"I"<<"quietly"<<"take", "; I quietly take");

	// Colon
	PHRASE_EQ("something"<<"like"<<"this"<<"{:}", "something like this:");
	PHRASE_EQ("this"<<"{:}"<<"though", "this: though");
	PHRASE_EQ("{:}"<<"Though"<<"I"<<"cannot", ": Though I cannot");
}

TEST(StenoEnglishPhrase, InvisiblePunctuation) {
	SPEECH_SETUP(steno::English);

	// Combine
	PHRASE_EQ("a{^}"<<"gain", "again");
	PHRASE_EQ("a{^}"<<"gain"<<"{^}st", "against");
	PHRASE_EQ("1"<<"{^}st", "1st");

	// DigitSequence
	PHRASE_EQ("{&2}"<<"{&5}"<<"{&1}", "251");
	PHRASE_EQ("{&25}"<<"{&1}", "251");
	PHRASE_EQ("{&2}"<<"{&51}", "251");
	PHRASE_EQ("{&251}", "251");
	PHRASE_EQ("{&B}"<<"{&4}", "B4");
	PHRASE_EQ("once"<<"{&B}"<<"{&4}", "once B4");
	PHRASE_EQ("{&B}"<<"{&4}"<<"us", "B4 us");
	PHRASE_EQ("{&B}"<<"and"<<"{&4}", "B and 4");
	PHRASE_EQ("{&4}"<<"holy"<<"name", "4 holy name");
	PHRASE_EQ("shall"<<"always"<<"{&B}", "shall always B");

	// Capitalize
	PHRASE_EQ("{-|}"<<"november", "November");
	PHRASE_EQ("drizzly"<<"{-|}"<<"november", "drizzly November");
	PHRASE_EQ("{-|}"<<"sabbath"<<"afternoon", "Sabbath afternoon");
}

#undef SPEECH_SETUP
#undef PHRASE_EQ
