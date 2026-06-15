# Hörmann HCP Protocol — Technical Specification

The Hörmann Control Protocol (HCP) is used by E4-generation garage door openers (UAP HCP, Promatic 4, RotaMatic P2, etc.) to communicate with keypads and other accessories over RS485. Electrically it is compatible with the 6-pin RJ12 connector found on the motor unit.

---

## Physical Layer

| Parameter | Value |
|-----------|-------|
| Interface | RS485, half-duplex |
| Connector | RJ12 (6P6C) |
| Baud rate | 57 600 baud |
| Frame format | **8E1** — 8 data bits, Even parity, 1 stop bit |
| Bits per character | 11 (start + 8 data + parity + stop) |
| Protocol framing | Modbus RTU |

---

## Device Addressing

| Role | Address |
|------|---------|
| Garage door motor (master) | `0x02` — only addressed device on the bus |
| Broadcast (status frames from motor) | `0x00` |

The bridge acts as a Modbus slave at address `0x02`. All unicast frames from the motor carry device ID `0x02`; periodic status broadcasts use ID `0x00`.

---

## Timing

| Parameter | Value | Notes |
|-----------|-------|-------|
| T3.5 inter-frame silence (spec) | 1 750 µs | Modbus RTU spec at >19 200 baud |
| T3.5 inter-frame silence (ESPHome) | **4 800 µs** | Empirically determined by hkiam/HCPBridge (`hciemulator.h`). The spec value of 1 750 µs works with a dedicated high-priority FreeRTOS busy-wait task; 4 800 µs is required for ESPHome's cooperative main-loop scheduler to avoid missing the inter-frame gap |
| Simulated keypress duration | 200 ms | Time between "button press" and "button release" frames |
| State-machine watchdog | 2 000 ms | Resets stuck command states if no new device-status frame is received |

---

## CRC

Standard **Modbus RTU CRC-16**:

- Polynomial: `0xA001` (bit-reversed `0x8005`)
- Initial value: `0xFFFF`
- Appended **little-endian** — low byte first, high byte second

```
crc = 0xFFFF
for each byte b:
    crc ^= b
    for 8 bits:
        if crc & 1: crc = (crc >> 1) ^ 0xA001
        else:       crc >>= 1
```

---

## Register Map

| Address | Name | Direction | Size | Description |
|---------|------|-----------|------|-------------|
| `0x9D31` | Broadcast status | Motor → Bridge | 9 registers (18 bytes) | Full door status (broadcast, FC 0x10) |
| `0x9C41` | Command register | Motor → Bridge (write) | 2–3 registers | Motor writes counter + command code; bridge fills its response |
| `0x9CB9` | Internal state | Bridge → Motor (read) | 2–8 registers | Bridge returns its own button/command state |

---

## Frame Types

Three frame types are observed on the bus. Frames are identified by device ID, function code, and total length.

### 1. Broadcast Status Frame

Sent periodically by the motor to all devices.

```
[0]  Unit ID = 0x00 (broadcast)
[1]  FC = 0x10 (Write Multiple Registers)
[2]  Start Register Hi = 0x9D
[3]  Start Register Lo = 0x31
[4]  Quantity Hi = 0x00
[5]  Quantity Lo = 0x09  (9 registers)
[6]  Byte count  = 0x12  (18 bytes)
[7]  Reg 0x9D31+0 Hi  — counter / timestamp
[8]  Reg 0x9D31+0 Lo
[9]  Reg 0x9D31+1 Hi  — target position  (0–200)
[10] Reg 0x9D31+1 Lo  — current position (0–200)
[11] Reg 0x9D31+2 Hi  — state_hi  ← door state code
[12] Reg 0x9D31+2 Lo  — state_lo  ← secondary state flag
[13] Reg 0x9D31+3 Hi
[14] Reg 0x9D31+3 Lo
[15] Reg 0x9D31+4 Hi
[16] Reg 0x9D31+4 Lo
[17] Reg 0x9D31+5 Hi  — reserved
[18] Reg 0x9D31+5 Lo
[19] Reg 0x9D31+6 Hi  — reg7_hi  ← relay indicator
[20] Reg 0x9D31+6 Lo  — reg7_lo  ← lamp + relay flags
[21] Reg 0x9D31+7 Hi
[22] Reg 0x9D31+7 Lo
[23] Reg 0x9D31+8 Hi
[24] Reg 0x9D31+8 Lo
[25] CRC Lo
[26] CRC Hi
     Total: 27 bytes (0x1B)
```

No response is sent to broadcast frames.

---

### 2. Device Status Frame (standard poll)

Unicast query from the motor requesting 8 registers.

