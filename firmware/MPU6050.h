#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"


static const char *const TAG = "mpu6050";

const uint8_t MPU6050_REGISTER_WHO_AM_I = 0x75;
const uint8_t MPU6050_REGISTER_POWER_MANAGEMENT_1 = 0x6B;
const uint8_t MPU6050_REGISTER_INTERRUPTS= 0x38;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG = 0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG = 0x1C;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H = 0x3B;
const uint8_t MPU6050_CLOCK_SOURCE_X_GYRO = 0b001;
const uint8_t MPU6050_SCALE_2000_DPS = 0b11;
const float MPU6050_SCALE_DPS_PER_DIGIT_2000 = 0.060975f;
const uint8_t MPU6050_RANGE_2G = 0b00;
const float MPU6050_RANGE_PER_DIGIT_2G = 0.000061f;
const uint8_t MPU6050_BIT_SLEEP_ENABLED = 6;
const uint8_t MPU6050_BIT_TEMPERATURE_DISABLED = 3;
const float GRAVITY_EARTH = 9.80665f;

class MPU6050Component : public PollingComponent, public i2c::I2CDevice {
public:
  MPU6050Component() : PollingComponent(1000), I2CDevice() {};
  
  void setup() override {
    this->set_i2c_address( 0x68 );

    ESP_LOGCONFIG(TAG, "Setting up MPU6050...");
    uint8_t who_am_i;
    if (!this->read_byte(MPU6050_REGISTER_WHO_AM_I, &who_am_i) || who_am_i != 0x68) {
      ESP_LOGD(TAG, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(who_am_i));
      ESP_LOGV(TAG, "WHO AM I:" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(who_am_i));
      this->mark_failed();
      return;
    }

    ESP_LOGV(TAG, "  Setting up Power Management...");
    // Setup power management
    uint8_t power_management;
    if (!this->read_byte(MPU6050_REGISTER_POWER_MANAGEMENT_1, &power_management)) {
      this->mark_failed();
      return;
    }
    ESP_LOGV(TAG, "  Input power_management: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(power_management));
    power_management = 0;
    ESP_LOGV(TAG, "  Output power_management: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(power_management));
    if (!this->write_byte(MPU6050_REGISTER_POWER_MANAGEMENT_1, power_management)) {
      this->mark_failed();
      return;
    }
    
    
    ESP_LOGV(TAG, "  Setting up Gyro Config...");
    // Set scale - 2000DPS
    uint8_t gyro_config;
    if (!this->read_byte(MPU6050_REGISTER_GYRO_CONFIG, &gyro_config)) {
      this->mark_failed();
      return;
    }
    ESP_LOGV(TAG, "  Input gyro_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gyro_config));
    gyro_config &= 0b11100111;
    gyro_config |= MPU6050_SCALE_2000_DPS << 3;
    ESP_LOGV(TAG, "  Output gyro_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(gyro_config));
    if (!this->write_byte(MPU6050_REGISTER_GYRO_CONFIG, gyro_config)) {
      this->mark_failed();
      return;
    }

    ESP_LOGV(TAG, "  Setting up Accel Config...");
    // Set range - 2G
    uint8_t accel_config;
    if (!this->read_byte(MPU6050_REGISTER_ACCEL_CONFIG, &accel_config)) {
      this->mark_failed();
      return;
    }
    ESP_LOGV(TAG, "    Input accel_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(accel_config));
    accel_config &= 0b11100111;
    accel_config |= (MPU6050_RANGE_2G << 3);
    ESP_LOGV(TAG, "    Output accel_config: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(accel_config));
    if (!this->write_byte(MPU6050_REGISTER_GYRO_CONFIG, gyro_config)) {
      this->mark_failed();
      return;
    }

    /*
      https://www.eluke.nl/2016/08/11/how-to-enable-motion-detection-interrupt-on-mpu6050/


    (optionally?) Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
    write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
    Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
    Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
    To register 0x20 (hex), write the desired motion duration, for example 40ms.
    to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
    write register 0x38, bit 6 (0x40), to enable motion detection interrupt.

     */
    ESP_LOGV(TAG, "  Setting up Motion Detection...");
    // if (!this->write_byte(0x68, 0x07)) { this->mark_failed(); return; }; // Reset foo
    if (!this->write_byte(0x37, 0x30)) { this->mark_failed(); return; }; // Active high, until any read
    if (!this->write_byte(0x1C, 0x04)) { this->mark_failed(); return; }; // 5Hz High Pass
    if (!this->write_byte(0x1F,  15)) { this->mark_failed(); return; }; // Motion Threshold 20 mg
    if (!this->write_byte(0x20,  10)) { this->mark_failed(); return; }; // 10ms movement for wakeup
    if (!this->write_byte(0x69, 0x15)) { this->mark_failed(); return; }; // Free-Fall and motion decrements to 1
    if (!this->write_byte(0x38, 0x40)) { this->mark_failed(); return; }; // Enable Motion Detection Interrupt
  }
  
  void dump_config() override {
    ESP_LOGCONFIG(TAG, "MPU6050:");
    LOG_I2C_DEVICE(this);
    if (this->is_failed()) {
      ESP_LOGE(TAG, "Communication with MPU6050 failed!");
    }
    LOG_UPDATE_INTERVAL(this);
    LOG_SENSOR("  ", "Acceleration X", this->accel_x_sensor_);
    LOG_SENSOR("  ", "Acceleration Y", this->accel_y_sensor_);
    LOG_SENSOR("  ", "Acceleration Z", this->accel_z_sensor_);
    LOG_SENSOR("  ", "Gyro X", this->gyro_x_sensor_);
    LOG_SENSOR("  ", "Gyro Y", this->gyro_y_sensor_);
    LOG_SENSOR("  ", "Gyro Z", this->gyro_z_sensor_);
    LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  }

  void update() override {
    ESP_LOGV(TAG, "    Updating MPU6050...");
    uint16_t raw_data[7];
    if (!this->read_bytes_16(MPU6050_REGISTER_ACCEL_XOUT_H, raw_data, 7)) {
      this->status_set_warning();
      return;
    }
    auto *data = reinterpret_cast<int16_t *>(raw_data);

    float accel_x = data[0] * MPU6050_RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
    float accel_y = data[1] * MPU6050_RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
    float accel_z = data[2] * MPU6050_RANGE_PER_DIGIT_2G * GRAVITY_EARTH;

    float temperature = data[3] / 340.0f + 36.53f;

    float gyro_x = data[4] * MPU6050_SCALE_DPS_PER_DIGIT_2000;
    float gyro_y = data[5] * MPU6050_SCALE_DPS_PER_DIGIT_2000;
    float gyro_z = data[6] * MPU6050_SCALE_DPS_PER_DIGIT_2000;

    ESP_LOGD(TAG,
	     "Got accel={x=%.3f m/s², y=%.3f m/s², z=%.3f m/s²}, "
	     "gyro={x=%.3f °/s, y=%.3f °/s, z=%.3f °/s}, temp=%.3f°C",
	     accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, temperature);

    if (this->accel_x_sensor_ != nullptr)
      this->accel_x_sensor_->publish_state(accel_x);
    if (this->accel_y_sensor_ != nullptr)
      this->accel_y_sensor_->publish_state(accel_y);
    if (this->accel_z_sensor_ != nullptr)
      this->accel_z_sensor_->publish_state(accel_z);

    if (this->temperature_sensor_ != nullptr)
      this->temperature_sensor_->publish_state(temperature);

    if (this->gyro_x_sensor_ != nullptr)
      this->gyro_x_sensor_->publish_state(gyro_x);
    if (this->gyro_y_sensor_ != nullptr)
      this->gyro_y_sensor_->publish_state(gyro_y);
    if (this->gyro_z_sensor_ != nullptr)
      this->gyro_z_sensor_->publish_state(gyro_z);


    // Vectors calculated by ./misc/vector_calc.sage
    const float FACES[12][4] = {
      // Remember: Rotation might be numerically imprecise
      /*  1 */ { +0.0000000, -0.0493718, +0.9987805 },
      /*  2 */ { -0.5257311, +0.7006446, +0.4823940 },
      /*  3 */ { +0.5257311, +0.7006446, +0.4823940 },
      /*  4 */ { +0.8506508, -0.2981359, +0.4330222 },
      /*  5 */ { +0.0000000, -0.9154162, +0.4025087 },
      /*  6 */ { -0.8506508, -0.2981359, +0.4330222 },
      /*  7 */ { -0.8506508, +0.2981359, -0.4330222 },
      /*  8 */ { +0.0000000, +0.9154162, -0.4025087 },
      /*  9 */ { +0.8506508, +0.2981359, -0.4330222 },
      /* 10 */ { +0.5257311, -0.7006446, -0.4823940 },
      /* 11 */ { -0.5257311, -0.7006446, -0.4823940 },
      /* 12 */ { +0.0000000, +0.0493718, -0.9987805 },
    };

    float magn = sqrt(pow(accel_x, 2) + pow(accel_y, 2) + pow(accel_z, 2));
    
    if (this->face_sensor_ != nullptr) {
      int closest = 0;
      float closestVal = -10.0;
      for(int i = 0; i < 12; i++)
      {
	const float *face = FACES[i];
	float val= (face[0] * accel_x + face[1] * accel_y + face[2] * accel_z) / magn;
	if(val > closestVal)
	{
	  closest = i;
	  closestVal = val;
	}
      }
      this->face_sensor_->publish_state(1 + closest);
    };
    
    this->status_clear_warning();
  }


  float get_setup_priority() const override { return setup_priority::DATA; };

  void set_accel_x_sensor(sensor::Sensor *accel_x_sensor) { accel_x_sensor_ = accel_x_sensor; }
  void set_accel_y_sensor(sensor::Sensor *accel_y_sensor) { accel_y_sensor_ = accel_y_sensor; }
  void set_accel_z_sensor(sensor::Sensor *accel_z_sensor) { accel_z_sensor_ = accel_z_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_gyro_x_sensor(sensor::Sensor *gyro_x_sensor) { gyro_x_sensor_ = gyro_x_sensor; }
  void set_gyro_y_sensor(sensor::Sensor *gyro_y_sensor) { gyro_y_sensor_ = gyro_y_sensor; }
  void set_gyro_z_sensor(sensor::Sensor *gyro_z_sensor) { gyro_z_sensor_ = gyro_z_sensor; }
  void set_face_sensor(sensor::Sensor *face_sensor) { face_sensor_ = face_sensor; }

protected:
  sensor::Sensor *accel_x_sensor_{nullptr};
  sensor::Sensor *accel_y_sensor_{nullptr};
  sensor::Sensor *accel_z_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *gyro_x_sensor_{nullptr};
  sensor::Sensor *gyro_y_sensor_{nullptr};
  sensor::Sensor *gyro_z_sensor_{nullptr};
  sensor::Sensor *face_sensor_{nullptr};
};
