#include <algorithm>
#include <variant> // for std::monostate
#include <optional>

namespace steno {

template <typename Data_t = std::monostate>
class Dictionary {
	struct MapEntry {
		// TODO: Better way to allow conflicts.
		std::string text {};
		Data_t data {}; // Allow custom user-data.
	};
	std::map<Strokes, MapEntry> m_entries {};

public:
	Dictionary() = default;

	Dictionary(std::span<Brief> span) {
		for (Brief b : span) m_entries.add(b);
	}

	Dictionary(std::initializer_list<Brief> il) {
		for (Brief b : il) m_entries.add(b);
	}

	using Entry = typename decltype(m_entries)::value_type;
	const auto& getEntries() const { return m_entries; }

	Dictionary& add(Brief b, Data_t d={}) {
		// Find our spot of interest.
		auto it = m_entries.find(b.strokes);
		// Assign if it's not taken.
		if (it == m_entries.end()) m_entries[b.strokes] = {{b.text}, d};
		// Otherwise create a conflict.
		else it->second.text = makeConflict(it->second.text, b.text);
		return *this;
	}

	Dictionary& merge(const Dictionary& other) {
		m_entries.insert(other.m_entries.begin(), other.m_entries.end());
		return *this;
	}

	std::string translate(Strokes xx) const {
		auto it = m_entries.find(xx);
		return it != m_entries.end()
			? it->second.text
			: toString(xx);
	}

private:
	static std::string makeConflict(std::string a, std::string b) {
		return a + '/' + b;
	}
};

} // namespace steno
