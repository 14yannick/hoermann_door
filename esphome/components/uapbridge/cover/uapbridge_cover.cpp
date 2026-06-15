#include "uapbridge_cover.h"
namespace esphome {
namespace uapbridge {

static const char* const TAG = "uapbridge.cover";

void UAPBridgeCover::setup() {
  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}

cover::CoverTraits UAPBridgeCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_is_assumed_state(false);
  traits.set_supports_position(true);
  traits.set_supports_stop(true);
  traits.set_supports_tilt(false);
  traits.set_supports_toggle(true);
  return traits;
}

void UAPBridgeCover::control(const cover::CoverCall& call) {
  if (call.get_position().has_value()) {
    const float pos = *call.get_position();
    if (pos >= 1.0f) {
      parent_->action_open();
      ESP_LOGI(TAG, "Opening the cover");
    } else if (pos <= 0.0f) {
      parent_->action_close();
      ESP_LOGI(TAG, "Closing the cover");
    } else {
      parent_->action_set_position(pos);
      ESP_LOGI(TAG, "Setting cover position to %.2f", pos);
    }
  }
  if (call.get_stop()) {
    parent_->action_stop();
    ESP_LOGI(TAG, "Stopping the cover");
  }
  if (call.get_toggle()) {
    this->parent_->action_impulse();
  }
}

void UAPBridgeCover::on_event_triggered() {
  const UAPBridge::door_state_t state = this->parent_->get_state();
  const float raw_position = this->parent_->get_current_position();

  cover::CoverOperation new_operation;
  switch (state) {
    case UAPBridge::DOOR_STATE_OPENING:
      new_operation = cover::COVER_OPERATION_OPENING;
      break;
    case UAPBridge::DOOR_STATE_CLOSING:
      new_operation = cover::COVER_OPERATION_CLOSING;
      break;
    case UAPBridge::DOOR_STATE_MOVE_HALF:
    case UAPBridge::DOOR_STATE_MOVE_VENTING:
      // direction inferred from whether position is increasing or decreasing
      if (raw_position >= 0.0f && raw_position < this->previousPosition_) {
        new_operation = cover::COVER_OPERATION_CLOSING;
      } else {
        new_operation = cover::COVER_OPERATION_OPENING;
      }
      break;
    default:
      new_operation = cover::COVER_OPERATION_IDLE;
      break;
  }

  if (raw_position >= 0.0f) {
    this->position = raw_position;
  } else {
    // E3 protocol: derive position from logical state
    switch (state) {
      case UAPBridge::DOOR_STATE_OPEN:
        this->position = 1.0f;
        break;
      case UAPBridge::DOOR_STATE_CLOSED:
        this->position = 0.0f;
        break;
      default:
        this->position = 0.1f;
        break;
    }
  }

  const bool operation_changed = (new_operation != this->previousOperation_);
  const bool state_changed = (state != this->previousState_);
  const bool position_changed = (this->position != this->previousPosition_);

  if (operation_changed || state_changed) {
    this->current_operation = new_operation;
    ESP_LOGV(TAG, "State/operation changed — publishing");
    this->publish_state();
  } else if (position_changed) {
    ESP_LOGV(TAG, "Position changed to %.3f — publishing without state trigger", this->position);
    this->publish_state(false);
  }

  this->previousState_ = state;
  this->previousOperation_ = new_operation;
  this->previousPosition_ = this->position;
}
}  // namespace uapbridge
}  // namespace esphome
