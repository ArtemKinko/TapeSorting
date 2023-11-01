#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <cmath>

#include <vector>

class Tape
{
private:
	std::fstream tape;
	std::fstream space_tape;
	long tape_position;
	std::string path_to_tape;
	std::string path_to_finish_tape;
	bool is_line_digit(std::string);

public:
	~Tape();
	Tape(std::string, std::string);
	bool load_config_from_file(std::string);
	void sort_tape();
};

