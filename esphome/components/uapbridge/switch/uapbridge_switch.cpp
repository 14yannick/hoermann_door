#include "uapbridge_switch.h"

namespace esphome {
namespace uapbridge {
static const char *const TAG = "uapbridge.switch";
void UAPBridgeSwitchVent::setup() {
    this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}
void UAPBridgeSwitchVent::on_event_triggered() {
  if (this->parent_->get_state() == UAPBridge::hoermann_state_t::hoermann_state_unkown) {
    // door state not known yet --> nothing reliable to publish
    return;
  }
  if (!this->initial_published_ || this->parent_->get_venting_enabled() != this->previousState_) {
    ESP_LOGD(TAG, "UAPBridgeSwitchVent::on_event_triggered() - adjusting state");
    this->publish_state(this->parent_->get_venting_enabled());
    this->previousState_ = this->parent_->get_venting_enabled();
    this->initial_published_ = true;
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
    LOG_SWITCH("", "UAPBridge Venting Switch", this);
}

void UAPBridgeSwitchLight::setup() {
    this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}
void UAPBridgeSwitchLight::on_event_triggered() {
  if (this->parent_->get_state() == UAPBridge::hoermann_state_t::hoermann_state_unkown) {
    // door state not known yet --> nothing reliable to publish
    return;
  }
  if (!this->initial_published_ || this->parent_->get_light_enabled() != this->previousState_) {
    ESP_LOGD(TAG, "UAPBridgeSwitchLight::on_event_triggered() - adjusting state");
    this->publish_state(this->parent_->get_light_enabled());
    this->previousState_ = this->parent_->get_light_enabled();
    this->initial_published_ = true;
  }
}
void UAPBridgeSwitchLight::write_state(bool state) {
  ESP_LOGD(TAG, "UAPBridgeSwitchLight::write_state() - write State triggered");
  if (this->parent_->get_light_enabled() != state){
    if (this->parent_->command_pending()) {
      // toggle would be dropped --> keep showing the real light state instead of the requested one
      ESP_LOGW(TAG, "UAPBridgeSwitchLight::write_state() - previous command still pending, light not toggled");
      this->publish_state(this->parent_->get_light_enabled());
      return;
    }
    this->parent_->action_toggle_light();
  }
  // optimistic: the real state is published again via on_event_triggered() once the door reports it
  this->publish_state(state);
}
void UAPBridgeSwitchLight::dump_config() {
    LOG_SWITCH("", "UAPBridge Light Switch", this);
}
}
}