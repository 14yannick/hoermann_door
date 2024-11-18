#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../uapbridge.h"

namespace esphome {
namespace uapbridge {

class UAPBridgeRelaySensor : public binary_sensor::BinarySensor, public Component {
public:
  void set_uapbridge_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void setup() override;
  void on_event_triggered();
  void dump_config() override;

protected:
  UAPBridge* parent_;
};

class UAPBridgeCommunication : public binary_sensor::BinarySensor, public Component {
public:
  void set_uapbridge_pic16_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void setup() override;
  void on_event_triggered();
  void dump_config() override;

protected:
  UAPBridge* parent_;
};

class UAPBridgeErrorSensor : public binary_sensor::BinarySensor, public Component {
public:
  void set_uapbridge_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void setup() override;
  void on_event_triggered();
  void dump_config() override;

protected:
  UAPBridge* parent_;
};

class UAPBridgePrewarnSensor : public binary_sensor::BinarySensor, public Component {
public:
  void set_uapbridge_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void setup() override;
  void on_event_triggered();
  void dump_config() override;

protected:
  UAPBridge* parent_;
};

class UAPBridgeGotValidBroadcast : public binary_sensor::BinarySensor, public Component {
public:
  void set_uapbridge_parent(UAPBridge* parent) {
    this->parent_ = parent;
  }
  void setup() override;
  void on_event_triggered();
  void dump_config() override;

protected:
  UAPBridge* parent_;
};

}  // namespace uapbridge
}  // namespace esphome
