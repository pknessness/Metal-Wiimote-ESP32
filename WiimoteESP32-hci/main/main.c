/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "esp_log.h"
#include "esp_hidd_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_gap_bt_api.h"
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "primordial_stack.c"

//static const char device_name[] = "Nintendo RVL-CNT-01";

void handleDataSendAvailable(){
    ESP_LOGI("HCI_CB", "MOTE_TX"); 
}

int handleDataRecieve(uint8_t *data, uint16_t len){
    ESP_LOGI_BUFFER("MOTE_RX", data, len);
    handleHCIPacket(data, len);
    return ESP_OK;
}

void app_main(void)
{
    const char *TAG = "app_main";
    esp_err_t ret;
    char bda_str[18] = {0};

    //commandSemaphore = xSemaphoreCreateBinary();
    vSemaphoreCreateBinary(commandSemaphore);

    if( commandSemaphore != NULL )
    {
        ESP_LOGI(TAG, "SEMAPHORE_CREATED");
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    // ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "initialize controller failed: %s", esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(TAG, "enable controller failed: %s", esp_err_to_name(ret));
        return;
    }

    // esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();

    // if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
    //     ESP_LOGE(TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
    //     return;
    // }

    // if ((ret = esp_bluedroid_enable()) != ESP_OK) {
    //     ESP_LOGE(TAG, "enable bluedroid failed: %s", esp_err_to_name(ret));
    //     return;
    // }

    // if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
    //     ESP_LOGE(TAG, "gap register failed: %s", esp_err_to_name(ret));
    //     return;
    // }
    const esp_vhci_host_callback_t callbacks = {handleDataSendAvailable, handleDataRecieve};

    ret = esp_vhci_host_register_callback(&callbacks);

    //uint8_t* address = esp_bt_dev_get_address();

    //resetCommand(false);
    // for(int i = 0; i < 5; i ++){
    //     readBDADDRCommand(true);
    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }
    // setControllerToHostFlowControlCommand(0x03, true);
    device_class_t class = {0};
    class.class[0] = 0x04;
    class.class[1] = 0x25;
    class.class[2] = 0x00;
    writeClassOfDeviceCommand(class, true);
    char name[248] = {0};
    memcpy(name, "PkNess's RVL-CNT-01", 19);
    writeLocalNameCommand(name, 248, true);
    //vTaskDelay(pdMS_TO_TICKS(10));
    //setInquiryScanActivityCommand(1600, 19, true);
    readClassOfDeviceCommand(class, true);

    setEventMaskCommand(0xFFFFFFFFFFFFFFFF, true);

    writeScanEnableCommand(0x03, true);

    //ESP_LOGI(TAG, "Own address:[%2X %2X %2X %2X %2X %2X]", address[0], address[1], address[2], address[3], address[4], address[5]); 
    ESP_LOGI(TAG, "exiting");
}
