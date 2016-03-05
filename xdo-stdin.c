/* NOTE: DISPLAY is currently hardcoded to ":1"
 * (because it was too easy to accidentally fuzz the wrong display)
 */

#define _POSIX_C_SOURCE 199309L

#include "xdo-stdin.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <xdo.h>
#include <time.h>

const char *DISPLAY = ":1";
const unsigned long DELAY = 1200;
const long EXIT_AFTER = 200;

static void do_exit() {
    struct timespec ts = {0, DELAY*1000*20};
    nanosleep(&ts, NULL);

    _exit(0);
}

static uint16_t read_uint16() {
    uint16_t x;
    if (fread(&x, 2, 1, stdin) != 1)
	do_exit();
    return x;
}

static uint8_t read_uint8() {
    int c = getchar();
    if (c == EOF)
	do_exit();
    return c;
}

static int read_mousebutton() {
    return read_uint8() % 5 + 1;
}

static void do_sleep() {
    static int count = 0;

    struct timespec ts = {0, DELAY*1000};
    nanosleep(&ts, NULL);

    if (++count == EXIT_AFTER) {
	/*fprintf(stderr, "Reached %lu, going to exit.\n", EXIT_AFTER);*/
	do_exit();
    }
}

void *xdo_stdin_main(void *param) {
    xdo_t *xdo = xdo_new(DISPLAY);
    if (!xdo) {
	fputs("No display.", stderr);
	_exit(1);
    }

    while (1) {
	int c = read_uint8();
	switch (c) {
	case 'm':
	    {
		/* move mouse */
		int x = read_uint16()%800, y = read_uint16()%800;
		/* printf("move %d %d\n", x, y); */
		xdo_move_mouse(xdo, x, y, 0);
		do_sleep();
	    }
	    break;
	case 'k':
	    {
		/* keypress */
		int letter = read_uint8();
		char let[2] = {letter, 0};
		if (letter < 128 && isprint(letter)) {
		    /* printf("keypress '%c' (%d)\n", let[0], letter); */
		    xdo_enter_text_window(xdo, CURRENTWINDOW, let, 12000);
		    do_sleep();
		}
	    }
	    break;
	case 'c':
	    {
		/* click */
		int button = read_mousebutton();
		/* printf("click %d\n", button); */
		xdo_click_window(xdo, CURRENTWINDOW, button);
		do_sleep();
	    }
	    break;
	case 'd':
	    {
		/* mouse down */
		int button = read_mousebutton();
		/* printf("mousedown %d\n", button); */
		xdo_mouse_down(xdo, CURRENTWINDOW, button);
		do_sleep();
	    }
	    break;
	case 'u':
	    {
		/* mouse up */
		int button = read_mousebutton();
		/* printf("mouseup %d\n", button); */
		xdo_mouse_up(xdo, CURRENTWINDOW, button);
		do_sleep();
	    }
	    break;
	}
    }
}
