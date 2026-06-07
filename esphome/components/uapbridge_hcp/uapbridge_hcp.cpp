#include "uapbridge_hcp.h"

namespace esphome {
namespace uapbridge_hcp {

static const uint8_t RESPONSE_TEMPLATE_FCN17_CMD03_L08[] = {
    0x02, 0x17, 0x10, 0x3E, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x1B
};

static const uint8_t RESPONSE_TEMPLATE_FCN17_CMD04_L02[] = {
    0x02, 0x17, 0x04, 0x0F, 0x00, 0x04, 0xFD, 0x0A, 0x72
};

static const uint8_t RESPONSE_TEMPLATE_FCN17_CMD02_L05[] = {
    0x02, 0x17, 0x0A, 0x00, 0x00, 0x02, 0x05, 0x04, 0x30,
    0x10, 0xFF, 0xA8, 0x45, 0x0E, 0xDF
};

void UAPBridge_hcp::setup() {
  ESP_LOGCONFIG(TAG_UAPBRIDGE_HCP, "Setting up Hoermann HCP UART bridge...");
  reset_rx_();
  tx_len_ = 0;
  state_.valid = false;
  if (high_freq_loop_)
    high_freq_requester_.start();
}

void UAPBridge_hcp::dump_config() {
  ESP_LOGCONFIG(TAG_UAPBRIDGE_HCP, "Hoermann HCP:");
  ESP_LOGCONFIG(TAG_UAPBRIDGE_HCP, "  UART-based Hörmann HCP emulator");
  ESP_LOGCONFIG(TAG_UAPBRIDGE_HCP, "  High-frequency loop: %s", high_freq_loop_ ? "enabled" : "disabled");
  this->check_uart_settings(57600, 1, uart::UART_CONFIG_PARITY_EVEN, 8);
}

void UAPBridge_hcp::loop() {
  process_incoming_();
}

void UAPBridge_hcp::add_on_state_callback(std::function<void()> &&callback) {
  this->state_callbacks_.push_back(std::move(callback));
}

bool UAPBridge_hcp::is_valid() const {
  return this->state_.valid;
}

float UAPBridge_hcp::get_current_position() const {
  return static_cast<float>(this->state_.door_current_position) / 200.0f;
}

UAPBridge_hcp::DoorState UAPBridge_hcp::get_logical_door_state() const {
  return state_.logical_door_state;
}

bool UAPBridge_hcp::get_light_state() const {
  return this->state_.lamp_on;
}

bool UAPBridge_hcp::get_relay_state() const {
  return this->state_.relay_on;
}

void UAPBridge_hcp::impulse_door() {
  if (state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = IMPULSE;
  }
}

void UAPBridge_hcp::process_incoming_() {
  while (this->available()) {
    uint8_t byte;
    if (!this->read_byte(&byte))
      break;

    if (rx_len_ >= RX_BUFFER_SIZE - 1) {
      ESP_LOGW(TAG_UAPBRIDGE_HCP, "RX buffer overflow, skipping next frame");
      reset_rx_();
      skip_frame_ = true;
      return;
    }

    rx_buffer_[rx_len_++] = byte;
    recv_time_us_ = micros();
  }

  if (rx_len_ > 0 && (micros() - recv_time_us_ > T3_5_US)) {
    if (!skip_frame_) {
      process_frame_();
      if (tx_len_ > 0) {
        send_response_();
      }
    }
    skip_frame_ = false;
    reset_rx_();
  }
}

void UAPBridge_hcp::process_frame_() {
  tx_len_ = 0;

  if (rx_len_ < 5) {
    ESP_LOGW(TAG_UAPBRIDGE_HCP, "Frame skipped, invalid length: %u", (unsigned) rx_len_);
    return;
  }

  if (rx_buffer_[0] != DEVICE_ID && rx_buffer_[0] != BROADCAST_ID) {
    return;
  }

  const uint16_t crc = read_crc_(rx_buffer_, rx_len_);
  if (crc != calculate_crc_(rx_buffer_, rx_len_ - 2)) {
    ESP_LOGW(TAG_UAPBRIDGE_HCP, "Frame skipped, CRC mismatch");
    return;
  }

  ESP_LOGV(TAG_UAPBRIDGE_HCP, "Incoming frame len=%u fc=0x%02X", (unsigned) rx_len_, rx_buffer_[1]);

  switch (rx_buffer_[1]) {
    case 0x10:  // Write Multiple Registers
      if (rx_len_ == 0x1B && rx_buffer_[0] == BROADCAST_ID) {
        process_broadcast_status_frame_();
        return;
      }
      break;

    case 0x17:  // Read/Write Multiple Registers
      if (rx_buffer_[0] == DEVICE_ID) {
        if (rx_len_ == 0x11) {
          process_device_status_frame_();
          return;
        }
        if (rx_len_ == 0x13) {
          process_device_bus_scan_frame_();
          return;
        }
      }
      break;

    default:
      break;
  }

  ESP_LOGV(TAG_UAPBRIDGE_HCP, "Unhandled frame");
}

void UAPBridge_hcp::process_device_status_frame_() {
  const uint8_t counter = rx_buffer_[11];
  const uint8_t cmd = rx_buffer_[12];

  if (rx_buffer_[5] == 0x08) {
    memcpy(tx_buffer_, RESPONSE_TEMPLATE_FCN17_CMD03_L08, sizeof(RESPONSE_TEMPLATE_FCN17_CMD03_L08));
    tx_buffer_[0] = rx_buffer_[0];
    tx_buffer_[3] = counter;
    tx_buffer_[5] = cmd;
    tx_len_ = sizeof(RESPONSE_TEMPLATE_FCN17_CMD03_L08);

    switch (state_machine_) {
      case OPEN_DOOR:
        tx_buffer_[7] = 0x02;
        tx_buffer_[8] = 0x10;
        state_machine_ = OPEN_DOOR_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case OPEN_DOOR_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[8] = 0x10;
          state_machine_ = WAITING;
        }
        break;

      case CLOSE_DOOR:
        tx_buffer_[7] = 0x02;
        tx_buffer_[8] = 0x20;
        state_machine_ = CLOSE_DOOR_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case CLOSE_DOOR_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[8] = 0x20;
          state_machine_ = WAITING;
        }
        break;
      case IMPULSE:
        tx_buffer_[7] = 0x02;
        tx_buffer_[8] = 0x40;
        state_machine_ = STOP_DOOR_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case STOP_DOOR:
        if (state_.door_current_position == 0 || state_.door_current_position == 200) {
          state_machine_ = WAITING;
        } else {
          tx_buffer_[7] = 0x02;
          tx_buffer_[8] = 0x40;
          state_machine_ = STOP_DOOR_RELEASE;
          last_state_time_ms_ = millis();
        }
        break;

      case STOP_DOOR_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[8] = 0x40;
          state_machine_ = WAITING;
        }
        break;