```
[0]  Unit ID = 0x02
[1]  FC = 0x17 (Read/Write Multiple Registers)
[2]  Read Address Hi  = 0x9C
[3]  Read Address Lo  = 0xB9  → read from 0x9CB9
[4]  Read Count Hi    = 0x00
[5]  Read Count Lo    = 0x08  (8 registers to read)
[6]  Write Address Hi = 0x9C
[7]  Write Address Lo = 0x41  → write to 0x9C41
[8]  Write Count Hi   = 0x00
[9]  Write Count Lo   = 0x02  (2 registers to write)
[10] Byte count       = 0x04
[11] Write Reg 0 Hi   — counter (echoed in response)
[12] Write Reg 0 Lo   — command  (echoed in response)
[13] Write Reg 1 Hi
[14] Write Reg 1 Lo
[15] CRC Lo
[16] CRC Hi
     Total: 17 bytes (0x11)
```

#### Response (21 bytes)

```
[0]  Unit ID = 0x02
[1]  FC = 0x17
[2]  Byte count = 0x10 (16 bytes)
[3]  counter  ← copied from request [11]
[4]  0x00
[5]  cmd      ← copied from request [12]
[6]  0x01
[7]  cmd_reg2_hi  ← active command byte (see Command Encoding)
[8]  cmd_reg2_lo
[9]  cmd_reg3_hi  ← secondary command byte (vent / half)
[10] cmd_reg3_lo
[11..18]  0x00 (padding)
[19] CRC Lo
[20] CRC Hi
     Total: 21 bytes
```

Default idle response template (before CRC fill):
```
02 17 10 3E 00 03 01 00 00 00 00 00 00 00 00 00 00 00 00 xx xx
```

---

### 3. Device Status Frame (short poll)

Same structure as the standard poll but requests only 2 registers (`Read Count Lo = 0x02`).

Response (9 bytes):
```
02 17 04 0F 00 04 FD 0A xx
                         └─ CRC (1 byte shown, actually 2 bytes total: [7..8])
```

Full template: `02 17 04 0F 00 04 FD 0A [CRC_Lo] [CRC_Hi]`

---

### 4. Bus Scan Frame

Sent once at startup to enumerate devices.

```
[0]  Unit ID = 0x02
[1]  FC = 0x17
[2..3]   Read Address 0x9CB9
[4..5]   Read Count  = 0x05  (5 registers)
[6..7]   Write Address 0x9C41
[8..9]   Write Count = 0x03  (3 registers)
[10]     Byte count  = 0x06
[11..16] Write data
[17] CRC Lo
[18] CRC Hi
     Total: 19 bytes (0x13)
```

Response (15 bytes) — bytes `[3]` and `[5]` are variable (same counter/cmd echo as the standard poll):

```
[0]  = 0x02         (unit ID, from request)
[1]  = 0x17
[2]  = 0x0A         (10 data bytes = 5 registers)
[3]  = counter      (rx[11] from request, high byte of 0x9CB9+0)
[4]  = 0x00         (low byte of 0x9CB9+0)
[5]  = cmd          (rx[12] from request, high byte of 0x9CB9+1)
[6]  = 0x05         (low byte of 0x9CB9+1)
[7]  = 0x04  \
[8]  = 0x30  / 0x9CB9+2 = 0x0430 (firmware version / device type)
[9]  = 0x10  \
[10] = 0xFF  / 0x9CB9+3 = 0x10FF
[11] = 0xA8  \
[12] = 0x45  / 0x9CB9+4 = 0xA845
[13] CRC Lo
[14] CRC Hi
```

---

## Door State Encoding

Carried in **Reg 0x9D31+2** of the broadcast frame (bytes `[11]` = state_hi, `[12]` = state_lo).

| state_hi | state_lo | Internal state | Description |
|----------|----------|----------------|-------------|
| `0x01` | any | `OPENING` | Door is opening |
| `0x02` | any | `CLOSING` | Door is closing |
| `0x20` | any | `OPEN` | Door fully open |
| `0x40` | any | `CLOSED` | Door fully closed |
| `0x80` | any | `HALFOPEN` | Door stopped at half position |
| `0x09` | any | `MOVE_VENTING` | Moving towards vent (ventilation) position |
| `0x05` | any | `MOVE_HALF` | Moving towards half position |
| `0x0A` | any | `VENT` | At vent (ventilation) position |
| `0x00` | `0x61` | `VENT` | At vent position (alternate encoding) |
| `0x00` | other | `STOPPED` | Door stopped mid-travel |
| other | any | `UNKNOWN` | Unrecognised state |

---

## Position Encoding

Position values in **Reg 0x9D31+1** (bytes `[9]` and `[10]`):

| Wire value | Meaning |
|-----------|---------|
| `0` | Fully closed |
| `200` | Fully open |
| `1–199` | Intermediate (linear) |

Conversion to ESPHome cover position (0.0–1.0):

```
cover_position = wire_value / 200.0
```

For the set-position command, the target is stored internally on the same 0–200 scale.  
ESPHome cover positions ≤ 5 % are mapped to full close; ≥ 95 % to full open.

