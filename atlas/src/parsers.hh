#pragma once
#include "../../steno.hh"
#include <map>
#include <string>

// TODO: Move these into the steno/IO folder, or devise a better plan than that.
namespace steno {
	using Dictionary = std::map<steno::Stroke, std::string>;

	Dictionary const dummyValuePlain {{Stroke {"PHRAEUPB"}, "Plain"}};
	Dictionary const dummyValueJSON  {{Stroke {"SKWR-PBS"}, "JSON"}};
	Dictionary const dummyValueRTF   {{Stroke {"R-FT"}, "RTF"}};

	std::optional<steno::Dictionary> parsePlain(std::istream&) { return dummyValuePlain; }
	std::optional<steno::Dictionary> parseJSON (std::istream&) { return dummyValueJSON;  }
	std::optional<steno::Dictionary> parseRTF  (std::istream&) { return dummyValueRTF;   }
}
