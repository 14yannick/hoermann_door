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
  UAPBridge::door_state_t current_state = this->parent_->get_state();

  if (state && current_state != UAPBridge::DOOR_STATE_VENTING) {
    ESP_LOGD(TAG, "UAPBridgeSwitchVent::write_state() - Setting door to vent");
    this->parent_->set_venting(state);
  } else if (!state && current_state != UAPBridge::DOOR_STATE_CLOSED) {
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

void UAPBridgeSwitchHalf::setup() {
    this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}

void UAPBridgeSwitchHalf::on_event_triggered() {
  const bool is_half = (this->parent_->get_state() == UAPBridge::DOOR_STATE_HALFOPEN);
  if (is_half != this->previousState_) {
    this->publish_state(is_half);
    this->previousState_ = is_half;
  }
}

void UAPBridgeSwitchHalf::write_state(bool state) {
  if (state && this->parent_->get_state() != UAPBridge::DOOR_STATE_HALFOPEN) {
    ESP_LOGD("uapbridge.switch", "UAPBridgeSwitchHalf::write_state() - opening to half");
    this->parent_->action_open_half();
  } else if (!state && this->parent_->get_state() != UAPBridge::DOOR_STATE_CLOSED) {
    ESP_LOGD("uapbridge.switch", "UAPBridgeSwitchHalf::write_state() - closing");
    this->parent_->action_close();
  }
}

void UAPBridgeSwitchHalf::dump_config() {
    ESP_LOGCONFIG("uapbridge.switch", "UAPBridgeSwitchHalf");
}
}
}