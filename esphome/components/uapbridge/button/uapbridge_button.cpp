#include "uapbridge_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace uapbridge {

static const char* const TAG = "uapbridge.button";

void UAPBridgeButtonVent::press_action() {
  ESP_LOGD(TAG, "UAPBridgeButtonVent::press_action() - Triggering vent position");
  this->parent_->action_venting();
}

void UAPBridgeButtonImpulse::press_action() {
  ESP_LOGD(TAG, "UAPBridgeButtonImpulse::press_action() - Triggering impulse action");
  this->parent_->action_impulse();
}

}  // namespace uapbridge
}  // namespace esphome
