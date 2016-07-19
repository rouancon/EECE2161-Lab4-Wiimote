/**
 * @brief This program reads values from event2 (button events) and prints the code and the value.
 * @author Gunar Schirner
 */

#include <stdio.h> 
#include <fcntl.h>
#include <unistd.h>

/** Event 2 is of 32 chars in size*/
#define WIIMOTE_EVT2_PKT_SIZE 32

/** device file name for event 2	*/
#define WIIMOTE_EVT2_FNAME "/dev/input/event2"

/** the code is placed in byte 10 */
#define WIIMOTE_EVT2_CODE 10
/** value is placed in byte 12	*/
#define WIIMOTE_EVT2_VALUE 12


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

	while(1) {
		// blocking read of  32 bytes from  file and put them into buf
		//   read blocks until sufficient bytes are available
		read(file, buf, WIIMOTE_EVT2_PKT_SIZE);

		// extract code from packet
		evt2Code = buf[WIIMOTE_EVT2_CODE];

		// extract value from packet
		evt2Value = buf[WIIMOTE_EVT2_VALUE];

		// the code is placed in byte 10, value is placed in byte 12
		printf("code: %X, value: %X\n", evt2Code, evt2Value);

	}
	close(file); // close the file

	return 0;
}
