/**
 * @file spi_op.c
 * @author windowsair
 * @brief Using SPI for common transfer operations
 * @change: 2020-11-25 first version
 *          2021-2-11 Support SWD sequence
 *          2021-3-10 Support 3-wire SPI
 *          2022-9-15 Support ESP32C3
 * @version 0.4
 * @date 2022-9-15
 *
 * @copyright MIT License
 *
 */
#include "sdkconfig.h"

#include <stdio.h>
#include <stdbool.h>


#include "dap_configuration.h" 

#include "cmsis_compiler.h"
#include "spi_op.h"
#include "spi_switch.h"
#include "gpio_common.h"
#include "../../components/components/soc/esp32c6/include/soc/spi_struct.h"

//spi_dev_t GPSPI2;
//#define DAP_SPI GPSPI2
spi_dev_t DAP_SPI;

#define SET_MOSI_BIT_LEN(x) DAP_SPI.ms_dlen.ms_data_bitlen = x
#define SET_MISO_BIT_LEN(x) DAP_SPI.ms_dlen.ms_data_bitlen = x
#define START_AND_WAIT_SPI_TRANSMISSION_DONE() \
    do {                                       \
        DAP_SPI.cmd.update = 1;                \
        while (DAP_SPI.cmd.update) continue;   \
        DAP_SPI.cmd.usr = 1;                   \
        while (DAP_SPI.cmd.usr) continue;      \
    } while(0)        


/**
 * @brief Calculate integer division and round up
 *
 * @param A
 * @param B
 * @return result
 */
__STATIC_FORCEINLINE int div_round_up(int A, int B)
{
    return (A + B - 1) / B;
}


/**
 * @brief Write bits. LSB & little-endian
 *        Note: No check. The pointer must be valid.
 * @param count Number of bits to be written (<= 64 bits, no length check)
 * @param buf Data Buf
 */
void DAP_SPI_WriteBits(const uint8_t count, const uint8_t *buf)
{
    DAP_SPI.user.usr_command = 0;
    DAP_SPI.user.usr_addr = 0;

    // have data to send
    DAP_SPI.user.usr_mosi = 1;
    DAP_SPI.user.usr_miso = 0;
    SET_MOSI_BIT_LEN(count - 1);
    // copy data to reg
    switch (count)
    {
    case 8:
        DAP_SPI.data_buf[0].val = (buf[0] << 0) | (0U << 8) | (0U << 16) | (0U << 24);
        break;
    case 16:
        DAP_SPI.data_buf[0].val = (buf[0] << 0) | (buf[1] << 8) | (0x000U << 16) | (0x000U << 24);
        break;
    case 33: // 32bits data & 1 bit parity
        DAP_SPI.data_buf[0].val = (buf[0] << 0) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
        DAP_SPI.data_buf[1].val = (buf[4] << 0) | (0x000U << 8) | (0x000U << 16) | (0x000U << 24);
        break;
    case 51: // for line reset
        DAP_SPI.data_buf[0].val = (buf[0] << 0) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
        DAP_SPI.data_buf[1].val = (buf[4] << 0) | (buf[5] << 8) | (buf[2] << 16) | (0x000U << 24);
        break;
    default:
    {
        uint32_t data_buf[2];
        uint8_t *pData = (uint8_t *)data_buf;
        int i;

        for (i = 0; i < div_round_up(count, 8); i++)
        {
            pData[i] = buf[i];
        }
        // last byte use mask:
        pData[i-1] = pData[i-1] & ((2U >> (count % 8)) - 1U);

        DAP_SPI.data_buf[0].val = data_buf[0];
        DAP_SPI.data_buf[1].val = data_buf[1];
    }
    }

    START_AND_WAIT_SPI_TRANSMISSION_DONE();
}



/**
 * @brief Read bits. LSB & little-endian
 *        Note: No check. The pointer must be valid.
 * @param count Number of bits to be read (<= 64 bits, no length check)
 * @param buf Data Buf
 */
void DAP_SPI_ReadBits(const uint8_t count, uint8_t *buf) {
    int i;
    uint32_t data_buf[2];

    uint8_t * pData = (uint8_t *)data_buf;

    DAP_SPI.user.usr_mosi = 0;
    DAP_SPI.user.usr_miso = 1;

#if (USE_SPI_SIO == 1)
    DAP_SPI.user.sio = true;
#endif

    SET_MISO_BIT_LEN(count - 1U);

    START_AND_WAIT_SPI_TRANSMISSION_DONE();

#if (USE_SPI_SIO == 1)
    DAP_SPI.user.sio = false;
#endif

    data_buf[0] = DAP_SPI.data_buf[0].val;
    data_buf[1] = DAP_SPI.data_buf[1].val;

    for (i = 0; i < div_round_up(count, 8); i++)
    {
        buf[i] = pData[i];
    }
    // last byte use mask:
    buf[i-1] = buf[i-1] & ((2 >> (count % 8)) - 1);
}



