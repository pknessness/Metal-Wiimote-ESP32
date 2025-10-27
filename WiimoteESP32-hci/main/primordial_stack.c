#include <inttypes.h>
#include "esp_log.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#pragma once

#define PRINTBUFFER_SIZE 500
char READ_PRINTBUFFER [PRINTBUFFER_SIZE+1];

void ESP_LOGI_BUFFER(char* tag, uint8_t *data, uint16_t len){
    int cx = 0;
    for(int i = 0; i < len; i ++){
        int wt = snprintf ( READ_PRINTBUFFER + cx, PRINTBUFFER_SIZE - cx, "%02X ", data[i] );
        if(wt < 0){
            break;
        }else if(wt > PRINTBUFFER_SIZE - cx){
            break;
        }
        cx += wt;
        //ESP_LOGI("HCI_CB", "[%s]", READ_PRINTBUFFER); 
    }
    READ_PRINTBUFFER[cx + 1] = 0;
    ESP_LOGI(tag, "[%s]", READ_PRINTBUFFER); 
}

SemaphoreHandle_t commandSemaphore = NULL;
bool commandCompleted = false;

enum H4_TYPES {
    COMMAND = 1,
    ASYNC_DATA = 2,
    SYNC_DATA = 3,
    EVENT = 4,
    EXTENDED_COMMAND = 9
};
enum L2CAP_COMMAND_CODES {
    COMMAND_REJECT_RSP = 0x01,
    CONNECTION_REQ = 0x02,
    CONNECTION_RSP = 0x03,
    CONFIGURATION_REQ = 0x04,
    CONFIGURATION_RSP = 0x05,
    DISCONNECTION_REQ = 0x06,
    DISCONNECTION_RSP = 0x07,
    ECHO_REQ = 0x08,
    ECHO_RSP = 0x09,
    INFORMATION_REQ = 0x0A,
    INFORMATION_RSP = 0x0B,
    CONNECTION_PARAMETER_UPDATE_REQ = 0x12,
    CONNECTION_PARAMETER_UPDATE_RSP = 0x13,
    LE_CREDIT_BASED_CONNECTION_REQ = 0x14,
    LE_CREDIT_BASED_CONNECTION_RSP = 0x15,
    FLOW_CONTROL_CREDIT_IND = 0x16,
    CREDIT_BASED_CONNECTION_REQ = 0x17,
    CREDIT_BASED_CONNECTION_RSP = 0x18,
    CREDIT_BASED_RECONFIGURE_REQ = 0x19,
    CREDIT_BASED_RECONFIGURE_RSP = 0x1A
};
enum HCI_EVENT_CODES {
    INQUIRY_COMPLETE = 0x01,
    INQUIRY_RESULT = 0x02,
    CONNECTION_COMPLETE = 0x03,
    CONNECTION_REQUEST = 0x04,
    DISCONNECTION_COMPLETE = 0x05,
    AUTHENTICATION_COMPLETE = 0x06,
    REMOTE_NAME_REQUEST_COMPLETE = 0x07,
    ENCRYPTION_CHANGE = 0x08,
    CHANGE_CONNECTION_LINK_KEY_COMPLETE = 0x09,
    MASTER_LINK_KEY_COMPLETE = 0x0A,
    READ_REMOTE_SUPPORTED_FEATURES_COMPLETE = 0x0B,
    READ_REMOTE_VERSION_INFORMATION_COMPLETE = 0x0C,
    QOS_SETUP_COMPLETE = 0x0D,
    COMMAND_COMPLETE = 0x0E,
    COMMAND_STATUS = 0x0F,
    HARDWARE_ERROR = 0x10,
    FLUSH_OCCURRED = 0x11,
    ROLE_CHANGE = 0x12,
    NUMBER_OF_COMPLETED_PACKETS = 0x13,
    MODE_CHANGE = 0x14,
    RETURN_LINK_KEYS = 0x15,
    PIN_CODE_REQUEST = 0x16,
    LINK_KEY_REQUEST = 0x17,
    LINK_KEY_NOTIFICATION = 0x18,
    LOOPBACK_COMMAND = 0x19,
    DATA_BUFFER_OVERFLOW = 0x1A,
    MAX_SLOTS_CHANGE = 0x1B,
    READ_CLOCK_OFFSET_COMPLETE = 0x1C,
    CONNECTION_PACKET_TYPE_CHANGED = 0x1D,
    QOS_VIOLATION = 0x1E,
    PAGE_SCAN_MODE_CHANGE = 0x1F,
    PAGE_SCAN_REPETITION_MODE_CHANGE = 0x20,
    FLOW_SPECIFICATION_COMPLETE = 0x21,
    INQUIRY_RESULT_WITH_RSSI = 0x22,
    READ_REMOTE_EXTENDED_FEATURES_COMPLETE = 0x23,
    SYNCHRONOUS_CONNECTION_COMPLETE = 0x2C,
    SYNCHRONOUS_CONNECTION_CHANGED = 0x2D
};
enum LINK_CONTROL_COMMANDS {
    INQUIRY = 0x0001,
    INQUIRY_CANCEL = 0x0002,
    PERIODIC_INQUIRY_MODE = 0x0003,
    EXIT_PERIODIC_INQUIRY_MODE = 0x0004,
    CREATE_CONNECTION = 0x0005,
    DISCONNECT = 0x0006,
    ADD_SCO_CONNECTION = 0x0007,
    ACCEPT_CONNECTION_REQUEST = 0x0009,
    REJECT_CONNECTION_REQUEST = 0x000A,
    LINK_KEY_REQUEST_REPLY = 0x000B,
    LINK_KEY_REQUEST_NEGATIVE_REPLY = 0x000C,
    PIN_CODE_REQUEST_REPLY = 0x000D,
    PIN_CODE_REQUEST_NEGATIVE_REPLY = 0x000E,
    CHANGE_CONNECTION_PACKET_TYPE = 0x000F,
    AUTHENTICATION_REQUESTED = 0x0011,
    SET_CONNECTION_ENCRYPTION = 0x0013,
    CHANGE_CONNECTION_LINK_KEY = 0x0015,
    MASTER_LINK_KEY = 0x0017,
    REMOTE_NAME_REQUEST = 0x0019,
    READ_REMOTE_SUPPORTED_FEATURES = 0x001B,
    READ_REMOTE_VERSION_INFORMATION = 0x001D,
    READ_CLOCK_OFFSET = 0x001F
};
enum POLICY_COMMANDS {
    HOLD_MODE = 0x0001,
    SNIFF_MODE = 0x0003,
    EXIT_SNIFF_MODE = 0x0004,
    PARK_MODE = 0x0005,
    EXIT_PARK_MODE = 0x0006,
    QOS_SETUP = 0x0007,
    ROLE_DISCOVERY = 0x0009,
    SWITCH_ROLE = 0x000B,
    READ_LINK_POLICY_SETTINGS = 0x000C,
    WRITE_LINK_POLICY_SETTINGS = 0x000D
};
enum HOST_CONTROLLER_BASEBAND_COMMANDS {
    SET_EVENT_MASK = 0x0001,
    RESET = 0x0003,
    SET_EVENT_FILTER = 0x0005,
    FLUSH = 0x0008,
    READ_PIN_TYPE = 0x0009,
    WRITE_PIN_TYPE = 0x000A,
    CREATE_NEW_UNIT_KEY = 0x000B,
    READ_STORED_LINK_KEY = 0x000D,
    WRITE_STORED_LINK_KEY = 0x0011,
    DELETE_STORED_LINK_KEY = 0x0012,
    CHANGE_LOCAL_NAME = 0x0013,
    READ_LOCAL_NAME = 0x0014,
    READ_CONNECTION_ACCEPT_TIMEOUT = 0x0015,
    WRITE_CONNECTION_ACCEPT_TIMEOUT = 0x0016,
    READ_PAGE_TIMEOUT = 0x0017,
    WRITE_PAGE_TIMEOUT = 0x0018,
    READ_SCAN_ENABLE = 0x0019,
    WRITE_SCAN_ENABLE = 0x001A,
    READ_PAGE_SCAN_ACTIVITY = 0x001B,
    WRITE_PAGE_SCAN_ACTIVITY = 0x001C,
    READ_INQUIRY_SCAN_ACTIVITY = 0x001D,
    WRITE_INQUIRY_SCAN_ACTIVITY = 0x001E,
    READ_AUTHENTICATION_ENABLE = 0x001F,
    WRITE_AUTHENTICATION_ENABLE = 0x0020,
    READ_ENCRYPTION_MODE = 0x0021,
    WRITE_ENCRYPTION_MODE = 0x0022,
    READ_CLASS_OF_DEVICE = 0x0023,
    WRITE_CLASS_OF_DEVICE = 0x0024,
    READ_VOICE_SETTING = 0x0025,
    WRITE_VOICE_SETTING = 0x0026,
    READ_AUTOMATIC_FLUSH_TIMEOUT = 0x0027,
    WRITE_AUTOMATIC_FLUSH_TIMEOUT = 0x0028,
    READ_NUM_BROADCAST_RETRANSMISSIONS = 0x0029,
    WRITE_NUM_BROADCAST_RETRANSMISSIONS = 0x002A,
    READ_HOLD_MODE_ACTIVITY = 0x002B,
    WRITE_HOLD_MODE_ACTIVITY = 0x002C,
    READ_TRANSMIT_POWER_LEVEL = 0x002D,
    READ_SCO_FLOW_CONTROL_ENABLE = 0x002E,
    WRITE_SCO_FLOW_CONTROL_ENABLE = 0x002F,
    SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL = 0x0031,
    HOST_BUFFER_SIZE = 0x0033,
    HOST_NUMBER_OF_COMPLETED_PACKETS = 0x0035,
    READ_LINK_SUPERVISION_TIMEOUT = 0x0036,
    WRITE_LINK_SUPERVISION_TIMEOUT = 0x0037,
    READ_NUMBER_OF_SUPPORTED_IAC = 0x0038,
    READ_CURRENT_IAC_LAP = 0x0039,
    WRITE_CURRENT_IAC_LAP = 0x003A,
    READ_PAGE_SCAN_PERIOD_MODE = 0x003B,
    WRITE_PAGE_SCAN_PERIOD_MODE = 0x003C,
    READ_PAGE_SCAN_MODE = 0x003D,
    WRITE_PAGE_SCAN_MODE = 0x003E
};
typedef struct __attribute__((packed)){
    uint8_t addr[6];
} bd_addr_t;
typedef struct __attribute__((packed)){
    uint8_t class[3];
} device_class_t;
typedef struct __attribute__((packed)){
    uint8_t status;
} inquiry_complete_t;
// typedef struct __attribute__((packed)){
//     uint8_t num_responses;
//     bd_addr_t responses_addr;
//     uint8_t page_scan_repetition_mode;
//     uint16_t reserved;
//     device_class_t class_of_device;
//     uint16_t clock_offset;
// } inquiry_result_t;
typedef struct __attribute__((packed)){
    uint8_t status;
    uint16_t connection_handle;
    bd_addr_t bd_addr;
    uint8_t link_type;
    uint8_t encryption_enabled;
} connection_complete_t;
typedef struct __attribute__((packed)){
    bd_addr_t bd_addr;
    device_class_t class_of_device;
    uint8_t link_type;
} connection_request_t;
typedef struct __attribute__((packed)){
    uint8_t status;
    uint16_t connection_handle;
    uint8_t reason;
} disconnection_complete_t;
typedef struct __attribute__((packed)){
    uint8_t status;
    uint16_t connection_handle;
} authentication_complete_t;
typedef struct __attribute__((packed)){
    uint8_t status;
    bd_addr_t bd_addr;
    char remote_name[248];
} remote_name_request_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t encryption_enabled;
} encryption_change_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
} change_connection_link_key_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
} master_link_key_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint64_t lmp_features;
} read_remote_supported_features_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t lmp_version;
    uint16_t manufacturer_name;
    uint16_t lmp_subversion;
} read_remote_version_information_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t flags;
    uint8_t service_type;
    uint32_t token_rate;
    uint32_t peak_bandwidth;
    uint32_t latency;
    uint32_t delay_variation;
} qos_setup_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t num_hci_command_packets;
    uint16_t command_opcode;
    uint8_t return_parameters[];
} command_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint8_t num_hci_command_packets;
    uint16_t command_opcode;
} command_status_t;
typedef struct __attribute__((packed)) {
    uint8_t hardware_code;
} hardware_error_t;
typedef struct __attribute__((packed)) {
    uint16_t connection_handle;
} flush_occurred_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    bd_addr_t bd_addr;
    uint8_t new_role;
} role_change_t;
typedef struct __attribute__((packed)) {
    uint8_t num_handles;
    uint16_t connection_handles[];
} number_of_completed_packets_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t current_mode;
    uint16_t interval;
} mode_change_t;
typedef struct __attribute__((packed)) {
    uint8_t num_keys;
    bd_addr_t link_keys[];
} return_link_keys_t;
typedef struct __attribute__((packed)) {
    bd_addr_t bd_addr;
} pin_code_request_t;
typedef struct __attribute__((packed)) {
    bd_addr_t bd_addr;
} link_key_request_t;
typedef struct __attribute__((packed)) {
    bd_addr_t bd_addr;
    uint8_t link_key[16];
} link_key_notification_t;
// typedef struct __attribute__((packed)) {
//     uint8_t hci_command_packet[];
// } loopback_command_t;
typedef struct __attribute__((packed)) {
    uint8_t link_type;
} data_buffer_overflow_t;
typedef struct __attribute__((packed)) {
    uint16_t connection_handle;
    uint8_t lmp_max_slots;
} max_slots_change_t;
// typedef struct __attribute__((packed)) {
//     uint8_t status;
//     uint16_t connection_handle;
//     clock_offset_t clock_offset;
// } read_clock_offset_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint16_t packet_type;
} connection_packet_type_changed_t;
typedef struct __attribute__((packed)) {
    uint16_t connection_handle;
} qos_violation_t;
typedef struct __attribute__((packed)) {
    bd_addr_t bd_addr;
    uint8_t page_scan_mode;
} page_scan_mode_change_t;
typedef struct __attribute__((packed)) {
    bd_addr_t bd_addr;
    uint8_t page_scan_repetition_mode;
} page_scan_repetition_mode_change_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t flags;
    uint8_t flow_direction;
    uint8_t service_type;
    uint32_t token_rate;
    uint32_t token_bucket_size;
    uint32_t peak_bandwidth;
    uint32_t access_latency;
} flow_specification_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t num_responses;
    bd_addr_t responses[];
} inquiry_result_with_rssi_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t page_number;
    uint8_t max_page_number;
    uint64_t extended_lmp_features;
} read_remote_extended_features_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    bd_addr_t bd_addr;
    uint8_t link_type;
    uint8_t transmission_interval;
    uint8_t retransmission_window;
    uint16_t rx_packet_length;
    uint16_t tx_packet_length;
    uint8_t air_mode;
} synchronous_connection_complete_t;
typedef struct __attribute__((packed)) {
    uint8_t status;
    uint16_t connection_handle;
    uint8_t transmission_interval;
    uint8_t retransmission_window;
    uint16_t rx_packet_length;
    uint16_t tx_packet_length;
} synchronous_connection_changed_t;
// BTCODE
#define BTCODE_HID			0xA2

