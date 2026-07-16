/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(StenoEnglishCode, Construction) {
	auto code = steno::LanguageCode {steno::English};
	EXPECT_EQ(code.name(), "eng");
	EXPECT_EQ(code.script(), "Latn");
	EXPECT_EQ(code.region(), "");
}

#define SPEECH_SETUP(Langage)                                                  \
	g_oss = {};                                                                \
	auto speech = steno::Speech {g_oss, Langage};                              \
	auto language = Langage;

#define PHRASE_EQ(Stream, Result)                                              \
	g_oss = {}, speech = {g_oss, language};                                    \
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
	PHRASE_EQ("{.}"<<"some", ". some");

	// Question mark
	PHRASE_EQ("how"<<"then"<<"is"<<"this"<<"{?}", "how then is this?");
	PHRASE_EQ("this"<<"{?}"<<"are", "this? Are");
	PHRASE_EQ("{?}"<<"are"<<"the"<<"green", "? are the green");

	// Exclamation point
	PHRASE_EQ("But"<<"look"<<"{!}", "But look!");
	PHRASE_EQ("look"<<"{!}"<<"here", "look! Here");
	PHRASE_EQ("{!}"<<"here"<<"come"<<"more", "! here come more");

	// Semicolon
	PHRASE_EQ("upon"<<"his"<<"sword"<<"{;}", "upon his sword;");
	PHRASE_EQ("sword"<<"{;}"<<"I", "sword; I");
	PHRASE_EQ("{;}"<<"I"<<"quietly"<<"take", "; I quietly take");

	// Colon
	PHRASE_EQ("something"<<"like"<<"this"<<"{:}", "something like this:");
	PHRASE_EQ("this"<<"{:}"<<"though", "this: though");
	PHRASE_EQ("{:}"<<"Though"<<"I"<<"cannot", ": Though I cannot");
}

#undef SPEECH_SETUP
#undef PHRASE_EQ
