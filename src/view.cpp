#include "beattab/view.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
namespace bt {
Layout layout(size_t bars,size_t current,int width,int height,int perRow,Metrics m) {
    Layout l; l.width=width;l.height=height;
    if(width<200 || height<m.headerHeight+m.footerHeight+m.rowHeight || perRow<2 || perRow>4 || !bars) return l;
    size_t rows=(height-m.headerHeight-m.footerHeight)/m.rowHeight;
    size_t capacity=rows*perRow;l.pages=(bars+capacity-1)/capacity;l.page=std::min(current,bars-1)/capacity;
    int cellWidth=(width-2*m.margin)/perRow;
    for(size_t i=l.page*capacity;i<std::min(bars,(l.page+1)*capacity);++i) {
        auto local=i-l.page*capacity;
        l.cells.push_back({{m.margin+int(local%perRow)*cellWidth,m.headerHeight+int(local/perRow)*m.rowHeight,cellWidth,m.rowHeight-12},i});
    }
    return l;
}
Canvas::Canvas(int w,int h):width(w),height(h),bits((w*h+7)/8,0){}
void Canvas::pixel(int x,int y,bool black) {
    if(x<0||y<0||x>=width||y>=height)return;
    auto bit=y*width+x;auto mask=uint8_t(1u<<(bit%8));
    if(black)bits[bit/8]|=mask;else bits[bit/8]&=uint8_t(~mask);
}
bool Canvas::pixel(int x,int y) const {if(x<0||y<0||x>=width||y>=height)return false;auto bit=y*width+x;return bits[bit/8]&(1u<<(bit%8));}
void Canvas::fill(Rect r,bool black) {for(int y=std::max(0,r.y);y<std::min(height,r.y+r.h);++y)for(int x=std::max(0,r.x);x<std::min(width,r.x+r.w);++x)pixel(x,y,black);}
void Canvas::box(Rect r,bool b,int t) {fill({r.x,r.y,r.w,t},b);fill({r.x,r.y+r.h-t,r.w,t},b);fill({r.x,r.y,t,r.h},b);fill({r.x+r.w-t,r.y,t,r.h},b);}
// Original compact 5x7 bitmap alphabet; deterministic and independent of OS fonts.
static std::array<uint8_t,7> glyph(char c) {
    switch(c) {
#define L(c,a,b,d,e,f,g,h) case c:return {a,b,d,e,f,g,h};
    L('a',0,0,14,1,15,17,15) L('b',16,16,22,25,17,17,30) L('c',0,0,14,17,16,17,14)
    L('d',1,1,13,19,17,17,15) L('e',0,0,14,17,31,16,14) L('f',6,8,8,28,8,8,8)
    L('g',0,0,15,17,15,1,14) L('h',16,16,22,25,17,17,17) L('i',4,0,12,4,4,4,14)
    L('j',2,0,6,2,2,18,12) L('k',16,16,18,20,24,20,18) L('l',12,4,4,4,4,4,14)
    L('m',0,0,26,21,21,21,21) L('n',0,0,22,25,17,17,17) L('o',0,0,14,17,17,17,14)
    L('p',0,0,30,17,30,16,16) L('q',0,0,15,17,15,1,1) L('r',0,0,22,25,16,16,16)
    L('s',0,0,15,16,14,1,30) L('t',8,8,28,8,8,9,6) L('u',0,0,17,17,17,19,13)
    L('v',0,0,17,17,17,10,4) L('w',0,0,17,17,21,21,10) L('x',0,0,17,10,4,10,17)
    L('y',0,0,17,17,15,1,14) L('z',0,0,31,2,4,8,31)
#undef L
    default:break;
    }
    switch(std::toupper(static_cast<unsigned char>(c))) {
#define G(c,a,b,d,e,f,g,h) case c:return {a,b,d,e,f,g,h};
    G('A',14,17,17,31,17,17,17) G('B',30,17,17,30,17,17,30) G('C',14,17,16,16,16,17,14)
    G('D',30,17,17,17,17,17,30) G('E',31,16,16,30,16,16,31) G('F',31,16,16,30,16,16,16)
    G('G',14,17,16,23,17,17,15) G('H',17,17,17,31,17,17,17) G('I',31,4,4,4,4,4,31)
    G('J',7,2,2,2,18,18,12) G('K',17,18,20,24,20,18,17) G('L',16,16,16,16,16,16,31)
    G('M',17,27,21,21,17,17,17) G('N',17,25,21,19,17,17,17) G('O',14,17,17,17,17,17,14)
    G('P',30,17,17,30,16,16,16) G('Q',14,17,17,17,21,18,13) G('R',30,17,17,30,20,18,17)
    G('S',15,16,16,14,1,1,30) G('T',31,4,4,4,4,4,4) G('U',17,17,17,17,17,17,14)
    G('V',17,17,17,17,17,10,4) G('W',17,17,17,21,21,27,17) G('X',17,17,10,4,10,17,17)
    G('Y',17,17,10,4,4,4,4) G('Z',31,1,2,4,8,16,31)
    G('0',14,17,19,21,25,17,14) G('1',4,12,4,4,4,4,14) G('2',14,17,1,2,4,8,31)
    G('3',30,1,1,14,1,1,30) G('4',2,6,10,18,31,2,2) G('5',31,16,16,30,1,1,30)
    G('6',14,16,16,30,17,17,14) G('7',31,1,2,4,8,8,8) G('8',14,17,17,14,17,17,14)
    G('9',14,17,17,15,1,1,14) G('-',0,0,0,31,0,0,0) G('/',1,2,2,4,8,8,16)
    G('#',10,31,10,10,31,10,0) G('.',0,0,0,0,0,12,12) G(':',0,12,12,0,12,12,0)
    G('+',0,4,4,31,4,4,0) G('>',16,8,4,2,4,8,16) G('<',1,2,4,8,4,2,1)
    G('(',2,4,8,8,8,4,2) G(')',8,4,2,2,2,4,8) G('"',10,10,0,0,0,0,0)
    G('!',4,4,4,4,4,0,4) G('?',14,17,1,2,4,0,4) G('=',0,31,0,31,0,0,0)
    G(' ',0,0,0,0,0,0,0) G('\'',4,4,0,0,0,0,0)
#undef G
    default:return {31,17,5,5,4,0,4};
    }
}
void Canvas::text(int x,int y,const std::string& s,int scale,bool black,int maxWidth) {
    if(scale<1)return;int start=x;
    for(char c:s) {if(x+5*scale>start+maxWidth)break;auto g=glyph(c);
        for(int row=0;row<7;++row)for(int col=0;col<5;++col)if(g[row]&(1u<<(4-col)))fill({x+col*scale,y+row*scale,scale,scale},black);
        x+=6*scale;
    }
}
uint64_t Canvas::hash() const {uint64_t h=14695981039346656037ull;for(auto b:bits){h^=b;h*=1099511628211ull;}return h;}
static std::string fit(std::string s,int width,int scale) {
    size_t chars=std::max(0,width)/(6*scale);if(s.size()>chars) {s.resize(chars);if(chars>=3)s.replace(chars-3,3,"...");}return s;
}
static void wrap(Canvas& c,int x,int y,std::string s,int width,int lines,bool black) {
    int chars=width/12;if(chars<1)return;
    for(int i=0;i<lines && !s.empty();++i) {
        if(i==lines-1) {c.text(x,y,fit(s,width,2),2,black,width);break;}
        size_t n=std::min(size_t(chars),s.size());
        if(n<s.size()) {auto space=s.rfind(' ',n);if(space!=std::string::npos && space>0)n=space;}
        c.text(x,y,s.substr(0,n),2,black,width);s.erase(0,n);while(!s.empty()&&s.front()==' ')s.erase(0,1);y+=18;
    }
}
Canvas render(const Song& s,const std::vector<BarRef>& refs,const Layout& l,size_t current,bool playing,Lyrics lyrics,double rate) {
    Canvas c(l.width,l.height);if(refs.empty()||current>=refs.size())return c;
    auto now=refs[current];auto& currentBar=s.sections[now.section].bars[now.bar];
    c.text(20,16,fit(s.title,l.width-(s.capo?150:40),3),3,true);
    if(s.capo) {
        // Six strings, frets and a thick clamp: compact monochrome capo pictogram.
        // Its geometry is symbolic; the adjacent number is the actual fret setting.
        int x=l.width-116, y=10;
        for(int string=0;string<6;++string)c.fill({x+3+string*5,y+3,1,29},true);
        for(int fret=0;fret<3;++fret)c.fill({x+3,y+3+fret*13,26,1},true);
        if(*s.capo>0) {c.fill({x,y+12,33,6},true);c.fill({x+30,y+15,3,8},true);}
        c.text(x+43,y,"CAPO",1,true);
        c.text(x+43,y+12,std::to_string(*s.capo),3,true);
    }
    c.text(20,49,fit(s.sections[now.section].name+"  /  BAR "+std::to_string(now.bar+1)+"  /  "+std::to_string(int(currentBar.bpm*rate))+" QPM",l.width-210,2),2,true);
    c.text(l.width-166,49,playing?"PLAYING":"PAUSED",2,true);c.fill({20,73,l.width-40,2},true);
    for(auto& cell:l.cells) {
        auto r=cell.bounds;auto ref=refs[cell.index];auto& b=s.sections[ref.section].bars[ref.bar];bool active=cell.index==current;
        c.box({r.x,r.y,r.w,78},true,active?3:1);
        if(active)c.fill({r.x+1,r.y+1,r.w-2,21},true);
        c.text(r.x+7,r.y+7,std::to_string(ref.bar+1)+" "+fit(s.sections[ref.section].name,r.w-70,1),1,!active,r.w-14);
        c.text(r.x+r.w-38,r.y+7,std::to_string(b.meter.beats)+"/"+std::to_string(b.meter.unit),1,!active,32);
        int usable=r.w-18;auto xpos=[&](Rational at){return r.x+9+int(usable*at.value()/b.length.value());};
        c.fill({r.x+9,r.y+65,usable,1},true);
        for(int j=0;Rational(j)<b.length;++j)c.fill({xpos(j),r.y+61,1,10},true);
        for(size_t i=0;i<b.events.size();++i) {
            auto& e=b.events[i];int x=xpos(e.offset);int available=r.x+r.w-6-x;
            if(e.kind==Kind::Chord) {
                for(size_t j=i+1;j<b.events.size();++j)if(b.events[j].kind==Kind::Chord){available=std::min(available,xpos(b.events[j].offset)-x-3);break;}
                int scale=std::max(1,std::min(lyrics==Lyrics::Inside?3:4,available/int(std::max(size_t(1),e.text.size())*6)));
                c.text(x,r.y+(lyrics==Lyrics::Inside?25:29),fit(e.text,available,scale),scale,true,available);
                c.fill({x,r.y+56,2,12},true);
            }
        }
        bool tabs=std::any_of(b.events.begin(),b.events.end(),[](auto& e){return e.kind==Kind::Tab;});
        if(tabs) {
            for(int st=1;st<=6;++st)c.fill({r.x+9,r.y+80+st*9,usable,1},true);
            for(auto& e:b.events)if(e.kind==Kind::Tab) {int x=xpos(e.offset),y=r.y+77+e.string*9;c.fill({x-1,y-1,14,9},false);c.text(x,y,std::to_string(e.fret),1,true,14);}
        }
        for(size_t i=0;i<b.events.size();++i) {
            auto& e=b.events[i];if(e.kind!=Kind::Lyric && e.kind!=Kind::Note)continue;
            int x=lyrics==Lyrics::Timeline?xpos(e.offset):r.x+8;
            int y=r.y+(tabs?136:lyrics==Lyrics::Inside?49:86);
            int width=r.x+r.w-8-x;
            // Multiple phrases flow below each other; timeline mode preserves horizontal anchors.
            int previous=0;for(size_t j=0;j<i;++j)if(b.events[j].kind==e.kind)++previous;
            y+=previous*18;
            if(lyrics==Lyrics::Inside && !tabs) {c.fill({x,y,width,17},false);c.text(x,y,fit(e.text,width,1),1,true,width);}
            else if(tabs)c.text(x,y,fit(e.text,width,1),1,true,width);
            else wrap(c,x,y,e.text,width,previous?1:2,true);
        }
    }
    c.fill({20,l.height-42,l.width-40,1},true);
    c.text(20,l.height-29,"< PREV",2,true);c.text(190,l.height-29,"RESTART",2,true);
    c.text(360,l.height-29,playing?"PAUSE":"PLAY",2,true);c.text(510,l.height-29,"NEXT >",2,true);
    c.text(l.width-106,l.height-27,std::to_string(l.page+1)+"/"+std::to_string(l.pages),2,true);
    return c;
}
Application::Application(const Song& s,Display& d):song_(&s),display_(d),timeline(expand(s)){}
const Bar& Application::bar() const {auto r=timeline[current];return song_->sections[r.section].bars[r.bar];}
double Application::beat() const {return timeline.empty()?1:1+elapsed/seconds(bar())*bar().length.value();}
void Application::seek(size_t i){if(timeline.empty())return;current=std::min(i,timeline.size()-1);elapsed=0;draw();}
void Application::action(Action a) {
    if(a==Action::Power){sleeping=!sleeping;playing=false;draw(true);return;}
    if(sleeping || timeline.empty())return;
    switch(a) {
    case Action::Toggle:if(!playing && current==timeline.size()-1 && elapsed>=seconds(bar())){current=0;elapsed=0;}playing=!playing;break;
    case Action::Restart:current=0;elapsed=0;break;
    case Action::Previous:if(current)--current;elapsed=0;break;
    case Action::Next:if(current+1<timeline.size())++current;elapsed=0;break;
    case Action::PreviousSection:{auto occurrence=timeline[current].occurrence;while(current && timeline[current-1].occurrence==occurrence)--current;if(current){--current;auto prev=timeline[current].occurrence;while(current && timeline[current-1].occurrence==prev)--current;}elapsed=0;break;}
    case Action::NextSection:{auto occurrence=timeline[current].occurrence;size_t i=current;while(i<timeline.size() && timeline[i].occurrence==occurrence)++i;if(i<timeline.size())current=i;elapsed=0;break;}
    case Action::Slower:rate=std::max(.25,rate-.05);break;
    case Action::Faster:rate=std::min(2.0,rate+.05);break;
    case Action::Power:break;
    }
    draw();
}
void Application::tick(double dt) {
    if(!playing || sleeping || timeline.empty() || !std::isfinite(dt) || dt<=0)return;
    elapsed+=dt*rate;bool changed=false;
    while(elapsed+1e-9>=seconds(bar())) {
        if(current+1==timeline.size()){elapsed=seconds(bar());playing=false;changed=true;break;}
        elapsed-=seconds(bar());++current;changed=true;
    }
    if(changed)draw();
}
void Application::draw(bool full) {
    auto l=layout(timeline.size(),current,800,480,perRow);
    auto frame=render(*song_,timeline,l,current,playing,lyrics,rate);
    if(sleeping){frame=Canvas();frame.text(220,220,"BEATTAB SLEEP",4,true);}
    display_.request(frame,full || l.page!=shownPage_?Refresh::Full:Refresh::Partial);shownPage_=l.page;
}
void Application::touch(int x,int y) {
    if(sleeping)return;
    if(y>=438){if(x<160)action(Action::Previous);else if(x<330)action(Action::Restart);else if(x<490)action(Action::Toggle);else if(x<670)action(Action::Next);return;}
    auto l=layout(timeline.size(),current,800,480,perRow);for(auto& cell:l.cells)if(cell.bounds.contains(x,y)){seek(cell.index);return;}
}
}
