/*
 * main.c
 *  Сервер открытого чата
 *  Created on: 20.01.2018
 *      Author: jake
 */

#include "add.h"

int main(void)
{
	WINDOW *wnd, *wnd_chat, *wnd_users;
	int messNumber = 0; // локальный счетчик сообщений
	int userNumber = 0; // локальный счетчик участников
	int i;
	key_t ipckey;
	int msgid,semid,shmid;
	char (*shmaddr)[MAXUSERNAME+MAXTEXT+2];
	struct {
		long mtype;
		char name[MAXUSERNAME];
		char mtext[MAXTEXT];
	}msg;
	struct sembuf newUser={0,1,0};//семафор-счетчик участников
	struct sembuf newMessage={1,1,0};// семафор-счетчик сообщений
	struct sembuf lock[2]={{2,0,0},{2,1,0}};
	//lock[0].sem_num = 0; /* ждем, пока первый элемент не станет равен нулю */
	//lock[0].sem_op = 0;
	//lock[0].sem_flg = 0;
	//lock[1].sem_num = 0; /* затем увеличиваем [0] на 1 */
	//lock[1].sem_op = 1;
	//lock[1].sem_flg = SEM_UNDO;
	struct sembuf unlock={2,-1,0};

	//memset(msg.mtext, 0, 256); // очистка текста
	//memset(msg.name, 0, 8); // очистка имени

	// отрисовка окон чата
	initscr(); // инициализация структуры данных в режиме ncurses
	signal (SIGWINCH, sig_winch);
	cbreak(); // включение неканонического режима ввода данных
	start_color();
	refresh();
	wnd = newwin(32,40,1,1); // фоновое окно
	box(wnd,'||','=');
	wrefresh(wnd);
	wnd_chat = derwin(wnd,wnd->_maxy-2,wnd->_maxx-MAXUSERNAME-3,1,1);
	wnd_users = derwin(wnd,wnd->_maxy-2,MAXUSERNAME+2,1,wnd_chat->_maxx+2);

	// очередь сообщений между сервером и участниками
	ipckey = ftok(FILEKEY,'B');
	msgid = msgget(ipckey,IPC_CREAT | 0666);
	// семафоры: 0 - порядковые номера участников (макс US)
	// 			1 - счетчик записей чата( макс MAXMESS)
	//			2 - блокировка shared memory
	ipckey = ftok(FILEKEY,'C');
	semid = semget(ipckey,3,IPC_CREAT | 0666);
	semctl(semid,0,SETALL,0);//инициализация нулями
	// для открытого чата
	ipckey = ftok(FILEKEY,'D');
	shmid = shmget(ipckey, (MAXMESS+1)*(MAXTEXT+MAXUSERNAME+2),IPC_CREAT | 0666);
	shmaddr = (char*)shmat(shmid,0,0);

	// цикличное получение сообщений от участников
	while(strcmp(msg.mtext,TEXT_EXIT)){
		// если получили стартовое (type 1..20) сообщение от нового участника
		if (msgrcv(msgid,&msg,MAXTEXT+MAXUSERNAME,(long)(semctl(semid,0,GETVAL,0)+1),IPC_NOWAIT) > 0)
		// увеличиваем семафор 1
		if(semop(semid,&newUser,1)==0) {
			//memset(shmaddr, 0, 266); // очистка подмассива-сообщения
			semop(semid,&lock,2);
			strcat(shmaddr,msg.name); // запись о новом участнике
			strcat(shmaddr,"\n");
			semop(semid,&unlock,1);
			wprintw(wnd_users,"%s\n", msg.name);
			wrefresh(wnd_users);
		}
	// проверка сообщений от зарегистрированных участников
		for (i=1;i<=semctl(semid,0,GETVAL,0);i++){
			if (msgrcv(msgid,&msg,MAXTEXT+MAXUSERNAME,(long)(US+i),IPC_NOWAIT) > 0){
				if (semop(semid,&newMessage,1) == 0) messNumber++;// инкремент счетчиков сообщений чата
				if (messNumber%20 == 0) messNumber = 0;// перезапись после 20 сообщений
				semop(semid,&lock,2);
				memset(shmaddr+messNumber, 0, 266); // очистка подмассива-сообщения
				semop(semid,&unlock,1);
				if (!strcmp(msg.mtext,TEXT_EXIT)) // при получении кодового слова пересылаем его в чат
					semop(semid,&lock,2);
					strcpy(shmaddr+messNumber,msg.mtext); else {
					strcpy(shmaddr+messNumber,msg.name); // запись в разд.память с 1 по 20 массивы
					strcat(shmaddr+messNumber,"->");
					strcat(shmaddr+messNumber,msg.mtext);
					semop(semid,&unlock,1);
					}
				semop(semid,&lock,2);
				wprintw(wnd_chat,"%s\n", shmaddr+messNumber);
				semop(semid,&unlock,1);
				wrefresh(wnd_chat);
				}
		}
	}
	semctl(semid,0,IPC_RMID,0);
	shmctl(shmid,IPC_RMID,0);
	msgctl(msgid,IPC_RMID,0);

	wclear(wnd_chat);
	wclear(wnd_users);
	wclear(wnd);
	delwin(wnd_chat);
	delwin(wnd_users);
	delwin(wnd);
	endwin();
}
