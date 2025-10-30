/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
 * GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

#undef ENABLE_BLE

#define BTSTACK_FILE__ "your_project.c"

// *****************************************************************************
/* EXAMPLE_START(hid_mouse_demo): HID Mouse Classic
 *
 * @text This HID Device example demonstrates how to implement
 * an HID keyboard. Without a HAVE_BTSTACK_STDIN, a fixed demo text is sent
 * If HAVE_BTSTACK_STDIN is defined, you can type from the terminal
 */
// *****************************************************************************

#undef ENABLE_BLE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "btstack.h"

#include "esp_log.h"



#ifdef HAVE_BTSTACK_STDIN
#include "btstack_stdin.h"
#endif

// to enable demo text on POSIX systems
// #undef HAVE_BTSTACK_STDIN

static uint8_t hid_service_buffer[270];
static uint8_t device_id_sdp_service_buffer[100];
// static const char hid_device_name[] = "BTstack HID Mouse";
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t l2cap_event_callback_registration;

static uint16_t hid_cid;

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

static uint8_t mote_service_buffer_1[] = {0x36, 0x00, 0x4d, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x10, 0x00, 0x09, 0x00, 0x04, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x00, 0x01, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x00, 0x35, 0x03, 0x09, 0x01, 0x00};
static uint8_t mote_service_buffer_2[] = {0x36, 0x01, 0xcc, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x11, 0x24, 0x09, 0x00, 0x04, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x11, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x24, 0x09, 0x01, 0x00, 0x09, 0x00, 0x0d, 0x35, 0x0f, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x13, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x01, 0x00, 0x25, 0x13, 0x4e, 0x69, 0x6e, 0x74, 0x65, 0x6e, 0x64, 0x6f, 0x20, 0x52, 0x56, 0x4c, 0x2d, 0x43, 0x4e, 0x54, 0x2d, 0x30, 0x31, 0x09, 0x01, 0x01, 0x25, 0x13, 0x4e, 0x69, 0x6e, 0x74, 0x65, 0x6e, 0x64, 0x6f, 0x20, 0x52, 0x56, 0x4c, 0x2d, 0x43, 0x4e, 0x54, 0x2d, 0x30, 0x31, 0x09, 0x01, 0x02, 0x25, 0x08, 0x4e, 0x69, 0x6e, 0x74, 0x65, 0x6e, 0x64, 0x6f, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x01, 0x09, 0x01, 0x11, 0x09, 0x02, 0x02, 0x08, 0x04, 0x09, 0x02, 0x03, 0x08, 0x33, 0x09, 0x02, 0x04, 0x28, 0x00, 0x09, 0x02, 0x05, 0x28, 0x01, 0x09, 0x02, 0x06, 0x35, 0xdf, 0x35, 0xdd, 0x08, 0x22, 0x25, 0xd9, 0x05, 0x01, 0x09, 0x05, 0xa1, 0x01, 0x85, 0x10, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95, 0x01, 0x06, 0x00, 0xff, 0x09, 0x01, 0x91, 0x00, 0x85, 0x11, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x12, 0x95, 0x02, 0x09, 0x01, 0x91, 0x00, 0x85, 0x13, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x14, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x15, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x16, 0x95, 0x15, 0x09, 0x01, 0x91, 0x00, 0x85, 0x17, 0x95, 0x06, 0x09, 0x01, 0x91, 0x00, 0x85, 0x18, 0x95, 0x15, 0x09, 0x01, 0x91, 0x00, 0x85, 0x19, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x1a, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x20, 0x95, 0x06, 0x09, 0x01, 0x81, 0x00, 0x85, 0x21, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x22, 0x95, 0x04, 0x09, 0x01, 0x81, 0x00, 0x85, 0x30, 0x95, 0x02, 0x09, 0x01, 0x81, 0x00, 0x85, 0x31, 0x95, 0x05, 0x09, 0x01, 0x81, 0x00, 0x85, 0x32, 0x95, 0x0a, 0x09, 0x01, 0x81, 0x00, 0x85, 0x33, 0x95, 0x11, 0x09, 0x01, 0x81, 0x00, 0x85, 0x34, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x35, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x36, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x37, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3d, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3e, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3f, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0xc0, 0x09, 0x02, 0x07, 0x35, 0x08, 0x35, 0x06, 0x09, 0x04, 0x09, 0x09, 0x01, 0x00, 0x09, 0x02, 0x08, 0x28, 0x00, 0x09, 0x02, 0x09, 0x28, 0x01, 0x09, 0x02, 0x0a, 0x28, 0x01, 0x09, 0x02, 0x0b, 0x09, 0x01, 0x00, 0x09, 0x02, 0x0c, 0x09, 0x0c, 0x80, 0x09, 0x02, 0x0d, 0x28, 0x00, 0x09, 0x02, 0x0e, 0x28, 0x00};
static uint8_t mote_service_buffer_3[] = {0x36, 0x00, 0x5a, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x12, 0x00, 0x09, 0x00, 0x04, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x00, 0x01, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x12, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x01, 0x09, 0x05, 0x7e, 0x09, 0x02, 0x02, 0x09, 0x03, 0x06, 0x09, 0x02, 0x03, 0x09, 0x06, 0x00, 0x09, 0x02, 0x04, 0x28, 0x01, 0x09, 0x02, 0x05, 0x09, 0x00, 0x02};

