#include "steno.hh"
#include "steno_parsers.hh"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

class ReverseDictionary {
	std::multimap<std::string, steno::Phrase> map {};

public:
	ReverseDictionary(steno::Dictionary const& d) {
		for (auto const& [phrase, text] : d) {
			auto [lower, upper] = map.equal_range(text);
			bool seen = false;
			for (auto it=lower; it!=upper; ++it) {
				if (it->second == phrase) { seen = true; break; }
			}
			if (!seen) map.insert({text, phrase});
		}
	}

	std::vector<steno::Brief> find(std::string text) const {
		std::vector<steno::Brief> result {};
		auto [lower, upper] = map.equal_range(text);
		for (auto it=lower; it!=upper; ++it) {
			result.emplace_back(it->second, it->first);
		}
		return result;
	}
};

class MorphemeIterator {
	std::istream* input {};
	std::string current {};
	/* Context goes here ... */

public:
	MorphemeIterator& operator++() {
		*input >> current;
		if (!*input) return *this = MorphemeIterator {};
		return *this;
	}

	using difference_type = unsigned;
	using value_type = std::string;

	MorphemeIterator() {}
	MorphemeIterator(std::istream& is): input{&is} {}
	auto operator<=>(MorphemeIterator const&) const = default;
	std::string operator*() { return current; }
	void operator++(int) { ++*this; }
};

steno::Brief best(std::vector<steno::Brief> const& options) {
	return options[0];
}

void reverseTranslate(
	std::istream& input,
	std::ostream& output,
	ReverseDictionary const& dict
) {
	std::string result {};
	using Iter = MorphemeIterator;
	for (Iter it {input}; it != Iter {}; ++it) {
		auto word = *it;
		auto options = dict.find(word);
		if (!options.empty()) {
			output << best(options).phrase() << "\n";
		}
		else output << word << "\n";
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
