#include "steno_parsers.hh"
#include <boost/parser/parser.hpp>
namespace bp = boost::parser;

/* ~~ Common Parsers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace /*anonymous*/ {

// "using Entry = steno::Dictionary::value_type" would not work in our parsers,
// because steno::Strokes is not allowed to be const as we parse it.
using Dictionary = steno::Dictionary;
using Entry = std::pair<steno::Strokes, std::string>;

bp::rule<struct stroke , steno::Stroke > stroke   = "steno stroke";
bp::rule<struct strokes, steno::Strokes> strokes  = "stroke strokes";
// STKPWHR AO*EU FRPBLGTSDZ  or  12K3W4R 50*EU 6R7B8G9SDZ
bp::rule<struct left     , std::string> left      = "left-hand consonants";
bp::rule<struct middle   , std::string> middle    = "vowel or asterisk";
bp::rule<struct right    , std::string> right     = "right-hand consonants";
// 1234 50 6789
bp::rule<struct digitSeq , std::string> digitSeq  = "digit sequence";
// Small helper parsers
const auto stenoChar = bp::char_("STKPWHRAO*EUFRPBLGTSDZ" "1234506789" "#-");
const auto leftFlags = -bp::char_("#");
const auto rightFlags = bp::eps;
const auto asString = bp::attr(std::string {});
const auto asEntries = bp::attr(std::vector<Entry> {});

#if 0 // For faster compilations while testing the file format parsers.
const auto stroke_def
	= 	bp::string_view[ bp::string("-") ]
;

const auto strokes_def
	= 	bp::string_view[ stroke % '/' ]
;

BOOST_PARSER_DEFINE_RULES(stroke, strokes);
#else
const auto stroke_def
	= 	bp::string_view[ asString >> &stenoChar
	>>	leftFlags
	>>	(
	  	  	left >> middle >> right
	  	| 	left >> !middle
	  	| 	digitSeq
	  	)
	>>	rightFlags
	]
;

const auto strokes_def
	= 	bp::string_view[ stroke % '/' ]
;

const auto left_def
	= 	asString
	>>	-bp::char_("S1")
	>>	-bp::char_("T2") >> -bp::char_('K')
	>>	-bp::char_("P3") >> -bp::char_('W')
	>>	-bp::char_("H4") >> -bp::char_('R')
;

const auto middle_def
	= 	asString >> bp::char_('-')
	| 	asString >> &bp::char_("AO*EU" "50")
	>>	-bp::char_("A5") >> -bp::char_("O0")
	>>	-bp::char_('*')
	>>	-bp::char_('E') >> -bp::char_('U')
;

const auto right_def
	= 	asString
	>>	-bp::char_("F6") >> -bp::char_('R')
	>>	-bp::char_("P7") >> -bp::char_('B')
	>>	-bp::char_("L8") >> -bp::char_('G')
	>>	-bp::char_("T9") >> -bp::char_('S')
	>>	-bp::char_('D')  >> -bp::char_('Z')
;

const auto digitSeq_def
	= 	asString >> &bp::char_("1234506789")
	>>	-bp::char_('1') >> -bp::char_('2')
	>>	-bp::char_('3') >> -bp::char_('4')
	>>	-bp::char_('5') >> -bp::char_('0')
	>>	-bp::char_('6') >> -bp::char_('7')
	>>	-bp::char_('8') >> -bp::char_('9')
;

BOOST_PARSER_DEFINE_RULES(
	stroke, strokes,
	left, middle, right,
	digitSeq
);
#endif

/* ~~ Plain-Text File Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace plain {

bp::rule<struct file, Dictionary> file = "plain-text file";
bp::rule<struct line, Entry     > line = "brief entry";

const auto file_def
	= 	+line
;

const auto line_def
	= 	strokes
	>>	'='
	>>	bp::lexeme[ *(bp::char_ - bp::eol) ]
;

BOOST_PARSER_DEFINE_RULES(file, line);

} // namespace plain

/* ~~ JSON File Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

auto fromContainer = [] (auto& ctx) {
	_val(ctx) = {_attr(ctx).begin(), _attr(ctx).end()};
};

auto flatten = [] (auto& ctx) {
	for (const auto& outer : _attr(ctx))
	for (const auto& inner : outer) {
		_val(ctx).push_back(inner);
	}
};

namespace JSON {

bp::rule<struct file     , Dictionary        > file      = "JSON file";
bp::rule<struct value    , std::vector<Entry>> value     = "JSON value";
bp::rule<struct array    , std::vector<Entry>> array     = "JSON array";
bp::rule<struct object   , std::vector<Entry>> object    = "JSON object";
bp::rule<struct objectVal, std::vector<Entry>> objectVal = "JSON object value";
bp::rule<struct entry    , Entry             > entry     = "brief entry";
const auto trailingComma = -bp::lit(',');

// I haven't figured out why this line doesn't work without the "fromContainer".
// the plain::file parser seems to be converting vector<Entry> -> Dicitonary,
// but not this one.
const auto file_def
	= 	value[ fromContainer ]
;

// TODO: JSON escape sequences
const auto string
	= 	bp::quoted_string
;

const auto value_def
	= 	object | array
	| 	asEntries
	>>	bp::omit[ bp::lit("null") | bp::bool_ | bp::double_ | string ]
;

using Nested = std::vector<std::vector<Entry>>;

const auto array_def
	= 	'['
	>>	(value % ',' >> trailingComma | bp::attr(Nested {}))[ flatten ]
	>>	']'
;

const auto object_def
	= 	'{'
	>>	(objectVal % ',' >> trailingComma | bp::attr(Nested {}))[ flatten ]
	>>	'}'
;

const auto objectVal_def
	= 	asEntries >> entry
	| 	bp::omit[ string ] >> ':' >> value
;

const auto entry_def
	= 	'"' >> strokes >> '"' >> ':' >> string
;

BOOST_PARSER_DEFINE_RULES(file, value, array, object, objectVal, entry);

} // namespace JSON

} // namespace /*anonymous*/

/* ~~ Parse API ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace steno {

std::optional<Dictionary> parsePlain(ParserInput input) {
	return bp::parse(input, plain::file, bp::ws);
}

std::optional<Dictionary> parseJSON(ParserInput input) {
	return bp::parse(input, JSON::file, bp::ws);
}

std::optional<Dictionary> parseRTF(ParserInput input) {
	if (std::string_view str_v {input.begin(), input.end()}
	;   !str_v.starts_with("{\\rtf1")) {
		return {};
	}
	return Dictionary {{{{"R*/T*/TP*"}, "RTF"}}};
}

std::optional<Dictionary> parseGuess(ParserInput input) {
	std::array order {
		parseRTF,
		parseJSON,
		parsePlain,
	};
	for (auto parser : order) {
		if (auto result = parser(input)) return result;
	}
	return {};
}

} // namespace steno
