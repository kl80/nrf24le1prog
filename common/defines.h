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
 
#ifndef DEFINES_H
#define DEFINES_H

#define MAJOR_VERSION 1
#define MINOR_VERSION 0

#define REQ_TEST_PROGRAMMER      0
#define REQ_TURN_PROG_ON         1
#define REQ_TURN_PROG_OFF        2
#define REQ_TEST_NRF             3
#define REQ_READ                 4
#define REQ_PROGRAM_PAGE         5
#define REQ_ERASE_PAGE           6

#define ERROR_OK               0
#define ERROR_READY_WAIT       1
#define ERROR_CANNOT_SET_WEN   2
#define ERROR_WEN_TEST_FAILED  3


// SPI commands
#define SPICMD_WREN      0x06
#define SPICMD_WRDIS     0x04
#define SPICMD_RDSR      0x05
#define SPICMD_WRSR      0x01
#define SPICMD_READ      0x03
#define SPICMD_PROGRAM   0x02
#define SPICMD_ERASEPAGE 0x52
#define SPICMD_ERASEALL  0x62
#define SPICMD_RDFPCR    0x89
#define SPICMD_RDISMB    0x85
#define SPICMD_ENDEBUG   0x86

// status flags
#define FSR_RESERVED0 (1 << 0)
#define FSR_RESERVED1 (1 << 1)
#define FSR_RDISMB    (1 << 2)
#define FSR_INFEN     (1 << 3)
#define FSR_RDYN      (1 << 4)
#define FSR_WEN       (1 << 5)
#define FSR_STP       (1 << 6)
#define FSR_ENDEBUG   (1 << 7)

#endif // DEFINES_H
