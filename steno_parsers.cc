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
		if (!*input) finish();
		else do {
			std::string line {};
			std::getline(*input, line);
			if (std::all_of(line.begin(), line.end(), isWhitespace)) continue;
			auto split = line.find('=');
			if (split == line.npos) { fail(); return; }
			current = Brief {
				Phrase {line.substr(0, split)},
				line.substr(split+1),
			};
		} while (!over() && current.failed());
	}
	else if (type == JSON) {
		do {
			std::string stringL {}, stringR {};
			enum { StrL, Colon, StrR, Accept } state {StrL};
			while (*input && state != Accept) {
				if (state == StrL) {
					while (*input && input->peek() != '"') input->get();
					stringL = parseStringJSON();
					state = Colon;
				}
				else if (state == Colon) {
					for (char c; input->get(c); /**/) {
						if (isWhitespace(c)) /**/;
						else if (c == ':') { state = StrR; break; }
						else { fail(); return; }
					}
				}
				else if (state == StrR) {
					stringR = parseStringJSON();
					state = Accept;
				}
			}
			current = Brief {Phrase {stringL}, stringR};
			if (input->eof()) finish();
		} while (!over() && current.failed());
	}
	else if (type == RTF) {
		static constexpr std::string_view primer {R"({\*\cxs )"};
		if (rtfState == rtfHeader) {
			unsigned count {0};
			for (char c; input->get(c); /**/) {
				if (count < primer.size()) {
					if (c == primer[count]) count++;
					else count = 0;
				}
				if (count == primer.size()) break;
			}
		}
		rtfState = rtfBody;

		if (rtfState == rtfFinal) finish();
		else do {
			std::string line {};
			unsigned count {0};
			for (char c; input->get(c); /**/) {
				line += c;
				if (count < primer.size()) {
					if (c == primer[count]) count++;
					else count = 0;
				}
				if (count == primer.size()) break;
			}
			auto ending = line.size() - (over()? 0: primer.size());
			auto split = line.find('}');
			if (split == line.npos) { fail(); return; }
			current = Brief {
				Phrase {line.substr(0, split)},
				line.substr(split+1, ending - (split+1)),
			};
			if (over()) rtfState = rtfFinal;
		} while (!over() && current.failed());
	} else {
		fail();
	}
}

std::string EntryIterator::parseStringJSON() {
	char c {}; std::string result {};
	while (input->get(c) && isWhitespace(c)) /**/;
	if (c != '"') return {};
	while (input->get(c)) {
		if (c == '\\') {
			if (!*input) { fail(); return {}; }
			char c = input->get();
			/**/ if (c == 'b') result += '\b';
			else if (c == 'f') result += '\f';
			else if (c == 'n') result += '\n';
			else if (c == 'r') result += '\r';
			else if (c == 't') result += '\t';
			else if (c == 'u') {
				/* TODO: UTF-8 encoding */
				std::string hex {};
				for (int i=0; i<4; i++) hex += input->get();
				// For the time being let's not discard any data.
				result += hex;
			}
			else result += c;
		}
		else if (c == '"') break;
		else result += c;
	}
	return result;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::optional<Dictionary> parseDictionary(ParserInput& input, FileType type) {
	if (type != NoFileType) {
		EntryIterator begin {input, type}, end {};
		if (begin == end) return {};
		return Dictionary {begin, end};
	}
	// In order to guess the file type we lose the luxury of being able to
	// iterate our data as it comes in. There's probably an advanced solution
	// which uses the first 100 or so bytes to determine the winning file type
	// and processes the (potentially ginormous) rest of the file that way.
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
