#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "usbhelper.h"

static int usbGetStringAscii(usb_dev_handle *dev, int index, char *buf, int buflen)
{
    char    buffer[256];
    int     rval, i;

    if ((rval = usb_get_string_simple(dev, index, buf, buflen)) >= 0) /* use libusb version if it works */
        return rval;
    if ((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, 0x0409, buffer, sizeof(buffer), 5000)) < 0)
        return rval;

    if (buffer[1] != USB_DT_STRING) {
        *buf = 0;
        return 0;
    }

    if ((unsigned char)buffer[0] < rval) {
        rval = (unsigned char)buffer[0];
    }

    rval /= 2;
    /* lossy conversion to ISO Latin1: */
    for (i = 1; i < rval; i++) {
        if (i > buflen)              /* destination buffer overflow */
            break;
        buf[i - 1] = buffer[2 * i];
        if (buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i - 1] = '?';
    }

    buf[i - 1] = 0;
    return i - 1;
}

static bool checkDevice(usb_dev_handle *handle, struct usb_device *dev, const char *productName, const char *vendorName, const char *serialName)
{
    char vendor[256], product[256], serial[256];
    int len;

    if (vendorName != NULL) {
        len = vendor[0] = 0;
        if (dev->descriptor.iManufacturer > 0) {
            len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, vendor, sizeof(vendor));
        }

        if (len < 0) {
            printf("   warning: cannot query manufacturer %s\n", usb_strerror());
            return false;
        }

        printf("   vendor name %s\n", vendor);
        if (strncmp(vendor, vendorName, 256)) {
            printf("   warning: vendor is not corrent\n");
            return false;
        }
    }

    if (productName != NULL) {
        len = product[0] = 0;
        if (dev->descriptor.iProduct > 0){
            len = usbGetStringAscii(handle, dev->descriptor.iProduct, product, sizeof(product));
        }

        if (len < 0) {
            printf("   warning: cannot query product: %s\n", usb_strerror());
            return false;
        }

        printf("   product name %s\n", product);
        if (strncmp(product, productName, 256)) {
            printf("   warning: product is not corrent\n");
            return false;
        }
    }

    if (serialName != NULL) {
        len = serial[0] = 0;
        if (dev->descriptor.iSerialNumber > 0){
            len = usbGetStringAscii(handle, dev->descriptor.iSerialNumber, serial, sizeof(serial));
        }

        if (len < 0) {
            printf("   warning: cannot query serial: %s\n", usb_strerror());
            return false;
        }

        printf("   serial %s\n", serial);
        if (strncmp(serial, serialName, 256)) {
            printf("   warning: serial is not corrent\n");
            return false;
        }
    }

    return true;
}

usb_dev_handle* usbOpenDevice(int vendorId, int productId, const char *vendorName, const char *productName, const char *serialName) {
    printf("scanning USB devices for vid = 0x%04x, pid = 0x%04x\n", vendorId, productId);

    if (vendorName) {
        printf("  vendor name: %s\n", vendorName);
    }

    if (productName) {
        printf("  product name: %s\n", productName);
    }

    if (serialName) {
        printf("  serial name: %s\n", serialName);
    }

    usb_dev_handle *handle = NULL;

    usb_find_busses();
    usb_find_devices();

    for (struct usb_bus *bus = usb_get_busses(); bus; bus = bus->next) {
        for (struct usb_device *dev = bus->devices; dev; dev = dev->next) {
            printf(" found device vid=0x%04x, pid=0x%04x\n", dev->descriptor.idVendor, dev->descriptor.idProduct);

            if (dev->descriptor.idVendor != vendorId || dev->descriptor.idProduct != productId) {
                continue;
            }

            // we need to open the device in order to query strings
            handle = usb_open(dev);
            if (!handle) {
                printf("   warning: cannot open device: %s\n", usb_strerror());
                continue;
            }

            if (checkDevice(handle, dev, productName, vendorName, serialName)) {
                break;
            }

            usb_close(handle);
            handle = NULL;
        }
    }

    if (handle == NULL) {
        printf(" error: cannot find device\n");
    } else {
        printf(" programmer has been found\n");
    }

    return handle;
}
