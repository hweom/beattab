#include "beattab/model.hpp"
#include <algorithm>
#include <charconv>
#include <iomanip>
#include <set>
#include <sstream>
namespace bt {
namespace {
bool integer(const std::string& s, int& n, int lo, int hi) {
    auto r = std::from_chars(s.data(),s.data()+s.size(),n);
    return r.ec == std::errc{} && r.ptr == s.data()+s.size() && n>=lo && n<=hi;
}
bool fraction(const std::string& s, Rational& r) {
    auto dot=s.find('.');
    if(dot!=std::string::npos) {
        // Parse fixed-point input exactly, without floating-point rounding.
        auto digits=s.size()-dot-1; int whole=0,part=0,scale=1;
        if(digits<1 || digits>3 || !integer(s.substr(0,dot),whole,0,1024) ||
           !integer(s.substr(dot+1),part,0,999)) return false;
        for(size_t i=0;i<digits;++i) scale*=10;
        if(whole==1024 && part!=0) return false;
        r={whole*scale+part,scale}; return true;
    }
    auto p=s.find('/'); int n=0,d=1;
    // Include exact canonical fractions produced by decimal input.
    if (!integer(s.substr(0,p),n,0,1024000)) return false;
    if (p!=std::string::npos && !integer(s.substr(p+1),d,1,1000)) return false;
    if(n>1024*d) return false;
    r={n,d}; return true;
}
bool meter(const std::string& s, Meter& m) {
    auto p=s.find('/');
    return p!=std::string::npos && integer(s.substr(0,p),m.beats,1,32) &&
        integer(s.substr(p+1),m.unit,1,32) && (m.unit & (m.unit-1))==0;
}
bool quoted(std::istringstream& in, std::string& s) {
    in >> std::ws; if (in.peek()!='"') return false;
    if (!(in >> std::quoted(s))) return false;
    return s.size()<=256 && s.find_first_of("\r\n") == std::string::npos;
}
bool end(std::istringstream& in) { in >> std::ws; return in.eof() || in.peek()=='#'; }
}
ParseResult parse(const std::string& text, const std::string& source) {
    ParseResult r; auto& s=r.song; size_t line=0; bool version=false;
    std::set<std::string> metadata, names;
    std::vector<size_t> playLines;
    std::istringstream file(text); std::string raw;
    auto error=[&](const std::string& msg){r.errors.push_back({source,line,msg});};
    if (text.size()>1024*1024) { error("song exceeds 1 MiB limit"); return r; }
    while (std::getline(file,raw)) {
        ++line; std::istringstream in(raw); std::string op; in >> op;
        if (op.empty() || op[0]=='#') continue;
        if (!version) { std::string v; in>>v; if(op!="beattab" || v!="1" || !end(in)) {error("expected beattab 1 header"); return r;} version=true; continue; }
        if (op=="id" || op=="title" || op=="artist" || op=="key" || op=="tempo" || op=="time" || op=="capo") {
            if (!s.sections.empty() || !metadata.insert(op).second) {error("metadata must be unique and precede sections"); continue;}
            std::string v;
            if (op=="tempo" || op=="time") { in>>v; if(op=="tempo" ? !integer(v,s.bpm,20,400) : !meter(v,s.meter)) error("invalid " + op); }
            else if (op=="capo") {
                in>>v; int fret=0;
                if(!integer(v,fret,0,24)) error("capo must be an integer fret from 0 to 24");
                else s.capo=fret;
            }
            else if (!quoted(in,v) || v.empty()) error("expected nonempty quoted value");
            else if(op=="id") {
                if(v.size()>80 || v.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-_.")!=std::string::npos) error("id must contain lowercase letters, digits, -, _ or .");
                s.id=v;
            } else if(op=="title") s.title=v; else if(op=="artist") s.artist=v; else s.key=v;
        } else if (op=="section") {
            std::string name;
            if(!quoted(in,name) || name.empty() || !names.insert(name).second || s.sections.size()>=128) {error("expected unique section name (maximum 128)");continue;}
            s.sections.push_back({name,{}});
        } else if (op=="bar") {
            if(s.sections.empty() || s.sections.back().bars.size()>=1024) {error("bar requires section; maximum 1024 bars per section");continue;}
            Bar b; b.meter=s.meter; b.bpm=s.bpm;
            // Changes inherit within a section, but every section begins from song defaults.
            if(!s.sections.back().bars.empty()) { b.meter=s.sections.back().bars.back().meter; b.bpm=s.sections.back().bars.back().bpm; }
            Rational len; bool explicitLen=false; std::string attr; std::set<std::string> attrs;
            while(in>>attr) {
                if(attr[0]=='#') break;
                auto p=attr.find('='); auto name=attr.substr(0,p); auto value=p==std::string::npos?"":attr.substr(p+1);
                if(!attrs.insert(name).second) {error("duplicate bar attribute");continue;}
                if(name=="time") { if(!meter(value,b.meter)) error("invalid bar meter"); }
                else if(name=="tempo") { if(!integer(value,b.bpm,20,400)) error("invalid bar tempo"); }
                else if(name=="length") {explicitLen=true; if(!fraction(value,len)) error("invalid bar length");}
                else error("unknown bar attribute: " + name);
            }
            b.length=explicitLen?len:Rational(b.meter.beats);
            if(!(Rational(0)<b.length) || Rational(b.meter.beats)<b.length) error("bar length must be positive and no longer than meter");
            s.sections.back().bars.push_back(b); continue;
        } else if (op=="chord" || op=="lyric" || op=="tab" || op=="note") {
            if(s.sections.empty() || s.sections.back().bars.empty()) {error("event requires a bar");continue;}
            auto& b=s.sections.back().bars.back(); Event e; std::string pos;
            e.kind=op=="chord"?Kind::Chord:op=="lyric"?Kind::Lyric:op=="tab"?Kind::Tab:Kind::Note;
            in>>pos; Rational beat;
            if(!fraction(pos,beat) || beat<Rational(1)) {error("beat must be an integer, decimal (up to 3 places), or fraction, starting at 1");continue;}
            e.offset=beat-Rational(1);
            if(!(e.offset<b.length)) {error("event starts outside bar");continue;}
            if(e.kind==Kind::Tab) {
                std::string st,fr,dur; in>>st>>fr>>dur;
                if(!integer(st,e.string,1,6) || !integer(fr,e.fret,0,24) || !fraction(dur,e.duration) || !(Rational(0)<e.duration) || b.length<e.offset+e.duration) {error("tab expects string 1..6, fret 0..24, positive duration fitting bar");continue;}
            } else if(!quoted(in,e.text) || e.text.empty()) {error("expected quoted event text");continue;}
            if(e.kind==Kind::Chord && e.text.size()>16) error("chord label exceeds 16 characters");
            for(auto& old:b.events) if(old.offset==e.offset && old.kind==e.kind && (e.kind!=Kind::Tab || old.string==e.string)) error("duplicate event at same beat");
            if(b.events.size()>=128) error("maximum 128 events per bar"); else b.events.push_back(e);
        } else if(op=="play") {
            Play p; std::string count;
            if(!quoted(in,p.section) || !(in>>count) || !integer(count,p.times,1,64)) {error("play expects quoted section and repeat count 1..64");continue;}
            s.order.push_back(p); playLines.push_back(line);
        } else error("unknown command: " + op);
        if(!end(in)) error("unexpected trailing text");
    }
    if(!version) error("missing beattab 1 header");
    for(auto key:{"id","title","artist","key","tempo","time"}) if(!metadata.count(key)) error(std::string("missing metadata: ")+key);
    if(s.sections.empty()) error("song has no sections");
    for(auto& sec:s.sections) {
        if(sec.bars.empty()) error("empty section: "+sec.name);
        for(auto& b:sec.bars) std::stable_sort(b.events.begin(),b.events.end(),[](auto& a,auto& z){return a.offset<z.offset || (a.offset==z.offset && (a.kind<z.kind || (a.kind==z.kind && a.string<z.string)));});
    }
    if(s.order.empty()) for(auto& sec:s.sections) s.order.push_back({sec.name,1});
    size_t total=0;
    for(size_t i=0;i<s.order.size();++i) {
        auto& p=s.order[i]; bool found=false;
        for(auto& sec:s.sections) if(sec.name==p.section) {found=true;total+=sec.bars.size()*p.times;}
        if(!found) r.errors.push_back({source,i<playLines.size()?playLines[i]:line,"unknown play section: "+p.section});
    }
    if(total>16384) error("expanded song exceeds 16384 bars");
    return r;
}
std::string serialize(const Song& s) {
    std::ostringstream o;
    o<<"beattab 1\nid "<<std::quoted(s.id)<<"\ntitle "<<std::quoted(s.title)<<"\nartist "<<std::quoted(s.artist)<<"\nkey "<<std::quoted(s.key)<<"\ntempo "<<s.bpm<<"\ntime "<<s.meter.beats<<'/'<<s.meter.unit<<'\n';
    if(s.capo) o<<"capo "<<*s.capo<<'\n';
    for(auto& sec:s.sections) {
        o<<"\nsection "<<std::quoted(sec.name)<<'\n';
        for(auto& b:sec.bars) {
            o<<"bar time="<<b.meter.beats<<'/'<<b.meter.unit<<" tempo="<<b.bpm<<" length="<<b.length.str()<<'\n';
            for(auto& e:b.events) {
                o<<"  "<<(e.kind==Kind::Chord?"chord":e.kind==Kind::Lyric?"lyric":e.kind==Kind::Tab?"tab":"note")<<' '<<(e.offset+Rational(1)).str()<<' ';
                if(e.kind==Kind::Tab) o<<e.string<<' '<<e.fret<<' '<<e.duration.str(); else o<<std::quoted(e.text);
                o<<'\n';
            }
        }
    }
    o<<'\n';for(auto& p:s.order) o<<"play "<<std::quoted(p.section)<<' '<<p.times<<'\n';
    return o.str();
}
}
