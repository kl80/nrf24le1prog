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

#ifndef NRF_PROG_ASP_H
#define NRF_PROG_ASP_H

#include <stdbool.h>
#include <stdlib.h>

bool prog_init();
void prog_destroy();

bool test_device();

bool erase_program_pages();

bool read_program_pages(uint8_t* buf, uint16_t count);
bool write_program_pages(const uint8_t* buf, uint16_t count);

#endif // NRF_PROG_ASP_H
