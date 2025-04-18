// Steno JSON Dictionary Parser //
// Only collects briefs and ignores everything else.

#include <boost/parser/parser.hpp>
#include "steno.hh"

namespace Parse {
	namespace bp = boost::parser;
	static std::vector<steno::Brief> dictionary {};

	// Semantic actions:
	auto dictEntry = [] (auto& ctx) {
		auto const& [strokes, text] = _attr(ctx);
		dictionary.emplace_back(strokes, text);
	};

	// Forward declare our recursive rule.
	bp::rule<class value, bp::none> const value = "JSON value";

	bp::symbols<char> const escape = {
		{"\\", '\\'},
		{"\'", '\''},
		{"\"", '\"'},
		{"/", '/'},
		{"b", '\b'},
		{"f", '\f'},
		{"n", '\n'},
		{"r", '\r'},
		{"t", '\t'},
		{"{", '{'}, // Extentions to the JSON grammar
		{"}", '}'}, // for Plover support.
	};

	auto const stringChar
	=	"\\" >> escape
//	|	"\\u" >> bp::repeat(4)[ bp::hex ]
	|	bp::char_
	;
	auto const string
	=	bp::lexeme[ '"' >> *(stringChar - '"') >> '"' ]
	;
	auto const objectVal
	=	(string >> ':' >> string)[ dictEntry ]
	|	string >> ':' >> value
	;
	// Allow trailing commas.
	auto const object
	=	'{' >> -(objectVal % ',' >> -bp::lit(',')) >> '}'
	;
	auto const array
	=	'[' >> -(value % ',' >> -bp::lit(',')) >> ']'
	;

	// Define our recursive rule.
	auto const value_def
	=	bp::omit[
	 	 	string | object | array | bp::lit("null") | bp::bool_ | bp::double_
	 	]
	;
	// Allow C-style comments.
	auto const comments
	=	bp::ws
	|	"//" >> *(bp::char_ - bp::eol) >> bp::eol
	|	"/*" >> *(bp::char_ - "*/") >> "*/"
	;

	BOOST_PARSER_DEFINE_RULES(value);

	auto JSON(bp::parsable_range auto input) {
		dictionary.clear();
		auto success = bp::parse(input, value, comments);
		std::optional o {std::move(dictionary)};
		return success? o: o={};
	}

	auto JSON(std::istream& is) {
		return JSON(std::string {
			std::istreambuf_iterator<char> {is},
			std::istreambuf_iterator<char> {},
		});
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main() {
	if (auto const result = Parse::JSON(std::cin)) {
		std::cout << "Accept!\n";
		for (steno::Brief b : *result) {	
			std::cout << b.strokes << "\t" << b.text << "\n";
		}
	}
	else {
		std::cout << "Reject!\n";
	}
}
