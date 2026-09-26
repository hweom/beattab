#include "beattab/library.hpp"
#include "beattab/eink.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
using namespace bt;
int checks=0;
#define CHECK(x) do {++checks;if(!(x)){std::cerr<<__FILE__<<':'<<__LINE__<<": " #x "\n";std::exit(1);}}while(false)
static std::string header="beattab 1\nid \"test\"\ntitle \"Test\"\nartist \"Original\"\nkey \"C\"\ntempo 120\ntime 4/4\n";
struct RecordingDisplay:Display {int full=0,partial=0;Canvas last;void request(const Canvas& c,Refresh m)override{last=c;if(m==Refresh::Full)++full;else ++partial;}};
int main() {
    CHECK(Rational(2,6)==Rational(1,3));CHECK(Rational(1,3)+Rational(2,3)==Rational(1));
    Library lib;lib.scan("songs");CHECK(lib.errors.empty());CHECK(lib.entries.size()==16);CHECK(lib.search("AMBER").size()==1);CHECK(lib.search("BeatTab originals").size()==3);
    for(auto& entry:lib.entries){auto saved=serialize(entry.song);auto p=parse(saved);CHECK(p);CHECK(saved==serialize(p.song));CHECK(!expand(p.song).empty());}
    auto good=parse(header+"section \"A\"\nbar\nchord 1 \"C\"\nchord 5/2 \"G\"\nbar length=1\nchord 1 \"F\"\nplay \"A\" 2\n");CHECK(good);
    CHECK(good.song.sections[0].bars[0].events[1].offset==Rational(3,2));CHECK(expand(good.song).size()==4);
    auto invalid=[&](std::string body){auto r=parse(header+body,"bad.song");CHECK(!r);CHECK(r.errors[0].source=="bad.song");CHECK(r.errors[0].line>0);};
    invalid("section \"A\"\nbar\nchord 5 \"C\"\n");invalid("section \"A\"\nbar\nchord 1/0 \"C\"\n");
    invalid("section \"A\"\nbar\nchord 1 \"C\" extra\n");invalid("section \"A\"\nbar\nchord 1 \"C\n");
    invalid("section \"A\"\nbar\nchord 1 \"C\"\nchord 1 \"G\"\n");invalid("section \"A\"\nbar time=4/3\n");
    invalid("section \"A\"\nbar tempo=0\n");invalid("section \"A\"\nbar length=5\n");invalid("section \"A\"\nbar length=0\n");
    invalid("section \"A\"\nbar\ntab 1 7 0 1\n");invalid("section \"A\"\nbar\ntab 4 1 0 2\n");
    invalid("section \"A\"\nbar\nplay \"missing\" 1\n");invalid("section \"A\"\nbar\nplay \"A\" 0\n");
    invalid("section \"A\"\nbar\nsection \"A\"\nbar\n");invalid("section \"Empty\"\n");invalid("bar\n");
    invalid("section \"A\"\nbar\nchord 99999999999999999999 \"C\"\n");
    // Decimal input is exact and interoperates with fractional positions.
    auto decimal=parse(header+"section \"A\"\nbar length=3.5\nlyric 1.5 \"Enter here\"\nchord 2.25 \"G\"\ntab 3.125 1 0 0.375\n");
    CHECK(decimal);auto& db=decimal.song.sections[0].bars[0];
    CHECK(db.length==Rational(7,2));CHECK(db.events[0].offset==Rational(1,2));
    CHECK(db.events[1].offset==Rational(5,4));CHECK(db.events[2].duration==Rational(3,8));
    CHECK(parse(serialize(decimal.song)));CHECK(serialize(parse(serialize(decimal.song)).song)==serialize(decimal.song));
    for(auto pos:{"1.0","1.500","1.001","4.999"}) CHECK(parse(header+"section \"A\"\nbar\nlyric "+pos+" \"Enter\"\n"));
    for(auto pos:{".5","1.","1.2.3","1.5/2","1e0","-1.5","+1.5","0.5","5.0","1.0001","999999999999999999.5"})
        invalid(std::string("section \"A\"\nbar\nlyric ")+pos+" \"Enter\"\n");
    invalid("section \"A\"\nbar\nlyric 1.5 \"One\"\nlyric 3/2 \"Duplicate\"\n");
    invalid("section \"A\"\nbar length=0.5\nlyric 1.5 \"Outside\"\n");
    invalid("section \"A\"\nbar\ntab 4.5 1 0 0.501\n");
    // In the default layout, a half-beat delay shifts the visible lyric by 1/8 bar.
    auto onset=parse(header+"section \"A\"\nbar\nlyric 1 \"Enter\"\n");CHECK(onset);
    RecordingDisplay lyricDisplay;Application lyricApp(onset.song,lyricDisplay);
    CHECK(lyricApp.lyrics==Lyrics::Timeline);
    for(int columns=2;columns<=4;++columns) {
        auto refs=expand(onset.song);auto grid=layout(1,0,800,480,columns);
        auto early=render(onset.song,refs,grid,0,false,lyricApp.lyrics,1);
        auto lateSong=onset.song;lateSong.sections[0].bars[0].events[0].offset=Rational(1,2);
        auto late=render(lateSong,refs,grid,0,false,lyricApp.lyrics,1);
        auto rect=grid.cells[0].bounds;int dx=(rect.w-18)/8;bool ink=false;
        for(int y=rect.y+86;y<rect.y+100;++y)for(int x=rect.x+9;x<rect.x+69;++x) {
            CHECK(early.pixel(x,y)==late.pixel(x+dx,y));ink=ink||early.pixel(x,y);
        }
        CHECK(ink);CHECK(early.hash()!=late.hash());
        for(int y=rect.y+86;y<rect.y+100;++y)for(int x=rect.x+9;x<rect.x+9+dx;++x) CHECK(!late.pixel(x,y));
    }
    auto changes=parse(header+"section \"A\"\nbar\nbar time=3/4 tempo=90\nbar\nbar time=6/8 tempo=120\nsection \"B\"\nbar\n");CHECK(changes);
    auto& bars=changes.song.sections[0].bars;CHECK(seconds(bars[0])==2);CHECK(seconds(bars[1])==2);CHECK(seconds(bars[2])==2);CHECK(seconds(bars[3])==1.5);CHECK(quarterLength(bars[3])==Rational(3));CHECK(changes.song.sections[1].bars[0].bpm==120);
    CHECK(!good.song.capo);
    CHECK(serialize(good.song).find("capo ")==std::string::npos);
    for(int fret:{0,2,12,24}) {
        auto capo=parse(header+"capo "+std::to_string(fret)+"\nsection \"A\"\nbar\n");
        CHECK(capo);CHECK(capo.song.capo && *capo.song.capo==fret);
        auto roundtrip=parse(serialize(capo.song));CHECK(roundtrip);CHECK(roundtrip.song.capo==capo.song.capo);
    }
    for(auto value:{"-1","25","2.5","1/2","two","\"2\"","","2 extra"})
        invalid(std::string("capo ")+value+"\nsection \"A\"\nbar\n");
    invalid("capo 2\ncapo 3\nsection \"A\"\nbar\n");
    invalid("section \"A\"\nbar\ncapo 2\n");
    auto& s=good.song;RecordingDisplay d;Application a(s,d);a.draw();CHECK(d.full==1);a.action(Action::Toggle);a.tick(1.99);CHECK(a.current==0);a.tick(.01);CHECK(a.current==1);a.tick(.5);CHECK(a.current==2);CHECK(a.elapsed<1e-8);
    a.action(Action::Toggle);a.tick(100);CHECK(a.current==2);a.action(Action::PreviousSection);CHECK(a.current==0);a.action(Action::NextSection);CHECK(a.current==2);
    a.action(Action::Restart);CHECK(a.current==0);a.action(Action::Faster);a.action(Action::Toggle);a.tick(2/a.rate);CHECK(a.current==1);
    a.tick(100);CHECK(!a.playing);CHECK(a.current==3);a.action(Action::Toggle);CHECK(a.current==0);CHECK(a.playing);
    a.action(Action::Power);CHECK(a.sleeping);CHECK(!a.playing);a.action(Action::Next);CHECK(a.current==0);a.action(Action::Power);CHECK(!a.sleeping);
    for(int n=2;n<=4;++n){auto l=layout(19,18,800,480,n);CHECK(l.pages==size_t((19+2*n-1)/(2*n)));CHECK(l.cells.back().index==18);for(auto& cell:l.cells){CHECK(cell.bounds.x>=0);CHECK(cell.bounds.y+cell.bounds.h<438);CHECK(cell.bounds.x+cell.bounds.w<=800);}}
    auto l=layout(4,0,800,480,2);CHECK(l.cells.size()==4);CHECK(layout(4,0,800,100,2).cells.empty());CHECK(layout(4,0,800,480,1).cells.empty());
    auto frame=render(s,a.timeline,l,0,false,Lyrics::Below,1);CHECK(frame.bits.size()==48000);CHECK(frame.pixel(21,85));CHECK(!frame.pixel(401,85));
    CHECK(frame.hash()==render(s,a.timeline,l,0,false,Lyrics::Below,1).hash());CHECK(frame.hash()!=render(s,a.timeline,l,1,false,Lyrics::Below,1).hash());
    auto capoSong=s;capoSong.capo=2;
    auto capoFrame=render(capoSong,a.timeline,l,0,false,Lyrics::Below,1);
    CHECK(capoFrame.pixel(684,22));CHECK(!frame.pixel(684,22));
    // Metadata must not alter musical content or the area below the header.
    for(int y=44;y<480;++y)for(int x=0;x<800;++x) {
        if(frame.pixel(x,y)!=capoFrame.pixel(x,y)){CHECK(false);}
    }
    capoSong.capo=0;auto noCapoFrame=render(capoSong,a.timeline,l,0,false,Lyrics::Below,1);
    CHECK(!noCapoFrame.pixel(684,22));CHECK(noCapoFrame.hash()!=frame.hash());
    capoSong.capo=24;capoSong.title=std::string(100,'W');
    auto longTitle=render(capoSong,a.timeline,l,0,true,Lyrics::Below,1);
    capoSong.title="Short";auto shortTitle=render(capoSong,a.timeline,l,0,true,Lyrics::Below,1);
    for(int y=0;y<44;++y)for(int x=670;x<800;++x) {
        if(longTitle.pixel(x,y)!=shortTitle.pixel(x,y)){CHECK(false);}
    }
    CHECK(seconds(capoSong.sections[0].bars[0])==seconds(s.sections[0].bars[0]));
    a.touch(400,460);CHECK(a.playing);a.touch(550,460);CHECK(a.current==1);
    RecordingDisplay pages;Application paged(lib.entries[lib.search("Amber")[0]].song,pages);paged.perRow=2;paged.draw();paged.seek(4);CHECK(pages.full==2);paged.seek(5);CHECK(pages.partial==1);
    EInk ink;Canvas black;black.fill({0,0,800,480},true);ink.request(black,Refresh::Full);CHECK(ink.busy());ink.tick(.4);CHECK(ink.fullCount==0);ink.tick(.4);CHECK(ink.fullCount==1);CHECK(ink.optical[0]==0);
    Canvas white;ink.request(white,Refresh::Partial);ink.tick(.16);CHECK(ink.partialCount==1);CHECK(ink.optical[0]>0 && ink.optical[0]<255);
    ink.request(white,Refresh::Full);ink.tick(.8);CHECK(ink.optical[0]==255);
    ink.request(black,Refresh::Partial);ink.request(black,Refresh::Full);ink.request(white,Refresh::Partial);ink.tick(10);CHECK(ink.fullCount==3);CHECK(ink.committed.hash()==white.hash());
    // Source line reporting, comments, escape handling, ASCII syntax strictness.
    auto bad=parse(header+"section \"A\"\nbar\nlyric 0 \"oops\"\n","line.song");CHECK(bad.errors.front().line==10);
    auto escaped=parse(header+"# comment\nsection \"A\"\nbar # normal\nlyric 1 \"Say \\\"hi\\\"\" # annotation\n");CHECK(escaped);CHECK(parse(serialize(escaped.song)));
    // Adversarial parser smoke: bounded, deterministic byte mutations must never crash.
    auto seed=serialize(good.song);for(size_t i=0;i<seed.size();++i){auto mutation=seed;mutation[i]=char((i*31)%127);parse(mutation);}
    std::cout<<checks<<" checks passed; 16 songs validated; parser mutation smoke passed.\n";
}
