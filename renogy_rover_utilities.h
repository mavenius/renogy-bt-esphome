#include <vector>
#include <string>
#include <cmath>
using namespace std;

vector<uint8_t> GetRoverRequest() {
    // in renogy-bt, this is 4 requests (sections)
    // {'register': 12, 'words': 8, 'parser': self.parse_device_info},
    // {'register': 26, 'words': 1, 'parser': self.parse_device_address},
    // {'register': 256, 'words': 34, 'parser': self.parse_chargin_info},
    // {'register': 57348, 'words': 1, 'parser': self.parse_battery_type}
    // we should be able to read the register out of the response (if it's included) and parse stuff accordingly
    // or, do like renogy-bt did and have a specific parser for each section request

    // hard-coded first section { device Id (255), read (3), register ([0,12]), words ([0,8]), checksum([145,209]) }
    //vector<uint8_t> dataBytes = {255, 3, 0, 12, 0, 8, 145, 209};

    // charging info
    vector<uint8_t> dataBytes = {255, 3, 1, 0, 0, 34, 209, 241};

    for (size_t i = 0; i < dataBytes.size(); ++i) {
        ESP_LOGD("GetRoverRequest", "Request Byte %d: 0x%02X", i, dataBytes[i]);
    }

    return dataBytes;
}

static uint16_t bytes_to_int(const std::vector<uint8_t>& data, size_t offset, size_t length) {
  uint16_t result = 0;
  for (size_t i = 0; i < length; i++) {
    result = (result << 8) | data[offset + i];
  }
  return result;
}

float format_temperature(float celsius, const std::string& unit = "C") {
    if (unit == "F") {
        return (celsius * 9.0f / 5.0f) + 32.0f;
    }
    return celsius;
}

float parse_temperature(uint8_t raw_value, const std::string& unit = "C") {
    int8_t sign = raw_value >> 7;
    float celsius = (sign == 1) ? -(static_cast<float>(raw_value) - 128.0f) : static_cast<float>(raw_value);
    return format_temperature(celsius, unit);
}

void parse_charging_info(const vector<uint8_t>& data) {  
  if (data.size() < 69) {
    ESP_LOGE("Renogy", "Received data is too short");
    return;
  }

  id(battery_percentage).publish_state(bytes_to_int(data, 3, 2));
  id(battery_voltage).publish_state(bytes_to_int(data, 5, 2) * 0.1f);
  id(battery_current).publish_state(bytes_to_int(data, 7, 2) * 0.01f);
  id(battery_temperature).publish_state(parse_temperature(data[10]));
  id(controller_temperature).publish_state(parse_temperature(data[9]));
  
  const char* load_status_str = (data[67] & 0x80) ? "on" : "off";
  id(load_status).publish_state(load_status_str);
  
  id(load_voltage).publish_state(bytes_to_int(data, 11, 2) * 0.1f);
  id(load_current).publish_state(bytes_to_int(data, 13, 2) * 0.01f);
  id(load_power).publish_state(bytes_to_int(data, 15, 2));
  id(pv_voltage).publish_state(bytes_to_int(data, 17, 2) * 0.1f);
  id(pv_current).publish_state(bytes_to_int(data, 19, 2) * 0.01f);
  id(pv_power).publish_state(bytes_to_int(data, 21, 2));
  id(max_charging_power_today).publish_state(bytes_to_int(data, 33, 2));
  id(max_discharging_power_today).publish_state(bytes_to_int(data, 35, 2));
  id(charging_amp_hours_today).publish_state(bytes_to_int(data, 37, 2));
  id(discharging_amp_hours_today).publish_state(bytes_to_int(data, 39, 2));
  id(power_generation_today).publish_state(bytes_to_int(data, 41, 2));
  id(power_consumption_today).publish_state(bytes_to_int(data, 43, 2));
  id(power_generation_total).publish_state(bytes_to_int(data, 59, 4) * 0.001f);

  const char* charging_status_str;
  switch (data[68]) {
    case 0: charging_status_str = "deactivated"; break;
    case 1: charging_status_str = "activated"; break;
    case 2: charging_status_str = "mppt"; break;
    case 3: charging_status_str = "equalizing"; break;
    case 4: charging_status_str = "boost"; break;
    case 5: charging_status_str = "floating"; break;
    case 6: charging_status_str = "current limiting"; break;
    default: charging_status_str = "unknown"; break;
  }
  id(charging_status).publish_state(charging_status_str);

  ESP_LOGI("Renogy", "Data parsed and published");
}

void HandleRoverData(const vector<uint8_t>& data) {
  
  //todo: if we handle different requests, decide which parser to use
  parse_charging_info(data);
  
}

vector<uint8_t> GetLoadControlRequest(bool turn_on) {
    // MODBUS write single register command
    // Device ID (255), Function Code (6), Register Address (0x010A), Value (0x0001 for on, 0x0000 for off)
    vector<uint8_t> dataBytes = {0xFF, 0x06, 0x01, 0x0A, 0x00, 0x00};  // Register address 0x010A
    
    // Set the value based on turn_on
    if (turn_on) {
        dataBytes[5] = 0x01;  // Value 0x0001 to turn on
        // Add hardcoded CRC for turn on command
        dataBytes.push_back(0xF4);  // CRC LSB
        dataBytes.push_back(0x69);  // CRC MSB
    } else {
        dataBytes[5] = 0x00;  // Value 0x0000 to turn off
        // Add hardcoded CRC for turn off command
        dataBytes.push_back(0x34);  // CRC LSB
        dataBytes.push_back(0xA8);  // CRC MSB
    }

    // Add a delay after sending the command
    delay(100);

    for (size_t i = 0; i < dataBytes.size(); ++i) {
        ESP_LOGD("GetLoadControlRequest", "Request Byte %d: 0x%02X", i, dataBytes[i]);
    }

    return dataBytes;
}