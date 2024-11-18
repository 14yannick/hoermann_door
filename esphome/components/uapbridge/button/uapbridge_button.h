#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "../uapbridge.h"

namespace esphome {
namespace uapbridge {

class UAPBridgeButtonVent : public button::Button, public Component {
public:
  void set_uapbridge_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void press_action() override;

protected:
  UAPBridge* parent_;
};

class UAPBridgeButtonImpulse : public button::Button, public Component {
public:
  void set_uapbridge_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void press_action() override;

protected:
  UAPBridge* parent_;
};

}  // namespace uapbridge
}  // namespace esphome
