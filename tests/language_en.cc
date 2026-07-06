/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define PHRASE_EQ(Sum, Result) EXPECT_EQ(steno::Phrase {} + Sum, Result)

TEST(StenoEnglishPhrase, Addition) {
	steno::CodeSwitch(steno::English);

	PHRASE_EQ("call"+"me"+"Ishmael", "call me Ishmael");
	PHRASE_EQ("some"+"years"+"ago", "some years ago");
	PHRASE_EQ("never"+"mind"+"how"+"long", "never mind how long");
}

TEST(StenoEnglishPhrase, Punctuation) {
	steno::CodeSwitch(steno::English);

	// Comma
	PHRASE_EQ("into"+"the"+"street{,}", "into the street{,}");
	PHRASE_EQ("street"+"{,}"+"and", "street, and");
	PHRASE_EQ("{,}"+"and"+"methodically", "{,}and methodically");

	// Period
	PHRASE_EQ("Ishmael"+"{.}", "Ishmael{.}");
	PHRASE_EQ("Ishmael"+"{.}"+"Some", "Ishmael. Some");
	PHRASE_EQ("{.}"+"some", "{.}some");

	// Question mark
	PHRASE_EQ("how"+"then"+"is"+"this"+"{?}", "how then is this{?}");
	PHRASE_EQ("this"+"{?}"+"are", "this? Are");
	PHRASE_EQ("{?}"+"are"+"the"+"green"+"fields", "{?}are the green fields");

	// Exclamation point
	PHRASE_EQ("But"+"look"+"{!}", "But look{!}");
	PHRASE_EQ("look"+"{!}"+"here", "look! Here");
	PHRASE_EQ("{!}"+"here"+"come"+"more"+"crowds", "{!}here come more crowds");

	// Semicolon
	PHRASE_EQ("upon"+"his"+"sword"+"{;}", "upon his sword{;}");
	PHRASE_EQ("sword"+"{;}"+"I", "sword; I");
	PHRASE_EQ("{;}"+"I"+"quietly"+"take", "{;}I quietly take");

	// Colon
	PHRASE_EQ("something"+"like"+"this"+"{:}", "something like this{:}");
	PHRASE_EQ("this"+"{:}"+"though", "this: though");
	PHRASE_EQ("{:}"+"Though"+"I"+"cannot"+"tell", "{:}Though I cannot tell");
}
