#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

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

#define MAX_ROWS 1025
typedef struct {
	char *buf;
	Row rows[MAX_ROWS];
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

#define MAX_STRING_SIZE 1025

void shift_rows(Buffer *buf, size_t index){
	assert(buf->row_s + 1 < MAX_ROWS);
	Row new_rows[MAX_ROWS] = {0};
	memcpy(new_rows, buf->rows, sizeof(Row)*buf->row_s);
	for(size_t i=index; i<buf->row_s; i++){
		new_rows[i+1] = buf->rows[i];
	}

	buf->row_s++;
	for(size_t i=index; i<buf->row_s; i++){
		buf_rows[i] = new_rows[i];
	}

}

void shift_str(char *dest, size_t *dest_s, char *str, size_t *str_s, size_t index){
	assert(index < MAX_STRING_SIZE);
	*dest_s = (*str_s - index);
	for(size_t i=index; i<*str_s; i++){
		dest[i % index] = str[i];
		str[i] = '\0';
	}
	*str_s = index+1;
}

int main(void){
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	Buffer buffer = {0};
	for(size_t i=0; i<1024; i++){
		buffer.rows[i].contents = calloc(MAX_STRING_SIZE, sizeof(char));
	}

	int row, col;
	(void)col;
	getmaxyx(stdscr, row, col);
	
	mvprintw(row-1, 0, stringify_mode());
	move(0,0);
	
	int ch = 0;

	size_t x = 0,y = 0;
	while(ch != ctrl('q') && QUIT != 1){
		clear();
		getmaxyx(stdscr, row, col);
		refresh();
		mvprintw(row-1, 0, stringify_mode());
		mvprintw(row-1, col/2, "%.3zu:%.3zu", buffer.row_index, buffer.cur_pos);


		for(size_t i=0; i<=buffer.row_s; i++){
			mvprintw(i, 0, buffer.rows[i].contents);
		}

		
		move(y, x);
		ch = getch();
		switch(mode){
			case NORMAL:
				if(ch == 'i'){
					mode = INSERT;
				}else if(ch == 'h'){
					if(buffer.cur_pos != 0) buffer.cur_pos--;
				}else if(ch == 'l'){
					if(buffer.cur_pos < buffer.rows[buffer.row_index].size) buffer.cur_pos++;
				}else if(ch == 'k'){
					if(buffer.row_index != 0) buffer.row_index--;
				}else if(ch == 'j'){
					if(buffer.row_index < buffer.row_s) buffer.row_index++;
				}else if(ch == ctrl('s')){
					FILE *file = fopen("put.txt", "w");
					for(size_t i=0; i<=buffer.row_s; i++){
						fwrite(buffer.rows[i].contents, buffer.rows[i].size, 1, file);
						fwrite("\n", sizeof("\n")-1, 1, file);
					}
					fclose(file);
					QUIT = 1;
				}
				if(buffer.cur_pos > buffer.rows[buffer.row_index].size) buffer.cur_pos = buffer.rows[buffer.row_index].size;
				x = buffer.cur_pos;
				y = buffer.row_index;
				move(y, x);
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
					Row *cur = &buffer.rows[buffer.row_index];
					Row *next = &buffer.rows[buffer.row_index+1];
					shift_rows(&buffer, buffer.row_index);
					shift_str(next->contents, &next->size, cur->contents, &cur->size, buffer.cur_pos);
					//cur->contents[cur->size] = ' ';
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