      case SET_POSITION_OPEN:
        tx_buffer_[7] = 0x02;
        tx_buffer_[8] = 0x10;
        state_machine_ = SET_POSITION_OPEN_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case SET_POSITION_OPEN_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[8] = 0x10;
          state_machine_ = SET_POSITION_OPEN_PROGRESS;
        }
        break;

      case SET_POSITION_OPEN_PROGRESS:
        if (state_.door_current_position >= state_.goto_position) {
          last_state_time_ms_ = millis();
          state_machine_ = STOP_DOOR;
        }
        break;

      case SET_POSITION_CLOSE:
        tx_buffer_[7] = 0x02;
        tx_buffer_[8] = 0x20;
        state_machine_ = SET_POSITION_CLOSE_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case SET_POSITION_CLOSE_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[8] = 0x20;
          state_machine_ = SET_POSITION_CLOSE_PROGRESS;
        }
        break;

      case SET_POSITION_CLOSE_PROGRESS:
        if (state_.door_current_position <= state_.goto_position) {
          last_state_time_ms_ = millis();
          state_machine_ = STOP_DOOR;
        }
        break;

      case VENTPOSITION:
        tx_buffer_[7] = 0x02;
        tx_buffer_[9] = 0x40;
        state_machine_ = VENTPOSITION_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case VENTPOSITION_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[9] = 0x40;
          state_machine_ = WAITING;
        }
        break;

      case OPEN_DOOR_HALF:
        tx_buffer_[7] = 0x02;
        tx_buffer_[9] = 0x04;
        state_machine_ = OPEN_DOOR_HALF_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case OPEN_DOOR_HALF_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x01;
          tx_buffer_[9] = 0x04;
          state_machine_ = WAITING;
        }
        break;

      case TOGGLE_LAMP:
        tx_buffer_[7] = 0x01;
        tx_buffer_[8] = 0x00;
        tx_buffer_[9] = 0x02;
        tx_buffer_[10] = 0x00;
        state_machine_ = TOGGLE_LAMP_RELEASE;
        last_state_time_ms_ = millis();
        break;

      case TOGGLE_LAMP_RELEASE:
        if (last_state_time_ms_ + SIMULATE_KEYPRESS_DELAY_MS < millis()) {
          tx_buffer_[7] = 0x08;
          tx_buffer_[8] = 0x00;
          tx_buffer_[9] = 0x02;
          tx_buffer_[10] = 0x00;
          state_machine_ = WAITING;
        }
        break;

      case WAITING:
      default:
        break;
    }

    return;
  }

  if (rx_buffer_[5] == 0x02) {
    memcpy(tx_buffer_, RESPONSE_TEMPLATE_FCN17_CMD04_L02, sizeof(RESPONSE_TEMPLATE_FCN17_CMD04_L02));
    tx_buffer_[0] = rx_buffer_[0];
    tx_buffer_[3] = counter;
    tx_buffer_[5] = cmd;
    tx_len_ = sizeof(RESPONSE_TEMPLATE_FCN17_CMD04_L02);
    return;
  }

  ESP_LOGV(TAG_UAPBRIDGE_HCP, "Unexpected status frame");
}

