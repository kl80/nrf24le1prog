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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file.h"
#include "progasp.h"

#include "../common/defines.h"

static void usage(char *name) {
    fprintf(stderr, " %s test                     - check hardware by triggering WEN flag\n", name);
    fprintf(stderr, " %s write code <filename>    - write code memory from file\n", name);
    fprintf(stderr, " %s read code <filename>     - read code memory to file\n", name);
    fprintf(stderr, " %s validate code <filename> - compare file with code memory\n", name);
    fprintf(stderr, " %s erase code               - erase programm memory.\n", name);
}

bool validate(uint8_t * buf, int size) {
    printf("validate:\n");

    uint8_t bufread[16384];
    memset(bufread, 0, size);
    if (!read_program_pages(bufread, size)) {
        return false;
    }

    bool valid = true;
    for (uint16_t i = 0; i < size; i++) {
        if (buf[i] != bufread[i]) {
            valid = false;
            break;
        }
    }

    printf("\nvalidation %s\n", (valid ? "succeeded" : "FAILED"));
    return valid;
}

bool write_code(const char * fname) {
    uint8_t buf[16384];
    int16_t size = read_from_bin_file(buf, 16384, fname);
    if (size < 0) {
        return false;
    }

    if (!write_program_pages(buf, size)) {
        return false;
    }

    return validate(buf, size);
}

bool read_code(const char * fname) {
    uint8_t buf[16384];
    memset(buf, 0, sizeof(buf));
    read_program_pages(buf, sizeof(buf));
    write_to_bin_file(buf, sizeof(buf), fname);
    return true;
}

bool validate_code(const char * fname) {
    uint8_t buf[16384];
    int16_t size = read_from_bin_file(buf, 16384, fname);
    if (size < 0) {
        return false;
    }

    return validate(buf, size);
}

int main(int argc, char ** argv)
{
    printf("nrf24le1 USBasp programmer %i.%i. Based on http://homes-smart.ru, https://github.com/derekstavis/nrf24le1-libbcm2835\n",
           MAJOR_VERSION, MINOR_VERSION);

    if (argc < 2) {   /* we need at least one argument */
        usage(argv[0]);
        return 1;
    }

    if (!prog_init()) {
        return 1;
    }

    sleep(1); // sometimes usb doesn't work without delay

    bool retcode = true;
    if (!strcasecmp(argv[1], "test")) {
        retcode = test_device();
    } else if (argc > 2 && !strcasecmp(argv[2], "code")) {
        if (!strcasecmp(argv[1], "write")) {
            retcode = write_code(argc > 3 ? argv[3] : "./main.bin");
        } else if (!strcasecmp(argv[1], "read")) {
            retcode = read_code(argc > 3 ? argv[3] : "./main-dump.bin");
        } else if (!strcasecmp(argv[1], "erase"))  {
            retcode = erase_program_pages() ;
        } else if (!strcasecmp(argv[1], "validate")) {
            retcode = validate_code(argc > 3 ? argv[3] : "./main.bin");
        } else {
            usage(argv[0]);
            retcode = false;
        }
    } else {
        usage(argv[0]);
        retcode = false;
    }

    prog_destroy();

    return retcode ? 0 : 1;
}
