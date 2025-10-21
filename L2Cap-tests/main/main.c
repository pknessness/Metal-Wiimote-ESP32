/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_l2cap_bt_api.h"

#include "esp_hidd_api.h"

#include "time.h"
#include "sys/time.h"
#include "esp_vfs.h"
#include "esp_sdp_api.h"
#include "bt_app_core.h"

#include "esp_random.h"

#define L2CAP_TAG                     "L2CAP_TAG"
#define SDP_TAG                       "SDP_TAG"
#define L2CAP_DATA_LEN                100
#define BT_UNUSED_RFCOMM              -1
#define BT_L2CAP_DYNMIC_PSM           0x1001
#define BT_UNKONWN_PROFILE_VERSION    0x0102

static const char local_device_name[] = "Nintendo RVL-CNT-01";
static esp_bt_l2cap_cntl_flags_t sec_mask = ESP_BT_L2CAP_SEC_AUTHENTICATE;
static char *sdp_service_name = "Unknown_profile";
static const uint8_t  UUID_UNKNOWN[] = {0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x10, 0x00,
                                            0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
                                        };

static void esp_bt_l2cap_cb(esp_bt_l2cap_cb_event_t event, esp_bt_l2cap_cb_param_t *param);
static void esp_hdl_bt_l2cap_cb_evt(uint16_t event, void *p_param);
static void esp_hdl_sdp_cb_evt(uint16_t event, void *p_param);

typedef struct {
    esp_hidd_app_param_t app_param;
    esp_hidd_qos_param_t both_qos;
    uint8_t protocol_mode;
    SemaphoreHandle_t mouse_mutex;
    TaskHandle_t mouse_task_hdl;
    uint8_t buffer[40];
    int8_t x_dir;
} local_param_t;

static local_param_t s_local_param = {0};

static char *bda2str(esp_bd_addr_t bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    return str;
}

