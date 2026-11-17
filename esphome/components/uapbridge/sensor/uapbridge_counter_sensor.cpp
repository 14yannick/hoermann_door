#include "uapbridge_counter_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace uapbridge {

static const char *const TAG = "uapbridge.sensor";

void UAPBridgeCounterSensor::update() {
  uint32_t value = 0;
  switch (this->type_) {
    case CounterType::BROADCASTS_RECEIVED:
      value = this->parent_->get_broadcasts_received();
      break;
    case CounterType::POLLS_ANSWERED:
      value = this->parent_->get_polls_answered();
      break;
    case CounterType::COMMANDS_SENT:
      value = this->parent_->get_commands_sent();
      break;
    case CounterType::COMMANDS_DROPPED:
      value = this->parent_->get_commands_dropped();
      break;
  }
  this->publish_state(static_cast<float>(value));
}

void UAPBridgeCounterSensor::dump_config() {
  LOG_SENSOR("", "UAPBridge Counter", this);
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace uapbridge
}  // namespace esphome
