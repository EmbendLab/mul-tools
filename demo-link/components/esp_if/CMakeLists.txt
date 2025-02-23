idf_component_register(SRCS "esp_if.c"
                             "DAP_handle.c"
                             "timer.c"
                            #  "uart_bridge.c"
                             "usbip_server.c"
                             "wifi_handle.c"
                    INCLUDE_DIRS "include"
                                 "../USBIP/include")
