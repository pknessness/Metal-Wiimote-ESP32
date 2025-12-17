/*
 * Copyright (C) 2020 BlueKitchen GmbH
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
 */

/*
 *  main.c
 *
 *  Minimal main application that initializes BTstack, prepares the example and enters BTstack's Run Loop.
 *
 *  If needed, you can create other threads. Please note that BTstack's API is not thread-safe and can only be
 *  called from BTstack timers or in response to its callbacks, e.g. packet handlers.
 */

#include "btstack_port_esp32.h"
#include "btstack_run_loop.h"
#include "btstack_stdio_esp32.h"
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"

#include <stddef.h>

#include "esp_mac.h"
#include "esp_log.h"

// warn about unsuitable sdkconfig
#include "sdkconfig.h"
#if !CONFIG_BT_ENABLED
#error "Bluetooth disabled - please set CONFIG_BT_ENABLED via menuconfig -> Component Config -> Bluetooth -> [x] Bluetooth"
#endif
#if !CONFIG_BT_CONTROLLER_ONLY
#error "Different Bluetooth Host stack selected - please set CONFIG_BT_CONTROLLER_ONLY via menuconfig -> Component Config -> Bluetooth -> Host -> Disabled"
#endif
#if ESP_IDF_VERSION_MAJOR >= 5
#if !CONFIG_BT_CONTROLLER_ENABLED
#error "Different Bluetooth Host stack selected - please set CONFIG_BT_CONTROLLER_ENABLED via menuconfig -> Component Config -> Bluetooth -> Controller -> Enabled"
#endif
#endif

extern int btstack_main(int argc, const char * argv[]);

#define ENABLE_LOG_BTSTACK_EVENTS

int app_main(void){


    uint8_t baseMac[6];

    esp_base_mac_addr_get(baseMac);
    // baseMac[0] = 0x2C;
    // baseMac[1] = 0x10;
    // baseMac[2] = 0xC1;
    baseMac[0] = 0x00;
    baseMac[1] = 0x1F;
    baseMac[2] = 0xC5;
    if (esp_base_mac_addr_set(baseMac) == ESP_OK) {
        ESP_LOGI("MAC_SET", "MAC address set successfully: %X:%X:%X:%X:%X:%X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    } else {
        ESP_LOGI("MAC_SET", "Failed to set MAC");
    }

    // optional: enable packet logger
    // hci_dump_init(hci_dump_embedded_stdout_get_instance());

#ifdef CONFIG_ESP_CONSOLE_UART
    // Enable buffered stdout
    btstack_stdio_init();
#endif

    // Configure BTstack for ESP32 VHCI Controller
    btstack_init();

    // Setup example
    btstack_main(0, NULL);

    // Enter run loop (forever)
    btstack_run_loop_execute();

    return 0;
}
