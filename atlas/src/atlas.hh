#pragma once
#include <vector>

struct Atlas {
	std::vector<uint8_t> image;

	static constexpr unsigned N = 256;
	Atlas() : image(N*N*4) {
		for (unsigned y=0; y<N; y++)
		for (unsigned x=0; x<N; x++) {
			std::size_t i = 256*y + x;
			image[4*i + 0] = x^y;
			image[4*i + 1] = x^y;
			image[4*i + 2] = x^y;
			image[4*i + 3] = 255;
		}
	}
};
