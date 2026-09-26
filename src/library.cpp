#include "beattab/library.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
namespace bt {
void Library::scan(const std::filesystem::path& root) {
    entries.clear();errors.clear();std::vector<std::filesystem::path> paths;std::error_code ec;
    std::filesystem::recursive_directory_iterator it(root,ec),end;
    for(;!ec && it!=end;it.increment(ec)) if(it->is_regular_file(ec) && it->path().extension()==".song") paths.push_back(it->path());
    if(ec) errors.push_back({root.string(),0,ec.message()});
    std::sort(paths.begin(),paths.end());std::set<std::string> ids;
    for(auto& p:paths) {
        if(std::filesystem::file_size(p,ec)>1024*1024 || ec) { errors.push_back({p.string(),0,"cannot read song or file exceeds 1 MiB"});ec.clear();continue; }
        std::ifstream f(p);if(!f){errors.push_back({p.string(),0,"cannot open file"});continue;}
        std::ostringstream buf;buf<<f.rdbuf();auto result=parse(buf.str(),p.string());
        if(!result) errors.insert(errors.end(),result.errors.begin(),result.errors.end());
        else if(!ids.insert(result.song.id).second) errors.push_back({p.string(),0,"duplicate song id: "+result.song.id});
        else entries.push_back({p,std::move(result.song)});
    }
    std::sort(entries.begin(),entries.end(),[](auto& a,auto& b){return a.song.artist==b.song.artist?a.song.title<b.song.title:a.song.artist<b.song.artist;});
}
static std::string lower(std::string s) {for(auto& c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
std::vector<size_t> Library::search(std::string q) const {
    q=lower(q);std::vector<size_t> out;
    for(size_t i=0;i<entries.size();++i) if(lower(entries[i].song.title+" "+entries[i].song.artist).find(q)!=std::string::npos) out.push_back(i);
    return out;
}
}
