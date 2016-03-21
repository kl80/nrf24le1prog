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
#include <stdbool.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <util/delay.h> 

#include "../common/defines.h"

#include "usbdrv/usbdrv.h"

// master SPI ports
#define SPI_PORTX  PORTB
#define SPI_DDRX   DDRB
#define SPI_MISO   4 
#define SPI_MOSI   3
#define SPI_SCK    5
#define SPI_SS     2

uint8_t progMode;
uint16_t redLedBlinkCount;

#define USB_OUTPUT_BUFFER_SIZE 32
uchar usbOutputBuffer[USB_OUTPUT_BUFFER_SIZE]; 

uint8_t reading;
uint16_t readBytesRemaining;
uint16_t writeBytesRemaining;
uint16_t wrote;
uint8_t currentPage;

// LEDs

void initLeds() {
    // setup LED control, !!! 1 is off !!!
    DDRC = 0x03;
    PORTC = 0x03;
}

inline void turnRedLedOn() {
    PORTC &= 0xFE;
}

inline void turnRedLedOff() {
    PORTC |= 1;
}

inline void turnYellowLedOn() {
    PORTC &= 0xFD;
}

inline void turnYellowLedOff() {
    PORTC |= 2;
}

// -----------------------------------
// SPI

inline void spiEnable() {
    turnYellowLedOn();
    SPI_PORTX &= ~(1 << SPI_SS);
}

inline void spiDisable() {
    SPI_PORTX |= (1 << SPI_SS);
    turnYellowLedOff();
}

inline uint8_t spiReadWriteByte(uint8_t value) {
    SPDR = value;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}

inline uint8_t spiWriteThenReadByte(uint8_t value) {
    SPDR = value;
    while (!(SPSR & (1 << SPIF)));

    SPDR = 0;
    while (!(SPSR & (1 << SPIF)));

    return SPDR;
}

// -----------------------------------
// utility methods

uint8_t readStatus() {
    uint8_t status;

    spiEnable();
    spiReadWriteByte(SPICMD_RDSR);
    status = spiReadWriteByte(0);
    spiDisable();

    return status;
}

bool enableWrite() {
    uint8_t status;
    uint8_t attempts;

    for (attempts = 0; attempts < 10; ++attempts) {
        // set wren
        spiEnable();
        spiReadWriteByte(SPICMD_WREN);
        spiDisable();

        // then check that it's set
        status = readStatus();

        if (status & FSR_WEN) {
            return true;
        }

        _delay_ms(2);
    }

    return false;
}

bool waitWenIsCleared() {
    uint8_t status;
    uint8_t attempts;

    for (attempts = 0; attempts < 10; ++attempts) {
        status = readStatus();

        if (!(status & FSR_WEN)) {
            return true;
        }

        _delay_ms(2);
    }

    return false;
}

bool waitForReady() {
    uint8_t status;
    uint8_t attempt;

    for (attempt = 0; attempt < 10; ++attempt) {
        status = readStatus();

        if (!(status & FSR_RDYN)) {
            return true;
        }

        _delay_ms(2);
    }

    return false;
}

bool checkWen() {
    uint8_t s1, s2, s3;

    s1 = readStatus();

    spiEnable();
    spiReadWriteByte(SPICMD_WREN);
    spiDisable();

    s2 = readStatus();

    spiEnable();
    spiReadWriteByte(SPICMD_WRDIS);
    spiDisable();

    s3 = readStatus();

    if (!(s1 & FSR_WEN)
            && (s2 & FSR_WEN)
            && !(s3 & FSR_WEN)) {
        return true;
    }

    return false;
}

void nrfReset() {
    // pulse reset for minimum 0.2 us as per datasheet
    PORTD &= 0xFE;
    _delay_ms(5);
    PORTD |= 1;
    _delay_ms(10);
}


// ---------- erase page -------------

usbMsgLen_t erasePage(uint8_t page) {
    if (page > 31) {
        return 0;
    }

    if (!waitForReady()) {
        return 0;
    }

    if (!enableWrite()) {
        return 0;
    }

    spiEnable();
    spiReadWriteByte(SPICMD_ERASEPAGE);
    spiReadWriteByte(page);
    spiDisable();

    waitWenIsCleared();

    return 0;
}

// ---------- read memory ----------

usbMsgLen_t readMemory(uint16_t startAddr, uint16_t size) {
    spiEnable();

    spiReadWriteByte(SPICMD_READ);
    spiReadWriteByte(startAddr >> 8);
    spiReadWriteByte(startAddr);

    readBytesRemaining = size;
    reading = 1;

    return USB_NO_MSG;
}

uint8_t usbFunctionRead(uint8_t *data, uint8_t len) {
    uint8_t i;

    if (!reading) {
        return 0;
    }

    if (len > readBytesRemaining) {
        len = readBytesRemaining;
    }

    for (i = 0; i < len; i++) {
        data[i] = spiReadWriteByte(0);
    }

    readBytesRemaining -= len;
    if (readBytesRemaining == 0 && reading) {
        spiDisable();
    }

    return len;
}

// ---------- write memory ----------

usbMsgLen_t programPage(uint16_t page, uint16_t len) {
    if (page > 31) {
        return 0;
    }

    if (!waitForReady()) {
        return 0;
    }

    currentPage = page;

    writeBytesRemaining = len;
    reading = 0;

    return USB_NO_MSG;
}

