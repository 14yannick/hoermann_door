#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/uapbridge/uapbridge.h"

namespace esphome {
namespace uapbridge_hcp {

class UAPBridgeHCPCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void control(const cover::CoverCall &call) override;
  cover::CoverTraits get_traits() override;

  void set_parent(uapbridge::UAPBridge *parent) { this->parent_ = parent; }

  void on_event_triggered();

 protected:
  uapbridge::UAPBridge *parent_{nullptr};
  float previous_position_{0.0f};
  cover::CoverOperation previous_operation_{cover::COVER_OPERATION_IDLE};
};

}  // namespace uapbridge_hcp
}  // namespace esphome
