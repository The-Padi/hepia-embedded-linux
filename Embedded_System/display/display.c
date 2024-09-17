/*
 * Code skeleton to draw pixels on the MyLab2 LCD screen.
 */

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

#include "ppm.h"

//----------------------------------------------------------------
// Constants and macros
//----------------------------------------------------------------

#define XRES 240
#define YRES 320

#define SPIDEVNODE "/dev/spidev1.0"

#define SPI_CMD_DELAY 30

// 16bpp (RGB 5:6:5) pixel format:
// |15-11|10-05|04-00|
// |  R  |  G  |  B  |

#define COL8TO5(x) (x >> 3)
#define COL8TO6(x) (x >> 2)

#define RGB(r, g, b) ((COL8TO5(r) << 11) | (COL8TO6(g) << 5) | (COL8TO5(b)))

#define GREEN RGB(0, 255, 0)
#define GREY RGB(100, 100, 100)
#define LIGHT_RED RGB(255, 120, 120)
#define BLACK RGB(0, 0, 0)

#define write_cmd(cmd) spi_write_cmd(disp, cmd)
#define write_data(data) spi_write_data(disp, data, sizeof(data))

// Datasheet doc: "Atmel-11121-32-bit-Cortex-A5-Microcontroller-SAMA5D3_Datasheet.pdf"

// Needed for mmap
// Addresses from p.30 of datasheet
#define SMC_BASE 0xffffc000
#define GPIOA_BASE_ADDR 0xfffff200
#define GPIOA_END_ADDR 0xfffff400

#define PIOA_MEM_SIZE (GPIOA_END_ADDR - SMC_BASE)
#define GPIOA_OFFSET (GPIOA_BASE_ADDR - SMC_BASE)

// Addresses from p.289 of datasheet
#define PIO_PER 0x0 // PIO Enable Register
#define PIO_PDR 0x4 // PIO Disable Register
#define PIO_PSR 0x8 // PIO Status Register

#define PIO_OER 0x10 // Output Enable Register
#define PIO_ODR 0x14 // Output Disable Register
#define PIO_OSR 0x18 // Output Status Register

#define PIO_SODR 0x30 // Set Output Data Register
#define PIO_CODR 0x34 // Clear Output Data Register

//----------------------------------------------------------------
// Types
//----------------------------------------------------------------

typedef struct
{
    int fd_spi;
    struct spi_ioc_transfer cmd;
} spi_display_t;

//----------------------------------------------------------------
// Variables
//----------------------------------------------------------------

static uint32_t spi_speed = 12000000;
static uint8_t spi_bits = 8;
static uint32_t spi_mode = 0;

static void *gpio_base_addr;

static uint32_t *gpio_pioa;

static uint32_t *gpio_pio_per;
static uint32_t *gpio_pio_pdr;
static uint32_t *gpio_pio_psr;

static uint32_t *gpio_pio_oer;
static uint32_t *gpio_pio_odr;
static uint32_t *gpio_pio_osr;

static uint32_t *gpio_pio_sodr;
static uint32_t *gpio_pio_codr;

static int fd;

//----------------------------------------------------------------
// Static functions
//----------------------------------------------------------------

/*
 * Print the global error message and abort execution.
 */
static void pabort(const char *s)
{
    perror(s);
    abort();
}

/**
 * @brief Write a command to the LCD screen through SPI
 * @param disp A pointer to a spi_display_t structure
 * @param cmd The command to be sent
 */
static void spi_write_cmd(spi_display_t *disp, uint8_t cmd)
{
    uint8_t unused[4096];

    // one byte for command
    struct spi_ioc_transfer msg = {
        .tx_buf = (unsigned long)&cmd,
        .rx_buf = (unsigned long)unused,
        .len = 1,
        .delay_usecs = SPI_CMD_DELAY,
        .speed_hz = spi_speed,
        .bits_per_word = spi_bits,
    };

    // Set data/cmd pin to 0
    *gpio_pio_codr = (1 << 20);

    // SPI_IOC_MESSAGE(1) indicates we are sending 1 message (the message "cmd"),
    // but we could send any number of messages at once
    // More details in kernel source: include/uapi/linux/spi/spidev.h
    if (ioctl(disp->fd_spi, SPI_IOC_MESSAGE(1), &msg) == -1)
        pabort("Failed sending spi message");
}

/**
 * @brief Write a command to the LCD screen through SPI
 * @param disp A pointer to a spi_display_t structure
 * @param data The data to be sent
 * @param data_len The data length in bytes
 */