---

## Relay and Light State Encoding

Carried in **Reg 0x9D31+6** of the broadcast frame (byte `[19]` = reg7_hi, byte `[20]` = reg7_lo).

| reg7_hi | reg7_lo | Light | Relay |
|---------|---------|-------|-------|
| `0x00` | `0x00` | Off | Off |
| `0x02` | `0x00` | Off | On |
| `0x02` | `0x10` | On | On |
| `0x00` | `0x10` | On | Off |
| `0x00` | `0x14` | On | On |
| `0x00` | `0x04` | Off | On |

Decoding rules:

```
lamp  = (reg7_lo == 0x10) || (reg7_lo == 0x14)
relay = (reg7_hi == 0x02) || (reg7_lo == 0x14) || (reg7_lo == 0x04)
```

`relay` reflects motor menu setting 30 (output relay function). Reported values `0x04` and `0x14` for reg7_lo are field-observed; value `0x14` for relay on+light on is assumed/unconfirmed on UAP HCP hardware.

---

## Command Encoding

Commands are simulated as a two-phase keypress sent in response to Device Status Frames.

Response bytes `[7..10]` carry the active command. The motor samples them on each poll.

| Command | Phase | `[7]` | `[8]` | `[9]` | `[10]` |
|---------|-------|--------|--------|--------|---------|
| **Open** | Press | `0x02` | `0x10` | `0x00` | `0x00` |
| | Release | `0x01` | `0x10` | `0x00` | `0x00` |
| **Close** | Press | `0x02` | `0x20` | `0x00` | `0x00` |
| | Release | `0x01` | `0x20` | `0x00` | `0x00` |
| **Stop / Impulse** | Press | `0x02` | `0x40` | `0x00` | `0x00` |
| | Release | `0x01` | `0x40` | `0x00` | `0x00` |
| **Vent position** | Press | `0x02` | `0x00` | `0x40` | `0x00` |
| | Release | `0x01` | `0x00` | `0x40` | `0x00` |
| **Half position** | Press | `0x02` | `0x00` | `0x04` | `0x00` |
| | Release | `0x01` | `0x00` | `0x04` | `0x00` |
| **Toggle lamp** | Press | `0x01` | `0x00` | `0x02` | `0x00` |
| | Release | `0x08` | `0x00` | `0x02` | `0x00` |
| **Idle (no command)** | — | `0x00` | `0x00` | `0x00` | `0x00` |

The keypress simulation works as follows:
1. On the first Device Status Frame after a command is queued, send the **Press** values.
2. After `SIMULATE_KEYPRESS_DELAY_MS` (200 ms), send the **Release** values.
3. Return to idle (`WAITING`).

Stop is only issued if the door is currently in a moving state (`OPENING`, `CLOSING`, `MOVE_HALF`, `MOVE_VENTING`).

---

## State Machine

The bridge uses a software state machine to serialise commands:

```
WAITING
  └─ action triggered
       ├─ OPEN_DOOR → OPEN_DOOR_RELEASE → WAITING
       ├─ CLOSE_DOOR → CLOSE_DOOR_RELEASE → WAITING
       ├─ STOP_DOOR (only if moving) → STOP_DOOR_RELEASE → WAITING
       ├─ IMPULSE → STOP_DOOR_RELEASE → WAITING
       ├─ VENTPOSITION → VENTPOSITION_RELEASE → WAITING
       ├─ OPEN_DOOR_HALF → OPEN_DOOR_HALF_RELEASE → WAITING
       ├─ TOGGLE_LAMP → TOGGLE_LAMP_RELEASE → WAITING
       └─ SET_POSITION_OPEN/CLOSE
            └─ _RELEASE → _PROGRESS (monitor position)
                 └─ target reached → STOP_DOOR → STOP_DOOR_RELEASE → WAITING
```

A **2-second watchdog** resets the state machine to `WAITING` if it remains in a non-progress command state without receiving a new Device Status Frame, preventing a stuck state from blocking the bus.

---

## Sources

- Protocol reverse-engineering: [dupas.be — Hörmann UAP HCP](https://blog.dupas.be/posts/hoermann-uap-hcp1/)
- Original direct UART implementation (HCI emulator, T3.5 timing, command encoding): [hkiam/HCPBridge — hciemulator.h](https://github.com/hkiam/HCPBridge/blob/master/HCPBridgeESP32/src/hciemulator.h)
- Modbus RTU library implementation with MQTT: [Gifford47/HCPBridgeMqtt](https://github.com/Gifford47/HCPBridgeMqtt)
- Initial ESPHome port (external Modbus library): [mapero/esphome-hcpbridge](https://github.com/mapero/esphome-hcpbridge)
- Direct UART implementation (no external dependency), protocol documentation, and integration with the E3 protocol under the unified `uapbridge` component family: this repository
