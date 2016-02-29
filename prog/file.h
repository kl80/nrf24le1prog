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

#ifndef NRF_FILE_ROUTINES_H
#define NRF_FILE_ROUTINES_H

/**
 * Writes to .bin file
 * @param buf buffer to write
 * @param size size of buffer in bytes
 * @param fname name of file
 */
void write_to_bin_file(uint8_t * buf, int size, const char * fname);

/**
 * Reads from .bin file
 * @param buf buffer to read
 * @param buf_size size of buffer
 * @param fname name of file
 * @return -1 in case of error, size of file > 16384 or buf_size < filesize, number of bytes overwise
 */
int read_from_bin_file(uint8_t * buf, int buf_size, const char * fname);

#endif // NRF_FILE_ROUTINES_H
