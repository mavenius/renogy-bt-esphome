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

    // todo: this is hard-coded to batteries 48, 49, 50. Update to use batteryId to dynamically work. 
    // If that's not possible, we could just make a giant switch for renogy_battery_1_* to renogy_battery_255_* 
    // so they are all handled, but that's kind of heavy handed...
    
    // use battery_id to decide which to update
    switch (batteryId){
        case 48:
            id(renogy_battery_48_current).publish_state(currentFloat);
            id(renogy_battery_48_voltage).publish_state(voltageFloat);
            id(renogy_battery_48_present_capacity).publish_state(presentCapacityFloat);
            id(renogy_battery_48_total_capacity).publish_state(totalCapacityFloat);
            id(renogy_battery_48_charge_level).publish_state(chargeLevelFloat);
        break;
        case 49:
            id(renogy_battery_49_current).publish_state(currentFloat);
            id(renogy_battery_49_voltage).publish_state(voltageFloat);
            id(renogy_battery_49_present_capacity).publish_state(presentCapacityFloat);
            id(renogy_battery_49_total_capacity).publish_state(totalCapacityFloat);
            id(renogy_battery_49_charge_level).publish_state(chargeLevelFloat);
        break;
        case 50:
            id(renogy_battery_50_current).publish_state(currentFloat);
            id(renogy_battery_50_voltage).publish_state(voltageFloat);
            id(renogy_battery_50_present_capacity).publish_state(presentCapacityFloat);
            id(renogy_battery_50_total_capacity).publish_state(totalCapacityFloat);
            id(renogy_battery_50_charge_level).publish_state(chargeLevelFloat);
        break;

    }
}
