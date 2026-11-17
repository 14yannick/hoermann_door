#include "uapbridge_event.h"
#include "esphome/core/log.h"

namespace esphome {
namespace uapbridge {

static const char *const TAG = "uapbridge.event";

void UAPBridgeEvent::setup() {
  this->parent_->add_on_door_event_callback([this](DoorEvent event) {
    switch (event) {
      case DoorEvent::OPENING:
        this->trigger("opening");
        break;
      case DoorEvent::OPENED:
        this->trigger("opened");
        break;
      case DoorEvent::CLOSING:
        this->trigger("closing");
        break;
      case DoorEvent::CLOSED:
        this->trigger("closed");
        break;
      case DoorEvent::STOPPED:
        this->trigger("stopped");
        break;
      case DoorEvent::VENTING:
        this->trigger("venting");
        break;
      case DoorEvent::ERROR_ON:
        this->trigger("error");
        break;
      case DoorEvent::PREWARN_ON:
        this->trigger("prewarn");
        break;
    }
  });
}

void UAPBridgeEvent::dump_config() { LOG_EVENT("", "UAPBridge Door Event", this); }

}  // namespace uapbridge
}  // namespace esphome
