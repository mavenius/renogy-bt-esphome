#include <vector>
using namespace std;

vector<uint8_t> GetBatteryRequest(uint8_t batteryNumber) {
    //todo: this is the first "section" from renogy-bt. Get the rest.
    //     self.sections = [
    //     {'register': 5000, 'words': 17, 'parser': self.parse_cell_volt_info},
    //     {'register': 5017, 'words': 17, 'parser': self.parse_cell_temp_info},
    //     {'register': 5042, 'words': 6, 'parser': self.parse_battery_info},
    //     {'register': 5122, 'words': 8, 'parser': self.parse_device_info},
    //     {'register': 5223, 'words': 1, 'parser': self.parse_device_address}
    // ]

    // request = self.create_generic_read_request(self.device_id, 3, self.sections[index]['register'], self.sections[index]['words']) 

    // def create_generic_read_request(self, device_id, function, regAddr, readWrd):                             
    //     data = None                                
    //     if regAddr != None and readWrd != None:
    //         data = []
    //         data.append(device_id)
    //         data.append(function)
    // register from sections
    //         data.append(int_to_bytes(regAddr, 0))
    //         data.append(int_to_bytes(regAddr, 1))
    // words from sections
    //         data.append(int_to_bytes(readWrd, 0))
    //         data.append(int_to_bytes(readWrd, 1))

    //         crc = crc16_modbus(bytes(data))
    //         data.append(crc[0])
    //         data.append(crc[1])

    // function: 0x03 is write (send a request), 0x06 is read 
    vector<uint8_t> dataBytes = { batteryNumber, 0x03, 0x13, 0xB2, 0x00, 0x06 };

    // add checksum to the end
    uint16_t checksum = crc16(&dataBytes[0], dataBytes.size());

    // this needs to be split into 2 bytes
    dataBytes.push_back((checksum >> 0) & 0xFF);
    dataBytes.push_back((checksum >> 8) & 0xFF);

    for (size_t i = 0; i < dataBytes.size(); ++i) {
        ESP_LOGD("GetBatteryRequest", "Request Byte %d: 0x%02X", i, dataBytes[i]);
    }
    return dataBytes;
}

void HandleBatteryData(const vector<uint8_t>& x) {
    uint8_t batteryId;
    std::memcpy(&batteryId, &x[0], sizeof(batteryId));
    ESP_LOGD("HandleBatteryData", "battery Id: %d", batteryId);
    
    // def on_data_received(self, response):
    //     self.read_timer.cancel()
    //     operation = bytes_to_int(response, 1, 1)

    //     if operation == 3: # read operation
    //         logging.info("on_data_received: response for read operation")
    //         if (self.section_index < len(self.sections) and
    //             self.sections[self.section_index]['parser'] != None and
    //             self.sections[self.section_index]['words'] * 2 + 5 == len(response)):
    //             # parse and update data
    //             self.sections[self.section_index]['parser'](response)

    //         if self.section_index >= len(self.sections) - 1: # last section, read complete
    //             self.section_index = 0
    //             self.on_read_operation_complete()
    //             self.data = {}
    //         else:
    //             # still more sections, so keep reading
    //             self.section_index += 1
    //             time.sleep(0.5)
    //             self.read_section()
    //     else:
    //         logging.warn("on_data_received: unknown operation={}".format(operation))


    // Parse the function (3 == Read)
    uint8_t function;
    std::memcpy(&function, &x[1], sizeof(function));
    // function = ntohs(function); // Convert from network byte order to host byte order
    ESP_LOGD("HandleBatteryData", "function: %d", function);
    // Parse the current
    int16_t current;
    std::memcpy(&current, &x[3], sizeof(current));
    current = ntohs(current); // Convert from network byte order to host byte order
    ESP_LOGD("HandleBatteryData", "current: %d", current);
    // Parse the voltage
    uint16_t voltage;
    std::memcpy(&voltage, &x[5], 2);
    voltage = ntohs(voltage); // Convert from network byte order to host byte order
    ESP_LOGD("HandleBatteryData", "voltage: %d", voltage);
    // Parse the present capacity
    uint32_t presentCapacity;
    std::memcpy(&presentCapacity, &x[7], 4);
    presentCapacity = ntohl(presentCapacity); // Convert from network byte order to host byte order
    ESP_LOGD("HandleBatteryData", "presentCapacity: %d", presentCapacity);
    // Parse the total capacity
    uint32_t totalCapacity;
    std::memcpy(&totalCapacity, &x[11], 4);
    totalCapacity = ntohl(totalCapacity); // Convert from network byte order to host byte order
    ESP_LOGD("HandleBatteryData", "totalCapacity: %d", totalCapacity);
    // Convert the values to the appropriate units
    float currentFloat = static_cast<float>(current) / 100.0f;
    float voltageFloat = static_cast<float>(voltage) / 10.0f;
    float presentCapacityFloat = static_cast<float>(presentCapacity) / 1000.0f;
    float totalCapacityFloat = static_cast<float>(totalCapacity) / 1000.0f;
    float chargeLevelFloat = (presentCapacityFloat / totalCapacityFloat) * 100.0f; 

    ESP_LOGD("HandleBatteryData", "currentFloat: %.1f", currentFloat);
    ESP_LOGD("HandleBatteryData", "voltageFloat: %.1f", voltageFloat);
    ESP_LOGD("HandleBatteryData", "presentCapacityFloat: %.1f", presentCapacityFloat);
    ESP_LOGD("HandleBatteryData", "totalCapacityFloat: %.1f", totalCapacityFloat);
    ESP_LOGD("HandleBatteryData", "chargeLevelFloat: %.1f", chargeLevelFloat);

    ESP_LOGD("HandleBatteryData", "Building partial Ids");
    string current_id(to_string(batteryId));
    current_id.append(" Current");

    string voltage_id(to_string(batteryId));
    voltage_id.append(" Voltage");

    string present_capacity_id(to_string(batteryId));
    present_capacity_id.append(" Present Capacity");

    string total_capacity_id(to_string(batteryId));
    total_capacity_id.append(" Total Capacity");

    string charge_level_id(to_string(batteryId));
    charge_level_id.append(" Charge Level");

    for (auto *obj : App.get_sensors()){
        ESP_LOGV("HandleBatteryData", "Looping sensors. Current name: %s", obj->get_name());
        if (obj->get_name().str().find(current_id) != string::npos) {
            obj->publish_state(currentFloat);
        }
        else if (obj->get_name().str().find(voltage_id) != string::npos) {
            obj->publish_state(voltageFloat);
        }
        else if (obj->get_name().str().find(present_capacity_id) != string::npos) {
            obj->publish_state(presentCapacityFloat);
        }
        else if (obj->get_name().str().find(total_capacity_id) != string::npos) {
            obj->publish_state(totalCapacityFloat);
        }
        else if (obj->get_name().str().find(charge_level_id) != string::npos) {
            obj->publish_state(chargeLevelFloat);
        }
    }
}
