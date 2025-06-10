#pragma once
#include "../../steno.hh"
#include "parsers.hh" // for steno::Dictionary
#include <vector>
#include <map>
#include <string>
#include <cstdint>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

unsigned hilbert(unsigned iteration, unsigned x, unsigned y) {
	const auto N = 1 << 2*iteration, n = N/4;
	const auto S = 1 <<   iteration, s = S/2;
	if (iteration == 0) return 0;

	unsigned q = 0;
	const auto qx = x>=s, qy = y>=s;
	if (!qx && !qy) q = 0;
	if (!qx &&  qy) q = 1;
	if ( qx &&  qy) q = 2;
	if ( qx && !qy) q = 3;
	const auto quadrant = q;

	unsigned xNew = 0, yNew = 0;
	switch (quadrant) {
		break; case 0: xNew = y      , yNew = x      ;
		break; case 1: xNew = x      , yNew = y - s  ;
		break; case 2: xNew = x - s  , yNew = y - s  ;
		break; case 3: xNew = s-1 - y, yNew = S-1 - x;
	}

	return n*quadrant + hilbert(iteration-1, xNew, yNew);
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

			auto h = hilbert(11, x, y);
			auto iter = dict.find(fromCustomBitOrdering(h));

			// int r = 235, g = 235, b = 235;

			// if (iter == dict.end()) r = 0, g = 0, b = 0;
			// else {
			// 	static const std::array patterns {
			// 		/*0*/ std::regex {R"=(( |^)and( |$))="},
			// 		/*1*/ std::regex {R"=(( |^)because( |$))="},
			// 		/*2*/ std::regex {R"=(( |^)but( |$))="},
			// 		/*3*/ std::regex {R"=(( |^)he( |$))="},
			// 		/*4*/ std::regex {R"=(( |^)I( |$))="},
			// 		/*5*/ std::regex {R"=(( |^)you( |$))="},
			// 	};

			// 	for (unsigned i=0; i<patterns.size(); i++) {
			// 		if (std::regex_search(iter->second, patterns[i])) {
			// 			if (i == 0) r  = 255, g  = 100, b  =   0;
			// 			if (i == 1) r  = 200, g  =  50, b  =   0;
			// 			if (i == 2) r  = 140, g  =   0, b  = 255;
			// 			if (i == 3) r +=   0, g += 155, b +=  80;
			// 			if (i == 4) r +=   0, g +=  80, b += 130;
			// 			if (i == 5) r +=   0, g +=   0, b += 255;
			// 		}
			// 	}
			// 	if (r > 255) r = 255;
			// 	if (g > 255) g = 255;
			// 	if (b > 255) b = 255;
			// }

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