void UAPBridge_hcp::process_device_bus_scan_frame_() {
  const uint8_t counter = rx_buffer_[11];
  const uint8_t cmd = rx_buffer_[12];

  memcpy(tx_buffer_, RESPONSE_TEMPLATE_FCN17_CMD02_L05, sizeof(RESPONSE_TEMPLATE_FCN17_CMD02_L05));
  tx_buffer_[0] = rx_buffer_[0];
  tx_buffer_[3] = counter;
  tx_buffer_[5] = cmd;
  tx_len_ = sizeof(RESPONSE_TEMPLATE_FCN17_CMD02_L05);
}

void UAPBridge_hcp::process_broadcast_status_frame_() {
  bool any = false;

  // Broadcast register mapping based on observed frame layout
  const uint8_t target_pos = rx_buffer_[9];
  const uint8_t current_pos = rx_buffer_[10];

  // State register split into high and low byte
  const uint8_t state_hi = rx_buffer_[11];
  const uint8_t state_lo = rx_buffer_[12];

  // Register 7 split into high byte and low byte
  const uint8_t reg7_hi = rx_buffer_[19];
  const uint8_t reg7_lo = rx_buffer_[20];

  const DoorState logical_state = decode_door_state_(state_hi, state_lo);

  any |= check_changed_set_(state_.lamp_on, reg7_lo == 0x10 || reg7_lo == 0x14);
  any |= check_changed_set_(state_.relay_on, reg7_hi == 0x02 || reg7_lo == 0x14 || reg7_lo == 0x04);

  any |= check_changed_set_(state_.door_target_position, target_pos);
  any |= check_changed_set_(state_.door_current_position, current_pos);

  any |= check_changed_set_(state_.door_state_hi, state_hi);
  any |= check_changed_set_(state_.door_state_lo, state_lo);
  any |= check_changed_set_(state_.logical_door_state, logical_state);

  any |= check_changed_set_(state_.reserved, rx_buffer_[17]);
  any |= check_changed_set_(state_.valid, true);

  if (any) {
    ESP_LOGD(
        TAG_UAPBRIDGE_HCP,
        "State changed: current=%u target=%u raw_state=0x%02X/0x%02X logical=%d lamp=%d relay=%d",
        state_.door_current_position,
        state_.door_target_position,
        state_.door_state_hi,
        state_.door_state_lo,
        static_cast<int>(state_.logical_door_state),
        state_.lamp_on,
        state_.relay_on
    );

    for (auto &cb : this->state_callbacks_) {
      cb();
    }

    state_.changed = false;
  }
}

