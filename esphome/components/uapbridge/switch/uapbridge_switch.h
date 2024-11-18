#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../uapbridge.h"

namespace esphome {
namespace uapbridge {

    class UAPBridgeSwitchVent : public switch_::Switch, public Component
    {
      public:
        void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
        void setup() override;
        void on_event_triggered();
        void write_state(bool state) override;
        void dump_config() override;
      private:
        UAPBridge *parent_;
        bool previousState_ = false;
    };
    class UAPBridgeSwitchLight : public switch_::Switch, public Component
    {
      public:
        void set_uapbridge_parent(UAPBridge *parent) { this->parent_ = parent; }
        void setup() override;
        void on_event_triggered();
        void write_state(bool state) override;
        void dump_config() override;
      private:
        UAPBridge *parent_;
        bool previousState_ = false;
    };
  }
}