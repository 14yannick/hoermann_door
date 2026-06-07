#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/uart.h"

#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

namespace esphome {
namespace uapbridge_hcp {

static const char *const TAG_UAPBRIDGE_HCP = "uapbridge_hcp";

class UAPBridge_hcp : public Component, public uart::UARTDevice {
 protected:
  enum StateMachine {
    WAITING,
    OPEN_DOOR,
    OPEN_DOOR_RELEASE,
    CLOSE_DOOR,
    CLOSE_DOOR_RELEASE,
    STOP_DOOR,
    STOP_DOOR_RELEASE,
    IMPULSE,
    SET_POSITION_OPEN,
    SET_POSITION_OPEN_RELEASE,
    SET_POSITION_OPEN_PROGRESS,
    SET_POSITION_CLOSE,
    SET_POSITION_CLOSE_RELEASE,
    SET_POSITION_CLOSE_PROGRESS,
    VENTPOSITION,
    VENTPOSITION_RELEASE,
    OPEN_DOOR_HALF,
    OPEN_DOOR_HALF_RELEASE,
    TOGGLE_LAMP,
    TOGGLE_LAMP_RELEASE,
  };

 public:
  enum DoorState {
    STATE_STOPPED,
    STATE_OPENING,
    STATE_CLOSING,
    STATE_OPEN,
    STATE_CLOSED,
    STATE_HALFOPEN,
    STATE_MOVE_VENTING,
    STATE_MOVE_HALF,
    STATE_VENT,
    STATE_UNKNOWN
  };

  void setup() override;
  void loop() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::BUS; }

  void open_door();
  void close_door();
  void stop_door();
  void open_door_half();
  void ventilation_position();
  void toggle_lamp();
  void set_position(uint8_t position);
  void impulse_door();

  void set_high_frequency_loop(bool enabled) { high_freq_loop_ = enabled; }

  void add_on_state_callback(std::function<void()> &&callback);
  bool is_valid() const;
  float get_current_position() const;
  DoorState get_logical_door_state() const;
  bool get_light_state() const;
  bool get_relay_state() const;

 protected:
  static constexpr uint8_t DEVICE_ID = 0x02;
  static constexpr uint8_t BROADCAST_ID = 0x00;
  static constexpr uint32_t BAUD_RATE = 57600;
  static constexpr uint32_t BITS_PER_CHAR = 11;  // 8E1

  // Replace the calculated constant with the same hardcoded value as hciemulator
  // static constexpr uint32_t T3_5_US = (3500000UL * BITS_PER_CHAR) / BAUD_RATE;
  static constexpr uint32_t T3_5_US = 4800;
  static constexpr size_t RX_BUFFER_SIZE = 256;
  static constexpr size_t TX_BUFFER_SIZE = 256;
  static constexpr uint16_t SIMULATE_KEYPRESS_DELAY_MS = 200;

  struct HCPState {
    bool valid{false};
    bool lamp_on{false};
    bool relay_on{false};
    uint8_t door_current_position{0};  // 0..200
    uint8_t door_target_position{0};   // 0..200
    uint8_t door_state_hi{0};
    uint8_t door_state_lo{0};
    uint8_t reserved{0};
    uint8_t goto_position{0};          // 0..200
    bool changed{false};
    DoorState logical_door_state{STATE_UNKNOWN};
  };

  void process_incoming_();
  void process_frame_();
  void process_device_status_frame_();
  void process_device_bus_scan_frame_();
  void process_broadcast_status_frame_();
  void send_response_();
  void reset_rx_();

  static uint16_t calculate_crc_(const uint8_t *buffer, size_t length);
  static uint16_t read_u16_be_(const uint8_t *buffer, size_t index);
  static uint16_t read_crc_(const uint8_t *buffer, size_t length);
  static DoorState decode_door_state_(uint8_t high_byte, uint8_t low_byte);

  template<typename T>
  bool check_changed_set_(T &target, const T &value) {
    if (target != value) {
      target = value;
      this->state_.changed = true;
      return true;
    }
    return false;
  }

  std::vector<std::function<void()>> state_callbacks_;
  HCPState state_;
  StateMachine state_machine_{WAITING};

  uint8_t rx_buffer_[RX_BUFFER_SIZE]{0};
  uint8_t tx_buffer_[TX_BUFFER_SIZE]{0};
  size_t rx_len_{0};
  size_t tx_len_{0};

  uint32_t recv_time_us_{0};
  uint32_t last_state_time_ms_{0};
  bool skip_frame_{false};
  bool high_freq_loop_{false};
  HighFrequencyLoopRequester high_freq_requester_;
};

}  // namespace uapbridge_hcp
}  // namespace esphome