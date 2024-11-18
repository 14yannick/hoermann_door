#include "uapbridge_pic16.h"

namespace esphome {
namespace uapbridge_pic16 {
static const char *const TAG = "uapbridge_pic16";

void UAPBridge_pic16::loop() {
  // Timeout mechanism for PIC16 communication
  if (millis() - this->last_parse_time > 60000) {
    this->pic16_com = false;
    this->data_has_changed = true;
  }

  if (this->read_rs232()) {
    this->parse_input();
  }

  if (this->actual_action != hoermann_action_none) {
    this->send_command();
    this->actual_action = hoermann_action_none;
  }

  if (this->data_has_changed) {
    ESP_LOGD(TAG, "UAPBridge_pic16::loop() - received Data has changed.");
    this->clear_data_changed_flag();
    this->state_callback_.call();
  }
}

void UAPBridge_pic16::action_open() {
  ESP_LOGD(TAG, "Action: open called");
  this->actual_action = hoermann_action_open;
}

void UAPBridge_pic16::action_close() {
  ESP_LOGD(TAG, "Action: close called");
  this->actual_action = hoermann_action_close;
}

void UAPBridge_pic16::action_stop() {
  ESP_LOGD(TAG, "Action: stop called");
  this->actual_action = hoermann_action_stop;
}

void UAPBridge_pic16::action_venting() {
  ESP_LOGD(TAG, "Action: venting called");
  this->actual_action = hoermann_action_venting;
}

void UAPBridge_pic16::action_toggle_light() {
  ESP_LOGD(TAG, "Action: toggle light called");
  this->actual_action = hoermann_action_toggle_light;
}

UAPBridge_pic16::hoermann_state_t UAPBridge_pic16::get_state() {
  return this->actual_state;
}

std::string UAPBridge_pic16::get_state_string() {
  return this->actual_state_string;
}

void UAPBridge_pic16::set_venting(bool state) {
  this->venting_enabled = state;
  if (state) {
    this->action_venting();
  } else {
    this->action_close();
  }
  ESP_LOGD(TAG, "Venting state set to %s", state ? "ON" : "OFF");
}

void UAPBridge_pic16::set_light(bool state) {
  this->light_enabled = state;
  if (state) {
    this->action_toggle_light();
  }
  ESP_LOGD(TAG, "Light state set to %s", state ? "ON" : "OFF");
}

bool UAPBridge_pic16::read_rs232() {
  static uint8_t counter = 0;
  static uint8_t len = 0;
  uint8_t data;

  while (this->available() > 0) {
    data = (uint8_t)this->read();

    if (data == SYNC_BYTE && counter == 0) {
      this->rx_buffer[counter++] = data;
      len = 0;
    } else if (counter > 0) {
      this->rx_buffer[counter++] = data;
      if (counter == 3) {
        if (data < 16) {
          len = data + 4; // 3 = SYNC + CMD + LEN + CHK, limit to 15 data bytes
        } else {
          counter = 0;  // Reset if length is invalid
        }
      } else if (counter == len) {
        if (this->calc_checksum(this->rx_buffer, len - 1) == data) {
          // Final check: make sure the length and checksum are valid
          if (len > 0) {  // You could add extra checks here
            counter = 0;
            this->last_parse_time = millis(); // Update last parse time here
            return true;
          }
        }
        counter = 0;  // Reset if checksum doesn't match
      }
    } else {
      ESP_LOGD(TAG, "read_rs232, wrong SYNC byte data = %i", data);
    }
  }
  return false;
}

void UAPBridge_pic16::parse_input() {
  if (this->rx_buffer[1] == 0x00 && this->rx_buffer[2] == 0x02) {
    hoermann_state_t new_state = hoermann_state_stopped;

    if ((this->rx_buffer[3] & 0x01) == 0x01) {
      new_state = hoermann_state_open;
    } else if ((this->rx_buffer[3] & 0x02) == 0x02) {
      new_state = hoermann_state_closed;
    } else if ((this->rx_buffer[3] & 0x60) == 0x40) {
      new_state = hoermann_state_opening;
    } else if ((this->rx_buffer[3] & 0x60) == 0x60) {
      new_state = hoermann_state_closing;
    } else if ((this->rx_buffer[3] & 0x80) == 0x80) {
      new_state = hoermann_state_venting;
    } else if ((this->rx_buffer[3] & 0x10) == 0x10) {
      new_state = hoermann_state_error;
    }

    if (new_state != this->actual_state) {
      this->handle_state_change(new_state);
    }

    this->update_boolean_state(this->relay_enabled, (this->rx_buffer[3] & 0x04) == 0x04);
    this->update_boolean_state(this->light_enabled, (this->rx_buffer[3] & 0x08) == 0x08);
    this->update_boolean_state(this->venting_enabled, (this->rx_buffer[3] & 0x80) == 0x80);
    this->update_boolean_state(this->error_state, (this->rx_buffer[3] & 0x10) == 0x10);
    this->update_boolean_state(this->prewarn_state, (this->rx_buffer[4] & 0x01) == 0x01);

    // Update valid_broadcast if a non-default message is received
    if (!this->valid_broadcast && (this->rx_buffer[3] != 0 || this->rx_buffer[4] != 0)) {
      this->valid_broadcast = true;
      this->data_has_changed = true;
    }

    if (!this->pic16_com) {
      this->pic16_com = true;
      this->data_has_changed = true;
    }
  }
}


void UAPBridge_pic16::send_command() {
  this->output_buffer[0] = SYNC_BYTE;
  this->output_buffer[1] = 0x01;
  this->output_buffer[2] = 0x01;
  this->output_buffer[3] = (uint8_t)this->actual_action;
  this->output_buffer[4] = this->output_buffer[0] + this->output_buffer[1] + this->output_buffer[2] + this->output_buffer[3];
  this->write_array(&this->output_buffer[0], 5);
}

uint8_t UAPBridge_pic16::calc_checksum(uint8_t *p_data, uint8_t length) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < length; ++i) {
    crc += p_data[i];
  }
  return crc;
}

void UAPBridge_pic16::handle_state_change(hoermann_state_t new_state) {
  this->actual_state = new_state;

  switch (new_state) {
    case hoermann_state_open:
      this->actual_state_string = "Open";
      break;
    case hoermann_state_closed:
      this->actual_state_string = "Closed";
      break;
    case hoermann_state_opening:
      this->actual_state_string = "Opening";
      break;
    case hoermann_state_closing:
      this->actual_state_string = "Closing";
      break;
    case hoermann_state_venting:
      this->actual_state_string = "Venting";
      break;
    case hoermann_state_error:
      this->actual_state_string = "Error";
      break;
    case hoermann_state_stopped:
    default:
      this->actual_state_string = "Stopped";
      break;
  }

  this->data_has_changed = true;
}

void UAPBridge_pic16::update_boolean_state(bool &current_state, bool new_state) {
  if (current_state != new_state) {
    current_state = new_state;
    this->data_has_changed = true;
  }
}

}  // namespace uapbridge_pic16
}  // namespace esphome
