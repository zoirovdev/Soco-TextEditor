#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <stdint.h>

#define ctrl(x) ((x) & 0x1f)
#define BACKSPACE  127
#define ESCAPE     27
#define ENTER      10

typedef enum {
	NORMAL,
	INSERT
} Mode;

typedef struct {
	size_t index;
	size_t size;
	char* contents;
} Row;

typedef struct {
	char *buf;
	Row rows[1024];
	size_t row_index;
	size_t cur_pos;
	size_t row_s;
} Buffer;



Mode mode = NORMAL;
int QUIT = 0;

char *stringify_mode(){
	switch(mode){
		case NORMAL:
			return "NORMAL";
		case INSERT:
			return "INSERT";
		default:
			return "NORMAL";
	}
}

int main(void){
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	Buffer buffer = {0};
	for(size_t i=0; i<1024; i++){
		buffer.rows[i].contents = calloc(1024, sizeof(char));
	}

	int row, col;
	(void)col;
	getmaxyx(stdscr, row, col);
	
	mvprintw(row-1, 0, stringify_mode());
	move(0,0);
	
	int ch = 0;

	size_t x = 0,y = 0;
	while(ch != ctrl('q') && QUIT != 1){
		refresh();
		mvprintw(row-1, 0, stringify_mode());
		mvprintw(row-1, col-7, "%.3zu:%.3zu", buffer.row_index, buffer.cur_pos);


		for(size_t i=0; i<=buffer.row_s; i++){
			mvprintw(i, 0, buffer.rows[i].contents);
		}

		
		move(y, x);
		ch = getch();
		switch(mode){
			case NORMAL:
				x = buffer.cur_pos;
				y = buffer.row_index;
				if(ch == 'i'){
					mode = INSERT;
				}else if(ch == 'h'){
					if(buffer.cur_pos != 0){
						buffer.cur_pos -= 1;
						move(y, x-1);
					}
				}else if(ch == 'l'){
					if(buffer.rows[buffer.row_index].size > 0){
						if(x >= buffer.rows[buffer.row_index].size-1) {
							x = buffer.rows[buffer.row_index].size-1;	
						} else {
							buffer.cur_pos += 1;
						}
						move(y, x+1);
					}
				}else if(ch == 'k'){
					if(y != 0){
						if(x >= buffer.rows[buffer.row_index--].size-1) x = buffer.rows[buffer.row_index].size-1;
						move(y-1, x);
					}
				}else if(ch == 'j'){
					if(y >= buffer.row_s)  
						y = buffer.row_s;
						buffer.cur_pos = y;
					else { 
						y++; 
						if(x > buffer.rows[buffer.row_index++].size-1) x = buffer.rows[buffer.row_index].size-1;	
					}
					move(y, x);	
				}else if(ch == ctrl('s')){
					FILE *file = fopen("put.txt", "w");
					for(size_t i=0; i<=buffer.row_s; i++){
						fwrite(buffer.rows[i].contents, buffer.rows[i].size, 1, file);
					}
					fclose(file);
					QUIT = 1;
				}

				break;
			case INSERT:{
				keypad(stdscr, FALSE);
				if(ch == BACKSPACE){
					getyx(stdscr, y, x);
					if(buffer.cur_pos == 0){
						if(buffer.row_index != 0){
							Row *cur = &buffer.rows[--buffer.row_index];
							buffer.cur_pos = cur->size;
							move(buffer.row_index, buffer.cur_pos);
						}
					} else {
						Row *cur = &buffer.rows[buffer.row_index];
						cur->contents[--buffer.cur_pos] = ' ';
						cur->size = buffer.cur_pos;
						move(y, buffer.cur_pos);
					}
				} else if(ch == ESCAPE){
					mode = NORMAL;
					keypad(stdscr, TRUE);
				} else if(ch == ENTER){
					buffer.rows[buffer.row_index].contents[buffer.rows[buffer.row_index].size] = '\n';
					buffer.row_index++;
					buffer.row_s++;
					buffer.cur_pos = 0;
					move(buffer.row_index, buffer.cur_pos);
				} else {
					Row *cur = &buffer.rows[buffer.row_index];
					cur->contents[buffer.cur_pos++] = ch;
					cur->size = buffer.cur_pos;
					move(y, buffer.cur_pos);
				}
				break;
			}
		}
		getyx(stdscr, y, x);
	}

	free(buffer.buf);
	refresh();
	getch();
	endwin();

	return 0;
}
