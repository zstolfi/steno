#include <algorithm>
#include <any>
#include <tuple> // for std::tie

namespace steno {

class Dictionary {
	struct MapEntry {
		// TODO: Better way to allow conflicts.
		std::string text {};
		std::any data {}; // Allow custom user-data.
	};
	std::map<Strokes, MapEntry> m_entries {};

public:
	Dictionary() = default;
	Dictionary(std::span<Brief>             bb) { for (auto b : bb) add(b); }
	Dictionary(std::initializer_list<Brief> bb) { for (auto b : bb) add(b); }

	const auto& getEntries() const { return m_entries; }

	using Entry = typename decltype(m_entries)::value_type;
	using Iterator = typename decltype(m_entries)::iterator;
	enum BlendingMode { Top, Behind, Conflict/*, Ask*/ };

	Iterator add(Brief b, std::any data={}, BlendingMode mode=Conflict) {
		// Find our spot of interest.
		auto it = m_entries.find(b.strokes);
		// Assign if it's not taken.
		if (it == m_entries.end()) {
			std::tie(it, std::ignore) = m_entries.insert(
				Entry {b.strokes, {{b.text}, data}}
			);
		}
		else switch (mode) {
		break; case Conflict: it->second.text = makeConflict(it->second.text, b.text);
		break; case Top     : it->second.text = b.text;
		break; case Behind  : {}
		}
		return it;
	}

	// TODO: add() for a std::span or std::iterator_list

	std::string translate(Strokes xx) const {
		auto it = m_entries.find(xx);
		return it != m_entries.end()
			? it->second.text
			: toString(xx);
	}

	Dictionary& merge(const Dictionary& other, BlendingMode=Top) {
		m_entries.insert(other.m_entries.begin(), other.m_entries.end());
		return *this;
	}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	Dictionary& genContraction(Strokes longHand, Strokes shortHand, BlendingMode=Top) {
		Dictionary result {};
		for (const auto& [strokes, translation] : m_entries) {
			// TODO: Account for multiple occurences in the same brief.
			auto it = std::search(
				strokes.list.begin(), strokes.list.end(),
				longHand.list.begin(), longHand.list.end()
			);
			if (it != strokes.list.end()) {
				const auto s = std::span {strokes.list};
				const auto i = std::distance(strokes.list.begin(), it);
				const Strokes left  {s.subspan(0, i)};
				const Strokes right {s.subspan(i + longHand.list.size())};
				result.add({left | shortHand | right, translation.text});
			}
		}
		return merge(result);
	}

private:
	static std::string makeConflict(std::string a, std::string b) {
		return a + '/' + b;
	}
};

} // namespace steno
