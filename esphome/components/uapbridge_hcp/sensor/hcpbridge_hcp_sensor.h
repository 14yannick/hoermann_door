#pragma once

#include "../uapbridge_hcp.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace uapbridge_hcp {

class UAPBridgeHCPSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(UAPBridge_hcp *parent) { this->parent_ = parent; }
  void setup() override;
  void dump_config() override;
  void update_state(float value);
  void on_event_triggered();

 protected:
  UAPBridge_hcp *parent_{nullptr};
  float previous_position_{0.0f};
};

}  // namespace uapbridge_hcp
}  // namespace esphome