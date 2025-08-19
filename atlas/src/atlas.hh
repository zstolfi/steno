#pragma once
#include "steno.hh"
#include "window.hh" // TODO: remove dependency on Texture class
#include <array>
#include <bit>
#include <vector>
#include <map>
#include <string>
#include <cstdint>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

namespace math {

// Line -> Square
auto hilbert(unsigned val) -> std::array<unsigned, 2> {
	std::array<unsigned, 2> pos {0, 0};
	auto& [x, y] = pos;

	bool flip = false;
	for (unsigned s=1; val; val/=4, s*=2, flip^=1) switch (val%4) {
	case 0: break;
	case 1: pos = { y + (flip? s  : 0    ),  x + (flip? 0    : s  )}; break;
	case 2: pos = { y + (flip? s  : s    ),  x + (flip? s    : s  )}; break;
	case 3: pos = {-x + (flip? s-1: 2*s-1), -y + (flip? 2*s-1: s-1)}; break;
	}

	return pos;
}

// Square -> Line
auto hilbert_inv(std::array<unsigned, 2> pos) -> unsigned {
	unsigned val = 0;
	auto& [x, y] = pos;

	// Definition of the first iteration of the Hilbert curve.
	//     curve: [3]──[2]    curve^T: [1]──[2]
	//                  │               │    │ 
	//            [0]──[1]             [0]  [3]
	static constexpr int curve [2][2] = {{0, 1}, {3, 2}};
	static constexpr int curveT[2][2] = {{0, 3}, {1, 2}};

	while (x || y) {
		unsigned s = std::max(std::bit_floor(x), std::bit_floor(y));
		bool flip = std::countr_zero(s) & 1;
		auto const quadrant = (flip? curveT: curve) [x >= s] [y >= s];
		switch (quadrant) {
		case 0: break;
		case 1: pos = { y + (flip? 0  : -s   ),  x + (flip? -s   : 0  )}; break;
		case 2: pos = { y + (flip? -s : -s   ),  x + (flip? -s   : -s )}; break;
		case 3: pos = {-x + (flip? s-1: 2*s-1), -y + (flip? 2*s-1: s-1)}; break;
		}
		val += s*s * quadrant;
	}

	return val;
}

} // namespace math

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// TODO: Possibly make this a singleton.
struct Mapping {
	virtual std::string name() const = 0;
	virtual std::array<unsigned, 2> size() const = 0;
	virtual bool displayable(steno::Phrase) const = 0;
	virtual char summary(std::string) const = 0;
	virtual std::array<unsigned, 2> toPosition(steno::Phrase) const = 0;
	virtual steno::Phrase toPhrase(std::array<unsigned, 2>) const = 0;
};

template <bool BitOrder>
struct HilbertMap final : Mapping {
	std::string name() const {
		return BitOrder? "prefix": "suffix";
	}

	std::array<unsigned, 2> size() const {
		return {2048, 2048};
	}

	bool displayable(steno::Phrase phrase) const {
		if (phrase.size() != 1) return false;
		// Ignore number bar phrase ... for now.
		steno::Stroke const Allowed {"STKPWHRAO*EUFRPBLGTSDZ"};
		steno::Stroke const first {phrase[0]};
		return (first & Allowed) == first;
	}

	char summary(std::string text) const {
		if (text.empty()) return {};
		return BitOrder? text.front(): text.back();
	}

	std::array<unsigned, 2> toPosition(steno::Phrase phrase) const {
		auto bitValue = customBitOrdering(phrase[0]);
		return math::hilbert(bitValue);
	}

	steno::Phrase toPhrase(std::array<unsigned, 2> position) const {
		return customBitOrdering_inv(math::hilbert_inv(position));
	}

private:
	uint32_t customBitOrdering(steno::Stroke x) const;
	steno::Stroke customBitOrdering_inv(uint32_t x) const;
};

