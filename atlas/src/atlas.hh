#pragma once
#include "steno.hh"
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

struct Mapping {
	virtual bool displayable(steno::Strokes) const = 0;
	virtual char summary(std::string) const = 0;
	virtual std::array<unsigned, 2> toPosition(steno::Strokes) const = 0;
	virtual steno::Strokes toStrokes(std::array<unsigned, 2>) const = 0;
};

template <bool BitOrder>
struct HilbertMap final : Mapping {
	bool displayable(steno::Strokes strokes) const {
		if (strokes.list.size() != 1) return false;
		// Ignore number bar strokes ... for now.
		steno::Stroke const Allowed {"STKPWHRAO*EUFRPBLGTSDZ"};
		steno::Stroke const first {strokes.list[0]};
		return (first & Allowed) == first;
	}

	char summary(std::string text) const {
		if (text.empty()) return {};
		return BitOrder? text.front(): text.back();
	}

	std::array<unsigned, 2> toPosition(steno::Strokes strokes) const {
		auto bitValue = customBitOrdering(strokes.list[0]);
		return math::hilbert(bitValue);
	}

	steno::Strokes toStrokes(std::array<unsigned, 2> position) const {
		return customBitOrdering_inv(math::hilbert_inv(position));
	}

private:
	uint32_t customBitOrdering(steno::Stroke x) const;
	steno::Stroke customBitOrdering_inv(uint32_t x) const;
};

