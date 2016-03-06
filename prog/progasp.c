/**
 * This is a part of nrf24le1 downloader.
 *
 * Copyright Konstantin Lyubchak, 2016
 *
 * Based on avrasp-usb downloader for nrf24le1 by MaksMS,
 *    http://homes-smart.ru/index.php/oborudovanie/bez-provodov-2-4-ggts/55-programmirovanie-nrf24le1-cherez-usbasp
 * Based on https://github.com/derekstavis/nrf24le1-libbcm2835
 *
 * The author provides no guarantees, warantees, or promises, implied or
 * otherwise. By using this software you agree to indemnify the author
 * of any damages incurred by using it.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>

#include "progasp.h"
#include "usbhelper.h"

#include "../common/defines.h"


const int defaultUsbTimeoutMs = 5000;
static usb_dev_handle * handle = NULL;

const int vid = 0x16C0;
const int pid = 0x05DC;
const char *vendor = "nrf24le1-diy";
const char *product = "nrf24le1-asp";

#define print_usb_error(func) printf("  failed to call usb method "#func"(0x%04x): %s\n", func, usb_strerror())

const char * prog_error_str(uint8_t error) {
    switch(error) {
    case ERROR_OK:
        return "ok";
    case ERROR_READY_WAIT:
        return "device is not ready";
    case ERROR_CANNOT_SET_WEN:
        return "write cannot be enabled";
    case ERROR_WEN_TEST_FAILED:
        return "write enable test failed";
    }

    return "unknown error";
}

void print_status(uint8_t fsr) {
    printf("  RDISMB: %i\n",  (fsr & FSR_RDISMB ? 1 : 0));
    printf("  INFEN: %i\n",   (fsr & FSR_INFEN ? 1 : 0));
    printf("  RDYN: %i\n",    (fsr & FSR_RDYN ? 1 : 0));
    printf("  WEN: %i\n",     (fsr & FSR_WEN ? 1 : 0));
    printf("  STP: %i\n",     (fsr & FSR_STP ? 1 : 0));
    printf("  ENDEBUG: %i\n", (fsr & FSR_ENDEBUG ? 1 : 0));
}

bool prog_init() {
    usb_init();
    handle = usbOpenDevice(vid, pid, vendor, product, NULL);
    if (!handle) {
        return false;
    }

    uint8_t buf[7];
    int res1 = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                    REQ_TEST_PROGRAMMER,
                    0x5678, 0xDEED,
                    (char*)buf, 7,
                    5000);

    if (res1 < 0) {
        print_usb_error(REQ_TURN_PROG_ON);
        return false;
    }

    if (res1 != 7 || buf[0] != ERROR_OK || buf[3] != 0x78 || buf[4] != 0x56 || buf[5] != 0xED || buf[6] != 0xDE) {
        printf("programmer self test failed\n");
        return false;
    }

    printf("avrasp firmware version %i.%i", buf[1], buf[2]);

    if (buf[1] != MAJOR_VERSION) {
        printf(" (unsupported version, expected %i.x)\n", MAJOR_VERSION);
        return false;
    } else {
        printf("\n");
    }

    printf("programmer self test passed\n");

    int res2 = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                    REQ_TURN_PROG_ON,
                    0, 0,
                    0, 0,
                    5000);

    if (res2 < 0) {
        print_usb_error(REQ_TURN_PROG_ON);
        return false;
    }

    printf("prog mode is on\n");

    return true;
}

void prog_destroy() {
    char buf[4];
    int res = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                    REQ_TURN_PROG_OFF,
                    0, 0,
                    buf, 4,
                    5000);

    if (res < 0) {
        print_usb_error(REQ_TURN_PROG_OFF);
    } else {
        printf("prog mode is off\n");
    }
}


bool test_device() {
    printf("checking nrf24le1 by toogling WEN in SFR: ");

    char buf[4];
    int res = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                    REQ_TEST_NRF,
                    0, 0,
                    buf, 4,
                    defaultUsbTimeoutMs);

    if (res < 0) {
        print_usb_error(REQ_TEST_NRF);
        return false;
    }

    if (res < 1) {
        printf("FAILED: wrong result, expected at least 1 byte\n");
        return false;
    }

    if (buf[0] != ERROR_OK) {
        printf("FAILED: %s\n", prog_error_str(buf[0]));
    } else {
        printf("\nSUCCEEDED\n\n");
    }

    if (res == 4) {
        printf("* FSR original\n");
        print_status(buf[1]);
        printf("* FSR after WREN is set, WEN should be 1\n");
        print_status(buf[2]);
        printf("* FSR after WRDIS is set, WEN should be 0\n");
        print_status(buf[3]);
    }

    return buf[0] == ERROR_OK;
}

bool erase_program_pages() {
	printf("erasing program pages\n");

    for (int page = 0; page < 31; page++) {
        printf("  - page %i\n", page);

        usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                        REQ_ERASE_PAGE,
                        page, 0,
                        0, 0,
                        defaultUsbTimeoutMs);
    }

	printf("program pages were erased\n");

	return true;
}

bool write_program_pages(const uint8_t *buf, uint16_t len) {
    printf("writing %i bytes to programm memory\n", len);

    usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                    REQ_PROGRAM_PAGE,
                    0, len,
                    (char*) buf, len,
                    defaultUsbTimeoutMs);

    printf("program has been written\n");

    return true;
}

bool read_program_pages(uint8_t* buf, uint16_t len) {
    printf("reading %i bytes\n", len);

    usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                    REQ_READ,
                    0, len,
                    (char*) buf, len,
                    defaultUsbTimeoutMs);

    printf("\nprogram has been read\n");

    return len;
}
