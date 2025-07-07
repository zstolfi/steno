#pragma once
#include "../../steno.hh"
#include <array>
#include <bit>
#include <vector>
#include <map>
#include <string>
#include <cstdint>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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

	while (x || y) {
		unsigned s = std::max(std::bit_floor(x), std::bit_floor(y));
		bool flip = std::countr_zero(s) & 1;
		auto const quadrant = flip
		?	2*(y >= s) + (x >= s) ^ (y >= s)
		:	2*(x >= s) + (y >= s) ^ (x >= s);
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

steno::Stroke fromCustomBitOrdering(std::bitset<22> b) {
	steno::Stroke result {};
	result.keys.S_  = b[21];
	result.keys.T_  = b[20];
	result.keys.K_  = b[19];
	result.keys.P_  = b[18];
	result.keys.W_  = b[17];
	result.keys.H_  = b[16];
	result.keys.R_  = b[15];
	result.keys.A   = b[14];
	result.keys.O   = b[13];
	result.keys.E   = b[12];
	result.keys.U   = b[11];
	result.keys.x   = b[10];
	result.keys._F  = b[ 9];
	result.keys._R  = b[ 8];
	result.keys._P  = b[ 7];
	result.keys._B  = b[ 6];
	result.keys._L  = b[ 5];
	result.keys._G  = b[ 4];
	result.keys._T  = b[ 3];
	result.keys._S  = b[ 2];
	result.keys._D  = b[ 1];
	result.keys._Z  = b[ 0];
	return result;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct Atlas {
	std::vector<uint8_t> image;
	Atlas() = default;

	static constexpr unsigned N = 2048;
	Atlas(steno::Dictionary dict) : image(N*N*4, {}) {
		for (std::size_t i=0; i<=N*N; i++) {
			auto x = i%N;
			auto y = i/N;
			y = N-1 - y;

			auto h = hilbert_inv({x, y});
			auto iter = dict.find(fromCustomBitOrdering(h));

			std::array<uint8_t, 3> rgb {};
			static const std::array<std::array<uint8_t, 3>, 26> hues {{
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
				/*z*/ {229,  25, 72},
			}};

			if (iter == dict.end()) rgb = {0, 0, 0};
			else {
				char c = iter->second[0];
				/**/ if ('a' <= c&&c <= 'z') rgb = hues[c-'a'];
				else if ('A' <= c&&c <= 'Z') rgb = hues[c-'A'];
				else rgb = {255, 255, 255};
			}

			image[4*i+0] = rgb[0];
			image[4*i+1] = rgb[1];
			image[4*i+2] = rgb[2];
			image[4*i+3] = 255;
		}
	}
};
