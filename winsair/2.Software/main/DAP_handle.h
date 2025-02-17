#ifndef __DAP_HANDLE_H__
#define __DAP_HANDLE_H__



enum reset_handle_t
{
    NO_SIGNAL = 0,
    RESET_HANDLE = 1,
    DELETE_HANDLE = 2,
};


void handle_dap_unlink();

int fast_reply(uint8_t *buf, uint32_t length);

#endif