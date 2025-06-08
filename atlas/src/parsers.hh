#pragma once
#include "../../steno.hh"
#include <istream>
#include <optional>
#include <map>
#include <string>

// TODO: Move these into the steno/IO folder, or devise a better plan than that.
namespace steno {
	template <class T>
	using Parser = std::optional<T> (std::istream&);
	using Dictionary = std::map<steno::Stroke, std::string>;

	Dictionary const dummyValueJSON  {{Stroke {"SKWR-PBS"}, "JSON"}};
	Dictionary const dummyValueRTF   {{Stroke {"R-FT"}, "RTF"}};

	std::optional<steno::Dictionary> parsePlain(std::istream& input) {
		steno::Dictionary result {};
		std::string const separator = "  =  ";

		for (std::string line; std::getline(input, line);) {
			if (std::size_t i = line.find(separator); i!=line.npos) {
				std::string stroke = line.substr(0, i);
				std::string text = line.substr(i+separator.size());
				result[stroke] = text;
			}
		}

		return result;
	}

	std::optional<steno::Dictionary> parseJSON (std::istream&) { return dummyValueJSON;  }

	std::optional<steno::Dictionary> parseRTF  (std::istream&) { return dummyValueRTF;   }
}