static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    /* when Legacy Pairing pin code requested, this event comes */
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(L2CAP_TAG, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,
                     bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
        } else {
            ESP_LOGE(L2CAP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    /* when Security Simple Pairing user confirmation requested, this event comes */
    // case ESP_BT_GAP_PIN_REQ_EVT:{
    //     ESP_LOGI(L2CAP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
    //     if (param->pin_req.min_16_digit) {
    //         ESP_LOGI(L2CAP_TAG, "Input pin code: 0000 0000 0000 0000");
    //         esp_bt_pin_code_t pin_code = {0};
    //         esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
    //     } else {
    //         ESP_LOGI(L2CAP_TAG, "Input pin code: 1234");
    //         esp_bt_pin_code_t pin_code;
    //         pin_code[0] = '1';
    //         pin_code[1] = '2';
    //         pin_code[2] = '3';
    //         pin_code[3] = '4';
    //         esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
    //     }
    //     break;
    // }
    /* when GAP mode changed, this event comes */
    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(L2CAP_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode:%d bda:[%s]", param->mode_chg.mode,
                 bda2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
        break;
    /* other */
    case ESP_BT_GAP_DISC_RES_EVT:
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
    case ESP_BT_GAP_RMT_SRVCS_EVT:
    case ESP_BT_GAP_RMT_SRVC_REC_EVT:
    case ESP_BT_GAP_PIN_REQ_EVT:
    case ESP_BT_GAP_CFM_REQ_EVT:
    case ESP_BT_GAP_KEY_NOTIF_EVT:
    case ESP_BT_GAP_KEY_REQ_EVT:
    case ESP_BT_GAP_READ_RSSI_DELTA_EVT:
    case ESP_BT_GAP_CONFIG_EIR_DATA_EVT:
    case ESP_BT_GAP_SET_AFH_CHANNELS_EVT:
    case ESP_BT_GAP_READ_REMOTE_NAME_EVT:
    case ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT:
    case ESP_BT_GAP_QOS_CMPL_EVT:
    case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:
    case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT:
    case ESP_BT_GAP_SET_PAGE_TO_EVT:
    case ESP_BT_GAP_GET_PAGE_TO_EVT:
    case ESP_BT_GAP_ACL_PKT_TYPE_CHANGED_EVT:
    case ESP_BT_GAP_ENC_CHG_EVT:
    case ESP_BT_GAP_SET_MIN_ENC_KEY_SIZE_EVT:
    case ESP_BT_GAP_GET_DEV_NAME_CMPL_EVT:
    case ESP_BT_GAP_EVT_MAX:
    default: {
        ESP_LOGI(L2CAP_TAG, "event: 0x%x", event);
        break;
    }
    }
    return;
}

static void l2cap_read_handle(void * param)
{
    int size = 0;
    int fd = (int)param;
    uint8_t *l2cap_data = NULL;

    l2cap_data = malloc(L2CAP_DATA_LEN);
    if (!l2cap_data) {
        ESP_LOGE(L2CAP_TAG, "malloc l2cap_data failed, fd:%d", fd);
        goto done;
    }

    do {
        /* The frequency of calling this function also limits the speed at which the peer device can send data. */
        size = read(fd, l2cap_data, L2CAP_DATA_LEN);
        if (size < 0) {
            break;
        } else if (size == 0) {
            /* There is no data, retry after 500 ms */
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            ESP_LOGI(L2CAP_TAG, "fd = %d data_len = %d", fd, size);
            /* To avoid task watchdog */
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    } while (1);
done:
    if (l2cap_data) {
        free(l2cap_data);
    }
    l2cap_wr_task_shut_down();
}

static void l2cap_write_handle(void * param)
{
    int size = 0;
    int fd = (int)param;
    uint8_t *l2cap_data = NULL;
    uint16_t i = 0;

    l2cap_data = malloc(L2CAP_DATA_LEN);
    if (!l2cap_data) {
        ESP_LOGE(L2CAP_TAG, "malloc l2cap_data failed, fd:%d", fd);
        goto done;
    }

    for (i = 0; i < L2CAP_DATA_LEN; ++i) {
        l2cap_data[i] = i;
    }

    do {
        /*
         * The write function is blocked until all the target length of data has been sent to the lower layer
         * successfully an error occurs.
         */
        size = write(fd, l2cap_data, L2CAP_DATA_LEN);
        if (size == -1) {
            break;
        } else if (size == 0) {
            /*write fail due to ringbuf is full, retry after 500 ms*/
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            ESP_LOGI(L2CAP_TAG, "fd = %d  data_len = %d", fd, size);
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    } while (1);
done:
    if (l2cap_data) {
        free(l2cap_data);
    }
    l2cap_wr_task_shut_down();
}

static void esp_bt_l2cap_cb(esp_bt_l2cap_cb_event_t event, esp_bt_l2cap_cb_param_t *param)
{
    switch (event) {
    case ESP_BT_L2CAP_INIT_EVT:
    case ESP_BT_L2CAP_UNINIT_EVT:
    case ESP_BT_L2CAP_OPEN_EVT:
    case ESP_BT_L2CAP_CLOSE_EVT:
    case ESP_BT_L2CAP_CL_INIT_EVT:
    case ESP_BT_L2CAP_START_EVT:
    case ESP_BT_L2CAP_SRV_STOP_EVT: {
        bt_app_work_dispatch(esp_hdl_bt_l2cap_cb_evt, event, param, sizeof(esp_bt_l2cap_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(L2CAP_TAG, "Invalid L2CAP event: %d", event);
        break;
    }
}

static void esp_hdl_bt_l2cap_cb_evt(uint16_t event, void *p_param)
{
    char bda_str[18] = {0};
    esp_bt_l2cap_cb_param_t *l2cap_param = (esp_bt_l2cap_cb_param_t *)p_param;

    switch (event) {
    case ESP_BT_L2CAP_INIT_EVT:
        if (l2cap_param->init.status == ESP_BT_L2CAP_SUCCESS) {
            ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_INIT_EVT: status:%d", l2cap_param->init.status);
            esp_bt_l2cap_vfs_register();
            esp_bt_l2cap_start_srv(sec_mask, BT_L2CAP_DYNMIC_PSM);
        } else {
            ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_INIT_EVT: status:%d", l2cap_param->init.status);
        }
        break;
    case ESP_BT_L2CAP_UNINIT_EVT:
        ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_UNINIT_EVT: status:%d", l2cap_param->uninit.status);
        break;
    case ESP_BT_L2CAP_OPEN_EVT:
        if (l2cap_param->open.status == ESP_BT_L2CAP_SUCCESS) {
            ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_OPEN_EVT: status:%d, fd = %d, tx mtu = %"PRIu32", remote_address:%s", l2cap_param->open.status,
                    l2cap_param->open.fd, l2cap_param->open.tx_mtu, bda2str(l2cap_param->open.rem_bda, bda_str, sizeof(bda_str)));
            l2cap_wr_task_start_up(l2cap_read_handle, l2cap_param->open.fd);
        } else {
            ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_OPEN_EVT: status:%d", l2cap_param->open.status);
        }
        break;
    case ESP_BT_L2CAP_CLOSE_EVT:
        ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_CLOSE_EVT: status:%d", l2cap_param->close.status);
        esp_bt_l2cap_start_srv(sec_mask, BT_L2CAP_DYNMIC_PSM); // bug, need to do fix
        break;
    case ESP_BT_L2CAP_CL_INIT_EVT:
        ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_CL_INIT_EVT: status:%d", l2cap_param->cl_init.status);
        break;
    case ESP_BT_L2CAP_START_EVT:
        if (l2cap_param->start.status == ESP_BT_L2CAP_SUCCESS) {
            ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_START_EVT: status:%d, hdl:0x%"PRIx32", sec_id:0x%x",
                l2cap_param->start.status, l2cap_param->start.handle, l2cap_param->start.sec_id);
        } else {
            ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_START_EVT: status:%d", l2cap_param->start.status);
        }
        break;
    case ESP_BT_L2CAP_SRV_STOP_EVT:
        ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_SRV_STOP_EVT: status:%d, psm = 0x%x", l2cap_param->srv_stop.status, l2cap_param->srv_stop.psm);
        break;
    default:
        ESP_LOGI(L2CAP_TAG, "ESP_BT_L2CAP_EVT: %2x",event);
        break;
    }
    return;
}

static void esp_sdp_cb(esp_sdp_cb_event_t event, esp_sdp_cb_param_t *param)
{
    switch (event) {
    case ESP_SDP_INIT_EVT:
    case ESP_SDP_DEINIT_EVT:
    case ESP_SDP_SEARCH_COMP_EVT:
    case ESP_SDP_CREATE_RECORD_COMP_EVT:
    case ESP_SDP_REMOVE_RECORD_COMP_EVT: {
        bt_app_work_dispatch(esp_hdl_sdp_cb_evt, event, param, sizeof(esp_sdp_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(SDP_TAG, "Invalid SDP event: %d", event);
        break;
    }
}

static void esp_hdl_sdp_cb_evt(uint16_t event, void *p_param)
{
    esp_bluetooth_sdp_raw_record_t record = {0};
    esp_sdp_cb_param_t *sdp_param = (esp_sdp_cb_param_t *)p_param;

    switch (event) {
    case ESP_SDP_INIT_EVT:
        ESP_LOGI(SDP_TAG, "ESP_SDP_INIT_EVT: status:%d", sdp_param->init.status);
        if (sdp_param->init.status == ESP_SDP_SUCCESS) {
            record.hdr.type = ESP_SDP_TYPE_RAW;
            record.hdr.uuid.len = sizeof(UUID_UNKNOWN);
            memcpy(record.hdr.uuid.uuid.uuid128, UUID_UNKNOWN, sizeof(UUID_UNKNOWN));
            record.hdr.service_name_length = strlen(sdp_service_name) + 1;
            record.hdr.service_name = sdp_service_name;
            record.hdr.rfcomm_channel_number = BT_UNUSED_RFCOMM;
            record.hdr.l2cap_psm = BT_L2CAP_DYNMIC_PSM;
            record.hdr.profile_version = BT_UNKONWN_PROFILE_VERSION;
            esp_sdp_create_record((esp_bluetooth_sdp_record_t *)&record);
        }
        break;
    case ESP_SDP_DEINIT_EVT:
        ESP_LOGI(SDP_TAG, "ESP_SDP_DEINIT_EVT: status:%d", sdp_param->deinit.status);
        break;
    case ESP_SDP_SEARCH_COMP_EVT:
        ESP_LOGI(SDP_TAG, "ESP_SDP_SEARCH_COMP_EVT: status:%d", sdp_param->search.status);
        break;
    case ESP_SDP_CREATE_RECORD_COMP_EVT:
        ESP_LOGI(SDP_TAG, "ESP_SDP_CREATE_RECORD_COMP_EVT: status:%d", sdp_param->create_record.status);
        if (sdp_param->create_record.status == ESP_SDP_SUCCESS) {
            esp_bt_gap_set_device_name(local_device_name);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        }
        break;
    case ESP_SDP_REMOVE_RECORD_COMP_EVT:
        ESP_LOGI(SDP_TAG, "ESP_SDP_REMOVE_RECORD_COMP_EVT: status:%d", sdp_param->remove_record.status);
        break;
    default:
        break;
    }
}

// void esp_bt_hidd_cb(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
// {
//     static const char *TAG = "esp_bt_hidd_cb";
//     switch (event) {
//     case ESP_HIDD_INIT_EVT:
//         if (param->init.status == ESP_HIDD_SUCCESS) {
//             ESP_LOGI(TAG, "setting hid parameters");
//             esp_bt_hid_device_register_app(&s_local_param.app_param, &s_local_param.both_qos, &s_local_param.both_qos);
//         } else {
//             ESP_LOGE(TAG, "init hidd failed!");
//         }
//         break;
//     case ESP_HIDD_DEINIT_EVT:
//         break;
//     case ESP_HIDD_REGISTER_APP_EVT:
//         if (param->register_app.status == ESP_HIDD_SUCCESS) {
//             ESP_LOGI(TAG, "setting hid parameters success!");
//             ESP_LOGI(TAG, "setting to connectable, discoverable");
//             esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
//             if (param->register_app.in_use) {
//                 ESP_LOGI(TAG, "start virtual cable plug!");
//                 esp_bt_hid_device_connect(param->register_app.bd_addr);
//             }
//         } else {
//             ESP_LOGE(TAG, "setting hid parameters failed!");
//         }
//         break;
//     case ESP_HIDD_UNREGISTER_APP_EVT:
//         if (param->unregister_app.status == ESP_HIDD_SUCCESS) {
//             ESP_LOGI(TAG, "unregister app success!");
//         } else {
//             ESP_LOGE(TAG, "unregister app failed!");
//         }
//         break;
//     case ESP_HIDD_OPEN_EVT:
//         if (param->open.status == ESP_HIDD_SUCCESS) {
//             if (param->open.conn_status == ESP_HIDD_CONN_STATE_CONNECTING) {
//                 ESP_LOGI(TAG, "connecting...");
//             } else if (param->open.conn_status == ESP_HIDD_CONN_STATE_CONNECTED) {
//                 ESP_LOGI(TAG, "connected to %02x:%02x:%02x:%02x:%02x:%02x", param->open.bd_addr[0],
//                     param->open.bd_addr[1], param->open.bd_addr[2], param->open.bd_addr[3], param->open.bd_addr[4],
//                     param->open.bd_addr[5]);
//                 bt_app_task_start_up();
//                 ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
//                 esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
//             } else {
//                 ESP_LOGE(TAG, "unknown connection status");
//             }
//         } else {
//             ESP_LOGE(TAG, "open failed!");
//         }
//         break;
//     case ESP_HIDD_CLOSE_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_CLOSE_EVT");
//         if (param->close.status == ESP_HIDD_SUCCESS) {
//             if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTING) {
//                 ESP_LOGI(TAG, "disconnecting...");
//             } else if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED) {
//                 ESP_LOGI(TAG, "disconnected!");
//                 bt_app_task_shut_down();
//                 ESP_LOGI(TAG, "making self discoverable and connectable again.");
//                 esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
//             } else {
//                 ESP_LOGE(TAG, "unknown connection status");
//             }
//         } else {
//             ESP_LOGE(TAG, "close failed!");
//         }
//         break;
//     case ESP_HIDD_SEND_REPORT_EVT:
//         if (param->send_report.status == ESP_HIDD_SUCCESS) {
//             ESP_LOGI(TAG, "ESP_HIDD_SEND_REPORT_EVT id:0x%02x, type:%d", param->send_report.report_id,
//                      param->send_report.report_type);
//         } else {
//             ESP_LOGE(TAG, "ESP_HIDD_SEND_REPORT_EVT id:0x%02x, type:%d, status:%d, reason:%d",
//                      param->send_report.report_id, param->send_report.report_type, param->send_report.status,
//                      param->send_report.reason);
//         }
//         break;
//     case ESP_HIDD_REPORT_ERR_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_REPORT_ERR_EVT");
//         break;
//     case ESP_HIDD_GET_REPORT_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_GET_REPORT_EVT id:0x%02x, type:%d, size:%d", param->get_report.report_id,
//                  param->get_report.report_type, param->get_report.buffer_size);
//         if (check_report_id_type(param->get_report.report_id, param->get_report.report_type)) {
//             uint8_t report_id;
//             uint16_t report_len;
//             if (s_local_param.protocol_mode == ESP_HIDD_REPORT_MODE) {
//                 report_id = 0;
//                 report_len = 4; //REPORT_PROTOCOL_MOUSE_REPORT_SIZE;
//             } else {
//                 // Boot Mode
//                 report_id = ESP_HIDD_BOOT_REPORT_ID_MOUSE;
//                 report_len = ESP_HIDD_BOOT_REPORT_SIZE_MOUSE - 1;
//             }
//             xSemaphoreTake(s_local_param.mouse_mutex, portMAX_DELAY);
//             esp_bt_hid_device_send_report(param->get_report.report_type, report_id, report_len, s_local_param.buffer);
//             xSemaphoreGive(s_local_param.mouse_mutex);
//         } else {
//             ESP_LOGE(TAG, "check_report_id failed!");
//         }
//         break;
//     case ESP_HIDD_SET_REPORT_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_SET_REPORT_EVT");
//         break;
//     case ESP_HIDD_SET_PROTOCOL_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_SET_PROTOCOL_EVT");
//         if (param->set_protocol.protocol_mode == ESP_HIDD_BOOT_MODE) {
//             ESP_LOGI(TAG, "  - boot protocol");
//             xSemaphoreTake(s_local_param.mouse_mutex, portMAX_DELAY);
//             s_local_param.x_dir = -1;
//             xSemaphoreGive(s_local_param.mouse_mutex);
//         } else if (param->set_protocol.protocol_mode == ESP_HIDD_REPORT_MODE) {
//             ESP_LOGI(TAG, "  - report protocol");
//         }
//         xSemaphoreTake(s_local_param.mouse_mutex, portMAX_DELAY);
//         s_local_param.protocol_mode = param->set_protocol.protocol_mode;
//         xSemaphoreGive(s_local_param.mouse_mutex);
//         break;
//     case ESP_HIDD_INTR_DATA_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_INTR_DATA_EVT");
//         break;
//     case ESP_HIDD_VC_UNPLUG_EVT:
//         ESP_LOGI(TAG, "ESP_HIDD_VC_UNPLUG_EVT");
//         if (param->vc_unplug.status == ESP_HIDD_SUCCESS) {
//             if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED) {
//                 ESP_LOGI(TAG, "disconnected!");
//                 bt_app_task_shut_down();
//                 ESP_LOGI(TAG, "making self discoverable and connectable again.");
//                 esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
//             } else {
//                 ESP_LOGE(TAG, "unknown connection status");
//             }
//         } else {
//             ESP_LOGE(TAG, "close failed!");
//         }
//         break;
//     default:
//         break;
//     }
// }

void app_main(void)
{
    char bda_str[18] = {0};
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    bluedroid_cfg.ssp_en = false;
    if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    bt_app_task_start_up();

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s gap register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_l2cap_register_callback(esp_bt_l2cap_cb)) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s l2cap register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_l2cap_init()) != ESP_OK) {
        ESP_LOGE(L2CAP_TAG, "%s l2cap init failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_sdp_register_callback(esp_sdp_cb)) != ESP_OK) {
        ESP_LOGE(SDP_TAG, "%s sdp register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_sdp_init()) != ESP_OK) {
        ESP_LOGE(SDP_TAG, "%s sdp init failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_code_t pin_code;
    pin_code[0] = esp_bt_dev_get_address()[5];
    pin_code[1] = esp_bt_dev_get_address()[4];
    pin_code[2] = esp_bt_dev_get_address()[3];
    pin_code[3] = esp_bt_dev_get_address()[2];
    pin_code[4] = esp_bt_dev_get_address()[1];
    pin_code[5] = esp_bt_dev_get_address()[0];
    esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 6, pin_code);

    ESP_LOGI(L2CAP_TAG, "Own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));

    while(1){
        uint32_t rand = esp_random();
        uint8_t one = (uint8_t)(rand & 0xFF);
        uint8_t two = (uint8_t)((rand >> 8) & 0xFF);
        ESP_LOGI("MAIN", "SENDING %2X %2X", one, two);
        uint8_t core[2] = {one, two};
        //esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INPUT, 0x30, 2, core);
        esp_bt_hid_device_send_report(0xa1, 0x30, 2, core);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
