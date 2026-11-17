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
    LOG_PIN("  RTS Pin: ", this->rts_pin_);
  }
  ESP_LOGCONFIG(TAG, "  Auto Correction: %s", this->auto_correction ? "true" : "false");
}

void UAPBridge::add_on_state_callback(std::function<void()> &&callback) {
  this->state_callback_.add(std::move(callback));
}

void UAPBridge::add_on_door_event_callback(std::function<void(DoorEvent)> &&callback) {
  this->door_event_callback_.add(std::move(callback));
  if (!this->door_event_detection_active_) {
    // hook into the existing state callback only when somebody actually listens
    this->door_event_detection_active_ = true;
    this->add_on_state_callback([this]() { this->detect_door_events_(); });
  }
}

void UAPBridge::detect_door_events_() {
  const hoermann_state_t state = this->get_state();
  const bool error = this->error_state;
  const bool prewarn = this->prewarn_state;
  // events only for changes between two known states (no events at boot or after a bus timeout)
  const bool was_known = this->last_event_state_ != hoermann_state_unkown;
  const bool is_known = state != hoermann_state_unkown;

  if (was_known && is_known) {
    if (state != this->last_event_state_) {
      switch (state) {
        case hoermann_state_opening:
          this->door_event_callback_.call(DoorEvent::OPENING);
          break;
        case hoermann_state_open:
          this->door_event_callback_.call(DoorEvent::OPENED);
          break;
        case hoermann_state_closing:
          this->door_event_callback_.call(DoorEvent::CLOSING);
          break;
        case hoermann_state_closed:
          this->door_event_callback_.call(DoorEvent::CLOSED);
          break;
        case hoermann_state_stopped:
          this->door_event_callback_.call(DoorEvent::STOPPED);
          break;
        case hoermann_state_venting:
          this->door_event_callback_.call(DoorEvent::VENTING);
          break;
        default:
          // e.g. the pic16 "error" door state: reported via the error bit below
          break;
      }
    }
    if (error && !this->last_event_error_) {
      this->door_event_callback_.call(DoorEvent::ERROR_ON);
    }
    if (prewarn && !this->last_event_prewarn_) {
      this->door_event_callback_.call(DoorEvent::PREWARN_ON);
    }
  }

  this->last_event_state_ = state;
  this->last_event_error_ = error;
  this->last_event_prewarn_ = prewarn;
}

}  // namespace uapbridge
}  // namespace esphome
