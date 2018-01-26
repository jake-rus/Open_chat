/*
 * sig_winch.c
 * обработчик сигнала SIGWINCH
 *  Created on: 21.12.2017
 *      Author: jake
 */

#include "add.h"

void sig_winch(int signo)
{
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
	resizeterm(size.ws_row, size.ws_col);
}
