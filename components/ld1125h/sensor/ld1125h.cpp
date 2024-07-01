#include "ld1125h.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace ld1125h {

    static const char *TAG = "ld1125h.text_sensor";

    int char_to_int(uint8_t *c) {
      return *c - '0';
    }

    bool is_valid_int_char(int *i) {
      return (*i >= 0 && *i <= 9);
    }

    void LD1125hComponent::setup() {
      // Create config input
      sprintf(this->config_strings_[this->max_config_position_++], "rmax=%0.2f\r\n", this->rmax_);
      sprintf(this->config_strings_[this->max_config_position_++], "mth1_mov=%i\r\n", this->mth1_mov_);
      sprintf(this->config_strings_[this->max_config_position_++], "mth2_mov=%i\r\n", this->mth2_mov_);
      sprintf(this->config_strings_[this->max_config_position_++], "mth3_mov=%i\r\n", this->mth3_mov_);
      sprintf(this->config_strings_[this->max_config_position_++], "mth1_occ=%i\r\n", this->mth1_occ_);
      sprintf(this->config_strings_[this->max_config_position_++], "mth2_occ=%i\r\n", this->mth2_occ_);
      sprintf(this->config_strings_[this->max_config_position_++], "mth3_occ=%i\r\n", this->mth3_occ_);
      sprintf(this->config_strings_[this->max_config_position_++], "ts_on=%i\r\n", this->ts_on_);
      sprintf(this->config_strings_[this->max_config_position_++], "ts_off=%i\r\n", this->ts_off_);
      sprintf(this->config_strings_[this->max_config_position_++], "output_mode=%i\r\n", this->output_mode_);
      sprintf(this->config_strings_[this->max_config_position_++], "test_mode=%i\r\n", this->test_mode_);
    }

    void LD1125hComponent::loop() {
      this->manage_config_tx();
      this->motion_timeout();

      // Consume UART
      while (this->available()) {
        uint8_t c;
        this->read_byte(&c);
        this->handle_char_(c);
      }
    }

    /* Check if the time since the last motion detection was more than timeout millis ago.*/
    void LD1125hComponent::motion_timeout() {
      if (
        this->last_motion_time_ > 0 && millis() - this->last_motion_time_ >= this->motion_timeout_) {
        if (this->motion_sensor_ != nullptr) {
          this->motion_sensor_->publish_state(false);
        }
        if (this->movement_sensor_ != nullptr) {
          this->movement_sensor_->publish_state(false);
        }
        if (this->occupancy_sensor_ != nullptr) {
          this->occupancy_sensor_->publish_state(false);
        }
      }
    }

    // Config the sensor. This happens in the loop because we need to process the
    // sensor outputs to properly sequence the configuration commands.
    void ld1125h::LD1125hComponent::manage_config_tx() {
      bool is_logger_ready = millis() > 15000; // TODO Replace with a check if the logger is running
      if (this->is_ready_to_send_next_config_) {
        if (!this->all_configs_are_sent_) {
          this->send_next_config();
        }
        else if (this->log_version_ && is_logger_ready) {
          this->write_str("VER\r\n");
          this->log_version_ = false;
          this->is_ready_to_send_next_config_ = false;
        }
        else if (this->log_get_all_ && is_logger_ready) {
          this->write_str("get_all\r\n");
          this->log_get_all_ = false;
          this->is_ready_to_send_next_config_ = false;
        }
      }
    }

    // The sensor discards configs if they come too quickly. While in some cases
    // the sensor still processes a command before outputing the "received message" of the
    // last command, but only when they are very close. Even still, it could be that the
    // sensor did send the "received message" but it wasn't yet picked up. As such, need
    // too check for a "received message" before sending the next command.
    void ld1125h::LD1125hComponent::send_next_config() {
      if (!this->is_ready_to_send_next_config_) {
        return;
      }
      if (this->config_position_ >= this->max_config_position_) {
        this->all_configs_are_sent_ = true;
        return;
      }
      this->write_str(this->config_strings_[this->config_position_++]);
      this->is_ready_to_send_next_config_ = false;
    }

    // read data is like the following:
    // mov, dis=1.23
    // occ, dis=4.39
    // mov, dis=0.98
    void LD1125hComponent::handle_char_(uint8_t c) {
      // handle signal termination
      if (this->log_sensor_output_) {
        rx_message_->push_back(c);
      }

      // Process end of message
      if (c == '\n') {
        if (this->is_data_a_motion_event_ && this->all_configs_are_sent_) {
          // Publish all requested values, but only if the sensor is fully configured
          this->last_motion_time_ = millis(); // any valid input means that there was motion
          if (this->motion_sensor_ != nullptr) {
            this->motion_sensor_->publish_state((this->is_movement_ || this->is_occupancy_));
          }
          if (this->movement_sensor_ != nullptr) {
            this->movement_sensor_->publish_state(this->is_movement_);
          }
          if (this->occupancy_sensor_ != nullptr) {
            this->occupancy_sensor_->publish_state(this->is_occupancy_);
          }
          if (this->distance_sensor_ != nullptr) {
            this->distance_sensor_->publish_state(this->distance_);
          }
        }

        // DEBUG INPUT STREAM
        if (this->log_sensor_output_) {
          std::string s(this->rx_message_->begin(), this->rx_message_->end());
          this->rx_message_->clear();
          ESP_LOGD("ld1125h", s.c_str());
        }

        // reset values for execution
        this->is_data_a_motion_event_ = true;
        this->is_occupancy_ = false;
        this->is_movement_ = false;
        this->input_position_ = 0;

        return; // return to avoid updating any reset variables.
      }

      // handle data stream
      else if (!this->is_data_a_motion_event_) {
        return;
      } // stop processing due to junk input
      else if (c == '\r') {
        return;
      } // comes before the new line and can be ignored

      // validate and process motion type
      else if (this->input_position_ <= 2) {
        this->motion_type_[this->input_position_] = c;
        if (this->input_position_ == 2) {
          // Check the type of movement
          if (this->motion_type_ == this->expected_movement_text_) {
            this->is_movement_ = true;
          }
          else if (this->motion_type_ == this->expected_occupancy_text_) {
            this->is_occupancy_ = true;
          }
          else {
            this->is_data_a_motion_event_ = false; // motion type is "occ" or "mov"
          }
          if (this->motion_type_ == this->expected_config_received_text_) {
            this->is_ready_to_send_next_config_ = true; // message will be "received message:___"
          }
        }
      }
      // validate filler text
      else if (this->input_position_ > 2 && this->input_position_ <= 8) {
        this->center_validation_[this->input_position_ - 3] = c;
        if (this->input_position_ == 8 && !(this->center_validation_ == this->expected_center_text_)) {
          this->is_data_a_motion_event_ = true; // middle string should be ", dis="
          // ESP_LOGD("ld1125h", "Invalid filter text. Got digit %c. Center data is not valid.", c);
        }
      }
      // calculate_this->distance_
      else if (this->input_position_ == 9 || this->input_position_ == 11 || this->input_position_ == 12) {
        int val = char_to_int(&c);
        if (is_valid_int_char(&val)) {
          if (this->input_position_ == 9) {
            this->distance_ = val;
          }
          else if (this->input_position_ == 11) {
            this->distance_ += val / 10.0f;
          }
          else {
            this->distance_ += val / 100.0f;
          }
        }
        else {
          this->is_data_a_motion_event_ = false; // incoming data should be a digit
          // ESP_LOGD("ld1125h", "Invalid distance. Got digit %c at position %i. Expected a digit.", c, this->input_position_);
        }
      }
      // validate decimal
      else if (this->input_position_ == 10 && c != '.') {
        this->is_data_a_motion_event_ = false; // distance should have form d.dd
        // ESP_LOGD("ld1125h", "Invalid distance. Got digit %c at position %i. Expected a '.'.", c, this->input_position_);
      }
      // data is too long
      else if (this->input_position_ >= 13) {
        // ESP_LOGD("ld1125h", "Data is too long. Got digit %c at position %i.", c, this->input_position_);
        this->is_data_a_motion_event_ = false; // data should only be 14 characters long
      }
      this->input_position_++;
      return;
    }

    void LD1125hComponent::dump_config() {
      ESP_LOGCONFIG(TAG, "LD1125h:");
      LOG_BINARY_SENSOR("  ", "Motion Sensor", this->motion_sensor_);
      LOG_BINARY_SENSOR("  ", "Movement Sensor", this->movement_sensor_);
      LOG_BINARY_SENSOR("  ", "Occupancy Sensor", this->occupancy_sensor_);
      LOG_SENSOR("  ", "Distance Sensor", this->distance_sensor_);
    }

  } // namespace ld1125h
} // namespace esphome