// HCI Command opcode group field(OGF) & Opcode Command Field (OCF)
// refer : http://software-dl.ti.com/lprf/simplelink_cc26x2_sdk-1.60/docs/ble5stack/vendor_specific_guide/BLE_Vendor_Specific_HCI_Guide/hci_interface.html

// Opcode Group Field (OGF) codes
#define HCI_OGF_LINK_CONTROL                 0x01  // Link control group
#define HCI_OGF_CONTROL_BASEBAND             0x03  // Host Controller & Baseband group
#define HCI_OGF_INFORMATIONAL_PARAMETERS     0x04  // Information parameters group (I DONT THINK THIS IS NEEDED)

// Informational parameter commands
// #define HCI_OCF_READ_BD_ADDR                 0x0009

// HCI Command opcodes(OGF + OCF)
// #define HCI_OPCODE_RESET                          (RESET | (HCI_OGF_CONTROL_BASEBAND << 10))
// #define HCI_OPCODE_WRITE_LOCAL_NAME               (CHANGE_LOCAL_NAME | (HCI_OGF_CONTROL_BASEBAND << 10))
// #define HCI_OPCODE_WRITE_CLASS_OF_DEVICE          (WRITE_CLASS_OF_DEVICE | (HCI_OGF_CONTROL_BASEBAND << 10))
// #define HCI_OPCODE_WRITE_SCAN_ENABLE              (WRITE_SCAN_ENABLE | (HCI_OGF_CONTROL_BASEBAND << 10))
// // #define HCI_OPCODE_READ_BD_ADDR                   (HCI_OCF_READ_BD_ADDR | (HCI_OGF_INFORMATIONAL_PARAMETERS << 10))
// #define HCI_OPCODE_INQUIRY                        (INQUIRY | (HCI_OGF_LINK_CONTROL << 10))
// #define HCI_OPCODE_INQUIRY_CANCEL                 (INQUIRY_CANCEL | (HCI_OGF_LINK_CONTROL << 10))
// #define HCI_OPCODE_CREATE_CONNECTION              (CREATE_CONNECTION | (HCI_OGF_LINK_CONTROL << 10))
// #define HCI_OPCODE_REMOTE_NAME_REQUEST            (REMOTE_NAME_REQUEST | (HCI_OGF_LINK_CONTROL << 10))

