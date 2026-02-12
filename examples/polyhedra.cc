#include <steno.hh>
#include <map>

int main() {
	auto dict = steno::Dictionary {};

	auto _hedron = steno::Stroke {"HAOED"};
	auto _hedra = steno::Stroke {"HAOERD"};
	auto _hedral = steno::Stroke {"HAOERLD"};

	/* ~~ The Platonic Solids ~~ */
	// https://www.youtube.com/watch?v=5xD7ndQPcg4
	dict[steno::Phrase {"PHRO*PBG"}] = "Platonic";
	dict[steno::Phrase {"POEUL"} | _hedron] = "polyhedron";
	dict[steno::Phrase {"TET"} | _hedron] = "tetrahedron";
	dict[steno::Phrase {"OBGT"} | _hedron] = "octahedron";
	dict[steno::Phrase {"K*UB"}] = "cube";
	dict[steno::Phrase {"AOEUBGS"} | _hedron] = "icosahedron";
	dict[steno::Phrase {"TKOEBGD"} | _hedron] = "dodecahedron";

	/* ~~ The Archimedean Solids ~~ */
	// https://www.youtube.com/watch?v=_It-7VJH6n4
	dict[steno::Phrase {"ARBG/PHAOED"}] = "Archimedean";
	dict[steno::Phrase {"TRUPBT"}] = "truncate"; // verb
	dict[steno::Phrase {"TRUPBGS"}] = "truncation"; // noun
	dict[steno::Phrase {"STPHUB"}] = "snub";
	dict[steno::Phrase {"K*UB/OBGT"} | _hedron] = "cuboctahedron";
	dict[steno::Phrase {"RAUPLS"}] = "rhombus"; // noun (irregular plural)
	dict[steno::Phrase {"RAUPL"}] = "rhombi";
	dict[steno::Phrase {"RAUPL/K*UB/OBGT"} | _hedron] = "rhombicuboctahedron";
	dict[steno::Phrase {"AOEUBGS/TKOEBGD"} | _hedron] = "icosidodecahedron";

	/* ~~ Prisms and Antiprisms ~~ */
	// https://www.youtube.com/watch?v=01fSnAs0q0Q
	dict[steno::Phrase {"PREUFPL"}] = "prism";
	dict[steno::Phrase {"AEPBT/PREUFPL"}] = "antiprism";
	dict[steno::Phrase {"TK*EU"} | _hedron] = "dihedron";
	dict[steno::Phrase {"SPAPBD"}] = "expand"; // verb
	dict[steno::Phrase {"RA*UPL"}] = "{rhombi^}";

	/* ~~ Dihedra, Hosohedra and Spherical Polyhedra ~~ */
	// https://www.youtube.com/watch?v=n7rqeRkqsU4
	dict[steno::Phrase {"TKAOEPBLGT"}] = "degenerate"; // adjective
	dict[steno::Phrase {"HOS"} | _hedron] = "hosohedron";
	
	auto inflections = steno::Dictionary {};
	// -HEDRON words
	for (auto const& entry : dict) if (entry.phrase().back() == _hedron) {
		// -HEDRON -> -HEDRA
		{
			auto [phrase, text] = entry;
			phrase.pop_back();
			phrase.push_back(_hedra);
			text = text.substr(0, text.find("hedron")) + "hedra";
			inflections[phrase] = text;
		}
		// -HEDRON -> -HEDRA (2 strokes)
		{
			auto [phrase, text] = entry;
			phrase.push_back(steno::Stroke {"RA"});
			text = text.substr(0, text.find("hedron")) + "hedra";
			inflections[phrase] = text;
		}
		// -HEDRON -> -HEDRONS
		{
			auto [phrase, text] = entry;
			phrase.back() += steno::Key::_Z;
			text += "s";
			inflections[phrase] = text;
		}
		// -HEDRON -> -HEDRAL
		{
			auto [phrase, text] = entry;
			phrase.pop_back();
			phrase.push_back(_hedral);
			text = text.substr(0, text.find("hedron")) + "hedral";
			inflections[phrase] = text;
		}
		// -HEDRON -> -HEDRAL (2 strokes)
		{
			auto [phrase, text] = entry;
			phrase.push_back(steno::Stroke {"RAL"});
			text = text.substr(0, text.find("hedron")) + "hedral";
			inflections[phrase] = text;
		}
	}
	dict.merge(std::move(inflections));

	inflections.clear();
	{
		// Verb entries
		dict[steno::Stroke {"TRUPBTD"}] = "truncated";
		dict[steno::Stroke {"TRUPBGT"}] = "truncating";
		dict[steno::Stroke {"TRUPBTS"}] = "truncates";

		dict[steno::Phrase {"SPAPBTD"}] = "expanded";
		dict[steno::Phrase {"SPAPBGD"}] = "expanding";
		dict[steno::Phrase {"SPAPBDZ"}] = "expands";
	
		// Noun entries
		dict[steno::Stroke {"TRUPBGSZ"}] = "truncations";
		dict[steno::Phrase {"RAUPLSZ"}] = "rhombuses";
	}

	// THE- entries
	inflections.clear();
	for (auto const& entry : dict) {
		{
			auto [phrase, text] = entry;
			phrase.front() += steno::Key::Num;
			text = "the " + text;
			inflections[phrase] = text;
		}
		// Special rules for adding "the" before certain letters
		auto const LeftMask = steno::Stroke {"STKPWHR-"};
		auto const AddThe = std::map<steno::Stroke, steno::Stroke> {
			{{"   PW  -"}, {" T PW  -"}},
			{{" TK    -"}, {" TK  H -"}},
			{{" TKPW  -"}, {" TKPWH -"}},
			{{"S K W R-"}, {"STK W R-"}},
			{{"  K    -"}, {" TK    -"}},
			{{"     HR-"}, {" T   HR-"}},
		};
		{
			auto [phrase, text] = entry;
			auto const newLeft = AddThe.find(phrase.front() & LeftMask);
			if (newLeft != AddThe.end()) {
				phrase.front() -= LeftMask;
				phrase.front() += newLeft->second;
				text = "the " + text;
				inflections[phrase] = text;
			}
		}
	}
	dict.merge(std::move(inflections));

	for (auto const& entry : dict) {
		std::cout << steno::Alpha << entry << "\n";	
	}
}
