#include <stdio.h>
#include <stdint.h>
#include <sys/neutrino.h>  // For ThreadCtl
#include <hw/inout.h>      // For in32() and out32()
#include <sys/mman.h>      // For mmap_device_io()
#include "gpio_control.h"

// Define GPIO register offsets
#define GPFSEL0 0x00        // GPIO Function Select 0
#define GPSET0  0x1C        // GPIO Pin Output Set 0
#define GPCLR0  0x28        // GPIO Pin Output Clear 0
#define GPLEV0  0x34        // GPIO Pin Level 0
#define GPIO_PUD0 0x94       //GPIO Pin Pull-up/down Enable
#define GPPUDCLK0 0x98      // GPIO Pin Pull-up/down Enable Clock 0





// Struct representing GPIO registers
typedef struct
{
    volatile uint32_t GPFSEL[6];   // Function Select registers (GPFSEL0 - GPFSEL5)
    volatile uint32_t RESERVED_1;  // Reserved
    volatile uint32_t GPSET[2];    // Set Output registers (GPSET0 - GPSET1)
    volatile uint32_t RESERVED_2;  // Reserved
    volatile uint32_t GPCLR[2];    // Clear Output registers (GPCLR0 - GPCLR1)
    volatile uint32_t RESERVED_3;  // Reserved
    volatile uint32_t GPLEV[2];    // Level registers (GPLEV0 - GPLEV1)
    volatile uint32_t RESERVED_4[4];  // Reserved
    volatile uint32_t GPIO_PUD[0];       // Pull-up/down Enable
    volatile uint32_t GPPUDCLK[2]; // Pull-up/down Enable Clock (for pins 0-53)
} GPIO_Registers;

static volatile GPIO_Registers *gpio = NULL;

// Initialize GPIO memory mapping
int gpio_init(void)
{
    // Request I/O privileges
    if (ThreadCtl(_NTO_TCTL_IO, NULL) == -1)
    {
        perror("Failed to get I/O privileges");
        return -1;
    }

    // Map GPIO memory into process address space
    uintptr_t gpio_base = mmap_device_io(GPIO_SIZE, GPIO_BASE);
    if (gpio_base == (uintptr_t)MAP_FAILED)
    {
        perror("mmap_device_io error");
        return -1;
    }

    gpio = (volatile GPIO_Registers *)gpio_base;
    return 0;  // Success
}

// Unmap GPIO memory when done
void gpio_close(void)
{
    if (gpio != NULL)
    {
        munmap_device_io((uintptr_t)gpio, GPIO_SIZE);
        gpio = NULL;
    }
}

// Set GPIO pin as output
void gpio_set_output(int pin)
{
    int register_index = pin / 10;  // Each GPFSEL register controls 10 pins
    int bit = (pin % 10) * 3;       // Each pin uses 3 bits for function select
    uint32_t val = in32((uintptr_t)&gpio->GPFSEL[register_index]);
    val &= ~(7 << bit);  // Clear the 3 bits
    val |= (1 << bit);   // Set as output (001)
    out32((uintptr_t)&gpio->GPFSEL[register_index], val);
}

// Set GPIO pin as input
void gpio_set_input(int pin)
{
    int register_index = pin / 10;
    int bit = (pin % 10) * 3;
    uint32_t val = in32((uintptr_t)&gpio->GPFSEL[register_index]);
    val &= ~(7 << bit);  // Clear the 3 bits (input is 000)
    out32((uintptr_t)&gpio->GPFSEL[register_index], val);
}

// Set pull-up resistor on a GPIO pin
void gpio_set_pullup(int pin)
{
    out32((uintptr_t)&gpio->GPIO_PUD[0], 0x2);  // Enable pull-up
    out32((uintptr_t)&gpio->GPPUDCLK[0], (1 << pin));  // Apply to specific pin
    out32((uintptr_t)&gpio->GPIO_PUD[0], 0x0);  // Disable pull-up/down control
    out32((uintptr_t)&gpio->GPPUDCLK[0], 0x0);  // Clear clock
}

// Set pull-down resistor on a GPIO pin
void gpio_set_pulldown(int pin)
{
    out32((uintptr_t)&gpio->GPIO_PUD[0], 0x1);  // Enable pull-down
    out32((uintptr_t)&gpio->GPPUDCLK[0], (1 << pin));  // Apply to specific pin
    out32((uintptr_t)&gpio->GPIO_PUD[0], 0x0);  // Disable pull-up/down control
    out32((uintptr_t)&gpio->GPPUDCLK[0], 0x0);  // Clear clock
}

// Clear pull-up/down resistor (floating pin)
void gpio_clear_pull(int pin)
{
    out32((uintptr_t)&gpio->GPIO_PUD[0], 0x0);  // Disable pull-up/down
    out32((uintptr_t)&gpio->GPPUDCLK[0], (1 << pin));  // Apply to specific pin
    out32((uintptr_t)&gpio->GPPUDCLK[0], 0x0);  // Clear clock
}

// Set a GPIO pin to high or low
void gpio_write(int pin, int value)
{
    if (value)
    {
        out32((uintptr_t)&gpio->GPSET[0], (1 << pin));  // Set pin high
    }
    else
    {
        out32((uintptr_t)&gpio->GPCLR[0], (1 << pin));  // Set pin low
    }
}

// Read the state of a GPIO pin (returns 1 for high, 0 for low)
int gpio_read(int pin)
{
    uint32_t val = in32((uintptr_t)&gpio->GPLEV[0]);
    return (val & (1 << pin)) ? 1 : 0;
}






