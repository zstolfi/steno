#include "steno_parsers.hh"
#include <algorithm>
#include <sstream>

namespace steno {

/* ~~ Plain-Text Dictionary Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <>
void EntryIterator<Plain>::next() {
	do {
		std::string line {};
		std::getline(*input, line);
		if (std::all_of(line.begin(), line.end(), isWhitespace)) continue;
		auto split = line.find('=');
		if (split == line.npos) return fail();
		current = Brief {
			Phrase {line.substr(0, split)},
			line.substr(split+1),
		};
	} while (!over() && current.issues());
	if (input->eof()) finish();
}

/* ~~ JSON Dictionary Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <>
void EntryIterator<Json>::next() {
	auto parseString = [this] () -> std::string {
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
	};

	if (state.value == JsonState::FirstChar) {
		// Make sure the first token is '[' or '{', otherwise the file is not
		// JSON or not interesting.
		while (*input && isWhitespace(input->peek())) input->get();
		if (!input) return fail();
		if (input->peek() != '[' && input->peek() != '{') return fail();;
		state.value = JsonState::Body;
	}
	do {
		std::string stringL {}, stringR {};
		enum { StrL, Colon, StrR, Accept } state {StrL};
		while (*input && state != Accept) {
			if (state == StrL) {
				while (*input && input->peek() != '"') input->get();
				stringL = parseString();
				state = Colon;
			}
			else if (state == Colon) {
				for (char c; input->get(c); /**/) {
					if (isWhitespace(c)) /**/;
					else if (c == ':') { state = StrR; break; }
					else return fail();
				}
			}
			else if (state == StrR) {
				stringR = parseString();
				state = Accept;
			}
		}
		current = Brief {Phrase {stringL}, stringR};
		if (input->eof()) finish();
	} while (!over() && current.issues());
}

/* ~~ RTF/CRE Dictionary Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <>
void EntryIterator<Rtf>::next() {
	static constexpr std::string_view magic {R"({\rtf1)"};
	static constexpr std::string_view primer {R"({\*\cxs )"};
	if (state.value == RtfState::Header) {
		static_assert(magic.size() != 0);
		std::string front {};
		for (char c; input->get(c); /**/) {
			front += c;
			if (front.size() == magic.size()) break;
		}
		if (front != magic) return fail();

		unsigned count {0};
		for (char c; input->get(c); /**/) {
			if (count < primer.size()) {
				if (c == primer[count]) count++;
				else count = 0;
			}
			if (count == primer.size()) break;
		}
	}
	state.value = RtfState::Body;

	if (state.value == RtfState::Final) finish();
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
		if (split == line.npos) return fail();
		current = Brief {
			Phrase {line.substr(0, split)},
			line.substr(split+1, ending - (split+1)),
		};
		if (over()) state.value = RtfState::Final;
	} while (!over() && current.issues());
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::optional<Dictionary> parseDictionary(ParserInput& input, FileType type) {
	if (type != NoFileType) {
		if (type == Plain) {
			EntryIterator<Plain> begin {input}, end {};
			if (begin == end) return {};
			return Dictionary {begin, end};
		}
		if (type == Json) {
			EntryIterator<Json> begin {input}, end {};
			if (begin == end) return {};
			return Dictionary {begin, end};
		}
		if (type == Rtf) {
			EntryIterator<Rtf> begin {input}, end {};
			if (begin == end) return {};
			return Dictionary {begin, end};
		}
	}
	// In order to guess the file type we lose the luxury of being able to
	// iterate our data as it comes in. There's probably an advanced solution
	// which uses the first 100 or so bytes to determine the winning file type
	// and processes the (potentially ginormous) rest of the file that way.
	else {
		std::istreambuf_iterator<char> begin {input}, end {};
		std::string entireFile {begin, end};
		for (auto guess : {Rtf, Json, Plain}) {
			std::istringstream iss {entireFile};
			if (auto result = parseDictionary(iss, guess)) {
				return result;
			}
		}
	}
	return {};
}

} // namespace steno
