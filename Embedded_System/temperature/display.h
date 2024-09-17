/*
 *  display.h
 *
 *  Created on: 24 mai 2024
 *      Author: Padi
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "font.h"


#define COL8TO5(x) (x >> 3)
#define COL8TO6(x) (x >> 2)

#define RGB(r, g, b) ((COL8TO5(r) << 11) | (COL8TO6(g) << 5) | (COL8TO5(b)))

#define GREEN RGB(0, 255, 0)
#define GREY RGB(100, 100, 100)
#define LIGHT_RED RGB(255, 120, 120)
#define BLACK RGB(0, 0, 0)

#define write_cmd(cmd) spi_write_cmd(disp, cmd)
#define write_data(data) spi_write_data(disp, data, sizeof(data))

#define XRES 240
#define YRES 320

#define SPIDEVNODE "/dev/spidev1.0"

#define SPI_CMD_DELAY 30

typedef struct
{
    int fd_spi;
    struct spi_ioc_transfer cmd;
} spi_display_t;

int gpio_init();
void disp_init(spi_display_t *disp);
void disp_clear(spi_display_t *disp, uint16_t color);
void destroy_gpio();
void w_letter(spi_display_t *disp, char let, int start_x, int start_y, int height, int width, int R_char, int G_char, int B_char, int R_back, int G_back, int B_back);
void w_text(spi_display_t *disp, char* txt, int start_x, int start_y, int R_char, int G_char, int B_char, int R_back, int G_back, int B_back);
void zone_clear(spi_display_t *disp, uint16_t color, int start_x, int start_y, int end_x, int end_y);

#endif /* DISPLAY_H_ */
