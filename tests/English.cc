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
TEST(StenoEnglishSpeech, Suffix_s) {
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
	PHRASE_EQ("obsequy {^s}", "	obsequies");
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

	// Plural examples not found in the above Wikipedia article
	PHRASE_EQ("box {^s}", "boxes");
	PHRASE_EQ("quiz {^s}", "quizzes");
}

// https://en.wikipedia.org/wiki/English_verbs#Past_tense
TEST(StenoEnglishSpeech, Suffix_ed) {
	PHRASE_EQ("play {^ed}", "played");
	PHRASE_EQ("like {^ed}", "liked");
	PHRASE_EQ("try {^ed}", "tried");
//	PHRASE_EQ("sky {^ed}", "skied" or "skyed");
	PHRASE_EQ("lay {^ed}", "laid");
	PHRASE_EQ("pay {^ed}", "paid");
	PHRASE_EQ("say {^ed}", "said");
	PHRASE_EQ("ship {^ed}", "shipped");
	PHRASE_EQ("fathom {^ed}", "fathomed");
	PHRASE_EQ("panic {^ed}", "panicked");
//	PHRASE_EQ("zinc {^ed}", "zincked" or "zinced");
	PHRASE_EQ("arc {^ed}", "arced");
//	PHRASE_EQ("spec {^ed}", "specced" or "speced");
//	PHRASE_EQ("sync {^ed}", "synced" or "synched");
//	PHRASE_EQ("travel {^ed}", "traveled" or "travelled");
	PHRASE_EQ("parallel {^ed}", "paralleled");
//	PHRASE_EQ("dial {^ed}", "dialed" or "dialled");
//	PHRASE_EQ("fuel {^ed}", "fueled" or "fuelled");
	PHRASE_EQ("backflip {^ed}", "backflipped");
	PHRASE_EQ("hobnob {^ed}", "hobnobbed");
	PHRASE_EQ("kidnap {^ed}", "kidnapped");
//	PHRASE_EQ("dialog {^ed}", "dialogued" or "dialogged");
//	PHRASE_EQ("hiccup {^ed}", "hiccupped" or "hiccuped");
//	PHRASE_EQ("program {^ed}", "programed" or "programmed");
	PHRASE_EQ("catalog {^ed}", "cataloged");
	PHRASE_EQ("pyramid {^ed}", "pyramided");
	PHRASE_EQ("format {^ed}", "formatted");
//	PHRASE_EQ("combat {^ed}", "combated" or "combatted");
//	PHRASE_EQ("bus {^ed}", "bused" or "bussed");
//	PHRASE_EQ("bias {^ed}", "biased" or "biassed");
//	PHRASE_EQ("focus {^ed}", "focused" or "focussed");
	PHRASE_EQ("drift {^ed}", "drifted");
	PHRASE_EQ("exceed {^ed}", "exceeded");
	PHRASE_EQ("cap {^ed}", "capped");
	PHRASE_EQ("pass {^ed}", "passed");
	PHRASE_EQ("buzz {^ed}", "buzzed");
	PHRASE_EQ("tango {^ed}", "tangoed");
}

