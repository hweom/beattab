#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bt {
// Reduced exact fractions. File input is bounded to prevent arithmetic overflow.
struct Rational {
    int64_t n = 0, d = 1;
    Rational(int64_t numerator = 0, int64_t denominator = 1);
    double value() const { return double(n) / double(d); }
    std::string str() const;
};
bool operator<(Rational a, Rational b);
bool operator==(Rational a, Rational b);
Rational operator+(Rational a, Rational b);
Rational operator-(Rational a, Rational b);
struct Meter { int beats = 4, unit = 4; };
enum class Kind { Chord, Lyric, Tab, Note };
struct Event {
    Kind kind = Kind::Chord;
    Rational offset; // zero-based, in denominator-note beats
    Rational duration; // zero = point annotation
    std::string text;
    int string = 0, fret = 0; // string 1 = high E
};
struct Bar { Meter meter; int bpm = 120; Rational length = 4; std::vector<Event> events; };
struct Section { std::string name; std::vector<Bar> bars; };
struct Play { std::string section; int times = 1; };
struct Song {
    std::string id, title, artist, key;
    int bpm = 120;
    std::optional<int> capo; // absent = unspecified, 0 = explicitly no capo
    Meter meter;
    std::vector<Section> sections;
    std::vector<Play> order;
};
struct Diagnostic { std::string source; size_t line = 0; std::string message; std::string str() const; };
struct ParseResult { Song song; std::vector<Diagnostic> errors; explicit operator bool() const { return errors.empty(); } };
ParseResult parse(const std::string& text, const std::string& source = "<input>");
std::string serialize(const Song& song);
struct BarRef { size_t section, bar, occurrence; };
std::vector<BarRef> expand(const Song& song);
// BPM always denotes quarter notes, including in compound meter.
Rational quarterLength(const Bar& bar);
double seconds(const Bar& bar);
}
