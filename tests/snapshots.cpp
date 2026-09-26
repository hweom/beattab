#include "beattab/library.hpp"
#include "beattab/view.hpp"
#include <fstream>
#include <iostream>
using namespace bt;
int main(){Library lib;lib.scan("songs");if(!lib.errors.empty())return 1;
for(auto name:{"Amber","Unusual","Timeline tab","Chords and tab"})for(int bars=2;bars<=4;++bars)for(int mode=0;mode<3;++mode){
auto matches=lib.search(name);if(matches.empty())return 1;auto& s=lib.entries[matches[0]].song;auto refs=expand(s);auto l=layout(refs.size(),0,800,480,bars);auto c=render(s,refs,l,0,false,Lyrics(mode),1);
std::string stem=s.id+"-"+std::to_string(bars)+"-"+std::to_string(mode);std::ofstream f("build/"+stem+".pgm",std::ios::binary);f<<"P5\n800 480\n255\n";for(int y=0;y<480;++y)for(int x=0;x<800;++x)f.put(char(c.pixel(x,y)?0:255));std::cout<<stem<<' '<<c.hash()<<'\n';}}
