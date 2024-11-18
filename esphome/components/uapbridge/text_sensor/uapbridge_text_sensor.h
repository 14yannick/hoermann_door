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
    void on_event_triggered();

  private:
    UAPBridge *parent_;
    std::string previousState_;
};

}  // namespace uapbridge
}  // namespace esphome