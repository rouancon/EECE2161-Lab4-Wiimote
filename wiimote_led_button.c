#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/** Event 2 is of 32 chars in size*/
#define WIIMOTE_EVT2_PKT_SIZE 32

/** device file name for event 2	*/
#define WIIMOTE_EVT2_FNAME "/dev/input/event2"

/** the code is placed in byte 10 */
#define WIIMOTE_EVT2_CODE 10
/** value is placed in byte 12	*/
#define WIIMOTE_EVT2_VALUE 12

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


#define WIIMOTE_BTN_UP 0x67     /* Code for UP button */
#define WIIMOTE_BTN_DOWN 0x6C   /* Code for DOWN button */
#define WIIMOTE_BTN_LEFT 0x6A   /* Code for LEFT button */
#define WIIMOTE_BTN_RIGHT 0x69  /* Code for RIGHT button */
#define WIIMOTE_BTN_A 0x30      /* Code for A button */
#define WIIMOTE_BTN_PLUS 0x97   /* Code for PLUS button */
#define WIIMOTE_BTN_MINUS 0x9C  /* Code for MINUS button */
#define WIIMOTE_BTN_HOME 0x3C   /* Code for HOME button */
#define WIIMOTE_BTN_1 0x1       /* Code for 1 button */
#define WIIMOTE_BTN_2 0x2       /* Code for 2 button */
#define WIIMOTE_BTN_B 0x31      /* Code for B button */

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
 * Main function
 * @return 0 upon success, -1 on error
 */
int main(int argc, char* argv[]) {
	unsigned char buf[WIIMOTE_EVT2_PKT_SIZE]; // allocate data for packet buffer
	unsigned char evt2Code;  /// event 2 code
	unsigned char evt2Value; /// event 2 value

	int file = open(WIIMOTE_EVT2_FNAME , O_RDONLY); //Opens the event2 file in read only mode

	// failed to open file?
	if (file == -1) {
		// error handling, print error and exit with code
		printf("Could not open event file '%s'\n", WIIMOTE_EVT2_FNAME);
		return -1;
	}
  
  int fd, ledNum, state, stayInLoop = 1;
  // open userio module
 	unsigned char *pMemBase = userio_init(&fd);
	
	if(pMemBase == MAP_FAILED)
	{
		perror("Mapping memory for absolute memory access failed -- Test Try\n");
		return -1;
	}	

	while(stayInLoop) {
		// blocking read of  32 bytes from  file and put them into buf
		//   read blocks until sufficient bytes are available
		read(file, buf, WIIMOTE_EVT2_PKT_SIZE);

		// extract code from packet
		evt2Code = buf[WIIMOTE_EVT2_CODE];

		// extract value from packet
		evt2Value = buf[WIIMOTE_EVT2_VALUE];
    
    if (evt2Code == WIIMOTE_BTN_UP) { userio_ledSet(pMemBase, 0, evt2Value); }  // Set LED1 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_DOWN) { userio_ledSet(pMemBase, 1, evt2Value); } // Set LED2 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_LEFT) { userio_ledSet(pMemBase, 2, evt2Value); } // Set LED3 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_RIGHT) { userio_ledSet(pMemBase, 3, evt2Value); } // Set LED4 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_A) { userio_ledSet(pMemBase, 4, evt2Value); } // Set LED5 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_B) { userio_ledSet(pMemBase, 5, evt2Value); } // Set LED6 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_1) { userio_ledSet(pMemBase, 6, evt2Value); } // Set LED7 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_2) { userio_ledSet(pMemBase, 7, evt2Value); } // Set LED8 to evt2Value
    else if (evt2Code == WIIMOTE_BTN_HOME) { stayInLoop = 0; } // Break if home button pressed
	}
	close(file); // close the file
  // close userio module
	userio_deinit(pMemBase, fd);

	return 0;
}