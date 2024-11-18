#include "uapbridge_switch.h"

namespace esphome {
namespace uapbridge {
static const char *const TAG = "uapbridge.switch";
void UAPBridgeSwitchVent::setup() {
    this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}
void UAPBridgeSwitchVent::on_event_triggered() {
  if (this->parent_->get_venting_enabled() != this->previousState_) {
    ESP_LOGD(TAG, "UAPBridgeSwitchVent::on_event_triggered() - adjusting state");
    this->publish_state(this->parent_->get_venting_enabled());
    this->previousState_ = this->parent_->get_venting_enabled();
  }
}

void UAPBridgeSwitchVent::write_state(bool state) {
  UAPBridge::hoermann_state_t current_state = this->parent_->get_state();

  if (state && current_state != UAPBridge::hoermann_state_t::hoermann_state_venting) {
    ESP_LOGD(TAG, "UAPBridgeSwitchVent::write_state() - Setting door to vent");
    this->parent_->set_venting(state);
  } else if (!state && current_state != UAPBridge::hoermann_state_t::hoermann_state_closed) {
    ESP_LOGD(TAG, "UAPBridgeSwitchVent::write_state() - Closing door");
    this->parent_->set_venting(state);
  } else {
    ESP_LOGD(TAG, "UAPBridgeSwitchVent::write_state() - Door already in desired state");
  }
}

void UAPBridgeSwitchVent::dump_config() {
    ESP_LOGCONFIG(TAG, "UAPBridgeSwitchVent");
}

void UAPBridgeSwitchLight::setup() {
    this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}
void UAPBridgeSwitchLight::on_event_triggered() {
  if (this->parent_->get_light_enabled() != this->previousState_) {
    ESP_LOGD(TAG, "UAPBridgeSwitchLight::on_event_triggered() - adjusting state");
    this->publish_state(this->parent_->get_light_enabled());
    this->previousState_ = this->parent_->get_light_enabled();
  }
}
void UAPBridgeSwitchLight::write_state(bool state) {
  ESP_LOGD(TAG, "UAPBridgeSwitchLight::write_state() - write State triggered");
  if (this->parent_->get_light_enabled() != state){
    this->parent_->action_toggle_light();
  }
  //@TODO Check if make sens or not
  publish_state(state);
}
void UAPBridgeSwitchLight::dump_config() {
    ESP_LOGCONFIG(TAG, "UAPBridgeSwitchLight");
}
}
}