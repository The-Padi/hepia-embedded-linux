#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <signal.h> 

#include "display.h"

#define I2C_DEVICE "/dev/i2c-0"
#define TMP2_I2C_ADDRESS 0x4B
#define TMP2_TEMP_REG 0x00

// Global variable to indicate if CTRL+C was pressed
volatile sig_atomic_t stop = 0;

// Signal handler function
void sigint_handler(int sig)
{
    // Set the flag to indicate that CTRL+C was pressed
    stop = 1; 
}

// Main function
int main(int argc, char *argv[])
{
    // Set up signal handler for SIGINT (CTRL+C)
    signal(SIGINT, sigint_handler);
    
    // Open I2C device
    int file;
    char filename[40];
    snprintf(filename, 19, I2C_DEVICE);
    
    file = open(filename, O_RDWR);
    
    if (file < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }

    if (ioctl(file, I2C_SLAVE, TMP2_I2C_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }
    
    // Init display
    spi_display_t disp;

    if ((disp.fd_spi = open(SPIDEVNODE, O_RDWR)) == -1)
        perror("Failed opening SPI device");
    
    gpio_init();
    disp_init(&disp);
    
    // Check if we want to clear the screen
    if(argc == 2)
    {
        disp_clear(&disp, BLACK);
    }

    while(!stop)
    {
        // Get the temp value
        uint8_t reg[1] = {TMP2_TEMP_REG};
        if (write(file, reg, 1) != 1)
        {
            perror("Failed to write to the i2c bus");
            exit(1);
        }

        uint8_t data[2];
        if (read(file, data, 2) != 2)
        {
            perror("Failed to read from the i2c bus");
            exit(1);
        }

        // Calculate the temp
        int16_t temp = (data[0] << 8) | data[1];
        float tmp;
        
        if (((temp >> 15) & 1) == 0)
        {
            tmp = temp / 128.0;
        }
        else
        {
            tmp = (temp - 65535) / 128.0;
        }
        
        // Copy the temp in a string
        char str[11];
        char ch=248;
        sprintf(str, "%.2f %cC", tmp, ch);
        
        // Clear the zone of the screen
        zone_clear(&disp, BLACK, 1, 308, 239, 319);
        
        // Display temp
        w_text(&disp, str, 1, 308, 255, 255, 255, 0, 0, 0);
        
        sleep(1);
    }

    // Clear the zone of the screen
    zone_clear(&disp, BLACK, 1, 308, 239, 319);
        
    // Close SPI and Screen communication
    close(disp.fd_spi);
    close(file);
    
    return 0;
}