// https://en.wikipedia.org/wiki/Frequentative#English
TEST(StenoEnglishSpeech, Suffix_le) {
	PHRASE_EQ("blab {^er}", "blabber");
	PHRASE_EQ("bob {^le}", "bobble");
	PHRASE_EQ("crack {^le}", "crackle");
	PHRASE_EQ("crumb {^le}", "crumble");
	PHRASE_EQ("curd {^le}", "curdle");
	PHRASE_EQ("dab {^le}", "dabble");
	PHRASE_EQ("drag {^le}", "draggle");
	PHRASE_EQ("daze {^le}", "dazzle");
	PHRASE_EQ("fizz {^le}", "fizzle");
	PHRASE_EQ("flit {^er}", "flitter");
	PHRASE_EQ("fond {^le}", "fondle");
	PHRASE_EQ("gob {^le}", "gobble");
	PHRASE_EQ("grunt {^le}", "gruntle");
	PHRASE_EQ("hag {^le}", "haggle");
	PHRASE_EQ("jig {^le}", "jiggle");
	PHRASE_EQ("mud {^le}", "muddle");
	PHRASE_EQ("nest {^le}", "nestle");
	PHRASE_EQ("pad {^le}", "paddle");
	PHRASE_EQ("pat {^er}", "patter");
	PHRASE_EQ("prate {^le}", "prattle");
	PHRASE_EQ("prick {^le}", "prickle");
	PHRASE_EQ("put {^er}", "putter");
	PHRASE_EQ("scuff {^le}", "scuffle");
	PHRASE_EQ("sniff {^le}", "sniffle");
	PHRASE_EQ("snuff {^le}", "snuffle");
	PHRASE_EQ("snug {^le}", "snuggle");
	PHRASE_EQ("spark {^le}", "sparkle");
	PHRASE_EQ("speck {^le}", "speckle");
	PHRASE_EQ("suck {^le}", "suckle");
	PHRASE_EQ("swag {^er}", "swagger");
	PHRASE_EQ("tick {^le}", "tickle");
	PHRASE_EQ("top {^le}", "topple");
	PHRASE_EQ("tramp {^le}", "trample");
	PHRASE_EQ("twink {^le}", "twinkle");
	PHRASE_EQ("wade {^le}", "waddle");
	PHRASE_EQ("wag {^le}", "waggle");
	PHRASE_EQ("wring {^le}", "wrangle");
	PHRASE_EQ("wrest {^le}", "wrestle");
}

// https://en.wikipedia.org/wiki/English_prefix#Native
TEST(StenoEnglishSpeech, NativePrefixes) {
	PHRASE_EQ("{a^} float", "afloat");
	PHRASE_EQ("{a^} tremble", "atremble");
	PHRASE_EQ("{after^} math", "aftermath");
	PHRASE_EQ("{after^} life", "afterlife");
	PHRASE_EQ("{back^} hoe", "backhoe");
	PHRASE_EQ("{back^} fire", "backfire");
	PHRASE_EQ("{be^} deviled", "bedeviled");
	PHRASE_EQ("{be^} calm", "becalm");
	PHRASE_EQ("{be^} dazzle", "bedazzle");
	PHRASE_EQ("{be^} witch", "bewitch");
	PHRASE_EQ("{by^} way", "byway");
	PHRASE_EQ("{by^} pass", "bypass");
	PHRASE_EQ("{by^} product", "byproduct");
	PHRASE_EQ("{down^} load", "download");
	PHRASE_EQ("{down^} right", "downright");
	PHRASE_EQ("{down^} beat", "downbeat");
	PHRASE_EQ("{en^} mesh", "enmesh");
	PHRASE_EQ("{em^} power", "empower");
	PHRASE_EQ("{fore^} arm", "forearm");
	PHRASE_EQ("{fore^} runner", "forerunner");
	PHRASE_EQ("{fore^} bode", "forebode");
	PHRASE_EQ("{hind^} sight", "hindsight");
	PHRASE_EQ("{hind^} quarters", "hindquarters");
	PHRASE_EQ("{mid^} stream", "midstream");
	PHRASE_EQ("{mid^} life", "midlife");
	PHRASE_EQ("{midi-^} length", "midi-length");
	PHRASE_EQ("{midi^} bus", "midibus");
	PHRASE_EQ("{mini^} market", "minimarket");
	PHRASE_EQ("{mini-^} room", "mini-room");
	PHRASE_EQ("{mini^} van", "minivan");
	PHRASE_EQ("{mis^} information", "misinformation");
	PHRASE_EQ("{mis^} guide", "misguide");
	PHRASE_EQ("{mis^} fortune", "misfortune");
	PHRASE_EQ("{mis^} behave", "misbehave");
	PHRASE_EQ("{mis^} spell", "misspell");
	PHRASE_EQ("{off-^} color", "off-color");
	PHRASE_EQ("{off^} ish", "offish");
	PHRASE_EQ("{off^} set", "offset");
	PHRASE_EQ("{on^} set", "onset");
	PHRASE_EQ("{on^} look", "onlook");
	PHRASE_EQ("{on^} going", "ongoing");
	PHRASE_EQ("{on^} coming", "oncoming");
	PHRASE_EQ("{out^} reach", "outreach");
	PHRASE_EQ("{out^} come", "outcome");
	PHRASE_EQ("{out^} lier", "outlier");
	PHRASE_EQ("{over^} react", "overreact");
	PHRASE_EQ("{over^} act", "overact");
	PHRASE_EQ("{over^} bearing", "overbearing");
	PHRASE_EQ("{self-^} sufficient", "self-sufficient");
	PHRASE_EQ("{self-^} explanatory", "self-explanatory");
	PHRASE_EQ("{step^} brother", "stepbrother");
	PHRASE_EQ("{step^} mother", "stepmother");
	PHRASE_EQ("{step^} father", "stepfather");
	PHRASE_EQ("{step^} sister", "stepsister");
	PHRASE_EQ("{twi^} bill", "twibill");
	PHRASE_EQ("{twi^} light", "twilight");
//	PHRASE_EQ("{twi^} ns", "twins");
	PHRASE_EQ("{un^} necessary", "unnecessary");
	PHRASE_EQ("{un^} equal", "unequal");
	PHRASE_EQ("{un^} desirable", "undesirable");
	PHRASE_EQ("{un^} happy", "unhappy");
	PHRASE_EQ("{un^} do", "undo");
	PHRASE_EQ("{un^} tie", "untie");
	PHRASE_EQ("{un^} expected", "unexpected");
	PHRASE_EQ("{un^} lock", "unlock");
	PHRASE_EQ("{under^} achieve", "underachieve");
	PHRASE_EQ("{under^} pass", "underpass");
	PHRASE_EQ("{under^} stand", "understand");
	PHRASE_EQ("{under^} go", "undergo");
	PHRASE_EQ("{up^} grade", "upgrade");
	PHRASE_EQ("{up^} lift", "uplift");
	PHRASE_EQ("{up^} right", "upright");
	PHRASE_EQ("{with^} stand", "withstand");
	PHRASE_EQ("{with^} hold", "withhold");

}