#define RAW_WIIMOTE_BUFFERS

// from USB HID Specification 1.1, Appendix B.2
const uint8_t hid_descriptor_mouse_boot_mode[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)

    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)

    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)

    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)

    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

uint8_t WiiMoteHIDDescriptor[] = {
    /*
    |-----------------------------|
    |           Wiimote           |
    |-----------------------------|
    */  
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x10,        //   Report ID (16) //Rumble
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x11,        //   Report ID (17) //Player LEDs
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x12,        //   Report ID (18) //Data Reporting Mode
    0x95, 0x02,        //   Report Count (2)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x13,        //   Report ID (19) //IR Camera Enable
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x14,        //   Report ID (20) //Speaker Enable
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x15,        //   Report ID (21) //Status Information Request
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x16,        //   Report ID (22) //Write Memory and Registers
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x17,        //   Report ID (23) //Read Memory and Registers
    0x95, 0x06,        //   Report Count (6)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x18,        //   Report ID (24) //Speaker Data
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x19,        //   Report ID (25) //Speaker Mute
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x1A,        //   Report ID (26) IR Camera Enable 2
    0x95, 0x01,        //   Report Count (1)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x20,        //   Report ID (32) //Status Information
    0x95, 0x06,        //   Report Count (6)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x21,        //   Report ID (33) //Read Memory and Registers Data
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x22,        //   Report ID (34) //Acknowledge output report, return function result
    0x95, 0x04,        //   Report Count (4)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x30,        //   Report ID (48) //Data Report: Core Buttons
    0x95, 0x02,        //   Report Count (2)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x31,        //   Report ID (49) //Data Report: Core Buttons and Accelerometer
    0x95, 0x05,        //   Report Count (5)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x32,        //   Report ID (50) //Data Report: Core Buttons with 8 Extension Bytes
    0x95, 0x0A,        //   Report Count (10)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x33,        //   Report ID (51) //Data Report: Core Buttons with 12 IR Bytes
    0x95, 0x11,        //   Report Count (17)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x34,        //   Report ID (52) //Data Report: Core Buttons with 19 Extension Bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x35,        //   Report ID (53) //Data Report: Core Buttons and Accelerometer with 16 Extension Bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x36,        //   Report ID (54) //Data Report: Core Buttons with 10 IR bytes and 9 Extension Bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x37,        //   Report ID (55) //Data Report: Core Buttons and Accelerometer with 10 IR bytes and 6 Extension Bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x3D,        //   Report ID (61) //Data Report: 21 Extension Bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x3E,        //   Report ID (62) //Data Report: Interleaved Core Buttons and Accelerometer with 36 IR bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x3F,        //   Report ID (63) //Data Report: Interleaved Core Buttons and Accelerometer with 36 IR bytes
    0x95, 0x15,        //   Report Count (21)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};

// HID Report sending
static void send_report(uint8_t buttons, int8_t dx, int8_t dy){
    // setup HID message: A1 = Input Report, Report ID, Payload
    uint8_t message[] = {0xa1, buttons, (uint8_t) dx, (uint8_t) dy};
    hid_device_send_interrupt_message(hid_cid, &message[0], sizeof(message));
    printf("Mouse: %d/%d - buttons: %02x\n", dx, dy, buttons);
}

static int dx;
static int dy;
static uint8_t buttons;
static int hid_boot_device = 0;

static void mousing_can_send_now(void){
    send_report(buttons, dx, dy);
    // reset
    dx = 0;
    dy = 0;
    if (buttons){
        buttons = 0;
        hid_device_request_can_send_now_event(hid_cid);
    }
}

// Demo Application

#ifdef HAVE_BTSTACK_STDIN

static const int MOUSE_SPEED = 30;

// On systems with STDIN, we can directly type on the console

static void stdin_process(char character){

    if (!hid_cid) {
        printf("Mouse not connected, ignoring '%c'\n", character);
        return;
    }

    switch (character){
        case 'a':
            dx -= MOUSE_SPEED;
            break;
        case 's':
            dy += MOUSE_SPEED;
            break;
        case 'd':
            dx += MOUSE_SPEED;
            break;
        case 'w':
            dy -= MOUSE_SPEED;
            break;
        case 'l':
            buttons |= 1;
            break;
        case 'r':
            buttons |= 2;
            break;
        default:
            return;
    }
    hid_device_request_can_send_now_event(hid_cid);
}

#else

// On embedded systems, simulate clicking on 4 corners of a square

#define MOUSE_PERIOD_MS 15

static int step;
static const int STEPS_PER_DIRECTION = 50;
static const int MOUSE_SPEED = 10;

static struct {
    int dx;
    int dy;
} directions[] = {
    {  1,  0 },
    {  0,  1 },
    { -1,  0 },
    {  0, -1 },
};

static btstack_timer_source_t mousing_timer;

static void mousing_timer_handler(btstack_timer_source_t * ts){

    if (!hid_cid) return;

    // simulate left click when corner reached
    if (step % STEPS_PER_DIRECTION == 0){
        buttons |= 1;
    }
    // simulate move
    int direction_index = step / STEPS_PER_DIRECTION;
    dx += directions[direction_index].dx * MOUSE_SPEED;
    dy += directions[direction_index].dy * MOUSE_SPEED;

    // next
    step++;
    if (step >= STEPS_PER_DIRECTION * 4) {
        step = 0;
    }

    // trigger send
    hid_device_request_can_send_now_event(hid_cid);

    // set next timer
    btstack_run_loop_set_timer(ts, MOUSE_PERIOD_MS);
    btstack_run_loop_add_timer(ts);
}

static void hid_embedded_start_mousing(void){
    printf("Start mousing..\n");

    step = 0;

    // set one-shot timer
    mousing_timer.process = &mousing_timer_handler;
    btstack_run_loop_set_timer(&mousing_timer, MOUSE_PERIOD_MS);
    btstack_run_loop_add_timer(&mousing_timer);
}
#endif

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
    UNUSED(channel);
    UNUSED(packet_size);
    switch (packet_type){
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)){
                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // ssp: inform about user confirmation request
                    log_info("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
                    log_info("SSP User Confirmation Auto accept\n");
                    break;

                case HCI_EVENT_HID_META:
                    switch (hci_event_hid_meta_get_subevent_code(packet)){
                        case HID_SUBEVENT_CONNECTION_OPENED:
                            if (hid_subevent_connection_opened_get_status(packet) != ERROR_CODE_SUCCESS) return;
                            hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
#ifdef HAVE_BTSTACK_STDIN
                            printf("HID Connected, control mouse using 'a','s',''d', 'w' keys for movement and 'l' and 'r' for buttons...\n");
#else
                            printf("HID Connected, simulating mouse movements...\n");
                            hid_embedded_start_mousing();
#endif
                            break;
                        case HID_SUBEVENT_CONNECTION_CLOSED:
                            printf("HID Disconnected\n");
                            hid_cid = 0;
                            break;
                        case HID_SUBEVENT_CAN_SEND_NOW:
                            mousing_can_send_now();
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    ESP_LOGI("PACKET_HANDLER", "EVENT: [0x%0X]", hci_event_packet_get_type(packet)); 
                    ESP_LOGI_BUFFER("PACKET_HANDLER: EVENT: ", packet, packet_size); 
                    break;
            }
            break;
        default:
            ESP_LOGI("PACKET_HANDLER", "PT:%x CH:%x", packet_type, channel); 
            ESP_LOGI_BUFFER("PACKET_HANDLER", packet, packet_size); 
            break;
    }
}

