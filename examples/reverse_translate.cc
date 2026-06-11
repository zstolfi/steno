#include "steno.hh"
#include "steno_parsers.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <regex>
#include <format>
#include <cctype>

std::vector<std::string> lex(std::string input) {
	std::regex const pattern {R"([A-Za-z]+|[^\s])"};
	using Iter = std::regex_iterator<std::string::iterator>;
	Iter begin {input.begin(), input.end(), pattern}, end {};

	std::set<char> const sentenceEnders = {'.', '?', '!'};
	bool sentenceStart = true;

	std::vector<std::string> result {};
	for (auto it=begin; it!=end; ++it) {
		auto morpheme = it->str();
		char& first = morpheme.front();
		if (sentenceStart && std::isalpha(first)) {
			first = std::tolower(first);
			sentenceStart = false;
		}
		if (sentenceEnders.contains(first)) sentenceStart = true;

		if (isalpha(first)) result.push_back(morpheme);
		else result.push_back('{' + morpheme + '}');
	}
	return result;
}

std::vector<std::string> lex(std::istream& input) {
	using Iter = std::istreambuf_iterator<char>;
	Iter begin {input}, end {};
	return lex(std::string {begin, end});
}

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

steno::Brief best(std::vector<steno::Brief> const& options) {
	return options[0];
}

void reverseTranslate(
	std::istream& input,
	std::ostream& output,
	ReverseDictionary const& dict
) {
	for (auto word : lex(input)) {
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
