#include "steno.hh"
#include "steno_parsers.hh"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

struct ReverseDictionary : std::multimap<std::string, steno::Phrase> {
	ReverseDictionary(steno::Dictionary const& d) {
		for (auto const& [phrase, text] : d) {
			auto [lower, upper] = this->equal_range(text);
			bool seen = false;
			for (auto it=lower; it!=upper; ++it) {
				if (it->second == phrase) { seen = true; break; }
			}
			if (!seen) this->insert({text, phrase});
		}
	}
};

using ReverseEntry = std::pair<std::string, steno::Phrase>;

struct Word {
	std::string text {};
	friend std::istream& operator>>(std::istream& is, Word& w) {
		is >> w.text;
		return is;
	}
};

ReverseEntry best(std::vector<ReverseEntry> const& options) {
	return options[0];
}

void reverseTranslate(
	std::istream& input,
	std::ostream& output,
	ReverseDictionary const& dict
) {
	std::string result {};
	using Iter = std::istream_iterator<Word>;
	for (Iter it {input}; it != Iter {}; ++it) {
		auto& word = *it;
		auto [begin, end] = dict.equal_range(word.text);
		std::vector<ReverseEntry> options {begin, end};
		if (!options.empty()) {
			output << best(options).second << "\n";
		}
		else output << word.text << "\n";
	}
}

int main(int argc, char const* argv[]) {
	std::vector<std::string> args {argv + 1, argv + argc};
	if (args.empty()) std::cerr << "No arguments provided\n";
	steno::Dictionary forwardDictionary {};
	for (std::string path : args) {
		if (std::ifstream file {path}) {
			std::istreambuf_iterator<char> begin {file}, end {};
			std::vector<char> bytes {begin, end};
			if (auto result = steno::parseGuess(bytes)) {
				for (auto const& entry : *result) {
					forwardDictionary.insert(entry);
				}
			}
			else std::cout << "Unable to parse " << path << "\n";
		}
		else std::cerr << "Unable to open " << path << "\n";
	}
	if (forwardDictionary.empty()) std::cerr << "No dictionaries loaded\n";
	reverseTranslate(std::cin, std::cout, forwardDictionary);
}
