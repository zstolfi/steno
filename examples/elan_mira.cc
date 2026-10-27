#define STENO_DEFAULT_LANGUAGE NoLanguage
#include "steno.hh"
#include <algorithm>
#include <bit>
#include <chrono>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <print>
#include <cstddef>

using Time = std::chrono::sys_time<std::chrono::milliseconds>;

using Error = std::string;

template <class T>
using Expected = std::expected<T, Error>;

struct Event {
	std::optional<Time> time {};
	steno::Stroke stroke {};
	steno::Phrase phrase {};
};

class Bytes {
	std::span<std::byte const> span_m {};

public:
	Bytes() = default;

	Bytes(std::convertible_to<std::span<std::byte const>> auto spanLike)
	:	span_m {spanLike} {}

//	bool operator== (Bytes const&) const = default;
//	auto operator<=>(Bytes const&) const = default;

	std::uint8_t bits(std::size_t byteIndex) const {
		return std::to_underlying(span_m[byteIndex]);
	}

	template <std::integral T>
	T bitsLe(std::size_t byteIndex) const {
		return bits_impl<T, std::endian::little>(byteIndex);
	}

	template <std::integral T>
	T bitsBe(std::size_t byteIndex) const {
		return bits_impl<T, std::endian::big>(byteIndex);
	}

	Bytes sub(auto ... args) const {
		return Bytes {span_m.subspan(args ... )};
	}

	std::size_t size() const {
		return span_m.size();
	}

	bool operator==(std::string str) const {
		if (span_m.size() != str.size()) return false;
		for (std::size_t i=0; i<span_m.size(); i++) {
			if (char(span_m[i]) != str[i]) return false;
		}
		return true;
	}

private:
	template <std::integral T, std::endian Endian>
	T bits_impl(std::size_t byteIndex) const {
		auto constexpr ByteCount = sizeof(T);
		T result {};
		for (std::size_t i=0; i<ByteCount; i++) {
			std::size_t j = i;
			if constexpr (Endian == std::endian::little) j = ByteCount-1 - i;
			result <<= 8;
			result |= std::to_underlying(span_m[byteIndex + j]);
		}
		return result;
	}
};

auto parseDateAndTime(Bytes file) {
	auto result = Expected<std::vector<Event>> {std::in_place};
	auto header = file.sub(0, 640);
	auto tape = file.sub(640);

	if (header.sub(0, 7) != "SGCAT32") {
		return result = std::unexpected("File is not an SGCAT file.\n");
	}

	if (header.size() != 640) {
		return result = std::unexpected("Incorrectly sized SGCAT file.\n");
	}

	if (tape.size() % 8) {
		return result = std::unexpected("Incorrectly sized SGCAT tape.\n");
	}

	auto dayBits = header.bitsLe<std::uint32_t>(16 /*can also be 24 or 28*/);
	auto day = std::chrono::sys_days {
		std::chrono::year  {signed(dayBits/10000 % 10000)} /
		std::chrono::month {       dayBits /100  % 100   } /
		std::chrono::day   {       dayBits /1    % 100   }
	};

	for (std::size_t i=0; i<tape.size()/8; i++) {
		auto strokeBits = tape.bitsBe<std::uint32_t>(8*i + 0) >> (32 - 23);
		auto timeBits   = tape.bitsLe<std::uint32_t>(8*i + 4);
		result->push_back(Event {
			.time = day + std::chrono::milliseconds {10 * timeBits},
			.stroke = steno::Stroke {steno::FromBits, strokeBits >> 1} +
				(strokeBits & 1? steno::Key::Num: steno::NoStroke)
			,
		});
	}

	return result;
}

auto parseClassic(Bytes strokes, Bytes timestamps={}) {
	auto result = Expected<std::vector<Event>> {};

//	interpolate(result);
	return result;
}

auto loadBytes(std::filesystem::path path) {
	auto result = std::vector<std::byte> {};
	if (std::ifstream fileStream {path}) {
		std::istreambuf_iterator<char> begin {fileStream}, end {};
		for (auto it=begin; it!=end; ++it) {
			result.push_back(std::byte(*it));
		}
	}
	else std::print(stderr, "Unable to open {}.\n", path.native());
	return result;
}

int main(int argc, char const* argv[]) {
	std::vector<std::string_view> args {argv+1, argv+argc};
	if (args.empty()) {
		std::print(stderr, "No files provided.\n");
		return EXIT_FAILURE;
	}

	auto file {loadBytes(args[0])};
	if (file.empty()) std::print(stderr, "There is no data to be parsed.\n");

	if (auto tape = parseDateAndTime(file))
	for (auto const& event: *tape) {
		std::string timeString {};
		std::string strokeString {};

		if (event.time) timeString = std::format("{}", *event.time);
		strokeString = steno::toString(
			event.stroke,
			steno::Wide | steno::Alpha
		);

		std::print("{:32}|{:23}|\n", timeString, strokeString);
	}

	return EXIT_SUCCESS;
}
