#include "uapbridge_hcp_cover.h"

namespace esphome {
namespace uapbridge_hcp {

using uapbridge::UAPBridge;

static const char *const TAG_COVER = "uapbridge_hcp.cover";

cover::CoverTraits UAPBridgeHCPCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_is_assumed_state(false);
  traits.set_supports_position(true);
  traits.set_supports_tilt(false);
  traits.set_supports_stop(true);
  traits.set_supports_toggle(true);
  return traits;
}

void UAPBridgeHCPCover::control(const cover::CoverCall &call) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG_COVER, "Parent not set");
    return;
  }

  if (call.get_stop()) {
    this->parent_->action_stop();
  }

  if (call.get_position().has_value()) {
    const float pos = *call.get_position();
    if (pos >= 1.0f) {
      this->parent_->action_open();
    } else if (pos <= 0.0f) {
      this->parent_->action_close();
    } else {
      this->parent_->action_set_position(pos);
    }
  }

  if (call.get_toggle()) {
    this->parent_->action_impulse();
  }
}

void UAPBridgeHCPCover::setup() {
  ESP_LOGD(TAG_COVER, "Setting up cover");

  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG_COVER, "Parent is null");
    this->mark_failed();
    return;
  }

  this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
}

void UAPBridgeHCPCover::on_event_triggered() {
  if (this->parent_ == nullptr) {
    return;
  }

  if (!this->parent_->is_valid()) {
    if (!this->status_has_warning()) {
      ESP_LOGD(TAG_COVER, "State invalid, setting warning");
      this->status_set_warning();
    }
    return;
  }

  if (this->status_has_warning()) {
    ESP_LOGD(TAG_COVER, "Clearing warning");
    this->status_clear_warning();
  }

  const float current_position = this->parent_->get_current_position();
  const UAPBridge::door_state_t state_value = this->parent_->get_state();

  cover::CoverOperation new_operation;
  switch (state_value) {
    case UAPBridge::DOOR_STATE_OPENING:
      new_operation = cover::COVER_OPERATION_OPENING;
      break;

    case UAPBridge::DOOR_STATE_CLOSING:
      new_operation = cover::COVER_OPERATION_CLOSING;
      break;

    case UAPBridge::DOOR_STATE_MOVE_HALF:
    case UAPBridge::DOOR_STATE_MOVE_VENTING:
      if (this->previous_position_ > current_position) {
        new_operation = cover::COVER_OPERATION_CLOSING;
      } else {
        new_operation = cover::COVER_OPERATION_OPENING;
      }
      break;

    default:
      new_operation = cover::COVER_OPERATION_IDLE;
      break;
  }

  this->position = current_position;

  const bool position_changed = (this->previous_position_ != this->position);
  const bool operation_changed = (this->previous_operation_ != new_operation);

  if (operation_changed) {
    this->current_operation = new_operation;
    this->publish_state();
    this->previous_operation_ = new_operation;
  } else if (position_changed) {
    this->publish_state(false);
  }

  if (position_changed) {
    this->previous_position_ = this->position;
  }
}

}  // namespace uapbridge_hcp
}  // namespace esphome