const char *STACK = "PRIMORDIAL_STACK";



void handleHCI_CMD(uint8_t *data, uint16_t len) {
    uint16_t opcode = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    uint8_t ogf = opcode >> 10 & 0x3F; 
    uint16_t ocf = opcode & 0x3FF;
    uint8_t cmdLength = data[2];
    ESP_LOGI(STACK, "CMD: OGF[%d] OCF[%d] LEN[%d]", ogf, ocf, cmdLength);
}

void handleHCI_ACL(uint8_t *data, uint16_t len) {
    
}

void handleHCI_SCO(uint8_t *data, uint16_t len) {
    
}

void handleHCI_EVT(uint8_t *data, uint16_t len) {
    uint8_t eventCode = data[0];
    uint8_t parametersLength = data[1];
    ESP_LOGI(STACK, "EVT: EC[%d] LEN[%d]", eventCode, parametersLength);
    switch(eventCode){
        case ((uint8_t)COMMAND_COMPLETE):
            command_complete_t* packet = (command_complete_t*)(data + 2);
            ESP_LOGI(STACK, "COMMAND_COMPLETE: LEN[%d] OPCODE[%x]", packet->num_hci_command_packets, packet->command_opcode);
            if(xSemaphoreTake(commandSemaphore, (TickType_t) 10) == pdTRUE){
                ESP_LOGI(STACK, "SEMAPHORE TAKE: %04X", packet->command_opcode);
                commandCompleted = true;
                xSemaphoreGive(commandSemaphore);
                ESP_LOGI(STACK, "SEMAPHORE GIVE: %04X", packet->command_opcode);
            }
        break;
            ESP_LOGI_BUFFER("UNKNOWN_EVENT", data, len);
        default:
        break;
    }
}

