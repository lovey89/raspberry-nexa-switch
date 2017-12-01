#include "gpio.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int mem_fd;

unsigned char *gpio_mem, *gpio_map;

volatile unsigned *gpio;

//
// Set up a memory regions to access GPIO
//
void Gpio__setup()
{
  /* open /dev/mem */
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
  {
    printf("Can't open /dev/mem \n");
    exit (-1);
  }

  /* mmap GPIO */
  // Allocate MAP block
  if ((gpio_mem = (unsigned char*)malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL)
  {
    printf("Allocation error \n");
    exit (-1);
  }

  // Make sure pointer is on 4K boundary
  if ((unsigned long)gpio_mem % PAGE_SIZE)
  {
    gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);
  }

  // Now map it
  gpio_map = (unsigned char *)mmap((caddr_t)gpio_mem,
                                   BLOCK_SIZE,
                                   PROT_READ|PROT_WRITE,
                                   MAP_SHARED|MAP_FIXED,
                                   mem_fd,
                                   GPIO_BASE
                                   );

  if ((long)gpio_map < 0)
  {
    printf("mmap error %d\n", (int)gpio_map);
    exit (-1);
  }

  // Always use volatile pointer!
  gpio = (volatile unsigned *)gpio_map;
}
