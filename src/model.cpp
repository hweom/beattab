#include "beattab/model.hpp"
#include <numeric>
#include <sstream>
namespace bt {
Rational::Rational(int64_t a, int64_t b) : n(a), d(b) {
    if (d == 0) { n = 0; d = 1; } // parser rejects zero denominators
    if (d < 0) { n = -n; d = -d; }
    auto g = std::gcd(n,d); n /= g; d /= g;
}
std::string Rational::str() const { return std::to_string(n) + (d == 1 ? "" : "/" + std::to_string(d)); }
bool operator<(Rational a, Rational b) { return a.n*b.d < b.n*a.d; }
bool operator==(Rational a, Rational b) { return a.n == b.n && a.d == b.d; }
Rational operator+(Rational a, Rational b) { return {a.n*b.d+b.n*a.d,a.d*b.d}; }
Rational operator-(Rational a, Rational b) { return {a.n*b.d-b.n*a.d,a.d*b.d}; }
std::string Diagnostic::str() const { return source + ":" + std::to_string(line) + ": " + message; }
Rational quarterLength(const Bar& b) { return {b.length.n*4,b.length.d*b.meter.unit}; }
double seconds(const Bar& b) { return quarterLength(b).value()*60/b.bpm; }
std::vector<BarRef> expand(const Song& s) {
    std::vector<BarRef> out; size_t occurrence = 0;
    for (auto& p : s.order) for (int r=0;r<p.times;++r,++occurrence)
        for (size_t j=0;j<s.sections.size();++j) if (s.sections[j].name == p.section)
            for (size_t k=0;k<s.sections[j].bars.size();++k) out.push_back({j,k,occurrence});
    return out;
}
}