static void l2cap_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
    ESP_LOGI("L2CAP_HANDLER", "PT:%x CH:%x", packet_type, channel); 
    ESP_LOGI_BUFFER("L2CAP_HANDLER", packet, packet_size); 
}

// void l2cap_packet_handler (void * connection, uint8_t packet_type,
// uint16_t channel, uint8_t *packet, uint16_t size){
//     ESP_LOGI("L2CAP", "C:%p T:%x CH:%x", connection, packet_type, channel); 
//     ESP_LOGI_BUFFER("L2CAP", packet, size); 
// }

/* @section Main Application Setup
 *
 * @text Listing MainConfiguration shows main application code.
 * To run a HID Device service you need to initialize the SDP, and to create and register HID Device record with it.
 * At the end the Bluetooth stack is started.
 */

/* LISTING_START(MainConfiguration): Setup HID Device */

int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
    (void)argc;
    (void)argv;

    // allow to get found by inquiry
    gap_discoverable_control(1);
    // use Limited Discoverable Mode; Peripheral; Pointing Device as CoD
    gap_set_class_of_device(0x2540);
    // set local name to be identified - zeroes will be replaced by actual BD ADDR
    gap_set_local_name("Nintendo RVL-CNT-01");
    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    // allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
    gap_set_allow_role_switch(true);

    // L2CAP
    l2cap_init();

