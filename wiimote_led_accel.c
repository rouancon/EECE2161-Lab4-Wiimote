#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

// physical base address of GPIO
#define GPIO_ADDRESS	0x400D0000
// length of MEM Mapped IO window
#define GPIO_MAP_LEN 0xFF

#define LED1_OFFSET	0x12C  /* Offset for LED1 */
#define LED2_OFFSET	0x130  /* Offset for LED2 */
#define LED3_OFFSET	0x134  /* Offset for LED3 */
#define LED4_OFFSET	0x138  /* Offset for LED4 */
#define LED5_OFFSET	0x13C  /* Offset for LED5 */
#define LED6_OFFSET	0x140  /* Offset for LED6 */
#define LED7_OFFSET	0x144  /* Offset for LED7 */
#define LED8_OFFSET	0x148  /* Offset for LED8 */

#define REG_WRITE(addr, off, val) (*(volatile int*)(addr+off)=(val))
#define REG_READ(addr,off) (*(volatile int*)(addr+off))

/** Event 2 is of 32 chars in size*/
#define WIIMOTE_EVT0_PKT_SIZE 16

/** device file name for event 2	*/
#define WIIMOTE_EVT0_FNAME "/dev/input/event0"

/** the code is placed in byte 10 */
#define WIIMOTE_EVT0_CODE 10

/** code for X acceleration */
#define WIIMOTE_EVT0_ACCEL_X 3
/** code for Y acceleration */
#define WIIMOTE_EVT0_ACCEL_Y 4
/** code for Z acceleration */
#define WIIMOTE_EVT0_ACCEL_Z 5

/** high portion of value is placed in byte 13	*/
#define WIIMOTE_EVT0_VALUE_H 13

/** high portion of value is placed in byte 12	*/
#define WIIMOTE_EVT0_VALUE_L 12


/**
 * Initialize userio module.
 *  - opens access to physical memory /dev/mem
 *  - mapps memory at offset 'PhysicalAddress' into virtual address space
 *
 *@param  fd	pointer to file descriptor (needed for later)
 *@return  address to virtual memory which is mapped to physical,
 *          or MAP_FAILED on error.
 */
unsigned char *userio_init(int *fd)
{
	unsigned long int PhysicalAddress = GPIO_ADDRESS;
	int map_len = GPIO_MAP_LEN;
	unsigned char *pBase;

	*fd = open( "/dev/mem", O_RDWR);

	pBase = (unsigned char*)mmap(NULL, map_len, PROT_READ |
				PROT_WRITE, MAP_SHARED, *fd, (off_t)PhysicalAddress);

	return pBase;
}

/**
 * close userio module and free resources
 * -  Unmap the address mapping at 'pBase' and close the file descriptor fd
 *@param pBase	base address
 *@param fd     file descriptor to close
 */
void userio_deinit(unsigned char *pBase, int fd)
{
	int map_len = GPIO_MAP_LEN;
	munmap((void *)pBase, map_len);
	close(fd);
}

/**
 * Allow control of individual LEDs
 *@param pBase	base address of userio
 *@param ledNr	LED number to control (LED0-LED7)
 *@param state	LED state - on or off (1 or 0)
 */
void  userio_ledSet(unsigned  char  *pBase,  unsigned  int ledNr, unsigned int state)
{
	unsigned int ledOffset = 0x12C + 4*ledNr; /*finds and stores offset for desired LED (0 LED + 4 (address increment) * the LED number)*/
	REG_WRITE(pBase, ledOffset, state); /*performs the action of writing the desired state to the LED*/
}


/**
 * Turns on all LEDs equal to or less than intensity
 */
void userio_ledBar(unsigned char *pMemBase, int intensity)
{
  for (int i = 0; i < 8; i++) {
      if ((intensity + 1) > i) {
         userio_ledSet(pMemBase, i, 1);
      }
      else {
         userio_ledSet(pMemBase, i, 0);
      }
  }
}



/**
 * Main function
 * @return 0 upon success, -1 on error
 */
int main() {
	unsigned char buf[WIIMOTE_EVT0_PKT_SIZE]; //each packet of data is 16 bytes
	unsigned char evt0Code;  /// event 2 code
	unsigned char evt0ValueL; /// event 2 value Low
	unsigned char evt0ValueH; /// event 2 value Low
	signed short accel;

    int file = open(WIIMOTE_EVT0_FNAME , O_RDONLY); //Opens the event2 file in read only mode
    int fd, ledNum, state;
	
	// open userio module
	unsigned char *pMemBase = userio_init(&fd);
	
    // return -1 if mapping failed
	if(pMemBase == MAP_FAILED)
	{
		perror("Mapping memory for absolute memory access failed -- Test Try\n");
		return -1;
	}	
  
	// failed to open file?
	if (file == -1) {
		// error handling, print error and exit with code
		printf("Could not open event file '%s'\n", WIIMOTE_EVT0_FNAME);
		return -1;
	}

	while(1)  {
		read(file, buf, WIIMOTE_EVT0_PKT_SIZE); // read 16 bytes from the file and put it in the buffer
		evt0Code   = buf[WIIMOTE_EVT0_CODE];       // extract code byte
		evt0ValueH = buf[WIIMOTE_EVT0_VALUE_H];  // extract high byte of accel
		evt0ValueL = buf[WIIMOTE_EVT0_VALUE_L];  // extract low byte of accel
   
        // read only X values
        if (evt0Code == 0x3) {
            accel = (evt0ValueH << 8) + evt0ValueL; // concat high and low bytes into single value
            int intensity = (int)(110 + accel)/25; // adjust and initialize intensity
            userio_ledBar(pMemBase, intensity); // set LEDs accordingly
        }
	}
	close(file); // close the file
    userio_deinit(pMemBase, fd);

	return 0;
}