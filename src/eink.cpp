#include "beattab/eink.hpp"
#include <algorithm>
namespace bt {
void EInk::begin(const Canvas& c,Refresh m){target_=c;mode_=m;remaining_=m==Refresh::Full?fullSeconds:partialSeconds;}
void EInk::request(const Canvas& c,Refresh m) {
    if(c.width!=800 || c.height!=480)return;
    if(!target_){begin(c,m);return;}
    // Coalesce new frames while BUSY; a pending full refresh cannot be downgraded.
    queued_=c;if(m==Refresh::Full)queuedMode_=m;
}
void EInk::tick(double dt) {
    if(dt<0)return;
    while(target_) {
        if(dt<remaining_){remaining_-=dt;return;}dt-=remaining_;
        for(int y=0;y<480;++y)for(int x=0;x<800;++x){auto i=y*800+x;int value=target_->pixel(x,y)?0:255;
            optical[i]=uint8_t(mode_==Refresh::Full?value:value*(1-std::clamp(ghost,0.0,1.0))+optical[i]*std::clamp(ghost,0.0,1.0));}
        committed=*target_;if(mode_==Refresh::Full)++fullCount;else ++partialCount;target_.reset();
        if(queued_){auto c=std::move(*queued_);queued_.reset();auto m=queuedMode_;queuedMode_=Refresh::Partial;begin(c,m);}
    }
}
}