// #ifdef ENABLE_BLE
//     // Initialize LE Security Manager. Needed for cross-transport key derivation
//     sm_init();
// #endif

    // SDP Server
    sdp_init();

#ifdef RAW_WIIMOTE_BUFFERS

    sdp_register_service(mote_service_buffer_1);
    sdp_register_service(mote_service_buffer_2);
    sdp_register_service(mote_service_buffer_3);

#else

    uint8_t hid_virtual_cable = 0;
    uint8_t hid_remote_wake = 1;
    uint8_t hid_reconnect_initiate = 1;
    uint8_t hid_normally_connectable = 1;

    hid_sdp_record_t hid_params = {
        // hid sevice subclass 2580 Mouse, hid counntry code 33 US
        0x2540, 33, 
        hid_virtual_cable, hid_remote_wake, 
        hid_reconnect_initiate, hid_normally_connectable,
        hid_boot_device, 
        0xFFFF, 0xFFFF, 3200,
        hid_descriptor_mouse_boot_mode,
        sizeof(hid_descriptor_mouse_boot_mode), 
        hid_device_name
    };

    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
    hid_create_sdp_record(hid_service_buffer, sdp_create_service_record_handle(), &hid_params);
    btstack_assert(de_get_len( hid_service_buffer) <= sizeof(hid_service_buffer));
    sdp_register_service(hid_service_buffer);

    // See https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers if you don't have a USB Vendor ID and need a Bluetooth Vendor ID
    // device info: BlueKitchen GmbH, product 2, version 1
    device_id_create_sdp_record(device_id_sdp_service_buffer, sdp_create_service_record_handle(), DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 2, 1);
    btstack_assert(de_get_len( device_id_sdp_service_buffer) <= sizeof(device_id_sdp_service_buffer));
    sdp_register_service(device_id_sdp_service_buffer);

#endif

    // HID Device
    hid_device_init(false, sizeof(WiiMoteHIDDescriptor), WiiMoteHIDDescriptor);
    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    l2cap_event_callback_registration.callback = &l2cap_packet_handler;
    l2cap_add_event_handler(&l2cap_event_callback_registration);

    // register for HID
    hid_device_register_packet_handler(&packet_handler);

    // ESP_LOGI("l2cap_reg_0x11", "%0X", l2cap_register_service(&l2cap_packet_handler, 0x11, 100, gap_get_security_level()));
    // ESP_LOGI("l2cap_reg_0x13", "%0X", l2cap_register_service(&l2cap_packet_handler, 0x13, 100, gap_get_security_level()));


#ifdef HAVE_BTSTACK_STDIN
    btstack_stdin_setup(stdin_process);
#endif
    // turn on!
    hci_power_control(HCI_POWER_ON);
    return 0;
}
/* LISTING_END */
/* EXAMPLE_END */