#pragma once

#include "esphome/core/component.h"
#include "esphome/components/event/event.h"
#include "../uapbridge.h"

namespace esphome {
namespace uapbridge {

// Home Assistant event entity that reports door events (opening, opened, closing, closed,
// stopped, venting, error, prewarn). Only reads state; never sends anything to the door.
class UAPBridgeEvent : public event::Event, public Component {
 public:
  void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
  void setup() override;
  void dump_config() override;

 protected:
  UAPBridge *parent_;
};

}  // namespace uapbridge
}  // namespace esphome
