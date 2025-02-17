#include "sdkconfig.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/wifi_configuration.h"
#include "main/uart_bridge.h"
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"



#define PIN_LED_WIFI_STATUS 10


static EventGroupHandle_t wifi_event_group;
static int ssid_index = 0;

const int IPV4_GOTIP_BIT = BIT0;

void ssid_change() {
    if (ssid_index > WIFI_LIST_SIZE - 1) {
        ssid_index = 0;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };

    strcpy((char *)wifi_config.sta.ssid, wifi_list[ssid_index].ssid);
    strcpy((char *)wifi_config.sta.password, wifi_list[ssid_index].password);
    ssid_index++;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
}


static EventGroupHandle_t wifi_event_group;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI("WIFI", "Disconnect reason : %d", event->reason);
        ssid_change();
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        ESP_LOGI("WIFI", "Got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
    }
}

static void wait_for_ip() {
    uint32_t bits = IPV4_GOTIP_BIT;
    ESP_LOGI("WIFI", "Waiting for AP connection...");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    ESP_LOGI("WIFI", "Connected to AP");
}

void wifi_init(void) {
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    wait_for_ip();
}