void handleHCIPacket(uint8_t *data, uint16_t len){
    uint8_t h4Flag = data[0];
    if(h4Flag == COMMAND){
        ESP_LOGI(STACK, "RECV COMMAND");
        handleHCI_CMD(data + 1,len - 1);
    }else if(h4Flag == ASYNC_DATA){
        ESP_LOGI(STACK, "RECV ASYNC_DATA");
        handleHCI_ACL(data + 1,len - 1);
    }else if(h4Flag == SYNC_DATA){
        ESP_LOGI(STACK, "RECV SYNC_DATA");
        handleHCI_SCO(data + 1,len - 1);
    }else if(h4Flag == EVENT){
        ESP_LOGI(STACK, "RECV EVENT");
        handleHCI_EVT(data + 1,len - 1);
    }else if(h4Flag == EXTENDED_COMMAND){
        ESP_LOGI(STACK, "EXTENDED_COMMAND");
    }else{
        ESP_LOGE(STACK, "SHOULD NEVER BE REACHED");
    }
}

void sendHCICommand(uint16_t opcode, uint8_t *data, uint16_t len, bool blocking){
    uint8_t buffer[255] = {0};
    buffer[0] = COMMAND; 
    buffer[1] = (uint8_t)opcode;
    buffer[2] = (uint8_t)(opcode >> 8);
    buffer[3] = (uint8_t)len;
    buffer[4] = (uint8_t)(len >> 8);
    memcpy(buffer + 4, data, len);
    ESP_LOGI(STACK, "HCI_COMMAND: %04X", opcode);
    ESP_LOGI_BUFFER("HCI_COMMAND:", buffer, len + 4);
    while(!esp_vhci_host_check_send_available()){}
    if(blocking){
        //ESP_LOGI(STACK, "AVAILABLE");
        if(xSemaphoreTake(commandSemaphore, (TickType_t) 10) == pdTRUE){
            commandCompleted = false;
            xSemaphoreGive(commandSemaphore);
        }
        //ESP_LOGI(STACK, "SEMAPHORE");
    }
    esp_vhci_host_send_packet(buffer, len + 4);
    //ESP_LOGI(STACK, "SENT");
    while(blocking && !commandCompleted){}
    ESP_LOGI(STACK, "COMMAND FINISHED: %04X", opcode);
}

