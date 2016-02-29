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
#include <errno.h>

#include "file.h"

void write_to_bin_file(uint8_t * buf, int size, const char * fname) {
    printf("writing %i bytes to  %s", size, fname);

    FILE * fd = fopen(fname, "w");
    if (fd) {
        int res = fwrite(buf, 1, size, fd);
        if (res != size) {
            printf(" - error: %s\n", strerror(errno));
        } else {
            printf(" - ok\n");
        }

        fclose(fd);
    } else {
        printf(" - error: %s\n", strerror(errno));
    }
}

int read_from_bin_file(uint8_t * buf, int buf_size, const char * fname) {
    printf("reading from %s ", fname);
    FILE * fd = fopen(fname, "r");
    if (fd == NULL) {
        printf(" - error: %s\n", strerror(errno));
        return -1;
    }

    uint8_t ibuf[16385];
    int readed = fread(ibuf, 1, 16384 + 1, fd);
    if (readed > 16384) {
        printf(" - files size exceeвed 16384 bytes\n");
        readed = -1;
    } else if (buf_size < readed) {
            printf(" - internal error, buffer (%i bytes) less then readed data (%i bytes)\n", buf_size, readed);
    } else {
        memcpy(buf, ibuf, readed);
        printf(" - readed %i bytes\n", readed);
    }

    fclose(fd);

    return readed;
}
