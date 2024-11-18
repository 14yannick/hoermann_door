#pragma once
#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
#include "../uapbridge.h"

namespace esphome {
namespace uapbridge {

class UAPBridgeCover : public cover::Cover, public Component {
 public:
  void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
  void setup() override;
  void on_event_triggered();
  void control(const cover::CoverCall &call) override;
  cover::CoverTraits get_traits() override;
 protected:
    UAPBridge *parent_;
    cover::CoverOperation previousOperation_ = cover::COVER_OPERATION_IDLE;
    UAPBridge::hoermann_state_t previousState_ = UAPBridge::hoermann_state_t::hoermann_state_stopped;
};

}  // namespace uapbridge
}  // namespace esphome

