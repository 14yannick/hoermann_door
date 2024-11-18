#include "uapbridge_text_sensor.h"

namespace esphome {
namespace uapbridge {
    static const char *const TAG = "uapbridge.text_sensor";
    void UAPBridgeTextSensor::setup() {
        this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
    }

    void UAPBridgeTextSensor::on_event_triggered() {
      if (this->parent_->get_state_string() != this->previousState_){
        this->previousState_ = this->parent_->get_state_string();
        ESP_LOGD(TAG, "UAPBridgeTextSensor::update() - %s", this->previousState_.c_str());
        this->publish_state(this->previousState_);
      }
    }

}  // namespace uapbridge
}  // namespace esphome