static void spi_write_data(spi_display_t *disp, uint8_t const *data, size_t data_len)
{
    uint8_t unused[4096];
    int pos = 0;

    // Set data/cmd pin to 1
    *gpio_pio_sodr = (1 << 20);

    while (data_len > 4096)
    {
        uint8_t send_data[4096];

        for (int i = 0; i < 4096; i++)
        {
            send_data[i] = data[pos + i];
        }

        // one byte for command
        struct spi_ioc_transfer msg = {
            .tx_buf = (unsigned long)send_data,
            .rx_buf = (unsigned long)unused,
            .len = 4096, // maximum data length is 4096 bytes
            .delay_usecs = SPI_CMD_DELAY,
            .speed_hz = spi_speed,
            .bits_per_word = spi_bits,
        };

        // SPI_IOC_MESSAGE(1) indicates we are sending 1 message (the message "cmd"),
        // but we could send any number of messages at once
        // More details in kernel source: include/uapi/linux/spi/spidev.h
        if (ioctl(disp->fd_spi, SPI_IOC_MESSAGE(1), &msg) == -1)
        {
            pabort("Failed sending spi message");
        }

        data_len -= 4096;
        pos += 4096;
    }

    if (data_len > 0)
    {
        uint8_t send_data[data_len];

        for (int i = 0; i < data_len; i++)
        {
            send_data[i] = data[pos + i];
        }

        // one byte for command
        struct spi_ioc_transfer msg = {
            .tx_buf = (unsigned long)send_data,
            .rx_buf = (unsigned long)unused,
            .len = data_len, // maximum data length is 4096 bytes
            .delay_usecs = SPI_CMD_DELAY,
            .speed_hz = spi_speed,
            .bits_per_word = spi_bits,
        };

        // SPI_IOC_MESSAGE(1) indicates we are sending 1 message (the message "cmd"),
        // but we could send any number of messages at once
        // More details in kernel source: include/uapi/linux/spi/spidev.h
        if (ioctl(disp->fd_spi, SPI_IOC_MESSAGE(1), &msg) == -1)
        {
            pabort("Failed sending spi message");
        }
    }
}

/**
 * @brief Set a window to be written
 * @param disp A pointer to a spi_display_t structure
 * @param x0 The abscissa of the top-left point
 * @param y0 The ordinate of the top-left point
 * @param x1 The abscissa of the bottom-right point
 * @param y1 The ordinate of the bottom-right point
 */
static void disp_setwindow(spi_display_t *disp, int x0, int y0, int x1, int y1)
{
    assert(!(x1 > XRES - 1 || y1 > YRES - 1 || x0 > x1 || y0 > y1));

    int YSH = y0 >> 8;
    int YSL = y0;
    int YEH = y1 >> 8;
    int YEL = y1;

    // Set column (left and right zone)
    write_cmd(0x2A);
    {
        uint8_t dat[] = {0x00, x0, 0x00, x1};
        write_data(dat);
    }

    // Set page address (top and bottom lines)
    write_cmd(0x2B);
    {
        uint8_t dat[] = {YSH, YSL, YEH, YEL};
        write_data(dat);
    }

    // Memory write
    write_cmd(0x2C);
}

/**
 * @brief Initialize the LCD screen
 * @param disp A pointer to a spi_display_t structure
 */
