#include "uapbridge_hcp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace uapbridge_hcp {

static const char *const TAG_SENSOR = "uapbridge_hcp.sensor";

void UAPBridgeHCPSensor::setup() {
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG_SENSOR, "Parent is null");
    this->mark_failed();
    return;
  }

  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
  this->update_state(0.0f);
}

void UAPBridgeHCPSensor::dump_config() {
  ESP_LOGCONFIG(TAG_SENSOR, "UAPBridge HCP Sensor:");
}

void UAPBridgeHCPSensor::update_state(float value) {
  this->publish_state(value * 100.0f);
  this->previous_position_ = value;
  ESP_LOGD(TAG_SENSOR, "Published new state: %.2f", value);
}

void UAPBridgeHCPSensor::on_event_triggered() {
  if (this->parent_ == nullptr) {
    return;
  }

  if (!this->parent_->is_valid()) {
    if (!this->status_has_warning()) {
      this->status_set_warning();
    }
    return;
  }

  if (this->status_has_warning()) {
    this->status_clear_warning();
  }

  const float current_position = this->parent_->get_current_position();

  if (this->previous_position_ != current_position) {
    ESP_LOGD(TAG_SENSOR, "Position changed: %.2f -> %.2f", this->previous_position_, current_position);
    this->update_state(current_position);
  }
}

}  // namespace uapbridge_hcp
}  // namespace esphome