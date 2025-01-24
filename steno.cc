#include "steno.hh"
#include <algorithm>
#include <cstdint>

namespace steno {

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke::Stroke(std::string s) {
	const std::string Whitespace = " \t";
	const std::string Left   = "STKPWHR";
	const std::string Middle = "AO*EU";
	const std::string Right  = "FRPBLGTSDZ";
	const std::string Misc   = "-";
	auto in = [](std::string set) {
		return [set](char c) { return set.find(c) != set.npos; };
	};

	// Remove whitespace.
	s.erase(
		std::remove_if(s.begin(), s.end(), in(Whitespace)),
		s.end()
	);

	// Sinple charset test:
	if (!std::all_of(s.begin(), s.end(), in(Left+Middle+Right+Misc))) {
		failConstruction(); return;
	}

	// Find unique middle secion (vowels).
	unsigned v0 {}, v1 {};
	if (auto dash = s.find('-'); dash != s.npos) {
		v0 = dash;
		v1 = dash+1;
	}
	else if (auto i = s.find_first_of(Middle); i != s.npos) {
		auto j = s.find_first_not_of(Middle, i);
		v0 = i;
		v1 = (j != s.npos) ? j : s.size();
	}
	// Fail if there is not a middle section.
	else { failConstruction(); return; }

	// Split 's' into substrings.
	const std::string sL = s.substr(0, v0);
	const std::string sM = s.substr(v0, v1-v0);
	const std::string sR = s.substr(v1, s.size()-v1);

	// More checks.
	if (!std::all_of(sL.begin(), sL.end(), in(Left  ))
	||  !std::all_of(sM.begin(), sM.end(), in(Middle))
	||  !std::all_of(sR.begin(), sR.end(), in(Right ))) {
		failConstruction(); return;
	}

	// Assign.
	for (char c : sL) this->keys.bits[Left  .find(c) +  1] = true;
	for (char c : sM) this->keys.bits[Middle.find(c) +  8] = true;
	for (char c : sR) this->keys.bits[Right .find(c) + 13] = true;
}

Stroke::Stroke(FromBits_t, std::bitset<23> b) {
	this->keys.Num = b[ 0];
	this->keys.S_  = b[ 1];
	this->keys.T_  = b[ 2];
	this->keys.K_  = b[ 3];
	this->keys.P_  = b[ 4];
	this->keys.W_  = b[ 5];
	this->keys.H_  = b[ 6];
	this->keys.R_  = b[ 7];
	this->keys.A   = b[ 8];
	this->keys.O   = b[ 9];
	this->keys.x   = b[10];
	this->keys.E   = b[11];
	this->keys.U   = b[12];
	this->keys._F  = b[13];
	this->keys._R  = b[14];
	this->keys._P  = b[15];
	this->keys._B  = b[16];
	this->keys._L  = b[17];
	this->keys._G  = b[18];
	this->keys._T  = b[19];
	this->keys._S  = b[20];
	this->keys._D  = b[21];
	this->keys._Z  = b[22];
}

Stroke::Stroke(FromBitsReversed_t, std::bitset<23> b) {
	this->keys.Num = b[22];
	this->keys.S_  = b[21];
	this->keys.T_  = b[20];
	this->keys.K_  = b[19];
	this->keys.P_  = b[18];
	this->keys.W_  = b[17];
	this->keys.H_  = b[16];
	this->keys.R_  = b[15];
	this->keys.A   = b[14];
	this->keys.O   = b[13];
	this->keys.x   = b[12];
	this->keys.E   = b[11];
	this->keys.U   = b[10];
	this->keys._F  = b[ 9];
	this->keys._R  = b[ 8];
	this->keys._P  = b[ 7];
	this->keys._B  = b[ 6];
	this->keys._L  = b[ 5];
	this->keys._G  = b[ 4];
	this->keys._T  = b[ 3];
	this->keys._S  = b[ 2];
	this->keys._D  = b[ 1];
	this->keys._Z  = b[ 0];
}

std::strong_ordering Stroke::operator<=>(const Stroke other) const {
	return this->keys.bits.to_ulong()
	<=>    other.keys.bits.to_ulong();
}

Stroke Stroke::operator+=(Stroke other) {
	this->keys.bits |= other.keys.bits;
	return *this;
}

void Stroke::failConstruction() { this->keys.FailedConstruction = true; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Strokes::Strokes(Stroke x) {
	this->list = List_t {x};
	normalize();
}

Strokes::Strokes(std::span<Stroke> span) {
	this->list = List_t (span.begin(), span.end());
	normalize();
}

Strokes::Strokes(std::initializer_list<Stroke> il) {
	this->list = List_t (il.begin(), il.end());
	normalize();
}

Strokes::Strokes(std::string s) {
	auto push = [&](unsigned i, unsigned j) {
		// if (i == j) return false;
		this->list.emplace_back(s.substr(i, j-i));
		if (this->list.back().keys.FailedConstruction) return false;
		return true;
	};

	signed i=0, j=0;
	while (j=s.find('/', i), j!=s.npos) {
		if (push(i, j) == false) return;
		i = j+1;
	}
	push(i, s.size());
	normalize();
}

// std::strong_ordering Strokes::operator<=>(Strokes other) {
// 	return this->list
// 	<=>    other.list;
// }

Stroke& Strokes::operator[](std::size_t i) /* */ { return this->list[i]; }
Stroke  Strokes::operator[](std::size_t i) const { return this->list[i]; }

Strokes& Strokes::append(Stroke x) {
	this->list.insert(this->list.end(), x);
	return *this;
}

Strokes& Strokes::prepend(Stroke x) {
	this->list.insert(this->list.begin(), x);
	return *this;
}

Strokes& Strokes::append(const Strokes& xx) {
	this->list.insert(this->list.end(), xx.list.begin(), xx.list.end());
	return *this;
}

Strokes& Strokes::prepend(const Strokes& xx) {
	this->list.insert(this->list.begin(), xx.list.begin(), xx.list.end());
	return *this;
}

Strokes& Strokes::operator|=(const Strokes& xx) {
	return this->append(xx);
}


void Strokes::normalize() {
	// // Remove empty strokes.
	// this->list.erase(
	// 	std::remove(this->list.begin(), this->list.end(), NoStroke),
	// 	this->list.end()
	// );
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

Stroke  operator+(Stroke  x , Stroke  y ) { return x += y ; }
Strokes operator+(Strokes xx, Stroke  y ) { return xx.list.back() += y; }
Strokes operator+(Stroke  x , Strokes yy) { return yy.list.front() += x; }

Strokes operator|(Stroke  x , Stroke  y ) { return Strokes {x, y}; }
Strokes operator|(Strokes xx, Stroke  y ) { return xx.append(y); }
Strokes operator|(Stroke  x , Strokes yy) { return yy.prepend(x); }
Strokes operator|(Strokes xx, Strokes yy) { return xx.append(yy); }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

} // namespace steno
