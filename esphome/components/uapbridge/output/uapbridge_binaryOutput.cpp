#include "uapbridge_binaryOutput.h"

#include "esphome/core/log.h"

namespace esphome {
namespace uapbridge {

static const char *TAG = "uapbridge.binary_output";

void UAPBridgeBinaryOutput::write_state(bool state) {
  if (this->parent_->get_light_enabled() != state) {
    ESP_LOGD(TAG, "UAPBridgeBinaryOutput::write_state() - setting light to %s", state ? "true" : "false");
    this->parent_->action_toggle_light();
  }
}

void UAPBridgeBinaryOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgeBinaryOutput:");
  if (this->parent_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Parent UAPBridge is set.");
  } else {
    ESP_LOGCONFIG(TAG, "  Warning: Parent UAPBridge is not set.");
  }
}

}  // namespace uapbridge
}  // namespace esphome
