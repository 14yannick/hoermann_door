#pragma once

#include "../uapbridge.h" // Including the UAPBridge header
#include "esphome/components/output/binary_output.h"
#include "esphome/core/component.h"

namespace esphome {
namespace uapbridge {

class UAPBridgeBinaryOutput : public output::BinaryOutput, public Component {
  public:
    void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
    void write_state(bool state) override;
    void dump_config() override;
  protected:
    UAPBridge *parent_;
};

}  // namespace uapbridge
}  // namespace esphome
