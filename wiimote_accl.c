/**
 * @brief  This program reads values from event0 and prints the values of x, y, and z.
 * @return
 */

#include <stdio.h> 
#include <fcntl.h>
#include <unistd.h>

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

		accel = (evt0ValueH << 8) + evt0ValueL;

		if (evt0Code != 0x0) { // ignore the zeroes
			// print content
			printf("code: %X, value %x%x, dec val %d\n", evt0Code, evt0ValueH, evt0ValueL, accel);
		}
	}
	close(file); // close the file

	return 0;
}
