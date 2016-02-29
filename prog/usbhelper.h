#ifndef NRF_USB_ROUTINES_H
#define NRF_USB_ROUTINES_H

#include <usb.h>

usb_dev_handle* usbOpenDevice(
                  int vendorID,
                  int productID,
                  const char *vendorName,
                  const char *productName,
                  const char *serialName);


#endif // NRF_USB_ROUTINES_H
