#include "sdkconfig.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "wifi_configuration.h"
#include "uart_bridge.h"

#include "../DAP/include/gpio_op.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "../../components/components/esp_wifi/include/esp_wifi.h"
//#include "esp_event_loop.h"
#include "esp_log.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #define PIN_LED_WIFI_STATUS 15
#elif defined CONFIG_IDF_TARGET_ESP32
    #define PIN_LED_WIFI_STATUS 27
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #define PIN_LED_WIFI_STATUS 10
#else
    #define PIN_LED_WIFI_STATUS 10
#endif

static EventGroupHandle_t wifi_event_group;
static int ssid_index = 0;

const int IPV4_GOTIP_BIT = BIT0;
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif
static const char *TAG = "wifi_event_handler";
static void ssid_change();
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                ESP_LOGI(TAG, "Wi-Fi STA started");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Wi-Fi STA connected");
                xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
                break;
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGI(TAG, "Wi-Fi STA disconnected, reason: %d", disconnected->reason);
                break;
            }
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
                ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
                break;
            }
            default:
                break;
        }
    }
}

static void ssid_change() {
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

static void wait_for_ip() {
#ifdef CONFIG_EXAMPLE_IPV6
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT;
#else
    uint32_t bits = IPV4_GOTIP_BIT;
#endif

    os_printf("Waiting for AP connection...\r\n");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    os_printf("Connected to AP\r\n");
}

void wifi_init(void) {
//     GPIO_FUNCTION_SET(PIN_LED_WIFI_STATUS);
//     GPIO_SET_DIRECTION_NORMAL_OUT(PIN_LED_WIFI_STATUS);

//     tcpip_adapter_init();

// #if (USE_STATIC_IP == 1)
//     tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_STA);

//     tcpip_adapter_ip_info_t ip_info;

// #define MY_IP4_ADDR(...) IP4_ADDR(__VA_ARGS__)
//     MY_IP4_ADDR(&ip_info.ip, DAP_IP_ADDRESS);
//     MY_IP4_ADDR(&ip_info.gw, DAP_IP_GATEWAY);
//     MY_IP4_ADDR(&ip_info.netmask, DAP_IP_NETMASK);
// #undef MY_IP4_ADDR

//     tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
// #endif // (USE_STATIC_IP == 1)

//     wifi_event_group = xEventGroupCreate();

//     ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//     ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

//     // os_printf("Setting WiFi configuration SSID %s...\r\n", wifi_config.sta.ssid);
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
//     ssid_change();
//     ESP_ERROR_CHECK(esp_wifi_start());


//     wait_for_ip();
}
