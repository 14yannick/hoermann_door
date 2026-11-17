#pragma once

#include "esphome/core/automation.h"
#include "uapbridge.h"

namespace esphome {
namespace uapbridge {

// Fires when the bridge reports the given door event (see UAPBridge::add_on_door_event_callback).
class DoorEventTrigger : public Trigger<> {
 public:
  DoorEventTrigger(UAPBridge *parent, DoorEvent event) {
    parent->add_on_door_event_callback([this, event](DoorEvent e) {
      if (e == event) {
        this->trigger();
      }
    });
  }
};

// Actions: each calls the same bridge method the existing buttons/cover/switches use.
template<typename... Ts> class OpenAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override { this->parent_->action_open(); }
};

template<typename... Ts> class CloseAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override { this->parent_->action_close(); }
};

template<typename... Ts> class StopAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override { this->parent_->action_stop(); }
};

template<typename... Ts> class VentingAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override { this->parent_->action_venting(); }
};

template<typename... Ts> class ImpulseAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override { this->parent_->action_impulse(); }
};

template<typename... Ts> class ToggleLightAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override { this->parent_->action_toggle_light(); }
};

// Switches the light to a fixed state: toggles only if the light differs. Does nothing while the
// door state is unknown (boot, bus timeout), because the real light state is not known then.
template<bool ON, typename... Ts> class SetLightAction : public Action<Ts...>, public Parented<UAPBridge> {
 public:
  void play(const Ts &...x) override {
    if (this->parent_->get_state() == UAPBridge::hoermann_state_unkown) {
      ESP_LOGW("uapbridge.automation", "light_%s ignored: door state not known yet", ON ? "on" : "off");
      return;
    }
    if (this->parent_->get_light_enabled() != ON) {
      this->parent_->action_toggle_light();
    }
  }
};
template<typename... Ts> using LightOnAction = SetLightAction<true, Ts...>;
template<typename... Ts> using LightOffAction = SetLightAction<false, Ts...>;

}  // namespace uapbridge
}  // namespace esphome
