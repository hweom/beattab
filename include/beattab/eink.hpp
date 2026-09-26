#pragma once
#include "view.hpp"
#include <optional>
namespace bt {
// Simulator-only optical model, with a bounded latest-frame queue.
class EInk : public Display {
    std::optional<Canvas> target_,queued_;
    Refresh mode_=Refresh::Full, queuedMode_=Refresh::Partial;
    double remaining_=0;
    void begin(const Canvas& c,Refresh mode);
public:
    Canvas committed;
    std::vector<uint8_t> optical;
    int fullCount=0,partialCount=0;
    double ghost=.18,fullSeconds=.8,partialSeconds=.16;
    int light=65;
    EInk():optical(800*480,255){}
    void request(const Canvas& c,Refresh mode) override;
    void tick(double dt);
    bool busy() const {return target_.has_value();}
    bool flashing() const {return busy() && mode_==Refresh::Full && remaining_>fullSeconds*.5;}
};
}
