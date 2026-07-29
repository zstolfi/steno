#include <steno.hh>
#include <map>

int main() {
	auto dict = steno::Dictionary {};

	auto _hedron = steno::Stroke {"HAOED"};
	auto _hedra = steno::Stroke {"HAOERD"};
	auto _hedral = steno::Stroke {"HAOERLD"};

	/* ~~ The Platonic Solids ~~ */
	// https://www.youtube.com/watch?v=5xD7ndQPcg4
	dict[steno::StrokeList {"PHRO*PBG"}] = "Platonic";
	dict[steno::StrokeList {"POEUL"} | _hedron] = "polyhedron";
	dict[steno::StrokeList {"TET"} | _hedron] = "tetrahedron";
	dict[steno::StrokeList {"OBGT"} | _hedron] = "octahedron";
	dict[steno::StrokeList {"K*UB"}] = "cube";
	dict[steno::StrokeList {"AOEUBGS"} | _hedron] = "icosahedron";
	dict[steno::StrokeList {"TKOEBGD"} | _hedron] = "dodecahedron";

	/* ~~ The Archimedean Solids ~~ */
	// https://www.youtube.com/watch?v=_It-7VJH6n4
	dict[steno::StrokeList {"ARBG/PHAOED"}] = "Archimedean";
	dict[steno::StrokeList {"TRUPBT"}] = "truncate"; // verb
	dict[steno::StrokeList {"TRUPBGS"}] = "truncation"; // noun
	dict[steno::StrokeList {"STPHUB"}] = "snub";
	dict[steno::StrokeList {"K*UB/OBGT"} | _hedron] = "cuboctahedron";
	dict[steno::StrokeList {"RAUPLS"}] = "rhombus"; // noun (irregular plural)
	dict[steno::StrokeList {"RAUPL"}] = "rhombi";
	dict[steno::StrokeList {"RAUPL/K*UB/OBGT"} | _hedron] = "rhombicuboctahedron";
	dict[steno::StrokeList {"AOEUBGS/TKOEBGD"} | _hedron] = "icosidodecahedron";

	/* ~~ Prisms and Antiprisms ~~ */
	// https://www.youtube.com/watch?v=01fSnAs0q0Q
	dict[steno::StrokeList {"PREUFPL"}] = "prism";
	dict[steno::StrokeList {"AEPBT/PREUFPL"}] = "antiprism";
	dict[steno::StrokeList {"TK*EU"} | _hedron] = "dihedron";
	dict[steno::StrokeList {"SPAPBD"}] = "expand"; // verb
	dict[steno::StrokeList {"RA*UPL"}] = "{rhombi^}";

	/* ~~ Dihedra, Hosohedra and Spherical Polyhedra ~~ */
	// https://www.youtube.com/watch?v=n7rqeRkqsU4
	dict[steno::StrokeList {"TKAOEPBLGT"}] = "degenerate"; // adjective
	dict[steno::StrokeList {"HOS"} | _hedron] = "hosohedron";

	auto inflections = steno::Dictionary {};
	// -HEDRON words
	for (auto const& entry : dict) if (entry.strokeList().back() == _hedron) {
		// -HEDRON -> -HEDRA
		{
			auto [strokeList, phrase] = entry;
			strokeList.pop_back();
			strokeList.push_back(_hedra);
			auto const str = phrase.string();
			phrase = str.substr(0, str.find("hedron")) + "hedra";
			inflections[strokeList] = phrase;
		}
		// -HEDRON -> -HEDRA (2 strokes)
		{
			auto [strokeList, phrase] = entry;
			strokeList.push_back(steno::Stroke {"RA"});
			auto const str = phrase.string();
			phrase = str.substr(0, str.find("hedron")) + "hedra";
			inflections[strokeList] = phrase;
		}
		// -HEDRON -> -HEDRONS
		{
			auto [strokeList, phrase] = entry;
			strokeList.back() += steno::Key::_Z;
			phrase += "s";
			inflections[strokeList] = phrase;
		}
		// -HEDRON -> -HEDRAL
		{
			auto [strokeList, phrase] = entry;
			strokeList.pop_back();
			strokeList.push_back(_hedral);
			auto const str = phrase.string();
			phrase = str.substr(0, str.find("hedron")) + "hedral";
			inflections[strokeList] = phrase;
		}
		// -HEDRON -> -HEDRAL (2 strokes)
		{
			auto [strokeList, phrase] = entry;
			strokeList.push_back(steno::Stroke {"RAL"});
			auto const str = phrase.string();
			phrase = str.substr(0, str.find("hedron")) + "hedral";
			inflections[strokeList] = phrase;
		}
	}
	dict.merge(std::move(inflections));

	inflections.clear();
	{
		// Verb entries
		dict[steno::Stroke {"TRUPBTD"}] = "truncated";
		dict[steno::Stroke {"TRUPBGT"}] = "truncating";
		dict[steno::Stroke {"TRUPBTS"}] = "truncates";

		dict[steno::StrokeList {"SPAPBTD"}] = "expanded";
		dict[steno::StrokeList {"SPAPBGD"}] = "expanding";
		dict[steno::StrokeList {"SPAPBDZ"}] = "expands";

		// Noun entries
		dict[steno::Stroke {"TRUPBGSZ"}] = "truncations";
		dict[steno::StrokeList {"RAUPLSZ"}] = "rhombuses";
	}

	// THE- entries
	inflections.clear();
	for (auto const& entry : dict) {
		{
			auto [strokeList, phrase] = entry;
			strokeList.front() += steno::Key::Num;
			phrase = "the " + phrase;
			inflections[strokeList] = phrase;
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
			auto [strokeList, phrase] = entry;
			auto const newLeft = AddThe.find(strokeList.front() & LeftMask);
			if (newLeft != AddThe.end()) {
				strokeList.front() -= LeftMask;
				strokeList.front() += newLeft->second;
				phrase = "the " + phrase;
				inflections[strokeList] = phrase;
			}
		}
	}
	dict.merge(std::move(inflections));

	for (auto const& entry : dict) {
		std::cout << steno::Alpha << entry << "\n";
	}
}
