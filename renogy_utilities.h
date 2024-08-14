#include <vector>
using namespace std;

uint16_t GetCRC16(vector<uint8_t> data);

vector<uint8_t> GetBatteryRequest(uint8_t batteryNumber) {
    vector<uint8_t> dataBytes = { batteryNumber, 0x03, 0x13, 0xB2, 0x00, 0x06 };

    // add checksum to the end
    uint16_t checksum = GetCRC16(dataBytes);

    // this needs to be split into 2 bytes
    dataBytes.push_back((checksum >> 0) & 0xFF);
    dataBytes.push_back((checksum >> 8) & 0xFF);

    for (size_t i = 0; i < dataBytes.size(); ++i) {
        ESP_LOGD("GetBatteryRequest", "Request Byte %d: 0x%02X", i, dataBytes[i]);
    }
    return dataBytes;
}

void HandleBatteryData(vector<uint8_t> x) {
    uint8_t batteryId;
    std::memcpy(&batteryId, &x[0], sizeof(batteryId));
    ESP_LOGD("HandleBatteryData", "battery Id: %d", batteryId);
    
    // Parse the function
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

uint16_t GetCRC16(vector<uint8_t> data) {
    uint16_t crc = 0xFFFF;
    int i;

	for(auto item = data.begin(); item != data.end(); ++item){
        crc ^= (uint16_t)*item;
        for (i = 0; i < 8; ++i) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }
    ESP_LOGD("GetCRC16", "CRC: %d", crc);
    
	return crc;
}