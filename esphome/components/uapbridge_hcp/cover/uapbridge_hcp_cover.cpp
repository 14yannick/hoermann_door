#include "uapbridge_hcp_cover.h"

namespace esphome {
namespace uapbridge_hcp {

static const char *const TAG_COVER = "uapbridge_hcp.cover";

void UAPBridgeHCPCover::on_go_to_open() {
  ESP_LOGD(TAG_COVER, "Opening");
  this->parent_->open_door();
}

void UAPBridgeHCPCover::on_go_to_close() {
  ESP_LOGD(TAG_COVER, "Closing");
  this->parent_->close_door();
}

void UAPBridgeHCPCover::on_go_to_half() {
  ESP_LOGD(TAG_COVER, "Half opening");
  this->parent_->open_door_half();
}

void UAPBridgeHCPCover::on_go_to_vent() {
  ESP_LOGD(TAG_COVER, "Ventilation position");
  this->parent_->ventilation_position();
}

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
    this->parent_->stop_door();
  }

  if (call.get_position().has_value()) {
    const float pos = *call.get_position();
    if (pos >= 1.0f) {
      this->parent_->open_door();
    } else if (pos <= 0.0f) {
      this->parent_->close_door();
    } else {
      this->parent_->set_position(static_cast<uint8_t>(pos * 100.0f));
    }
  }

  if (call.get_toggle()) {
    this->parent_->impulse_door();
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
  const auto state_value = this->parent_->get_logical_door_state();

  switch (state_value) {
    case UAPBridge_hcp::STATE_OPENING:
      this->current_operation = cover::COVER_OPERATION_OPENING;
      break;

    case UAPBridge_hcp::STATE_CLOSING:
      this->current_operation = cover::COVER_OPERATION_CLOSING;
      break;

    case UAPBridge_hcp::STATE_MOVE_HALF:
    case UAPBridge_hcp::STATE_MOVE_VENTING:
      if (this->previous_position_ > current_position) {
        this->current_operation = cover::COVER_OPERATION_CLOSING;
      } else {
        this->current_operation = cover::COVER_OPERATION_OPENING;
      }
      break;

    case UAPBridge_hcp::STATE_OPEN:
    case UAPBridge_hcp::STATE_CLOSED:
    case UAPBridge_hcp::STATE_HALFOPEN:
    case UAPBridge_hcp::STATE_VENT:
    case UAPBridge_hcp::STATE_STOPPED:
    case UAPBridge_hcp::STATE_UNKNOWN:
    default:
      this->current_operation = cover::COVER_OPERATION_IDLE;
      break;
  }

  this->position = current_position;

  const bool position_changed = this->previous_position_ != this->position;
  const bool operation_changed = this->previous_operation_ != this->current_operation;

  if (operation_changed) {
    this->publish_state();
    this->previous_operation_ = this->current_operation;
  } else if (position_changed) {
    this->publish_state(false);
  }

  if (position_changed) {
    this->previous_position_ = this->position;
  }
}

}  // namespace uapbridge_hcp
}  // namespace esphome