static void disp_init(spi_display_t *disp)
{
    // SPI mode
    if (ioctl(disp->fd_spi, SPI_IOC_WR_MODE32, &spi_mode) == -1)
        pabort("Failed setting spi mode");

    // SPI bits per word
    if (ioctl(disp->fd_spi, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) == -1)
        pabort("Failed setting spi bits per word");

    // SPI max speed in hz
    if (ioctl(disp->fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) == -1)
        pabort("Failed setting spi max speed");

    printf("SPI mode: 0x%x\n", spi_mode);
    printf("SPI bits per word: %d\n", spi_bits);
    printf("SPI max speed: %d Hz (%d KHz)\n", spi_speed, spi_speed / 1000);

    // Initialization sequence for the ILI9341 display (mylab2)
    write_cmd(0x01); // Software reset
    usleep(50000);   // Wait 50ms

    write_cmd(0x11);
    usleep(120000); // Wait 120ms

    write_cmd(0xCF);
    {
        uint8_t dat[] = {0x00, 0x83, 0x30};
        write_data(dat);
    }

    write_cmd(0xED);
    {
        uint8_t dat[] = {0x64, 0x03, 0x12, 0x81};
        write_data(dat);
    }

    write_cmd(0xE8);
    {
        uint8_t dat[] = {0x85, 0x01, 0x79};
        write_data(dat);
    }

    write_cmd(0xCB);
    {
        uint8_t dat[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
        write_data(dat);
    }

    write_cmd(0xF7);
    {
        uint8_t dat[] = {0x20};
        write_data(dat);
    }

    write_cmd(0xEA);
    {
        uint8_t dat[] = {0x00, 0x00};
        write_data(dat);
    }

    write_cmd(0xC1); // Power control
    {
        uint8_t dat[] = {0x11}; // SAP[2:0];BT[3:0]
        write_data(dat);
    }

    write_cmd(0xC5); // VCM control 1
    {
        uint8_t dat[] = {0x34, 0x3D};
        write_data(dat);
    }

    write_cmd(0xC7); // VCM control 2
    {
        uint8_t dat[] = {0xC0};
        write_data(dat);
    }

    write_cmd(0x36); // Memory Access Control
    {
        uint8_t dat[] = {0x08};
        write_data(dat);
    }

    write_cmd(0x3A); // Pixel format
    {
        uint8_t dat[] = {0x55}; // 16bpp
        write_data(dat);
    }

    write_cmd(0xB1); // Frame rate
    {
        uint8_t dat[] = {0x00, 0x1D}; // 65Hz
        write_data(dat);
    }

    write_cmd(0xB6); // Display Function Control
    {
        uint8_t dat[] = {0x0A, 0xA2, 0x27, 0x00};
        write_data(dat);
    }

    write_cmd(0xb7); // Entry mode
    {
        uint8_t dat[] = {0x07};
        write_data(dat);
    }

    write_cmd(0xF2); // 3Gamma Function Disable
    {
        uint8_t dat[] = {0x08};
        write_data(dat);
    }

    write_cmd(0x26); // Gamma curve selected
    {
        uint8_t dat[] = {0x01};
        write_data(dat);
    }

    write_cmd(0xE0); // positive gamma correction
    {
        uint8_t dat[] = {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00};
        write_data(dat);
    }

    write_cmd(0xE1); // negamma correction
    {
        uint8_t dat[] = {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F};
        write_data(dat);
    }

    write_cmd(0x11); // Exit sleep
    usleep(120000);

    write_cmd(0x29); // Display on
    usleep(50000);

    // Set backlight pin to 1
    *gpio_pio_sodr = (1 << 22);
}

//----------------------------------------------------------------
// Functions
//----------------------------------------------------------------

/**
 * @brief Clear the screen - Display a uniform color
 * @param disp A pointer to a spi_display_t structure
 * @param color The color to be displayed
 * @note For performance and debug purpose, only clear a quarter of the screen
 */
void disp_clear(spi_display_t *disp, uint16_t color)
{

    uint8_t dat_fast[(XRES * YRES) * 2];

    int k = 0;

    disp_setwindow(disp, 0, 0, XRES - 1, YRES - 1);

    for (int i = 0; i < YRES; i++)
    {
        for (int j = 0; j < XRES; j++)
        {
            uint8_t m, n;
            m = color >> 8;
            n = color;

            dat_fast[k] = m;
            k++;
            dat_fast[k] = n;
            k++;
        }
    }

    write_data(dat_fast);
}

/**
 * @brief Set a pixel of the screen in a given color
 * @param disp A pointer to a spi_display_t structure
 * @param x The pixel abscissa
 * @param y The pixel ordinate
 * @param color The pixel color
 */
void disp_setpix(spi_display_t *disp, int x, int y, uint16_t color)
{
    disp_setwindow(disp, x, y, x, y);
    uint8_t m, n;
    m = color >> 8;
    n = color;
    {
        uint8_t dat[] = {m, n};
        write_data(dat);
    }
}

int gpio_init()
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (fd == -1)
    {
        fprintf(stderr, "Failed opening /dev/mem\n");
        return EXIT_FAILURE;
    }

    // Physical region [SMC_BASE, SMC_BASE+PIOA_MEM_SIZE] is mapped at virtual address gpio_base_addr.
    gpio_base_addr = mmap(0, PIOA_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SMC_BASE);

    if (gpio_base_addr == MAP_FAILED)
    {
        fprintf(stderr, "Failed memory mapping registers\n");
        close(fd);
        return EXIT_FAILURE;
    }

    gpio_pioa = gpio_base_addr + GPIOA_OFFSET;
    gpio_pio_per = gpio_pioa + PIO_PER / 4;
    gpio_pio_pdr = gpio_pioa + PIO_PDR / 4;
    gpio_pio_psr = gpio_pioa + PIO_PSR / 4;
    gpio_pio_oer = gpio_pioa + PIO_OER / 4;
    gpio_pio_odr = gpio_pioa + PIO_ODR / 4;
    gpio_pio_osr = gpio_pioa + PIO_OSR / 4;
    gpio_pio_sodr = gpio_pioa + PIO_SODR / 4;
    gpio_pio_codr = gpio_pioa + PIO_CODR / 4;

    // Enable pin
    // Command
    *gpio_pio_per = (1 << 20);
    // Backlight
    *gpio_pio_per = (1 << 22);

    // Configure pin as output (enable outpout)
    // Command
    *gpio_pio_oer = (1 << 20);
    // Write 0 to pin (clear)
    *gpio_pio_codr = (1 << 20);
    // Backlight
    *gpio_pio_oer = (1 << 22);
    // Write 0 to pin (clear)
    *gpio_pio_codr = (1 << 22);

    return EXIT_SUCCESS;
}

