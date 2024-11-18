#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/uapbridge/uapbridge.h"

#define SYNC_BYTE 0x55

namespace esphome {
namespace uapbridge_pic16 {

class UAPBridge_pic16 : public esphome::uapbridge::UAPBridge {
  public:
    // Enumeration for actions
    enum hoermann_action_t {
      hoermann_action_stop = 0,
      hoermann_action_open,
      hoermann_action_close,
      hoermann_action_venting,
      hoermann_action_toggle_light,
      hoermann_action_none
    };
    
    void loop() override;

    void action_open();
    void action_close();
    void action_stop();
    void action_venting();
    void action_toggle_light();

    hoermann_state_t get_state();
    std::string get_state_string();
    void set_venting(bool state);
    void set_light(bool state);

  protected:
    hoermann_state_t actual_state = hoermann_state_unkown;
    hoermann_state_t last_door_state = hoermann_state_unkown;
    hoermann_action_t actual_action = hoermann_action_none;
    std::string actual_state_string = "unknown";

    uint8_t last_rx_buffer[16] = {0};
    uint8_t rx_buffer[16];
    uint8_t output_buffer[16];
    uint32_t last_parse_time = millis();

    // Internal methods
    bool read_rs232();
    void parse_input();
    void send_command();
    uint8_t calc_checksum(uint8_t *p_data, uint8_t length);
    void handle_state_change(hoermann_state_t new_state);
    void update_boolean_state(bool &current_state, bool new_state);
};

}  // namespace uapbridge_pic16
}  // namespace esphome