using HilbertByPrefix = HilbertMap<true>;
using HilbertBySuffix = HilbertMap<false>;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Atlas {
	std::vector<uint8_t> image;
	HilbertByPrefix mapping;
	unsigned count = 0;
	Atlas() = default;

	static constexpr unsigned N = 2048;
	Atlas(steno::Dictionary dict): image{EmptyImage()} {
		std::for_each(dict.begin(), dict.end(), [this] (auto entry) {
			auto const& [strokes, text] = entry;
			if (!mapping.displayable(strokes)) return;
			auto [posX, posY] = mapping.toPosition(strokes);

			std::array<uint8_t, 3> rgb {255, 255, 255};
			if (char const c = mapping.summary(text)) {
				if ('a' <= c&&c <= 'z') rgb = hues[c - 'a'];
				if ('A' <= c&&c <= 'Z') rgb = hues[c - 'A'];
			}

			auto const i = N * (N-1 - posY) + posX;
			image[4*i+0] = rgb[0];
			image[4*i+1] = rgb[1];
			image[4*i+2] = rgb[2];
			image[4*i+3] = 255;
			this->count++;
		});
	}

	unsigned getCount() const { return count; }

	std::vector<std::vector<uint8_t>> getMipmaps() const {
		std::vector result (1, image);
		for (unsigned n=N; n/2; n/=2) {
			auto smaller = EmptyImage(n/2);
			auto& bigger = result.back();
			// Sparse atlases benefit from brighter bitmaps. Here we estimate
			// when is a good time to stop adding brightness.
			bool const average = n*n/(count+1) < 10;
			auto addToPixel = [&] (auto& to, auto from) {
				to = std::min(0xFF, to + (average? from/4: from));
			};
			for (unsigned y=0; y<n; y++)
			for (unsigned x=0; x<n; x++) {				
				auto const i = (n/2) * (y/2) + (x/2);
				auto const j = ( n ) * ( y ) + ( x );
				addToPixel(smaller[4*i+0], bigger[4*j+0]);
				addToPixel(smaller[4*i+1], bigger[4*j+1]);
				addToPixel(smaller[4*i+2], bigger[4*j+2]);
			}
			result.push_back(smaller);
		}
		return result;
	}

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

	static std::vector<uint8_t> EmptyImage(unsigned n = N) {
		std::vector<uint8_t> result (4*n*n, 0x00);
		for (auto i=0; i<n*n; i++) result[4*i+3] = 0xFF;
		return result;
	};
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// TODO: BitOrdering class to simplify this data.

template <>
uint32_t HilbertMap<true>::customBitOrdering(steno::Stroke x) const {
	return x.keys.S_ << 21
	|      x.keys.T_ << 20
	|      x.keys.K_ << 19
	|      x.keys.P_ << 18
	|      x.keys.W_ << 17
	|      x.keys.H_ << 16
	|      x.keys.R_ << 15
	|      x.keys.A  << 14
	|      x.keys.O  << 13
	|      x.keys.E  << 12
	|      x.keys.U  << 11
	|      x.keys.x  << 10
	|      x.keys._F <<  9
	|      x.keys._R <<  8
	|      x.keys._P <<  7
	|      x.keys._B <<  6
	|      x.keys._L <<  5
	|      x.keys._G <<  4
	|      x.keys._T <<  3
	|      x.keys._S <<  2
	|      x.keys._D <<  1
	|      x.keys._Z <<  0;
}

template <>
steno::Stroke HilbertMap<true>::customBitOrdering_inv(uint32_t x) const {
	steno::Stroke result {};
	if (x>>21 & 1) result.set(steno::Key::S_);
	if (x>>20 & 1) result.set(steno::Key::T_);
	if (x>>19 & 1) result.set(steno::Key::K_);
	if (x>>18 & 1) result.set(steno::Key::P_);
	if (x>>17 & 1) result.set(steno::Key::W_);
	if (x>>16 & 1) result.set(steno::Key::H_);
	if (x>>15 & 1) result.set(steno::Key::R_);
	if (x>>14 & 1) result.set(steno::Key::A );
	if (x>>13 & 1) result.set(steno::Key::O );
	if (x>>12 & 1) result.set(steno::Key::E );
	if (x>>11 & 1) result.set(steno::Key::U );
	if (x>>10 & 1) result.set(steno::Key::x );
	if (x>> 9 & 1) result.set(steno::Key::_F);
	if (x>> 8 & 1) result.set(steno::Key::_R);
	if (x>> 7 & 1) result.set(steno::Key::_P);
	if (x>> 6 & 1) result.set(steno::Key::_B);
	if (x>> 5 & 1) result.set(steno::Key::_L);
	if (x>> 4 & 1) result.set(steno::Key::_G);
	if (x>> 3 & 1) result.set(steno::Key::_T);
	if (x>> 2 & 1) result.set(steno::Key::_S);
	if (x>> 1 & 1) result.set(steno::Key::_D);
	if (x>> 0 & 1) result.set(steno::Key::_Z);
	return result;
}

template <>
uint32_t HilbertMap<false>::customBitOrdering(steno::Stroke x) const {
	return x.keys.S_ <<  0
	|      x.keys.T_ <<  1
	|      x.keys.K_ <<  2
	|      x.keys.P_ <<  3
	|      x.keys.W_ <<  4
	|      x.keys.H_ <<  5
	|      x.keys.R_ <<  6
	|      x.keys.A  <<  7
	|      x.keys.O  <<  8
	|      x.keys.E  <<  9
	|      x.keys.U  << 10
	|      x.keys.x  << 11
	|      x.keys._F << 12
	|      x.keys._R << 13
	|      x.keys._P << 14
	|      x.keys._B << 15
	|      x.keys._L << 16
	|      x.keys._G << 17
	|      x.keys._T << 18
	|      x.keys._S << 19
	|      x.keys._D << 20
	|      x.keys._Z << 21;
}

template <>
steno::Stroke HilbertMap<false>::customBitOrdering_inv(uint32_t x) const {
	steno::Stroke result {};
	if (x>> 0 & 1) result.set(steno::Key::S_);
	if (x>> 1 & 1) result.set(steno::Key::T_);
	if (x>> 2 & 1) result.set(steno::Key::K_);
	if (x>> 3 & 1) result.set(steno::Key::P_);
	if (x>> 4 & 1) result.set(steno::Key::W_);
	if (x>> 5 & 1) result.set(steno::Key::H_);
	if (x>> 6 & 1) result.set(steno::Key::R_);
	if (x>> 7 & 1) result.set(steno::Key::A );
	if (x>> 8 & 1) result.set(steno::Key::O );
	if (x>> 9 & 1) result.set(steno::Key::E );
	if (x>>10 & 1) result.set(steno::Key::U );
	if (x>>11 & 1) result.set(steno::Key::x );
	if (x>>12 & 1) result.set(steno::Key::_F);
	if (x>>13 & 1) result.set(steno::Key::_R);
	if (x>>14 & 1) result.set(steno::Key::_P);
	if (x>>15 & 1) result.set(steno::Key::_B);
	if (x>>16 & 1) result.set(steno::Key::_L);
	if (x>>17 & 1) result.set(steno::Key::_G);
	if (x>>18 & 1) result.set(steno::Key::_T);
	if (x>>19 & 1) result.set(steno::Key::_S);
	if (x>>20 & 1) result.set(steno::Key::_D);
	if (x>>21 & 1) result.set(steno::Key::_Z);
	return result;
}
