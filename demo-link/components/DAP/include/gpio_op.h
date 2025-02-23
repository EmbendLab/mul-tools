#ifndef __GPIO_OP_H__
#define __GPIO_OP_H__

#include "sdkconfig.h"
#include "cmsis_compiler.h"
#include "gpio_common.h"


__STATIC_INLINE __UNUSED void GPIO_FUNCTION_SET(int io_num)
{
  // Disable USB Serial JTAG if pins 18 or pins 19 needs to select an IOMUX function
  if (io_num == IO_MUX_GPIO18_REG || io_num == IO_MUX_GPIO19_REG) {
      CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_USB_PAD_ENABLE);
  }
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[io_num], PIN_FUNC_GPIO);
}





__STATIC_INLINE __UNUSED void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
    GPIO.enable_w1ts.enable_w1ts = (0x1 << io_num);
    // PP out
    GPIO.pin[io_num].pad_driver = 0;
}





__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_HIGH(int io_num)
{
  gpio_ll_set_level(&GPIO, io_num, 1);
}
__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_LOW(int io_num)
{
  gpio_ll_set_level(&GPIO, io_num, 0);
}




__STATIC_INLINE __UNUSED int GPIO_GET_LEVEL(int io_num)
{
  return gpio_ll_get_level(&GPIO, io_num);
}






__STATIC_INLINE __UNUSED void GPIO_PULL_UP_ONLY_SET(int io_num)
{
  // disable pull down
  REG_CLR_BIT(GPIO_PIN_MUX_REG[io_num], FUN_PD);
  // enable pull up
  REG_SET_BIT(GPIO_PIN_MUX_REG[io_num], FUN_PU);
}





#endif