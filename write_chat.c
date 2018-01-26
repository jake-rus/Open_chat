/*
 * write_chat.c
 *	Автоматическое заполнение чата и окна участников
 *  Created on: 25 янв. 2018 г.
 *      Author: jake
 */

#include "add.h"
#include <pthread.h>

typedef struct {	// структура для потока чата
	int semid;		// идентификатор семафоров
	WINDOW *wnd;	// указатель на фоновое окно
	} chat_struct;

void *write_chat(void *args)
{
	WINDOW *wnd_users;	// указатель на окно участников
	WINDOW *wnd_chat;	// указатель на окно участников
	chat_struct * all_mess = (chat_struct *)args;
	char (*shmaddr)[MAXUSERNAME+MAXTEXT+2];
	int shmid; // идентификатор разделяемой памяти
	key_t ipckey;
	int i;
	struct sembuf lock[2]={{2,0,0},{2,1,0}};
	struct sembuf unlock={2,-1,0};

	// для открытого чата
	ipckey = ftok(FILEKEY,'D');
	shmid = shmget(ipckey, MAXMESS*(MAXTEXT+MAXUSERNAME+2), 0);
	shmaddr = (char*)shmat(shmid,0,0);

	wnd_chat = derwin(all_mess->wnd,all_mess->wnd->_maxy-7,all_mess->wnd->_maxx-MAXUSERNAME-3,1,1);
	wnd_users = derwin(all_mess->wnd,all_mess->wnd->_maxy-7,MAXUSERNAME+2,1,wnd_chat->_maxx+2);

	while(strcmp(shmaddr+semctl(all_mess->semid,1,GETVAL,0)+1,TEXT_EXIT)){
	wclear(wnd_users);
	semop(all_mess->semid,&lock,2);
	wprintw(wnd_users,shmaddr);
	semop(all_mess->semid,&unlock,1);
	wrefresh(wnd_users);
	wclear(wnd_chat);
	for (i=1;i<semctl(all_mess->semid,1,GETVAL,0)+1;i++){
		semop(all_mess->semid,&lock,2);
		wprintw(wnd_chat,"%s\n", shmaddr+i);
		semop(all_mess->semid,&unlock,1);
	}
	wrefresh(wnd_chat);
	sleep(1);
	}

	wclear(wnd_chat);
	wclear(wnd_users);
	wrefresh(wnd_chat);
	wrefresh(wnd_users);
	delwin(wnd_chat);
	delwin(wnd_users);
	pthread_exit(0);
}

