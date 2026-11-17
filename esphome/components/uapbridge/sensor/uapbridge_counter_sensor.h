#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uapbridge_esp/uapbridge_esp.h"

namespace esphome {
namespace uapbridge {

enum class CounterType : uint8_t {
  BROADCASTS_RECEIVED,
  POLLS_ANSWERED,
  COMMANDS_SENT,
  COMMANDS_DROPPED,
};

// Diagnostic bus counter of the ESP bridge, published every update_interval.
// Counters live in RAM only (no flash writes) and start at 0 after every reboot.
class UAPBridgeCounterSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_uapbridge_parent(uapbridge_esp::UAPBridge_esp *parent) { this->parent_ = parent; }
  void set_counter_type(CounterType type) { this->type_ = type; }
  void update() override;
  void dump_config() override;

 protected:
  uapbridge_esp::UAPBridge_esp *parent_;
  CounterType type_{CounterType::BROADCASTS_RECEIVED};
};

}  // namespace uapbridge
}  // namespace esphome
