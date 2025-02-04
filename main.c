#include <stdio.h>
#include <curses.h>

#define ctrl(x) ((x) & 0x1f)


int main(void){
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();


	int row;
	int col;
	getmaxyx(stdscr, row, col);
	
	mvprintw(row-1, 0, "Normal");
	move(0,0);
	
	int ch = getch();
	addch(ch);
	while(ch != ctrl('q')){
		ch = getch();
		printw("%d ", ch);
		if(ch == 263){
			int x,y;
			getyx(stdscr, y, x);
			move(y, x-1);
			delch();
		} else addch(ch);
	}

	refresh();
	getch();
	endwin();

	return 0;
}
