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

//static const char device_name[] = "Nintendo RVL-CNT-01";

void handleDataSendAvailable(){
    ESP_LOGI("HCI_CB", "MOTE_TX"); 
}

#define PRINTBUFFER_SIZE 500
char READ_PRINTBUFFER [PRINTBUFFER_SIZE+1];

int handleDataRecieve(uint8_t *data, uint16_t len){
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
    ESP_LOGI("HCI_CB", "MOTE_RX:[%s]", READ_PRINTBUFFER); 
    return len;
}

void app_main(void)
{
    const char *TAG = "app_main";
    esp_err_t ret;
    char bda_str[18] = {0};

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "initialize controller failed: %s", esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(TAG, "enable controller failed: %s", esp_err_to_name(ret));
        return;
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();

    if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "enable bluedroid failed: %s", esp_err_to_name(ret));
        return;
    }

    // if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
    //     ESP_LOGE(TAG, "gap register failed: %s", esp_err_to_name(ret));
    //     return;
    // }
    const esp_vhci_host_callback_t callbacks = {handleDataSendAvailable, handleDataRecieve};

    ret = esp_vhci_host_register_callback(&callbacks);

    uint8_t* address = esp_bt_dev_get_address();

    ESP_LOGI(TAG, "Own address:[%2X %2X %2X %2X %2X %2X]", address[0], address[1], address[2], address[3], address[4], address[5]); 
    ESP_LOGI(TAG, "exiting");
}