uint8_t usbFunctionWrite(uint8_t *data, uint8_t len) {
    uint8_t i;
    uint16_t startAddr;

    if (reading) {
        return 1;
    }

    if (len > writeBytesRemaining) {
        len = writeBytesRemaining;
    }

    if (wrote == 0) {
        if (!enableWrite()) {
            return 0;
        }

        spiEnable();
        spiReadWriteByte(SPICMD_ERASEPAGE);
        spiReadWriteByte(currentPage);
        spiDisable();

        // wait WEN flag is cleared and flash is ready
        if (!waitWenIsCleared()) {
            return 0;
        }

        // enable WEN flag
        if (!enableWrite()) {
            return 0;
        }

        startAddr = currentPage * 512;

        // start writing
        spiEnable();
        spiReadWriteByte(SPICMD_PROGRAM);
        spiReadWriteByte(startAddr >> 8);
        spiReadWriteByte(startAddr);
    }

    for (i = 0; i < len; i++) {
        spiReadWriteByte(data[i]);
        wrote++;
    }

    writeBytesRemaining -= len;

    if (wrote == 512 || writeBytesRemaining == 0) {
        wrote = 0;
        spiDisable();
        waitWenIsCleared();

        ++currentPage;
    }

    if (writeBytesRemaining == 0) {
        return 1;
    }

    return 0;
}

// ------- test nrf24le1 --------

usbMsgLen_t nrfTest() {
    if (!waitForReady()) {
        usbOutputBuffer[0] = ERROR_READY_WAIT;
        usbOutputBuffer[1] = usbOutputBuffer[2] = usbOutputBuffer[3] = readStatus();
        return 4;
    }

    spiEnable();
    usbOutputBuffer[1] = readStatus();
    spiDisable();

    spiEnable();
    spiReadWriteByte(SPICMD_WREN);
    spiDisable();

    spiEnable();
    usbOutputBuffer[2] = readStatus();
    spiDisable();

    spiEnable();
    spiReadWriteByte(SPICMD_WRDIS);
    spiDisable();

    spiEnable();
    usbOutputBuffer[3] = readStatus();
    spiDisable();

    if (!(usbOutputBuffer[1] & FSR_WEN) && (usbOutputBuffer[2] & FSR_WEN) && !(usbOutputBuffer[3] & FSR_WEN)) {
        usbOutputBuffer[0] = ERROR_OK;
    } else {
        usbOutputBuffer[0] = ERROR_WEN_TEST_FAILED;
    }

    return 4;
}

usbMsgLen_t turnProgOn() {
    // setup master SPI for programming
    SPI_DDRX = (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS) | (0 << SPI_MISO);
    SPI_PORTX = (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS) | (0 << SPI_MISO);

    SPCR = (1 << SPE) | (0 << DORD) | (1 << MSTR) | (0 << CPOL) | (0 << CPHA) | (0 << SPR1) | (0 << SPR0);
    SPSR = (1 << SPI2X);

    // prog pin to 1
    PORTD |= 2;

    // reset to stop executing of current flow, overwise programming could be unsuccessful
    nrfReset();

    progMode = 1;
    turnRedLedOn();
    return 0;
}

usbMsgLen_t turnProgOff() {
    SPI_DDRX = (0 << SPI_MOSI) | (0 << SPI_SCK) | (0 << SPI_SS) | (0 << SPI_MISO);
    SPI_PORTX = (0 << SPI_MOSI) | (0 << SPI_SCK) | (0 << SPI_SS) | (0 << SPI_MISO);

    // prog pin to 0
    PORTD &= 0xFD;

    nrfReset();

    progMode = 0;
    turnRedLedOff();
    return 0;
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (void *)data;

    usbMsgPtr = usbOutputBuffer;

    if (rq->bRequest == REQ_TEST_PROGRAMMER) {
        usbOutputBuffer[0] = ERROR_OK;
        usbOutputBuffer[1] = MAJOR_VERSION;
        usbOutputBuffer[2] = MINOR_VERSION;
        usbOutputBuffer[3] = rq->wValue.bytes[0];
        usbOutputBuffer[4] = rq->wValue.bytes[1];
        usbOutputBuffer[5] = rq->wIndex.bytes[0];
        usbOutputBuffer[6] = rq->wIndex.bytes[1];
        return 7;
    } else if (rq->bRequest == REQ_TURN_PROG_ON) {
        return turnProgOn();
    } else if (rq->bRequest == REQ_TURN_PROG_OFF) {
        return turnProgOff();
    } else if (rq->bRequest == REQ_TEST_NRF) {
        return nrfTest();
    } else if (rq->bRequest == REQ_PROGRAM_PAGE) {
        return programPage(rq->wValue.word, rq->wIndex.word);
    } else if(rq->bRequest == REQ_READ) {
        return readMemory(rq->wValue.word, rq->wIndex.word);
    } else if(rq->bRequest == REQ_ERASE_PAGE) {
        return erasePage(rq->wValue.word);
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void) {
    uint8_t i;

    initLeds();

    wdt_enable(WDTO_1S);

    // reenumerate USB on startup by disconnecting for >250ms
    usbInit();
    usbDeviceDisconnect();
    i = 0;
    while (--i) {
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    wdt_reset();

    // setup ports for RESET and PROG contolling
    DDRD |= 3;

    sei();

    for (;;) {
        // blink led in idle mode
        if (!progMode) {
            ++redLedBlinkCount;
            if (redLedBlinkCount == 0) {
                turnRedLedOn();
            } else if (redLedBlinkCount == 0xAFFF) {
                turnRedLedOff();
            }
        }

        wdt_reset();
        usbPoll();
    }
}
