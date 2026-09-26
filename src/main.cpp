#include "beattab/library.hpp"
#include "beattab/eink.hpp"
#include <SDL.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

using namespace bt;
static void savePBM(const Canvas& c,const std::string& path) {
    std::ofstream f(path);f<<"P1\n"<<c.width<<' '<<c.height<<'\n';
    for(int y=0;y<c.height;++y){for(int x=0;x<c.width;++x)f<<(c.pixel(x,y)?'1':'0')<<' ';f<<'\n';}
}
int main(int argc,char** argv) {
    std::string root="songs",query;bool validate=false;std::string snapshot;
    for(int i=1;i<argc;++i){std::string a=argv[i];
        if(a=="--validate")validate=true;
        else if((a=="--library"||a=="--search"||a=="--snapshot")&&i+1<argc){auto value=argv[++i];if(a=="--library")root=value;else if(a=="--search")query=value;else snapshot=value;}
        else {std::cout<<"Usage: beattab [--library songs] [--search text] [--validate] [--snapshot output.pbm]\n";return a=="--help"?0:2;}}
    Library library;library.scan(root);
    for(auto& d:library.errors)std::cerr<<d.str()<<'\n';
    auto matches=library.search(query);
    if(validate){for(auto i:matches)std::cout<<library.entries[i].song.id<<"  "<<library.entries[i].song.artist<<" / "<<library.entries[i].song.title<<'\n';return library.errors.empty()?0:1;}
    if(matches.empty()){std::cerr<<"No valid matching songs in "<<root<<'\n';return 1;}
    EInk display;size_t selected=0;
    auto app=std::make_unique<Application>(library.entries[matches[selected]].song,display);app->draw(true);
    if(!snapshot.empty()){display.tick(10);savePBM(display.committed,snapshot);return 0;}
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){std::cerr<<SDL_GetError()<<'\n';return 1;}
    SDL_Window* window=SDL_CreateWindow("BeatTab | X4 Pro performance lab",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1120,700,0);
    SDL_Renderer* renderer=window?SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE):nullptr;
    SDL_Texture* texture=renderer?SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,1120,700):nullptr;
    if(!texture){std::cerr<<SDL_GetError()<<'\n';SDL_Quit();return 1;}
    std::vector<uint32_t> pixels(1120*700);bool running=true;uint64_t last=SDL_GetPerformanceCounter();
    std::string notice=library.errors.empty()?"ORIGINAL EXAMPLES + SYNTHETIC FIXTURES":"LIBRARY HAS ERRORS - SEE TERMINAL";
    auto changeSong=[&](int delta){selected=(selected+matches.size()+delta)%matches.size();app=std::make_unique<Application>(library.entries[matches[selected]].song,display);app->draw(true);};
    auto reload=[&](){std::string id=library.entries[matches[selected]].song.id;app.reset();library.scan(root);matches=library.search(query);selected=0;
        for(auto& d:library.errors)std::cerr<<d.str()<<'\n';
        if(matches.empty()){notice="NO VALID SONGS - FIX FILES AND RESTART";return;}
        for(size_t i=0;i<matches.size();++i)if(library.entries[matches[i]].song.id==id)selected=i;
        app=std::make_unique<Application>(library.entries[matches[selected]].song,display);app->draw(true);notice=library.errors.empty()?"LIBRARY RELOADED":"VALIDATION ERRORS - SEE TERMINAL";};
    while(running) {
        uint64_t now=SDL_GetPerformanceCounter();double dt=double(now-last)/SDL_GetPerformanceFrequency();last=now;
        SDL_Event e;while(SDL_PollEvent(&e)) {
            if(e.type==SDL_QUIT)running=false;
            if(e.type==SDL_KEYDOWN && !e.key.repeat) {
                auto k=e.key.keysym.sym;if(k==SDLK_ESCAPE)running=false;
                if(k==SDLK_F5){reload();continue;}if(!app)continue;
                if(k==SDLK_SPACE)app->action(Action::Toggle);else if(k==SDLK_r)app->action(Action::Restart);
                else if(k==SDLK_UP)app->action(Action::Previous);else if(k==SDLK_DOWN)app->action(Action::Next);
                else if(k==SDLK_LEFT)app->action(Action::PreviousSection);else if(k==SDLK_RIGHT)app->action(Action::NextSection);
                else if(k==SDLK_EQUALS || k==SDLK_PLUS)app->action(Action::Faster);else if(k==SDLK_MINUS)app->action(Action::Slower);
                else if(k==SDLK_p)app->action(Action::Power);
                else if(k>=SDLK_2 && k<=SDLK_4){app->perRow=int(k-SDLK_0);app->draw(true);}
                else if(k==SDLK_l){app->lyrics=Lyrics((int(app->lyrics)+1)%3);app->draw(true);}
                else if(k==SDLK_f)app->draw(true);else if(k==SDLK_u)app->draw();
                else if(k==SDLK_g)display.ghost=display.ghost<.1?.18:display.ghost<.3?.4:0;
                else if(k==SDLK_LEFTBRACKET)display.light=std::max(0,display.light-10);
                else if(k==SDLK_RIGHTBRACKET)display.light=std::min(100,display.light+10);
                else if(k==SDLK_n)changeSong(1);else if(k==SDLK_b)changeSong(-1);
            }
            if(e.type==SDL_MOUSEBUTTONUP && app) {
                int x=e.button.x,y=e.button.y;
                if(Rect{50,100,800,480}.contains(x,y))app->touch(x-50,y-100);
                else if(Rect{860,250,22,65}.contains(x,y))app->action(Action::Next);
                else if(Rect{860,345,22,65}.contains(x,y))app->action(Action::Previous);
                else if(Rect{740,64,60,18}.contains(x,y))app->action(Action::Power);
                else if(x>=920 && x<1100) {
                    if(y>=126 && y<158)app->action(Action::Toggle);
                    else if(y>=174 && y<206)app->action(Action::Restart);
                    else if(y>=222 && y<254){app->perRow=app->perRow==4?2:app->perRow+1;app->draw(true);}
                    else if(y>=270 && y<302){app->lyrics=Lyrics((int(app->lyrics)+1)%3);app->draw(true);}
                    else if(y>=318 && y<350)app->draw(true);
                    else if(y>=366 && y<398)app->draw();
                    else if(y>=414 && y<446)display.ghost=display.ghost<.1?.18:display.ghost<.3?.4:0;
                    else if(y>=462 && y<494)display.light=display.light>=100?0:std::min(100,display.light+20);
                    else if(y>=510 && y<542)changeSong(1);
                }
            }
        }
        if(app)app->tick(dt);display.tick(dt);
        Canvas shell(1120,700);
        shell.text(30,22,"BEATTAB",4,true);shell.text(270,32,"MUSICAL TIME / PERFORMANCE LAB",2,true);
        shell.box({28,78,844,534},true,5);shell.box({860,250,22,65},true,3);shell.box({860,345,22,65},true,3);shell.box({740,64,60,18},true,3);
        shell.text(330,594,"XTEINK X4 PRO",1,true);shell.text(40,640,notice,1,true);
        shell.text(40,668,"UP/DOWN: BAR   LEFT/RIGHT: SECTION   SPACE: PLAY   +/-: TEMPO   F5: RELOAD   N/B: SONG",1,true);
        shell.text(920,91,"DEVICE LAB",2,true);
        auto button=[&](int y,std::string label){shell.box({920,y,180,32},true);shell.text(932,y+10,label,1,true,160);};
        button(126,app&&app->playing?"SPACE / PAUSE":"SPACE / PLAY");button(174,"R / RESTART");
        button(222,"2/3/4 / BARS: "+std::to_string(app?app->perRow:3));
        button(270,"L / LYRICS: "+std::string(!app||app->lyrics==Lyrics::Below?"BELOW":app->lyrics==Lyrics::Inside?"INSIDE":"TIMELINE"));
        button(318,"F / FULL REFRESH");button(366,"U / PARTIAL REFRESH");
        button(414,"G / GHOST: "+std::to_string(int(display.ghost*100))+"%");button(462,"[ ] / LIGHT: "+std::to_string(display.light));
        button(510,"N / NEXT SONG");shell.text(920,562,"FULL "+std::to_string(display.fullCount)+" / PART "+std::to_string(display.partialCount),1,true);
        shell.text(920,584,display.busy()?"PANEL BUSY":"PANEL READY",1,true);
        if(app)shell.text(920,606,"BEAT "+std::to_string(app->beat()).substr(0,4),1,true);
        for(int y=0;y<700;++y)for(int x=0;x<1120;++x) {
            int v=shell.pixel(x,y)?35:235;
            if(Rect{50,100,800,480}.contains(x,y)){v=display.flashing()?35:display.optical[(y-100)*800+x-50];v=int(v*(.70+.003*display.light));}
            pixels[y*1120+x]=0xff000000u|uint32_t(v<<16|v<<8|v);
        }
        SDL_UpdateTexture(texture,nullptr,pixels.data(),1120*4);SDL_RenderClear(renderer);SDL_RenderCopy(renderer,texture,nullptr,nullptr);SDL_RenderPresent(renderer);SDL_Delay(16);
    }
    SDL_DestroyTexture(texture);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
