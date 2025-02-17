/**
 * @file DAP_handle.c
 * @brief Handle DAP packets and transaction push
 * @version 0.5
 * @change: 2020.02.04 first version
 *          2020.11.11 support WinUSB mode
 *          2021.02.17 support SWO
 *          2021.10.03 try to handle unlink behavior
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdint.h>
#include <string.h>


#include "main/DAP_handle.h"
#include "main/dap_configuration.h"
#include "main/wifi_configuration.h"




#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#if ((USE_MDNS == 1) || (USE_OTA == 1))
    #define DAP_BUFFER_NUM 10
#else
    #define DAP_BUFFER_NUM 20
#endif

#if (USE_WINUSB == 1)
typedef struct
{
    uint32_t length;
    uint8_t buf[DAP_PACKET_SIZE];
} DapPacket_t;
#else
typedef struct
{
    uint8_t buf[DAP_PACKET_SIZE];
} DapPacket_t;
#endif

#define DAP_HANDLE_SIZE (sizeof(DapPacket_t))


extern int kSock;
extern TaskHandle_t kDAPTaskHandle;

int kRestartDAPHandle = NO_SIGNAL;


static DapPacket_t DAPDataProcessed;
static int dap_respond = 0;

// SWO Trace
static uint8_t *swo_data_to_send = NULL;
static uint32_t swo_data_num;

// DAP handle
static RingbufHandle_t dap_dataIN_handle = NULL;
static RingbufHandle_t dap_dataOUT_handle = NULL;
static SemaphoreHandle_t data_response_mux = NULL;


void malloc_dap_ringbuf() {
    if (data_response_mux && xSemaphoreTake(data_response_mux, portMAX_DELAY) == pdTRUE)
    {
        if (dap_dataIN_handle == NULL) {
            dap_dataIN_handle = xRingbufferCreate(DAP_HANDLE_SIZE * DAP_BUFFER_NUM, RINGBUF_TYPE_BYTEBUF);
        }
        if (dap_dataOUT_handle == NULL) {
            dap_dataOUT_handle = xRingbufferCreate(DAP_HANDLE_SIZE * DAP_BUFFER_NUM, RINGBUF_TYPE_BYTEBUF);
        }

        xSemaphoreGive(data_response_mux);
    }
}

void free_dap_ringbuf() {
    if (data_response_mux && xSemaphoreTake(data_response_mux, portMAX_DELAY) == pdTRUE) {
        if (dap_dataIN_handle) {
            vRingbufferDelete(dap_dataIN_handle);
        }
        if (dap_dataOUT_handle) {
            vRingbufferDelete(dap_dataOUT_handle);
        }

        dap_dataIN_handle = dap_dataOUT_handle = NULL;
        xSemaphoreGive(data_response_mux);
    }

}








// SWO Data Queue Transfer
//   buf:    pointer to buffer with data
//   num:    number of bytes to transfer
void SWO_QueueTransfer(uint8_t *buf, uint32_t num)
{
    swo_data_to_send = buf;
    swo_data_num = num;
}



void handle_dap_unlink()
{
    // `USBIP_CMD_UNLINK` means calling `usb_unlink_urb()` or `usb_kill_urb()`.
    // Note that execution of an URB is inherently an asynchronous operation, and there may be
    // synchronization problems in the following solutions.

    // One of the reasons this happens is that the host wants to abort the URB transfer operation
    // as soon as possible. USBIP network fluctuations will also cause this error, but I don't know
    // whether this is the main reason.

    // Unlink may be applied to zero length URB of "DIR_IN", or a URB containing data.
    // In the case of unlink, for the new "DIR_IN" request, it may always return an older response,
    // which will lead to panic. This code is a compromise for eliminating the lagging response
    // caused by UNLINK. It will clean up the buffers that may have data for return to the host.
    // In general, we assume that there is at most one piece of data that has not yet been returned.
    if (dap_respond > 0)
    {
        DapPacket_t *item;
        size_t packetSize = 0;
        item = (DapPacket_t *)xRingbufferReceiveUpTo(dap_dataOUT_handle, &packetSize,
                                                     pdMS_TO_TICKS(10), DAP_HANDLE_SIZE);
        if (packetSize == DAP_HANDLE_SIZE)
        {
            if (xSemaphoreTake(data_response_mux, portMAX_DELAY) == pdTRUE)
            {
                --dap_respond;
                xSemaphoreGive(data_response_mux);
            }

            vRingbufferReturnItem(dap_dataOUT_handle, (void *)item);
        }
    }
}