using HilbertByPrefix = HilbertMap<true>;
using HilbertBySuffix = HilbertMap<false>;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Atlas {
	struct View {
		// TODO: Image class (kind of like a std::mdspn)
		Mapping* mapping;
		std::vector<uint8_t> image;
		Texture texture;
		unsigned count = 0;

		View() = default;
		View(steno::Dictionary dict, Mapping* m): mapping{m}, image{EmptyImage()} {
			std::for_each(dict.begin(), dict.end(), [this] (auto entry) {
				auto const& [phrase, text] = entry;
				if (!mapping->displayable(phrase)) return;
				auto [posX, posY] = mapping->toPosition(phrase);

				std::array<uint8_t, 3> rgb {255, 255, 255};
				if (char const c = mapping->summary(text)) {
					if ('a' <= c&&c <= 'z') rgb = hues[c - 'a'];
					if ('A' <= c&&c <= 'Z') rgb = hues[c - 'A'];
				}

				auto const i = N * (N-1 - posY) + posX;
				image[4*i+0] = rgb[0];
				image[4*i+1] = rgb[1];
				image[4*i+2] = rgb[2];
				image[4*i+3] = 255;
				count++;
			});
			// Generate mipmaps.
			std::vector mipmaps (1, image);
			for (unsigned n=N; n/2; n/=2) {
				auto smaller = EmptyImage(n/2);
				auto& bigger = mipmaps.back();
				// Sparse atlases benefit from brighter bitmaps. Here we estimate
				// when is a good time to stop adding brightness.
				bool const darken = n*n/(count+1) < 10;
				auto addToPixel = [&] (auto& to, auto from) {
					to = std::min(0xFF, to + (darken? from/4: from));
				};
				for (unsigned y=0; y<n; y++)
				for (unsigned x=0; x<n; x++) {				
					auto const i = (n/2) * (y/2) + (x/2);
					auto const j = ( n ) * ( y ) + ( x );
					addToPixel(smaller[4*i+0], bigger[4*j+0]);
					addToPixel(smaller[4*i+1], bigger[4*j+1]);
					addToPixel(smaller[4*i+2], bigger[4*j+2]);
				}
				mipmaps.push_back(smaller);
			}
			texture = Texture {mipmaps, Atlas::N, Atlas::N};
		}

		static std::vector<uint8_t> EmptyImage(unsigned n = N) {
			std::vector<uint8_t> result (4*n*n, 0x00);
			for (auto i=0; i<n*n; i++) result[4*i+3] = 0xFF;
			return result;
		}
	};

	static inline std::array<Mapping*, 2> const Mappings = {{
		new HilbertByPrefix,
		new HilbertBySuffix,
	}};

	std::array<View, Mappings.size()> views;
	std::optional<unsigned> viewIndex;
	View const& getView() const { return views[*viewIndex]; }

public:
	Atlas() = default;

	static constexpr unsigned N = 2048;
	Atlas(steno::Dictionary dict) {
		for (int i=0; i<Mappings.size(); i++) {
			views[i] = View {dict, Mappings[i]};
		}
		viewIndex = 0;
	}

	unsigned getViewCount() const { return viewIndex? Mappings.size(): 0; }
	unsigned getViewIndex() const { return *viewIndex; }
	void setViewIndex(unsigned i) { viewIndex = i; }

	std::vector<uint8_t> const& getImage() const { return getView().image; }

	Mapping const* getMapping() const { return getView().mapping; }

	unsigned getCount() const { return getView().count; }

	ImTextureID getTexture() const { return getView().texture.get(); }

private:
	static constexpr std::array<std::array<uint8_t, 3>, 26> hues {{
		/*a*/ {229,  25,  25},
		/*b*/ {229,  72,  25},
		/*c*/ {229, 119,  25},
		/*d*/ {229, 166,  25},
		/*e*/ {229, 213,  25},
		/*f*/ {198, 229,  25},
		/*g*/ {151, 229,  25},
		/*h*/ {103, 229,  25},
		/*i*/ { 56, 229,  25},
		/*j*/ { 25, 229,  41},
		/*k*/ { 25, 229,  88},
		/*l*/ { 25, 229, 135},
		/*m*/ { 25, 229, 182},
		/*n*/ { 25, 229, 229},
		/*o*/ { 25, 182, 229},
		/*p*/ { 25, 135, 229},
		/*q*/ { 25,  88, 229},
		/*r*/ { 25,  41, 229},
		/*s*/ { 56,  25, 229},
		/*t*/ {103,  25, 229},
		/*u*/ {151,  25, 229},
		/*v*/ {198,  25, 229},
		/*w*/ {229,  25, 213},
		/*x*/ {229,  25, 166},
		/*y*/ {229,  25, 119},
		/*z*/ {229,  25,  72},
	}};
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// TODO: BitOrdering class to simplify this data.

template <>
uint32_t HilbertMap<true>::customBitOrdering(steno::Stroke stroke) const {
	using enum steno::Key;
	return stroke[S_] << 21
	|      stroke[T_] << 20
	|      stroke[K_] << 19
	|      stroke[P_] << 18
	|      stroke[W_] << 17
	|      stroke[H_] << 16
	|      stroke[R_] << 15
	|      stroke[A ] << 14
	|      stroke[O ] << 13
	|      stroke[E ] << 12
	|      stroke[U ] << 11
	|      stroke[x ] << 10
	|      stroke[_F] <<  9
	|      stroke[_R] <<  8
	|      stroke[_P] <<  7
	|      stroke[_B] <<  6
	|      stroke[_L] <<  5
	|      stroke[_G] <<  4
	|      stroke[_T] <<  3
	|      stroke[_S] <<  2
	|      stroke[_D] <<  1
	|      stroke[_Z] <<  0;
}

template <>
steno::Stroke HilbertMap<true>::customBitOrdering_inv(uint32_t n) const {
	steno::Stroke result {};
	using enum steno::Key;
	result[S_] = (n>>21) & 1;
	result[T_] = (n>>20) & 1;
	result[K_] = (n>>19) & 1;
	result[P_] = (n>>18) & 1;
	result[W_] = (n>>17) & 1;
	result[H_] = (n>>16) & 1;
	result[R_] = (n>>15) & 1;
	result[A ] = (n>>14) & 1;
	result[O ] = (n>>13) & 1;
	result[E ] = (n>>12) & 1;
	result[U ] = (n>>11) & 1;
	result[x ] = (n>>10) & 1;
	result[_F] = (n>> 9) & 1;
	result[_R] = (n>> 8) & 1;
	result[_P] = (n>> 7) & 1;
	result[_B] = (n>> 6) & 1;
	result[_L] = (n>> 5) & 1;
	result[_G] = (n>> 4) & 1;
	result[_T] = (n>> 3) & 1;
	result[_S] = (n>> 2) & 1;
	result[_D] = (n>> 1) & 1;
	result[_Z] = (n>> 0) & 1;
	return result;
}

template <>
uint32_t HilbertMap<false>::customBitOrdering(steno::Stroke stroke) const {
	using enum steno::Key;
	return stroke[S_] <<  0
	|      stroke[T_] <<  1
	|      stroke[K_] <<  2
	|      stroke[P_] <<  3
	|      stroke[W_] <<  4
	|      stroke[H_] <<  5
	|      stroke[R_] <<  6
	|      stroke[A ] <<  7
	|      stroke[O ] <<  8
	|      stroke[E ] <<  9
	|      stroke[U ] << 10
	|      stroke[x ] << 11
	|      stroke[_F] << 12
	|      stroke[_R] << 13
	|      stroke[_P] << 14
	|      stroke[_B] << 15
	|      stroke[_L] << 16
	|      stroke[_G] << 17
	|      stroke[_T] << 18
	|      stroke[_S] << 19
	|      stroke[_D] << 20
	|      stroke[_Z] << 21;
}

template <>
steno::Stroke HilbertMap<false>::customBitOrdering_inv(uint32_t n) const {
	steno::Stroke result {};
	using enum steno::Key;
	result[S_] = (n>> 0) & 1;
	result[T_] = (n>> 1) & 1;
	result[K_] = (n>> 2) & 1;
	result[P_] = (n>> 3) & 1;
	result[W_] = (n>> 4) & 1;
	result[H_] = (n>> 5) & 1;
	result[R_] = (n>> 6) & 1;
	result[A ] = (n>> 7) & 1;
	result[O ] = (n>> 8) & 1;
	result[E ] = (n>> 9) & 1;
	result[U ] = (n>>10) & 1;
	result[x ] = (n>>11) & 1;
	result[_F] = (n>>12) & 1;
	result[_R] = (n>>13) & 1;
	result[_P] = (n>>14) & 1;
	result[_B] = (n>>15) & 1;
	result[_L] = (n>>16) & 1;
	result[_G] = (n>>17) & 1;
	result[_T] = (n>>18) & 1;
	result[_S] = (n>>19) & 1;
	result[_D] = (n>>20) & 1;
	result[_Z] = (n>>21) & 1;
	return result;
}
