#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
  namespace ld1125h {

    class LD1125hComponent : public Component, public uart::UARTDevice {
    public:
      float get_setup_priority() const override { return setup_priority::LATE; };
      void setup() override;
      void loop() override;
      void dump_config() override;
      void motion_timeout();
      void send_next_config();
      void manage_config_tx();

      // config setters
      void set_motion_timeout(int timeout) { this->motion_timeout_ = timeout; };
      void set_log_sensor_output(bool log) {
        this->log_sensor_output_ = log;
        this->rx_message_ = new std::vector<uint8_t>();
      };

      // Setters for sensor variables
      void set_rmax(float rmax) { this->rmax_ = rmax; };
      void set_mth1_mov(int mth1) { this->mth1_mov_ = mth1; };
      void set_mth2_mov(int mth2) { this->mth2_mov_ = mth2; };
      void set_mth3_mov(int mth3) { this->mth3_mov_ = mth3; };
      void set_mth1_occ(int mth1) { this->mth1_occ_ = mth1; };
      void set_mth2_occ(int mth2) { this->mth2_occ_ = mth2; };
      void set_mth3_occ(int mth3) { this->mth3_occ_ = mth3; };
      void set_ts_on(int ts) { this->ts_on_ = ts; };
      void set_ts_off(int ts) { this->ts_off_ = ts; };
      void set_output_mode(int mode) { this->output_mode_ = mode; };
      void set_test_mode(int mode) { this->test_mode_ = mode; };
      void set_log_version(bool send) { this->log_version_ = send; };
      void set_log_get_all(bool send) { this->log_get_all_ = send; };

      // sub-sensor setters
      void set_distance_sensor(sensor::Sensor *sensor) { this->distance_sensor_ = sensor; };
      void set_motion_sensor(binary_sensor::BinarySensor *sensor) { this->motion_sensor_ = sensor; };
      void set_movement_sensor(binary_sensor::BinarySensor *sensor) { this->movement_sensor_ = sensor; };
      void set_occupancy_sensor(binary_sensor::BinarySensor *sensor) { this->occupancy_sensor_ = sensor; };

    protected:
      void handle_char_(uint8_t c);
      std::vector<uint8_t> *rx_message_{ nullptr };

      // conf variables
      bool log_sensor_output_ = false;
      int motion_timeout_;

      // LD1125h sensor variables
      float rmax_;
      int mth1_mov_;
      int mth2_mov_;
      int mth3_mov_;
      int mth1_occ_;
      int mth2_occ_;
      int mth3_occ_;
      int ts_on_;
      int ts_off_;
      int output_mode_;
      int test_mode_;
      bool log_version_;
      bool log_get_all_;

      // variables used to pass configs to the sensor
      char config_strings_[11][20];
      int config_position_ = 0;
      int max_config_position_ = 0;
      bool is_ready_to_send_next_config_ = true;
      bool all_configs_are_sent_ = false;

      // internal sensors
      sensor::Sensor *distance_sensor_{ nullptr };
      binary_sensor::BinarySensor *motion_sensor_{ nullptr };
      binary_sensor::BinarySensor *movement_sensor_{ nullptr };
      binary_sensor::BinarySensor *occupancy_sensor_{ nullptr };

      // variables supporting parsing of input stream
      std::array<char, 3> motion_type_;
      std::array<char, 6> center_validation_;
      std::array<char, 3> expected_movement_text_ = { 'm', 'o', 'v' };
      std::array<char, 3> expected_occupancy_text_ = { 'o', 'c', 'c' };
      std::array<char, 3> expected_config_received_text_ = { 'r', 'e', 'c' };
      std::array<char, 6> expected_center_text_ = { ',', ' ', 'd', 'i', 's', '=' };

      int input_position_ = 0;
      bool is_data_a_motion_event_ = true;

      // variables tracking sensor states before sending
      bool is_movement_ = false;
      bool is_occupancy_ = false;
      float distance_;

      // variables supporting motion timeout
      uint32_t last_motion_time_ = 0;
    };
  } // namespace ld1125hs
} // namespace esphome