/*
 * add.h
 *
 *  Created on: 25 янв. 2018 г.
 *      Author: jake
 */

#ifndef ADD_H_
#define ADD_H_


#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <curses.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#define FILEKEY "/home/jake/123.txt"
#define US 20	// максимальное количество участников чата
#define MAXMESS 20 // максимальное количество сообщений в окне чата
#define MAXTEXT 256 // максимальный текст в сообщении
#define MAXUSERNAME 8 // максимальное имя участника,включая 0-байт
#define TEXT_EXIT "exit" // кодовое сообщение завершения работы сервера и всех учаcтников

void sig_winch(int signo);
void *write_chat(void *args);

#endif /* ADD_H_ */
