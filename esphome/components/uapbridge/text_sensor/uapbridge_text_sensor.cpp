#include "uapbridge_text_sensor.h"

namespace esphome {
namespace uapbridge {
    static const char *const TAG = "uapbridge.text_sensor";
    void UAPBridgeTextSensor::setup() {
        this->parent_->add_on_state_callback([this]() { this->on_event_triggered(); });
    }

    void UAPBridgeTextSensor::dump_config() { LOG_TEXT_SENSOR("", "UAPBridge State", this); }

    void UAPBridgeTextSensor::on_event_triggered() {
      if (this->parent_->get_state_string() != this->previousState_){
        this->previousState_ = this->parent_->get_state_string();
        ESP_LOGD(TAG, "UAPBridgeTextSensor::update() - %s", this->previousState_.c_str());
        this->publish_state(this->previousState_);
      }
    }

    void UAPBridgeRawStatusTextSensor::dump_config() { LOG_TEXT_SENSOR("", "UAPBridge Raw Status", this); }

    void UAPBridgeRawStatusTextSensor::loop() {
      if (this->parent_->get_state() == UAPBridge::hoermann_state_unkown) {
        // state lost again (e.g. bus timeout): report it once instead of keeping a stale value
        if (this->published_ && !this->unknown_published_) {
          this->publish_state(std::string("unknown"));
          this->unknown_published_ = true;
        }
        return;
      }
      const uint16_t raw = this->parent_->get_raw_status();
      if (!this->published_ || this->unknown_published_ || raw != this->last_raw_status_) {
        char buf[8];
        snprintf(buf, sizeof(buf), "0x%04X", raw);
        this->publish_state(std::string(buf));
        this->last_raw_status_ = raw;
        this->published_ = true;
        this->unknown_published_ = false;
      }
    }

}  // namespace uapbridge
}  // namespace esphome