void resetCommand(bool blocking){
    sendHCICommand((RESET | (HCI_OGF_CONTROL_BASEBAND << 10)), 0, 0, blocking);
}

void writeScanEnableCommand(uint8_t scan_enable, bool blocking){
    sendHCICommand((WRITE_SCAN_ENABLE | (HCI_OGF_CONTROL_BASEBAND << 10)), &scan_enable, 1, blocking);
}

void writeClassOfDeviceCommand(device_class_t device_class, bool blocking){
    sendHCICommand((WRITE_CLASS_OF_DEVICE | (HCI_OGF_CONTROL_BASEBAND << 10)), (uint8_t*)&device_class, 3, blocking);
}

void writeLocalNameCommand(char* local_name, uint16_t len, bool blocking){
    sendHCICommand((CHANGE_LOCAL_NAME | (HCI_OGF_CONTROL_BASEBAND << 10)), (uint8_t*)local_name, len, blocking);
}

void setControllerToHostFlowControlCommand(uint8_t flow_control_enable, bool blocking){
    sendHCICommand((SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL | (HCI_OGF_CONTROL_BASEBAND << 10)), &flow_control_enable, 1, blocking);
}

void setInquiryScanActivityCommand(uint16_t inquiry_scan_interval, uint16_t inquiry_scan_window, bool blocking){
    uint8_t packet[4] = {(uint8_t)(inquiry_scan_interval & 0xFF), (uint8_t)(inquiry_scan_interval>>8 & 0xFF), (uint8_t)(inquiry_scan_window & 0xFF), (uint8_t)(inquiry_scan_window>>8 & 0xFF)};
    sendHCICommand((SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL | (HCI_OGF_CONTROL_BASEBAND << 10)), (uint8_t*)packet, 4, blocking);
}

void readBDADDRCommand(bool blocking){
    sendHCICommand((0x0009 | (HCI_OGF_INFORMATIONAL_PARAMETERS << 10)), 0, 0, blocking);
}