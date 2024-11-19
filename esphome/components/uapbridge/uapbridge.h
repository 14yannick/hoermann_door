#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"


namespace esphome {
namespace uapbridge {

class UAPBridge : public uart::UARTDevice, public Component {
  public:
    // Enumeration for states
    enum hoermann_state_t {
      hoermann_state_stopped      = 0x0000,
      hoermann_state_open         = 0x0001,
      hoermann_state_closed       = 0x0002,
      hoermann_state_opt_relay    = 0x0004,
      hoermann_state_light_relay  = 0x0008,
      hoermann_state_error        = 0x0010,
      hoermann_state_direction    = 0x0020,
      hoermann_state_moving       = 0x0040,
      hoermann_state_opening      = 0x0040,
      hoermann_state_closing      = 0x0060,
      hoermann_state_venting      = 0x0080,
      hoermann_state_prewarn      = 0x0100,
      hoermann_state_unkown       = 0xFFFF
    };

    void setup() override;
    void dump_config() override;
    void add_on_state_callback(std::function<void()> &&callback);
    void set_rts_pin(InternalGPIOPin *rts_pin) { this->rts_pin_ = rts_pin; }
    void set_auto_correction(bool value) { this->auto_correction = value; }

    virtual void action_open() = 0;
    virtual void action_close() = 0;
    virtual void action_stop() = 0;
    virtual void action_venting() = 0;
    virtual void action_toggle_light() = 0;
    virtual void action_impulse() = 0;

    virtual hoermann_state_t get_state() = 0;
    virtual std::string get_state_string() = 0;
    virtual void set_venting(bool state) = 0;
    bool get_venting_enabled() const { return this->venting_enabled; }
    virtual void set_light(bool state) = 0;
    bool get_light_enabled() const { return this->light_enabled; }
    bool get_relay_enabled() const { return this->relay_enabled; }
    void set_relay_enabled(bool value) { this->relay_enabled = value; }
    bool get_error_state() const { return this->error_state; }
    void set_error_state(bool value) { this->error_state = value; }
    bool get_prewarn_state() const { return this->prewarn_state; }
    void set_prewarn_state(bool value) { this->prewarn_state = value; }
    bool get_pic16_com() const { return this->pic16_com; }
    void set_pic16_com(bool value) { this->pic16_com = value; }
    bool get_valid_broadcast() const { return this->valid_broadcast; }
    void set_valid_broadcast(bool value) { this->valid_broadcast = value; }
    bool has_data_changed() const { return this->data_has_changed; }
    void clear_data_changed_flag() { this->data_has_changed = false; }

  protected:
    // yaml parameters
    InternalGPIOPin *rts_pin_ = nullptr;
    bool auto_correction = false;
    // \yaml parameters
    CallbackManager<void()> state_callback_;
    // state variables
    bool venting_enabled = false;
    bool light_enabled = false;
    bool relay_enabled = false;
    bool error_state = false;
    bool prewarn_state = false;
    bool pic16_com = false;
    bool valid_broadcast = false;
    bool data_has_changed = false;
    // \state variables
};

}  // namespace uapbridge
}  // namespace esphome
