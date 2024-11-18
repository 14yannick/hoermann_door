#include "esphome/core/log.h"
#include "uapbridge_light.h"

namespace esphome {
namespace uapbridge {

static const char* TAG = "uapbridge.light";

light::LightTraits UAPBridgeLight::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::ON_OFF});
  return traits;
}

void UAPBridgeLight::setup() {
  ESP_LOGD(TAG, "UAPBridgeLight::setup() - setup method called");
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}

void UAPBridgeLight::write_state(light::LightState* state) {
  bool binary;
  state->current_values_as_binary(&binary);
  if (binary) {
    this->output_->turn_on();
  } else {
    this->output_->turn_off();
  }
}

void UAPBridgeLight::on_event_triggered() {
  if (this->state_->current_values.is_on() != this->parent_->get_light_enabled()) {
    // Adjust the state of the light based on the external light state
    ESP_LOGD(TAG, "UAPBridgeLight::update() - adjusting state");
    if (this->parent_->get_light_enabled()) {
      this->state_->turn_on().perform();
    } else {
      this->state_->turn_off().perform();
    }
  }
}

void UAPBridgeLight::dump_config() {
  ESP_LOGCONFIG(TAG, "UAPBridgeLight:");
}
}  // namespace uapbridge
}  // namespace esphome