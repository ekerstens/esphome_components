# Hi-Link LD1125h mmWave Radar
Based on an LD1125h sensor with version RKB1125H BW1800M 20230412 V4.1

# Configuration Options

```yaml
# Example configuration entry
sensor:
    platform: ld1125h
    uart_id: LD1115H_UART_BUS
    id: LD1115H_UART_Text
    distance:
      name: Distance
      filters:
        - timeout: 1s
        - throttle_average: 1s
    occupancy: 
      name: Occupancy
    movement:
      name: Movement
    motion: 
      name: Motion
    motion_timeout: 1s
    log_sensor_output: true
    log_version: true 
    log_get_all: true 
    rmax: 6.00
    mth1_mov: 80
    mth2_mov: 50
    mth3_mov: 20
    mth1_occ: 60
    mth2_occ: 55
    mth3_occ: 20
    output_mode: 0
    test_mode: 0

# Corresponding uart:
  id: LD1125H_UART_BUS
  tx_pin: GPIO17  
  rx_pin: GPIO18 
  baud_rate: 115200
  data_bits: 8
  stop_bits: 1
  parity: NONE
  setup_priority: 200 # Set Priority To Prevent Boot Loop or Fail
```

## Configuration Variables
### Sensors
- **distance**: If set, creates a sensor to track the distance reported by the sensor. It is suggested to use `timeout`, as otherwise the distance will persist even when nothing is updated. It is also suggested to use `throttle_average` or a similar filter to avoid sending distance outputs too frequently.
- **occupancy**: Configures a binary sensor to show when the sensor detects occupancy.
- **movement**: Configures a binary sensor to show when the sensor detects movement.
- **motion**: Configures a binary sensor to show when the sensor detections motion. This triggers so long as either movement or occupancy is detected.

### Options
- **motion_timeout**: Time without any serial data after which to reset all motion sensors. The sensor uses serial data to indicate when new motion is detected, but doesn't explicitely indicate when nothing is detected.
- **log_sensor_output**: If set, all serial data produced by the sensor is logged. Useful for troubleshooting.
- **log_version**: If set, "VER" is sent to the sensor. The sensor will return the version of the firmware. Should use with `log_sensor_output` to view the sensor response.
- **log_get_all**: If set, "get_all" is sent to the sensor. The sensor will return all configuration values. Should use with `log_sensor_output` to view the sensor response.

All other options match configuration options for the sensor and are transmitted over UART at startup. If unset, options are reset to default at boot.
- **rmax**:		 Maxiumum distance to detect movement (in meters)
- **mth1_mov**:	 Movement minimum signal at distances < 2.8m
- **mth2_mov**:	 Movement minimum signal at distances 2.8m-8m
- **mth3_mov**:	 Movement minimum signal at distances > 8m
- **mth1_occ**:	 Occupancy minimum signal at distances < 2.8m
- **mth2_occ**:	 Occupancy minimum signal at distances 2.8m-8m
- **mth3_occ**:	 Occupancy minimum signal at distances > 8m
- **ts_on**:		 ??? Might be seconds of detected motion/occupancy to trigger GPIO output below
- **ts_off**:		 ??? Might be seconds of no detected motion/occupancy to clear GPIO output below
- **output_mode**:	 Set the sensor to either display output via serial (default 0), or via GPIO port on second header (1)
- **test_mode**:	 Set to 1 to output the signal strength (Default 0). Example: occ, dis=3.62, str=61.93

Regarding `mth` settings for sensor sensitivity, per the sensor datasheet:
> The larger the sensitivity value, the less sensitive the module is. The main reason for segment sensitivity is that the reflected signal of the short-range target will be relatively stronger, so the same sensitivity as the long-distance target is not required at short distances. At the same time, according to different environments, you can also make targeted settings. The segment sensitivity can be debuggedwiththe testm

# Wiring
## ESP32-S3:
- LD1125H URX (RX) <----> ESP32 GPIO17 (TX)
- LD1125H UTX (TX) <----> ESP32 GPIO18 (RX)
- LD1125H GND <----> ESP32 GND
- LD1125H Vcc <----> 3.3V Source

## Other
For other chips, check the Espressif documentation to find the correct TX and RX pins.

# Inspiration and Reference Material
- [LD1125h Reference](https://github.com/patrick3399/Hi-Link_mmWave_Radar_ESPHome/tree/main/LD1125H)
- [UART Example](https://github.com/ssieb/esphome_components/tree/master/components/serial)
- [Reddit post discussing a sensor with the same version](https://www.reddit.com/r/esp32/comments/16rfsa0/ld1125h_mmwave_sensor_detects_3d_printer_movement/)
- [Why did I pick the LD1125H mmWave Sensor - Full Segment](https://www.youtube.com/watch?v=9j5Yy5M8YOs)