void UAPBridge_hcp::send_response_() {
  const uint16_t crc = calculate_crc_(tx_buffer_, tx_len_ - 2);
  tx_buffer_[tx_len_ - 2] = crc & 0xFF;
  tx_buffer_[tx_len_ - 1] = crc >> 8;

  this->write_array(tx_buffer_, tx_len_);
  this->flush();
}

void UAPBridge_hcp::reset_rx_() {
  rx_len_ = 0;
  recv_time_us_ = 0;
}

uint16_t UAPBridge_hcp::calculate_crc_(const uint8_t *buffer, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= buffer[i];
    for (uint8_t j = 0; j < 8; j++) {
      const bool lsb = crc & 0x0001;
      crc >>= 1;
      if (lsb)
        crc ^= 0xA001;
    }
  }
  return crc;
}

uint16_t UAPBridge_hcp::read_u16_be_(const uint8_t *buffer, size_t index) {
  return (static_cast<uint16_t>(buffer[index]) << 8) | buffer[index + 1];
}

uint16_t UAPBridge_hcp::read_crc_(const uint8_t *buffer, size_t length) {
  return (static_cast<uint16_t>(buffer[length - 1]) << 8) | buffer[length - 2];
}

void UAPBridge_hcp::open_door() {
  if (state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = OPEN_DOOR;
  }
}

void UAPBridge_hcp::open_door_half() {
  if (state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = OPEN_DOOR_HALF;
  }
}

void UAPBridge_hcp::close_door() {
  if (state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = CLOSE_DOOR;
  }
}

void UAPBridge_hcp::stop_door() {
  const bool moving =
      state_.logical_door_state == STATE_OPENING ||
      state_.logical_door_state == STATE_CLOSING ||
      state_.logical_door_state == STATE_MOVE_HALF ||
      state_.logical_door_state == STATE_MOVE_VENTING;

  if (moving && state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = STOP_DOOR;
  }
}

void UAPBridge_hcp::toggle_lamp() {
  if (state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = TOGGLE_LAMP;
  }
}

void UAPBridge_hcp::ventilation_position() {
  if (state_machine_ == WAITING) {
    last_state_time_ms_ = millis();
    state_machine_ = VENTPOSITION;
  }
}

void UAPBridge_hcp::set_position(uint8_t position) {
  if (state_machine_ != WAITING)
    return;

  if (position <= 5) {
    close_door();
    return;
  }
  if (position >= 95) {
    open_door();
    return;
  }

  state_.goto_position = position * 2;
  last_state_time_ms_ = millis();

  if (state_.goto_position > state_.door_current_position)
    state_machine_ = SET_POSITION_OPEN;
  else if (state_.goto_position < state_.door_current_position)
    state_machine_ = SET_POSITION_CLOSE;
}
UAPBridge_hcp::DoorState UAPBridge_hcp::decode_door_state_(uint8_t high_byte, uint8_t low_byte) {
  switch (high_byte) {
    case 0x01:
      return STATE_OPENING;
    case 0x02:
      return STATE_CLOSING;
    case 0x20:
      return STATE_OPEN;
    case 0x40:
      return STATE_CLOSED;
    case 0x80:
      return STATE_HALFOPEN;
    case 0x09:
      return STATE_MOVE_VENTING;
    case 0x05:
      return STATE_MOVE_HALF;
    case 0x0A:
      return STATE_VENT;
    case 0x00:
      // high byte 0x00 + low byte 0x61 means VENT, otherwise STOPPED
      return (low_byte == 0x61) ? STATE_VENT : STATE_STOPPED;
    default:
      return STATE_UNKNOWN;
  }
}

}  // namespace uapbridge_hcp
}  // namespace esphome