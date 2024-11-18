#include "uapbridge_binarySensor.h"

namespace esphome {
namespace uapbridge {

static const char* const TAG = "uapbridge.binary_sensor";

// Relay Sensor
void UAPBridgeRelaySensor::setup() {
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
  this->publish_state(this->parent_->get_relay_enabled());
}

void UAPBridgeRelaySensor::on_event_triggered() {
  if (this->parent_->get_relay_enabled() != this->state) {
    this->publish_state(this->parent_->get_relay_enabled());
  }
}

void UAPBridgeRelaySensor::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgeRelaySensor");
}

// Communication Sensor
void UAPBridgeCommunication::setup() {
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
  this->publish_state(this->parent_->get_pic16_com());
}

void UAPBridgeCommunication::on_event_triggered() {
  if (this->parent_->get_pic16_com() != this->state) {
    this->publish_state(this->parent_->get_pic16_com());
  }
}

void UAPBridgeCommunication::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgeCommunication");
}

// Error Sensor
void UAPBridgeErrorSensor::setup() {
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
  this->publish_state(this->parent_->get_error_state());
}

void UAPBridgeErrorSensor::on_event_triggered() {
  if (this->parent_->get_error_state() != this->state) {
    this->publish_state(this->parent_->get_error_state());
  }
}

void UAPBridgeErrorSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgeErrorSensor");
}

void UAPBridgePrewarnSensor::setup() {
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
  this->publish_state(this->parent_->get_prewarn_state());
}

void UAPBridgePrewarnSensor::on_event_triggered() {
  if (this->parent_->get_prewarn_state() != this->state) {
    this->publish_state(this->parent_->get_prewarn_state());
  }
}

void UAPBridgePrewarnSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgePrewarnSensor");
}

// GotValidBroadcast Sensor (formerly DataHasChanged)
void UAPBridgeGotValidBroadcast::setup() {
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
  this->publish_state(this->parent_->get_valid_broadcast());
}

void UAPBridgeGotValidBroadcast::on_event_triggered() {
  if (this->parent_->get_valid_broadcast() != this->state) {
    this->publish_state(this->parent_->get_valid_broadcast());
  }
}

void UAPBridgeGotValidBroadcast::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgeGotValidBroadcast");
}

}  // namespace uapbridge
}  // namespace esphome
