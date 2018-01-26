/*
 * user.c
 *	участник чата
 *  Created on: 21.01.2018
 *      Author: jake
 */

#include "add.h"
#include <pthread.h>

typedef struct {	// структура для потока чата
	int semid;		// идентификатор семафоров
	WINDOW *wnd;	// указатель на фоновое окно
	} chat_struct;

int main(void)
{
	WINDOW *wnd,*wnd_message;
	int i;
	int userNumber = 0; // порядковый номер участника
	key_t ipckey;
	int msgid,semid;
	struct {
		long mtype;
		char name[MAXUSERNAME];
		char mtext[MAXTEXT];
	}msg;
	pthread_t tidChat; //идентификатор потока-панели
	chat_struct all_mess; // структура передачи в поток чата
	chat_struct *arg_chat = &all_mess; // аргумент для pthread

	memset(msg.mtext, 0, 256); // очистка массива-текста
	memset(msg.name, 0, 8); // очистка массива-имени

	// отрисовка окон чата
		initscr(); // инициализация структуры данных в режиме ncurses
		signal (SIGWINCH, sig_winch);
		cbreak(); // включение неканонического режима ввода данных
		start_color();
		refresh();
		wnd = newwin(32,40,1,1); // фоновое окно
		box(wnd,'||','=');
		wrefresh(wnd);
		wnd_message = derwin(wnd,4,wnd->_maxx-2,wnd->_maxy-5,1);

	// очередь сообщений между сервером и участниками
	ipckey = ftok(FILEKEY,'B');
	msgid = msgget(ipckey,0);
	// семафоры: 0 - порядковые номера участников (макс US)
	// 			1 - счетчик записей чата( макс MAXMESS)
	//			2 - блокировка shared memory
	ipckey = ftok(FILEKEY,'C');
	semid = semget(ipckey,3,0);

	// поток заполнения чата
	all_mess.semid=semid;
	all_mess.wnd=wnd;
	pthread_create(&tidChat,NULL,write_chat,arg_chat);

	 // вводим ник участника
	wprintw(wnd_message,"Input name: ");
	wrefresh(wnd_message);
	wgetnstr(wnd_message,msg.name,MAXUSERNAME);
	// получаем номер предыдущего участника из семафора 0
	msg.mtype = semctl(semid,0,GETVAL,0)+1;
	// отправляем стартовое сообщение для регистрации
	msgsnd(msgid,&msg,MAXUSERNAME+MAXTEXT,0);
	userNumber = msg.mtype;
	// индивидуальный тип сообщений участника
	msg.mtype=(20+userNumber);

	// цикличная отправка сообщений
	while(strcmp(msg.mtext,TEXT_EXIT)){
		wclear(wnd_message);
		wrefresh(wnd_message);
		wprintw(wnd_message,"message: ");
		wgetnstr(wnd_message,msg.mtext,255);
		msgsnd(msgid,&msg,MAXUSERNAME+MAXTEXT,0);
	}
	wclear(wnd_message);
	wclear(wnd);
	delwin(wnd_message);
	delwin(wnd);
	endwin();
	pthread_join(tidChat, NULL); // завершение потока-чата
	return 0;
}
