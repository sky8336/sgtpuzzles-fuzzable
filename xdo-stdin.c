/* Copyright (c) 2016 Sami Liedes
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
#include <stdlib.h>

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
    const char *DISPLAY = getenv("DISPLAY");
    xdo_t *xdo;

    if (!DISPLAY) {
	fputs("DISPLAY variable not set.", stderr);
	_exit(1);
    }

    if (strcmp(DISPLAY, ":0") == 0) {
	fputs("Cowardly refusing to fuzz display :0.", stderr);
	_exit(1);
    }

    xdo = xdo_new(DISPLAY);
    if (!xdo) {
	fputs("No display.", stderr);
	_exit(1);
    }

    while (1) {
	int c = read_uint8();
	c %= 5;
	switch (c) {
	case 0:
	    {
		/* move mouse */
		int x = read_uint16()%800, y = read_uint16()%800;
		/* printf("move %d %d\n", x, y); */
		xdo_move_mouse(xdo, x, y, 0);
		do_sleep();
	    }
	    break;
	case 1:
	    {
		/* keypress */
		int letter = read_uint8();
		char let[2] = {0};
		let[0] = letter;
		if (letter < 128 && isprint(letter)) {
		    /* printf("keypress '%c' (%d)\n", let[0], letter); */
		    xdo_enter_text_window(xdo, CURRENTWINDOW, let, 12000);
		    do_sleep();
		}
	    }
	    break;
	case 2:
	    {
		/* click */
		int button = read_mousebutton();
		/* printf("click %d\n", button); */
		xdo_click_window(xdo, CURRENTWINDOW, button);
		do_sleep();
	    }
	    break;
	case 3:
	    {
		/* mouse down */
		int button = read_mousebutton();
		/* printf("mousedown %d\n", button); */
		xdo_mouse_down(xdo, CURRENTWINDOW, button);
		do_sleep();
	    }
	    break;
	case 4:
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
