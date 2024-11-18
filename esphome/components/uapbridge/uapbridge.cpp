#include "uapbridge.h"

namespace esphome {
namespace uapbridge {
static const char *const TAG = "UAPBridge";

void UAPBridge::setup() {
  if (this->rts_pin_ != nullptr) {
    this->rts_pin_->setup();
    this->rts_pin_->pin_mode(gpio::Flags::FLAG_OUTPUT);
    this->rts_pin_->digital_write(false);// LOW(false) = listen, HIGH(true) = transmit
  }
  ESP_LOGCONFIG(TAG, "Garage setup called!");
}

void UAPBridge::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridge");
  if (this->rts_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  RTS Pin: %s", this->rts_pin_->dump_summary().c_str());
  }
  ESP_LOGCONFIG(TAG, "  Auto Correction: %s", this->auto_correction ? "true" : "false");
}

void UAPBridge::add_on_state_callback(std::function<void()> &&callback) {
  this->state_callback_.add(std::move(callback));
}

}  // namespace uapbridge
}  // namespace esphome
