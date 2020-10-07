#include <ncurses.h>

#include <cstring>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>

#include "tools.hpp"

int main(int argc, char *argv[]){

	//open file
	int fd = -1;
	if(argc == 2){
		fd = open(argv[1], O_RDWR);
		if(fd == -1){
			std::cerr << "Error: can not open file \'" << argv[1] << "\'" << std::endl;
			return 1;
		}
	}else{
		return 1;
	}

	char *mem = nullptr;

	//get file length
	struct stat file_info;
	if(fd != -1){
		if(fstat(fd, &file_info) == -1){
			std::cerr << "Error: fstat failed" << std::endl;
			close(fd);
			return 1;
		}
		mem = (char*)mmap(nullptr, file_info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(mem == MAP_FAILED){
			std::cerr << "Error: mmap fail" << std::endl;
			return 1;
		}
		//close(fd);
		ff::file_opened = true;
		//do correct c-string
		mem[file_info.st_size] = '\0';
	}

	std::vector<std::string> v;
	fill_vector(v, mem);

	initscr();

	wclear(stdscr);
	raw();
	noecho();
	start_color();
	keypad(stdscr, true);
	
	init_pair(color_pair::p1, COLOR_WHITE, COLOR_BLACK);
	init_pair(color_pair::p2, COLOR_BLACK, COLOR_GREEN);
	init_pair(color_pair::p3, COLOR_BLACK, COLOR_RED);
	init_pair(color_pair::p4, COLOR_YELLOW, COLOR_GREEN);
	init_pair(color_pair::p5, COLOR_BLACK, COLOR_CYAN);
	init_pair(color_pair::p6, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(color_pair::p7, COLOR_BLUE, COLOR_RED);
	init_pair(color_pair::p8, COLOR_BLACK, COLOR_WHITE);
	init_pair(color_pair::p9, COLOR_CYAN, COLOR_MAGENTA);

	int max_width, max_height;
	getmaxyx(stdscr, max_height, max_width);
	wbkgd(stdscr, COLOR_PAIR(color_pair::p1));

	int width, height;
	width = height = 1;
	
	int bar_line_length = number_length(height) + number_length(width) + strlen(line_row_format);
	box(stdscr, 0 ,0);
	bar_line_length = number_length(height) + number_length(width) + strlen(line_row_format);
	print_line_row(max_height-2, max_width-bar_line_length-1, height, width);
	print_saved_status(max_height-2, 1);

	fill_screen(v);
	move(1, 1);
	box(stdscr, 0, 0);
	wrefresh(stdscr);
	bool run = true;

	while(run){
		int ch = wgetch(stdscr);
		wclear(stdscr);
		
		switch(ch){
			case KEY_LEFT:
				key_left(width, height, v);
				break;
			case KEY_RIGHT:
				key_right(width, height, v);
				break;
			case KEY_DOWN:
				key_down(width, height, v);
				break;
			case KEY_UP:
				key_up(width, height, v);
				break;
			case KEY_BACKSPACE:
				key_backspace(width, height, v);
				break;
			case ctrl('c'):
				//exit
				run = false;
				break;
			case ctrl('s'):
				//save
				if(!ff::file_saved){
					save_to_file(argv[1], v);
				}
				break;
			case CUSTOM_ENTER:
				key_enter(width, height, max_height, v);
				break;
			case ALT_LEFT:
				if(color_pair::i > 0){
					color_pair::i--;
				}
				wbkgd(stdscr, COLOR_PAIR(color_pair::i));
				break;
			case ALT_RIGHT:
				if(color_pair::i < color_pair::themes_count-1){
					color_pair::i++;
				}
				wbkgd(stdscr, COLOR_PAIR(color_pair::i));
				break;
			default:
				add_symbol(width, height, ch, v);
				break;
		}

		bar_line_length = number_length(height) + number_length(width) + strlen(line_row_format);
		print_line_row(max_height-2, max_width-bar_line_length-1, height, width);
		print_saved_status(max_height-2, 1);

		fill_screen(v);			

		move(height, width);
		box(stdscr, 0, 0);
		
		wrefresh(stdscr);

	}

	if(fd != -1){
		munmap((void*)mem, file_info.st_size);
	}
	
	endwin();

	return 0;
}
