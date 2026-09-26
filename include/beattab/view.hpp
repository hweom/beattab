#pragma once
#include "model.hpp"
#include <array>
namespace bt {
struct Rect { int x,y,w,h; bool contains(int px,int py) const {return px>=x && py>=y && px<x+w && py<y+h;} };
struct Cell { Rect bounds; size_t index; };
enum class Lyrics { Below, Inside, Timeline };
struct Layout { std::vector<Cell> cells; size_t page=0,pages=0; int width=800,height=480; };
struct Metrics { int rowHeight=156, headerHeight=84, footerHeight=40, margin=20; };
Layout layout(size_t bars,size_t current,int width,int height,int perRow,Metrics metrics={});
// Portable raster primitives; packed 1-bit framebuffer, white=0, black=1.
class Canvas {
public:
    int width,height;
    std::vector<uint8_t> bits;
    Canvas(int w=800,int h=480);
    void pixel(int x,int y,bool black);
    bool pixel(int x,int y) const;
    void fill(Rect r,bool black);
    void box(Rect r,bool black,int weight=1);
    void text(int x,int y,const std::string& text,int scale,bool black,int maxWidth=10000);
    uint64_t hash() const;
};
Canvas render(const Song& song,const std::vector<BarRef>& timeline,const Layout& l,size_t current,bool playing,Lyrics lyrics,double rate);
enum class Refresh { Partial, Full };
class Display {
public:
    virtual ~Display()=default;
    virtual void request(const Canvas& frame,Refresh mode)=0;
};
enum class Action { Toggle, Restart, Previous, Next, PreviousSection, NextSection, Slower, Faster, Power };
class Application {
    const Song* song_;
    Display& display_;
    size_t shownPage_=size_t(-1);
public:
    std::vector<BarRef> timeline;
    size_t current=0;
    double elapsed=0,rate=1;
    bool playing=false,sleeping=false;
    int perRow=3;
    Lyrics lyrics=Lyrics::Timeline;
    explicit Application(const Song& song,Display& display);
    const Bar& bar() const;
    double beat() const;
    void action(Action action);
    void seek(size_t index);
    void tick(double seconds);
    void draw(bool forceFull=false);
    void touch(int x,int y);
};
}