// https://en.wikipedia.org/wiki/English_prefix#Neo-classical
TEST(StenoEnglishSpeech, NeoClassicalPrefixes) {
	PHRASE_EQ("{a^} cyclic", "acyclic");
	PHRASE_EQ("{a^} sexual", "asexual");
	PHRASE_EQ("{a^} tonal", "atonal");
	PHRASE_EQ("{a^} theist", "atheist");
	PHRASE_EQ("{Afro-^} American", "Afro-American");
	PHRASE_EQ("{Afro-^} Caribbean", "Afro-Caribbean");
	PHRASE_EQ("{ambi^} dextrous", "ambidextrous");
	PHRASE_EQ("{ambi^} tendency", "ambitendency");
	PHRASE_EQ("{amphi^} aster", "amphiaster");
	PHRASE_EQ("{amphi^} theatre", "amphitheatre");
	PHRASE_EQ("{amphi^} bian", "amphibian");
	PHRASE_EQ("{an^} emic", "anemic");
	PHRASE_EQ("{an^} archy", "anarchy");
	PHRASE_EQ("{a^} symmetric", "asymmetric");
	PHRASE_EQ("{ana^} log", "analog");
	PHRASE_EQ("{ana^} glyph", "anaglyph");
	PHRASE_EQ("{an^} ode", "anode");
	PHRASE_EQ("{Anglo-^} Norman", "Anglo-Norman");
	PHRASE_EQ("{Anglo-^} Saxon", "Anglo-Saxon");
	PHRASE_EQ("{Anglo-^} American", "Anglo-American");
	PHRASE_EQ("{ante^} natal", "antenatal");
	PHRASE_EQ("{ante^} chamber", "antechamber");
	PHRASE_EQ("{ante^} date", "antedate");
	PHRASE_EQ("{ant^} agonist", "antagonist");
	PHRASE_EQ("{anti^} venom", "antivenom");
	PHRASE_EQ("{apo^} gee", "apogee");
	PHRASE_EQ("{apo^} morphine", "apomorphine");
	PHRASE_EQ("{ap^} helion", "aphelion");
	PHRASE_EQ("{arch^} angel", "archangel");
	PHRASE_EQ("{arch^} aen", "archaen");
	PHRASE_EQ("{arch^} conservative", "archconservative");
	PHRASE_EQ("{astro^} biology", "astrobiology");
	PHRASE_EQ("{astro^} logy", "astrology");
	PHRASE_EQ("{astro^} nomy", "astronomy");
	PHRASE_EQ("{auto^} biography", "autobiography");
	PHRASE_EQ("{auto^} matic", "automatic");
	PHRASE_EQ("{auto^} nomy", "autonomy");
	PHRASE_EQ("{bi^} cycle", "bicycle");
	PHRASE_EQ("{bi^} ped", "biped");
	PHRASE_EQ("{bi^} nomial", "binomial");
	PHRASE_EQ("{bi^} gamy", "bigamy");
	PHRASE_EQ("{bi^} nary", "binary");
	PHRASE_EQ("{bi^} weekly", "biweekly");
	PHRASE_EQ("{bi^} monthly", "bimonthly");
	PHRASE_EQ("{bio^} logy", "biology");
	PHRASE_EQ("{bio^} tic", "biotic");
	PHRASE_EQ("{circum^} locution", "circumlocution");
	PHRASE_EQ("{circum^} navigate", "circumnavigate");
	PHRASE_EQ("{circum^} ference", "circumference");
	PHRASE_EQ("{cis^} lunar", "cislunar");
	PHRASE_EQ("{cis^} atlantic", "cisatlantic");
	PHRASE_EQ("{con^} federation", "confederation");
	PHRASE_EQ("{co^} habit", "cohabit");
	PHRASE_EQ("{com^} mingle", "commingle");
	PHRASE_EQ("{col^} league", "colleague");
	PHRASE_EQ("{cor^} relation", "correlation");
	PHRASE_EQ("{contra^} dict", "contradict");
	PHRASE_EQ("{contra^} indication", "contraindication");
	PHRASE_EQ("{contro^} vert", "controvert");
	PHRASE_EQ("{counter^} act", "counteract");
	PHRASE_EQ("{counter^} part", "counterpart");
	PHRASE_EQ("{cryo^} genics", "cryogenics");
	PHRASE_EQ("{cryo^} banks", "cryobanks");
	PHRASE_EQ("{crypt^} ic", "cryptic");
	PHRASE_EQ("{crypto^} graphy", "cryptography");
	PHRASE_EQ("{crypto^} currency", "cryptocurrency");
	PHRASE_EQ("{de^} press", "depress");
	PHRASE_EQ("{de^} scend", "descend");
	PHRASE_EQ("{de^} crease", "decrease");
	PHRASE_EQ("{demi^} god", "demigod");
	PHRASE_EQ("{demi^} glace", "demiglace");
//	PHRASE_EQ("{demi-^} plié", "demi-plié");
	PHRASE_EQ("{demo^} cracy", "democracy");
	PHRASE_EQ("{demo^} graphy", "demography");
	PHRASE_EQ("{deuter^} agonist", "deuteragonist");
	PHRASE_EQ("{deuter^} ogamy", "deuterogamy");
	PHRASE_EQ("{di^} cotyledon", "dicotyledon");
	PHRASE_EQ("{di^} oxide", "dioxide");
	PHRASE_EQ("{dia^} lysis", "dialysis");
	PHRASE_EQ("{dia^} meter", "diameter");
	PHRASE_EQ("{dis^} sect", "dissect");
	PHRASE_EQ("{di^} vide", "divide");
	PHRASE_EQ("{dif^} fer", "differ");
//	PHRASE_EQ("{du^} al", "dual");
//	PHRASE_EQ("{du^} et", "duet");
	PHRASE_EQ("{duo^} tone", "duotone");
	PHRASE_EQ("{eco^} system", "ecosystem");
	PHRASE_EQ("{electro-^} analysis", "electro-analysis");
	PHRASE_EQ("{electro^} magnetic", "electromagnetic");
	PHRASE_EQ("{en^} ergetic", "energetic");
	PHRASE_EQ("{el^} lipsis", "ellipsis");
	PHRASE_EQ("{em^} phasis", "emphasis");
	PHRASE_EQ("{epi^} centre", "epicentre");
	PHRASE_EQ("{epi^} dermis", "epidermis");
	PHRASE_EQ("{ep^} hemeron", "ephemeron");
	PHRASE_EQ("{eu^} karyote", "eukaryote");
	PHRASE_EQ("{eu^} logy", "eulogy");
	PHRASE_EQ("{eu^} phoria", "euphoria");
	PHRASE_EQ("{Euro^} centric", "Eurocentric");
//	PHRASE_EQ("{ex^} it", "exit");
	PHRASE_EQ("{ex^} pel", "expel");
	PHRASE_EQ("{ex^} plode", "explode");
	PHRASE_EQ("{ex^} ploit", "exploit");
	PHRASE_EQ("{ex^} plore", "explore");
	PHRASE_EQ("{ex^} port", "export");
	PHRASE_EQ("{extra^} curricular", "extracurricular");
	PHRASE_EQ("{extra^} ordinary", "extraordinary");
	PHRASE_EQ("{extra^} terrestrial", "extraterrestrial");
	PHRASE_EQ("{Franco^} phile", "Francophile");
	PHRASE_EQ("{Franco-^} British", "Franco-British");
	PHRASE_EQ("{Franco-^} German", "Franco-German");
	PHRASE_EQ("{geo^} graphy", "geography");
	PHRASE_EQ("{geo^} logy", "geology");
	PHRASE_EQ("{geo^} metry", "geometry");
	PHRASE_EQ("{gyro^} copter", "gyrocopter");
	PHRASE_EQ("{gyro^} scope", "gyroscope");
	PHRASE_EQ("{gyro^} sphere", "gyrosphere");
	PHRASE_EQ("{hetero^} chromia", "heterochromia");
	PHRASE_EQ("{hetero^} geneous", "heterogeneous");
	PHRASE_EQ("{hetero^} troph", "heterotroph");
	PHRASE_EQ("{hetero^} zygous", "heterozygous");
	PHRASE_EQ("{hemi^} morphic", "hemimorphic");
	PHRASE_EQ("{hemi^} sphere", "hemisphere");
//	PHRASE_EQ("{Hispano^} américa", "Hispanoamérica");
	PHRASE_EQ("{hispano^} phobia", "hispanophobia");
	PHRASE_EQ("{homo^} geneous", "homogeneous");
	PHRASE_EQ("{homo^} genize", "homogenize");
	PHRASE_EQ("{homo^} logous", "homologous");
	PHRASE_EQ("{homo^} phone", "homophone");
	PHRASE_EQ("{homo^} zygous", "homozygous");
	PHRASE_EQ("{hydro^} electricity", "hydroelectricity");
//	PHRASE_EQ("{hydra^} nt", "hydrant");
	PHRASE_EQ("{hydro^} gen", "hydrogen");
	PHRASE_EQ("{hyper^} thermia", "hyperthermia");
	PHRASE_EQ("{hyper^} active", "hyperactive");
	PHRASE_EQ("{hyper^} sensitive", "hypersensitive");
	PHRASE_EQ("{hypo^} thermia", "hypothermia");
	PHRASE_EQ("{hypo^} dermic", "hypodermic");
	PHRASE_EQ("{hypo^} allergenic", "hypoallergenic");
	PHRASE_EQ("{ideo^} graph", "ideograph");
	PHRASE_EQ("{ideo^} logy", "ideology");
	PHRASE_EQ("{idio^} lect", "idiolect");
	PHRASE_EQ("{idio^} pathic", "idiopathic");
	PHRASE_EQ("{in^} clude", "include");
	PHRASE_EQ("{in^} sert", "insert");
	PHRASE_EQ("{Indo-^} European", "Indo-European");
	PHRASE_EQ("{in^} appropriate", "inappropriate");
	PHRASE_EQ("{in^} exact", "inexact");
	PHRASE_EQ("{il^} legal", "illegal");
	PHRASE_EQ("{il^} licit", "illicit");
	PHRASE_EQ("{im^} patient", "impatient");
	PHRASE_EQ("{im^} possible", "impossible");
	PHRASE_EQ("{ir^} regular", "irregular");
	PHRASE_EQ("{ir^} responsible", "irresponsible");
	PHRASE_EQ("{infra^} red", "infrared");
	PHRASE_EQ("{infra^} structure", "infrastructure");
	PHRASE_EQ("{infra^} sonic", "infrasonic");
	PHRASE_EQ("{inter^} cede", "intercede");
	PHRASE_EQ("{inter^} net", "internet");
	PHRASE_EQ("{inter^} national", "international");
	PHRASE_EQ("{intra^} venous", "intravenous");
	PHRASE_EQ("{intra^} cranial", "intracranial");
	PHRASE_EQ("{intra^} net", "intranet");
	PHRASE_EQ("{iso^} chromatic", "isochromatic");
	PHRASE_EQ("{iso^} therm", "isotherm");
	PHRASE_EQ("{italo^} philia", "italophilia");
	PHRASE_EQ("{italo^} phobia", "italophobia");
	PHRASE_EQ("{macro^} biotic", "macrobiotic");
	PHRASE_EQ("{macro^} cosm", "macrocosm");
	PHRASE_EQ("{mal^} nourish", "malnourish");
	PHRASE_EQ("{mal^} adjusted", "maladjusted");
	PHRASE_EQ("{maxi-^} skirt", "maxi-skirt");
//	PHRASE_EQ("{maxi^} mum", "maximum");
	PHRASE_EQ("{mega^} star", "megastar");
	PHRASE_EQ("{megalo^} polis", "megalopolis");
	PHRASE_EQ("{meso^} sphere", "mesosphere");
	PHRASE_EQ("{meso^} derm", "mesoderm");
	PHRASE_EQ("{meso^} zoa", "mesozoa");
	PHRASE_EQ("{meta^} physics", "metaphysics");
	PHRASE_EQ("{meta^} communication", "metacommunication");
	PHRASE_EQ("{micro^} bacillus", "microbacillus");
	PHRASE_EQ("{micro^} scope", "microscope");
	PHRASE_EQ("{mono^} gamy", "monogamy");
	PHRASE_EQ("{mono^} tone", "monotone");
	PHRASE_EQ("{mono^} syllabic", "monosyllabic");
	PHRASE_EQ("{mono^} mial", "monomial");
	PHRASE_EQ("{mono^} brow", "monobrow");
	PHRASE_EQ("{mon^} arch", "monarch");
	PHRASE_EQ("{multi^} cultural", "multicultural");
	PHRASE_EQ("{multi-^} storey", "multi-storey");
	PHRASE_EQ("{multi^} tude", "multitude");
	PHRASE_EQ("{mult^} angular", "multangular");
	PHRASE_EQ("{neo^} lithic", "neolithic");
	PHRASE_EQ("{non^} existent", "nonexistent");
	PHRASE_EQ("{non-^} fiction", "non-fiction");
	PHRASE_EQ("{ob^} ject", "object");
	PHRASE_EQ("{ob^} ligate", "obligate");
	PHRASE_EQ("{omni^} potent", "omnipotent");
	PHRASE_EQ("{omni^} present", "omnipresent");
	PHRASE_EQ("{omni^} vore", "omnivore");
	PHRASE_EQ("{ortho^} dontics", "orthodontics");
	PHRASE_EQ("{ortho^} tropic", "orthotropic");
	PHRASE_EQ("{paleo^} lithic", "paleolithic");
	PHRASE_EQ("{paleo^} ntologist", "paleontologist");
	PHRASE_EQ("{pan-^} American", "pan-American");
	PHRASE_EQ("{pan^} demic", "pandemic");
	PHRASE_EQ("{pan^} orama", "panorama");
	PHRASE_EQ("{para^} llel", "parallel");
	PHRASE_EQ("{para^} plegic", "paraplegic");
	PHRASE_EQ("{para^} sail", "parasail");
	PHRASE_EQ("{ped^} al", "pedal");
	PHRASE_EQ("{ped^} estrian", "pedestrian");
	PHRASE_EQ("{pen^} insula", "peninsula");
	PHRASE_EQ("{pen^} ultimate", "penultimate");
	PHRASE_EQ("{pen^} umbra", "penumbra");
	PHRASE_EQ("{per^} meate", "permeate");
	PHRASE_EQ("{per^} mute", "permute");
	PHRASE_EQ("{peri^} helion", "perihelion");
	PHRASE_EQ("{peri^} phrase", "periphrase");
	PHRASE_EQ("{photo^} electric", "photoelectric");
	PHRASE_EQ("{photo^} graphy", "photography");
	PHRASE_EQ("{photo^} synthesis", "photosynthesis");
	PHRASE_EQ("{pleo^} nasm", "pleonasm");
//	PHRASE_EQ("{pleo^} roma", "pleroma");
	PHRASE_EQ("{pod^} iatrist", "podiatrist");
	PHRASE_EQ("{pod^} ium", "podium");
	PHRASE_EQ("{poly^} gon", "polygon");
	PHRASE_EQ("{poly^} hedron", "polyhedron");
	PHRASE_EQ("{poly^} gamy", "polygamy");
	PHRASE_EQ("{post^} fix", "postfix");
	PHRASE_EQ("{post^} pone", "postpone");
	PHRASE_EQ("{post^} script", "postscript");
	PHRASE_EQ("{pre^} dict", "predict");
	PHRASE_EQ("{pre^} pare", "prepare");
	PHRASE_EQ("{pre^} view", "preview");
	PHRASE_EQ("{pre^} school", "preschool");
	PHRASE_EQ("{pre^} write", "prewrite");
	PHRASE_EQ("{pre^} fix", "prefix");
	PHRASE_EQ("{preter^} mit", "pretermit");
	PHRASE_EQ("{preter^} natural", "preternatural");
	PHRASE_EQ("{pro^} consul", "proconsul");
	PHRASE_EQ("{pro^} cambium", "procambium");
	PHRASE_EQ("{pros^} thesis", "prosthesis");
	PHRASE_EQ("{pros^} trate", "prostrate");
//	PHRASE_EQ("{pros^} e", "prose");
	PHRASE_EQ("{proto^} plasm", "protoplasm");
	PHRASE_EQ("{proto^} type", "prototype");
	PHRASE_EQ("{pseudo^} nym", "pseudonym");
	PHRASE_EQ("{pseudo^} morph", "pseudomorph");
	PHRASE_EQ("{pyro^} kinetic", "pyrokinetic");
	PHRASE_EQ("{pyro^} technic", "pyrotechnic");
	PHRASE_EQ("{quadri^} lateral", "quadrilateral");
	PHRASE_EQ("{quadri^} nomial", "quadrinomial");
	PHRASE_EQ("{quasi-^} religious", "quasi-religious");
	PHRASE_EQ("{retro^} grade", "retrograde");
	PHRASE_EQ("{semi^} circle", "semicircle");
	PHRASE_EQ("{semi^} serious", "semiserious");
	PHRASE_EQ("{socio^} path", "sociopath");
	PHRASE_EQ("{sub^} marine", "submarine");
	PHRASE_EQ("{sub^} terranean", "subterranean");
	PHRASE_EQ("{sub^} urban", "suburban");
	PHRASE_EQ("{sup^} port", "support");
	PHRASE_EQ("{super^} visor", "supervisor");
	PHRASE_EQ("{super^} intendent", "superintendent");
	PHRASE_EQ("{supra^} renal", "suprarenal");
	PHRASE_EQ("{sur^} real", "surreal");
	PHRASE_EQ("{sur^} render", "surrender");
	PHRASE_EQ("{sur^} plus", "surplus");
	PHRASE_EQ("{syn^} thesis", "synthesis");
	PHRASE_EQ("{sy^} stem", "system");
	PHRASE_EQ("{syl^} lable", "syllable");
	PHRASE_EQ("{sym^} bol", "symbol");
	PHRASE_EQ("{tele^} graph", "telegraph");
	PHRASE_EQ("{tele^} phone", "telephone");
	PHRASE_EQ("{tele^} scope", "telescope");
	PHRASE_EQ("{tele^} vision", "television");
	PHRASE_EQ("{trans^} atlantic", "transatlantic");
	PHRASE_EQ("{trans^} verse", "transverse");
	PHRASE_EQ("{trans^} form", "transform");
	PHRASE_EQ("{tri^} cycle", "tricycle");
	PHRASE_EQ("{tri^} partite", "tripartite");
	PHRASE_EQ("{tri^} angle", "triangle");
	PHRASE_EQ("{tri^} color", "tricolor");
	PHRASE_EQ("{tri^} nomial", "trinomial");
	PHRASE_EQ("{ultra^} magnetic", "ultramagnetic");
	PHRASE_EQ("{ultra^} sonic", "ultrasonic");
	PHRASE_EQ("{ultra^} violet", "ultraviolet");
	PHRASE_EQ("{uni^} cycle", "unicycle");
	PHRASE_EQ("{uni^} versal", "universal");
	PHRASE_EQ("{vice-^} president", "vice-president");
	PHRASE_EQ("{vice-^} principal", "vice-principal");
	PHRASE_EQ("{vice-^} admiral", "vice-admiral");
}

#undef PHRASE_EQ
