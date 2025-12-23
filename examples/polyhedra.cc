#include <steno.hh>

int main() {
	auto dict = steno::Dictionary {};

	auto poly_ = steno::Stroke {"POEUL"};
	auto _hedron = steno::Stroke {"HAOED"};
	auto _hedra = steno::Stroke {"HAOERD"};
	auto _hedral = steno::Stroke {"HAOERLD"};

	// https://www.youtube.com/watch?v=5xD7ndQPcg4
	dict[steno::Stroke {"PHRO*PBG"}] = "Platonic";
	dict[poly_ | _hedron] = "polyhedron";
	dict[steno::Stroke {"TET"} | _hedron] = "tetrahedron";
	dict[steno::Stroke {"OBGT"} | _hedron] = "octahedron";
	dict[steno::Stroke {"K*UB"}] = "cube";
	dict[steno::Stroke {"AOEUBGS"} | _hedron] = "icosahedron";
	dict[steno::Stroke {"TKOEBGD"} | _hedron] = "dodecahedron";

	auto inflections = steno::Dictionary {};
	for (auto const& entry : dict) if (entry.phrase().back() == _hedron) {
		// HEDRON -> HEDRA
		{
			auto [phrase, text] = entry;
			phrase.pop_back();
			phrase.push_back(_hedra);
			text = text.substr(0, text.find("hedron")) + "hedra";
			inflections[phrase] = text;
		}
		// HEDRON -> HEDRA (2 strokes)
		{
			auto [phrase, text] = entry;
			phrase.push_back(steno::Stroke {"RA"});
			text = text.substr(0, text.find("hedron")) + "hedra";
			inflections[phrase] = text;
		}
		// HEDRON -> HEDRONS
		{
			auto [phrase, text] = entry;
			phrase.back() += steno::Key::_Z;
			text += "s";
			inflections[phrase] = text;
		}
		// HEDRON -> HEDRAL
		{
			auto [phrase, text] = entry;
			phrase.pop_back();
			phrase.push_back(_hedral);
			text = text.substr(0, text.find("hedron")) + "hedral";
			inflections[phrase] = text;
		}
		// HEDRON -> HEDRAL (2 strokes)
		{
			auto [phrase, text] = entry;
			phrase.push_back(steno::Stroke {"RAL"});
			text = text.substr(0, text.find("hedron")) + "hedral";
			inflections[phrase] = text;
		}
	}
	dict.merge(std::move(inflections));

	for (auto const& entry : dict) {
		std::cout << entry << "\n";	
	}
}
