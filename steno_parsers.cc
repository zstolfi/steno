#include "steno_parsers.hh"
#include <algorithm>
#include <sstream>

namespace /*anonymous*/ {

bool isWhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // namespace /*anonymous*/

namespace steno {

void EntryIterator::next() {
	if (type == Plain) {
		do {
			std::string line {};
			std::getline(*input, line);
			if (std::all_of(line.begin(), line.end(), isWhitespace)) continue;
			auto split = line.find('=');
			if (split == line.npos) fail();
			current = Brief {
				Phrase {line.substr(0, split)},
				line.substr(split+1),
			};
		} while (!over() && current.failed());
	}
	// TODO: Listen for escape characters.
	else if (type == JSON) {
		do {
			std::string stringL {}, stringR {};
			enum { Form, StrL, Colon, StrR, Accept } state {Form};
			for (char c; input->get(c); /**/) {
				/**/ if (state != StrL && state != StrR && isWhitespace(c)) ;
				else if (state == Form && c == '"') state = StrL;
				else if (state == StrL && c == '"') state = Colon;
				else if (state == StrL) stringL += c;
				else if (state == Colon && c == '"') state = StrR;
				else if (state == Colon && c != ':') break;
				else if (state == StrR && c == '"') state = Accept;
				else if (state == StrR) stringR += c;
				else if (state == Accept) {
					current = Brief {Phrase {stringL}, stringR};
					break;
				}
			}
		} while (!over() && current.failed());
	}
	else if (type == RTF) {
		fail(/* TODO */);
	} else {
		fail();
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::optional<Dictionary> parseDictionary(ParserInput& input, FileType type) {
	if (type != NoFileType) {
		EntryIterator begin {input, type}, end {};
		if (begin == end) return {};
		// TODO: Proper range constructors for steno types.
		return Dictionary {std::vector<Brief> {begin, end}};
	}
	// In order to guess the file type we lose the luxury of being able to
	// iterate our data as it comes in. There's probably an advanced solution
	// which uses the first 100 or so bytes to determine the winning file type
	// and process the (potentially ginormous) rest of the file that way.
	else {
		std::istreambuf_iterator<char> begin {input}, end {};
		std::string entireFile {begin, end};
		for (auto guess : {RTF, JSON, Plain}) {
			std::istringstream iss {entireFile};
			if (auto result = parseDictionary(iss, guess)) {
				return result;
			}
		}
	}
	return {};
}

} // namespace steno
