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
	bool got = false;
	do {
		if (type == Plain) {
			std::string line {};
			std::getline(*input, line);
			if (std::all_of(line.begin(), line.end(), isWhitespace)) continue;
			auto split = line.find('=');
			if (split == line.npos) fail();
			current = Brief {
				Phrase {line.substr(0, split)},
				line.substr(split+1),
			};
			got = true;
		}
		else if (type == JSON) {
			fail(/* TODO */);
		}
		else if (type == RTF) {
			fail(/* TODO */);
		} else {
			fail();
		}
	} while (!over() && !got);
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
