#pragma once

#include <vector>
#include <string>
#include <ncurses.h>

#define ctrl(x) ((x)&0x1f)

#define CUSTOM_ENTER 10

#define ALT_LEFT 543
#define ALT_RIGHT 558

const char *line_row_format = "Line , Column ";
namespace color_pair{
	int i = 0;
	const int themes_count = 9;
	enum{p1, p2, p3, p4, p5, p6, p7, p8, p9};
}

namespace ff{
	bool file_saved = true;
	bool file_opened = false;
}

int number_length(int x){
	int res = 0;
	do{
		x /= 10;
		res++;
	}while(x > 0);
	return res;
}

void fill_vector(std::vector<std::string> &v, const char *str){
	if(str == nullptr){
		return;
	}
	int i = 0;
	while(str[i] != '\0'){
		std::string temp_str = "";
		while(str[i] != '\0'){
			if(str[i] == '\n'){
				i++;
				break;
			}
			temp_str += str[i];
			i++;
		}
		v.push_back(temp_str);
	}
}

void print_line_row(int x, int y, int height, int width){
	move(x, y);
	attron(A_REVERSE);
	printw("Line %d, Column %d", height, width);
	attroff(A_REVERSE);
}

void print_saved_status(int height, int width){
	move(height, width);
	attron(A_REVERSE);
	if(ff::file_saved){
		printw("[All changes saved]");
	}else{
		printw("[File not saved]");
	}
	attroff(A_REVERSE);
}

void fill_screen(const std::vector<std::string> &v){
	for(int i = 0; i < v.size(); i++){
		move(i+1, 1);
		printw("%s\n", v[i].c_str());
	}
}

void key_right(int &width, int &height, std::vector<std::string> &v){
	int i = height-1;
	int j = width-1;
	if(j == v[i].length() || v[i].empty()){
		//go to line below
		if(i == v.size()-1){
			//no lines below
			return;
		}else{
			height++;
			width = 1;
		}
	}else{
		width++;
	}
}

void key_left(int &width, int &height, std::vector<std::string> &v){
	int i = height-1;
	int j = width-1;
	if(j == 0){
		//go to line up
		if(i == 0){
			//no lines upper
			return;
		}else{
			width = v[i-1].length()+1;
			height--;
		}
	}else{
		width--;
	}
}

void key_up(int &width, int &height, std::vector<std::string> &v){
	int i = height-1;
	int j = width-1;
	if(i == 0){
		//no lines upper
		return;
	}else{
		height--;
		if(v[i-1].length() < width){
			width = v[i-1].length();
			if(v[i-1].empty()){
				width++;
			}
		}
	}
}

void key_down(int &width, int &height, std::vector<std::string> &v){
	int i = height-1;
	int j = width-1;
	if(i == v.size()-1){
		//no lines below
		return;
	}else{
		height++;
		if(v[i+1].length() < width){
			width = v[i+1].length();
			if(v[i+1].empty()){
				width++;
			}
		}
	}
}

void key_backspace(int &width, int &height, std::vector<std::string> &v){
	int i = height-1;
	int j = width-1;
	if(i == 0 && j == 0){
		//left up border, no delete is possible
		return;
	}else if(j == 0){
		//concat strings
		width = v[i-1].length()+1;
		v[i-1] += v[i];
		v.erase(v.begin()+i);
		height--;
	}else{
		//middle or end of the line
		v[i].erase(j-1, 1);
		width--;
	}
	ff::file_saved = false;
}

void key_enter(int &width, int &height, const int max_height, std::vector<std::string> &v){
	int i = height-1;
	int j = width-1;

	if(v.size() > max_height-4){
		return;
	}

	if(j == 0){
		//create new line before current
		v.insert(v.begin()+i, std::string(""));
	}else if(j == v[i].length()){
		//create new line after current
		v.insert(v.begin()+i+1, std::string(""));
	}else{
		//somewhere in the middle...
		if(v[i].empty()){
			v.insert(v.begin()+i+1, "");
		}else{
			std::string str1 = v[i].substr(0, j);
			std::string str2 = v[i].substr(j, std::string::npos);
			v[i] = str1;
			v.insert(v.begin()+i+1, str2);
		}
	}
	height++;
	width = 1;
	ff::file_saved = false;
}

void add_symbol(int &width, int &height, int ch, std::vector<std::string> &v){
	if(ch == '\t'){
		//insert 4 spaces
		v[height-1].insert(width-1, 1, ' ');
		v[height-1].insert(width-1, 1, ' ');
		v[height-1].insert(width-1, 1, ' ');
		v[height-1].insert(width-1, 1, ' ');
		width += 4;
	}else{
		v[height-1].insert(width-1, 1, ch);
		width++;
	}
	ff::file_saved = false;
}

void save_to_file(const char *str, const std::vector<std::string> &v){
	FILE *f = fopen(str, "w");
	if(f == NULL){
		std::cerr << "Can not open file for saving" << std::endl;
		exit(1);
	}
	for(int i = 0; i < v.size(); i++){
		fprintf(f, "%s\n", v[i].c_str());
	}
	fclose(f);
	ff::file_saved = true;
}