__FORCEINLINE void DAP_SPI_Send_Header(const uint8_t packetHeaderData, uint8_t *ack, uint8_t TrnAfterACK)
{
    uint32_t dataBuf;

    // have data to send
    DAP_SPI.user.usr_mosi = 0;
    DAP_SPI.user.usr_command = 1;
    DAP_SPI.user.usr_miso = 1;

    // 8bits Header + 1 bit Trn(Before ACK) - 1(prescribed)
    DAP_SPI.user2.usr_command_bitlen = 8U + 1U - 1U;
    DAP_SPI.user2.usr_command_value = packetHeaderData;


#if (USE_SPI_SIO == 1)
    DAP_SPI.user.sio = true;
#endif

    // 3bits ACK + TrnAferACK  - 1(prescribed)
    SET_MISO_BIT_LEN(3U + TrnAfterACK - 1U);

    START_AND_WAIT_SPI_TRANSMISSION_DONE();

#if (USE_SPI_SIO == 1)
    DAP_SPI.user.sio = false;
#endif

    DAP_SPI.user.usr_command = 0;

    dataBuf = DAP_SPI.data_buf[0].val;
    *ack = dataBuf & 0b111;
}



/**
 * @brief Step2: Read Data
 *
 * @param resData data from target
 * @param resParity parity from target
 */
__FORCEINLINE void DAP_SPI_Read_Data(uint32_t *resData, uint8_t *resParity)
{
    volatile uint64_t dataBuf;
    uint32_t *pU32Data = (uint32_t *)&dataBuf;

    DAP_SPI.user.usr_mosi = 0;
    DAP_SPI.user.usr_miso = 1;

#if (USE_SPI_SIO == 1)
    DAP_SPI.user.sio = true;
#endif

    // 1 bit Trn(End) + 32bis data + 1bit parity - 1(prescribed)
    SET_MISO_BIT_LEN(1U + 32U + 1U - 1U);

    START_AND_WAIT_SPI_TRANSMISSION_DONE();

#if (USE_SPI_SIO == 1)
    DAP_SPI.user.sio = false;
#endif

    pU32Data[0] = DAP_SPI.data_buf[0].val;
    pU32Data[1] = DAP_SPI.data_buf[1].val;

    *resData = (dataBuf >> 0U) & 0xFFFFFFFFU;  // 32bits Response Data
    *resParity = (dataBuf >> (0U + 32U)) & 1U; // 1bit parity
}



__FORCEINLINE void DAP_SPI_Write_Data(uint32_t data, uint8_t parity)
{
    DAP_SPI.user.usr_mosi = 1;
    DAP_SPI.user.usr_miso = 0;

    // esp32c3 can not send 33 bits of data correctly, we need to send an additional bit
    // that will not be recognized as the start bit.
    SET_MOSI_BIT_LEN(32U + 1U + 1U - 1U);
    DAP_SPI.data_buf[0].val = data;
    DAP_SPI.data_buf[1].val = parity == 0 ? 0b00 : 0b01;

    START_AND_WAIT_SPI_TRANSMISSION_DONE();
}




__FORCEINLINE void DAP_SPI_Generate_Cycle(uint8_t num)
{
    //// TODO: It may take long time to generate just one clock
    DAP_SPI.user.usr_mosi = 0;
    DAP_SPI.user.usr_miso = 1;

    // esp32c3 can not send a single bit, therefore we use read operation instead.
    SET_MISO_BIT_LEN(num - 1U);

    START_AND_WAIT_SPI_TRANSMISSION_DONE();
}



__FORCEINLINE void DAP_SPI_Fast_Cycle()
{
    DAP_SPI_Release();
    DAP_SPI_Acquire();
}


/**
 * @brief Generate Protocol Error Cycle
 *
 */
__FORCEINLINE void DAP_SPI_Protocol_Error_Read()
{
    DAP_SPI.user.usr_mosi = 1;
    DAP_SPI.user.usr_miso = 0;
    SET_MOSI_BIT_LEN(32U + 1U - 1); // 32bit ignore data + 1 bit - 1(prescribed)

    DAP_SPI.data_buf[0].val = 0xFFFFFFFFU;
    DAP_SPI.data_buf[1].val = 0xFFFFFFFFU;

    START_AND_WAIT_SPI_TRANSMISSION_DONE();
}


/**
 * @brief Generate Protocol Error Cycle
 *
 */
__FORCEINLINE void DAP_SPI_Protocol_Error_Write()
{
    DAP_SPI.user.usr_mosi = 1;
    DAP_SPI.user.usr_miso = 0;
    SET_MOSI_BIT_LEN(1U + 32U + 1U - 1); // 1bit Trn + 32bit ignore data + 1 bit - 1(prescribed)

    DAP_SPI.data_buf[0].val = 0xFFFFFFFFU;
    DAP_SPI.data_buf[1].val = 0xFFFFFFFFU;

    START_AND_WAIT_SPI_TRANSMISSION_DONE();
}
