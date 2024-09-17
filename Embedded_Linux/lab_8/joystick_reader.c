#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define JOYSTICK_DEVICE "/dev/JoyStick"

int main()
{
    // Open Joystick
    int fd = open(JOYSTICK_DEVICE, O_RDWR | O_SYNC);

    // Check if everything went well
    if (fd == -1)
    {
        perror("Error opening joystick device");
        return -1;
    }

    int joystick_state;

    while (1)
    {
        // Read the Joystick state in the kernel module
        ssize_t count = read(fd, &joystick_state, sizeof(joystick_state));

        if (count == -1)
        {
            perror("Error reading joystick state");
            break;
        }

        // If a button is pushed, show it's state
        if (joystick_state != 0)
        {
            switch (joystick_state)
            {
            case 1:
                printf("Joystick Left\n");
                break;
            case 2:
                printf("Joystick Right\n");
                break;
            case 3:
                printf("Joystick Up\n");
                break;
            case 4:
                printf("Joystick Down\n");
                break;
            case 5:
                printf("Joystick Button Pressed\n");

                // Close the program
                close(fd);
                return 0;
            }
        }

        // usleep to prevent saturation of the CPU
        usleep(10000);
    }
}