void destroy_gpio()
{
    // Disable pin
    // Command
    *gpio_pio_pdr = (1 << 20);
    // Backlight
    *gpio_pio_pdr = (1 << 22);

    close(fd);
    munmap(gpio_base_addr, PIOA_MEM_SIZE);
}

void display_ppm_image(spi_display_t *disp, int x, int y, img_t *img_ppm)
{
    printf("w: %d / h: %d\n", img_ppm->width, img_ppm->height);

    // Set window size
    disp_setwindow(disp, x, y, img_ppm->width - 1, img_ppm->height - 1);

    uint8_t send_data[((img_ppm->width) * (img_ppm->height)) * 2];

    int k = 0;

    for (int i = 0; i < img_ppm->height; i++)
    {
        for (int j = 0; j < img_ppm->width; j++)
        {
            pixel_t *pixel = &img_ppm->pix2d[i][j];
            uint16_t color = RGB(pixel->r, pixel->g, pixel->b);

            uint8_t m, n;

            m = color >> 8;
            n = color;

            send_data[k] = m;
            k++;
            send_data[k] = n;
            k++;
        }
    }

    write_data(send_data);
}

void display_image(spi_display_t *disp, int x, int y, char *path)
{
    size_t len = strlen(path);

    if (len < 4)
    {
        return;
    }

    img_t *img_ppm;
    char output_path[1000];

    // Check if image is not a PPM
    if (strcmp(path + len - 4, ".ppm") != 0)
    {

        snprintf(output_path, sizeof(output_path), "%s.ppm", path);

        char command[1000];
        snprintf(command, sizeof(command), "magick %s %s", path, output_path);

        if (system(command) != 0)
        {
            printf("Failed to convert image to PPM format.\n");
            return;
        }

        img_ppm = load_ppm(output_path);
    }
    else
    {
        snprintf(output_path, sizeof(output_path), "%s", path);

        img_ppm = load_ppm(output_path);
    }

    // If the image is too big
    if (img_ppm->width > XRES)
    {
        char command[1000];
        snprintf(command, sizeof(command), "magick %s -resize '%d' %s", output_path, XRES, output_path);

        if (system(command) != 0)
        {
            printf("Failed to resize image (width).\n");
            return;
        }

        img_ppm = load_ppm(output_path);
    }

    if (img_ppm->height > YRES)
    {
        char command[1000];
        snprintf(command, sizeof(command), "magick %s -resize 'x%d' %s", output_path, YRES, output_path);

        if (system(command) != 0)
        {
            printf("Failed to resize image (height).\n");
            return;
        }

        img_ppm = load_ppm(output_path);
    }

    display_ppm_image(disp, x, y, img_ppm);
}

/**
 * @brief Main entry point
 */
int main(int argc, char *argv[])
{
    spi_display_t disp;

    if ((disp.fd_spi = open(SPIDEVNODE, O_RDWR)) == -1)
        pabort("Failed opening SPI device");

    // GPIOs initialization
    gpio_init();

    // Display initialization
    disp_init(&disp);
    disp_clear(&disp, BLACK);

    // Display image
    display_image(&disp, 0, 0, argv[1]);

    // GPIOs shutdown
    // disp_clear(&disp, BLACK);
    // destroy_gpio();

    // close(disp.fd_spi);

    return EXIT_SUCCESS;
}
