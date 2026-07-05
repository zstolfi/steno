/* ~~ Phrase Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

steno::CodeSwitch(steno::English);

TEST(StenoEnglishPhrase, Addition) {
	"call"+"me"+"Ishmael" == "call me Ishmael"
	"some"+"years"+"ago" == "some years ago"
	"never"+"mind"+"how"+"long"+"precisely" == "never mind how long precisely"
}

TEST(StenoEnglishPhrase, Punctuation) {
	// Comma
	"stepping"+"into"+"the"+"street{,}" == "stepping into the street{,}"
	"street"+"{,}"+"and" == "street, and"
	"{,}"+"and"+"methodically"+"knocking" == "{,}and methodically knocking"

	// Period
	"Ishmael"+"{.}" == "Ishmael{.}"
	"Ishmael"+"{.}"+"Some" == "Ishmael. Some"
	"{.}"+"some" == "{.}some"

	// Question mark
	"how"+"then"+"is"+"this"+"{?}" == "how then is this{?}"
	"this"+"{?}"+"are" == "this? Are"
	"{?}"+"are"+"the"+"green"+"fields"+"gone" == "{?}are the green fields gone"

	// Exclamation point
	"But"+"look"+"{!}" == "But look{!}"
	"look"+"{!}"+"here" == "look! Here"
	"{!}"+"here"+"come"+"more"+"crowds" == "{!}here come more crowds"

	// Semicolon
	"upon"+"his"+"sword"+"{;}" == "upon his sword{;}"
	"sword"+"{;}"+"I" == "sword; I"
	"{;}"+"I"+"quietly"+"take" == "{;}I quietly take"

	// Colon
	"something"+"like"+"this"+"{:}" == "something like this{:}"
	"this"+"{:}"+"though" == "this: though"
	"{:}"+"Though"+"I"+"cannot"+"tell"+"why" == "{:}Though I cannot tell why"
}

/* ~~ Text Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(StenoEnglishText, English) {
	"call" + "me" == "Call me"
	"call" + "me" + "Ishmael" == "Call me Ishmael"
	"call" + "me" + "Ishmael" + "{.}" == "Call me Ishmael."
}
