#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "../uapbridge.h"

namespace esphome {
namespace uapbridge {

class UAPBridgeTextSensor : public text_sensor::TextSensor, public Component
{
  public:
    void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
    void setup() override;
    void dump_config() override;
    void on_event_triggered();

  private:
    UAPBridge *parent_;
    std::string previousState_;
};

// Diagnostic: raw door status word as hex (e.g. "0x0102"), published only when it changes.
// Nothing is published before the door state is known; "unknown" once if it is lost again.
class UAPBridgeRawStatusTextSensor : public text_sensor::TextSensor, public Component
{
  public:
    void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
    void loop() override;
    void dump_config() override;

  private:
    UAPBridge *parent_;
    uint16_t last_raw_status_ = 0;
    bool published_ = false;          // a status value has been published
    bool unknown_published_ = false;  // "unknown" is currently published
};

}  // namespace uapbridge
}  // namespace esphome