#include "Tape.h"


Tape::Tape(std::string path_to_tape, std::string path_to_finish_tape) {
	this->tape_position = 0;
	this->path_to_tape = path_to_tape;
	this->path_to_finish_tape = path_to_finish_tape;
	this->tape.open(path_to_tape, std::fstream::in | std::fstream::out);

	// ������������ ��� ������ � ����� �� ��������� � ����������� ��������
	this->space_tape.open("tmp/space_tape.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
	std::string line;

	while (std::getline(this->tape, line))
	{
		if (this->is_line_digit(line)) {
			this->space_tape << line;
			for (int i = 0; i < (20 - line.length()); i++)
				this->space_tape << " ";
			this->space_tape << std::endl;
		}
	}
	this->space_tape.seekg(this->space_tape.beg);
}

Tape::~Tape() {
	this->tape.close();
	this->space_tape.close();
}

// ����� ��� �������� ������, ����� �� ���
bool Tape::is_line_digit(std::string line) {
	for (int i = 0; i < line.size(); i++)
		if (!(std::isdigit(line[i]) || line[i] == '-' || line[i] == ' ')) {
			return false;
		}
	return true;
}

// ����� ��� �������� ���������� ����� ������������
bool Tape::load_config_from_file(std::string path_to_config) {
	std::ifstream config_file;
	config_file.open(path_to_config, std::ios_base::in);


	// �������� � ����� - ������, � �������� ����� ����� �����������������
	std::vector<unsigned int> bookmarks;
	std::vector<bool> is_write_commands;
	std::vector<int> bookmark_nums;

	// ������� �������
	std::string line;

	// ���� ����������� ����� �� n ����� ��������� ��������
	// 0 - ��� ����������� ��������� ��������
	// -1 - ����������� ������ ��������� ��������
	// 1 - ����������� ������� ��������� ��������
	short swap_mode = 0;

	// ���� ������ ��������
	bool is_next_to_write = false;

	// ����������� ������ ������, � ������� ����������, �������� ������������
	long max_tape_position = 0;

	// ������������� ������� �� �����
	while (std::getline(config_file, line))
	{
		// ������� ����������� ����� ������
		if (line == "<")
			swap_mode = 1;
		// ������� ����������� ����� �����
		else if (line == ">")
			swap_mode = -1;
		// ������� ��� ������ � ������
		else if (line == "I") {
			is_next_to_write = true;
		}

		// ������� ��� ������ �� ������
		else if (line == "O") {
			bookmarks.push_back(this->tape_position);
			bookmark_nums.push_back(0);
			is_write_commands.push_back(false);
		}
		else {
			// ���������, ����� �� ��������� �������
			bool is_digit = this->is_line_digit(line);
			

			// ���� ���������� ������� - ���� �����������, � ������� - �����, ���������� �����
			if (swap_mode != 0 && is_digit) {
				this->tape_position = this->tape_position + (std::stoi(line) * swap_mode);
				max_tape_position = std::max(this->tape_position, max_tape_position);
				is_next_to_write = false;
				swap_mode = 0;
				
				// ��������� ���������� ����� �����
				if (this->tape_position < 0) {
					std::cout << "���������� ����� ����� (�����). ���������� ��������� ������������.\n";
					return false;
				}
			}
			// ���������� ������� - ������, ������� - �����, ���������� � ������
			else if (is_next_to_write && is_digit) {
				bookmarks.push_back(this->tape_position);
				bookmark_nums.push_back(std::stoi(line));
				is_write_commands.push_back(true);
				is_next_to_write = false;

			}
			else {
				// ����� ��� ���������� �������
				std::cout << "�������� ���������. ������� ����� ��� ���������� ������� ������/������ �����. ���������� ��������� ������������.\n";
				return false;
			}
			
		}
	}

	int number_of_lines = 0;
	std::vector<std::streampos>* bookmark_pos = new std::vector<std::streampos>(bookmarks.size());
	// ��������� ����� � ������� ���������� �����, ����������� ��������
	while (std::getline(this->space_tape, line)) {
		if (line == "") {
			continue;
		}
		else if (this->is_line_digit(line)) {
			for (int i = 0; i < bookmarks.size(); i++) {
				if (bookmarks.at(i) == (number_of_lines + 1)) {
					bookmark_pos->at(i) = this->space_tape.tellg();
				}
			}
			number_of_lines += 1;
		}
		else {
			std::cout << "����� �������� ��������� ������. ���������� ��������� ������������.\n";
			return false;
		}
	}

	// ��������� ����������� �������� ������
	if (max_tape_position >= (number_of_lines)) {
		std::cout << "���������� ����� ����� (������). ���������� ��������� ������������.\n";
		this->tape.close();
		return false;
	}
	else {
		// ������������ ��� ������� ������� � ������
		this->space_tape.clear();
		for (int i = 0; i < bookmarks.size(); i++) {
			// ������� �������� ������ � �������
			if (!is_write_commands[i]) {
				this->space_tape.seekg(bookmark_pos->at(i));
				std::getline(this->space_tape, line);
				std::cout << "�������� ������: " << line << "\n";
				this->space_tape.clear();
			}
			// �������� �������� ������
			else {
				std::string spaces = "";
				for (int i = 0; i < line.length(); i++) {
					spaces += " ";
				}
				this->space_tape.seekg(bookmark_pos->at(i));
				this->space_tape.write(spaces.c_str(), spaces.length());
				this->space_tape.seekg(bookmark_pos->at(i));
				this->space_tape.write(std::to_string(bookmark_nums[i]).c_str(), std::to_string(bookmark_nums[i]).length());
				std::cout << "�������� � ������ �����: " << bookmark_nums[i] << std::endl;
			}
		}
	}

	std::cout << "������������ ������� ���������.\n";
	return true;
}

// ����������� ���������� ����� � ���������
void Tape::sort_tape() {
	// ���� ��� ��������, ��������� ���� �����
	bool got_any_nozero = true;
	int current_dec = 0;

	// ��������� ����� ����� ����������� ����������
	std::fstream merge_tape;
	merge_tape.open(this->path_to_finish_tape, std::fstream::in | std::fstream::out | std::fstream::trunc);
	this->space_tape.seekg(this->space_tape.beg);
	merge_tape << this->space_tape.rdbuf();

	// ������������ ���������� (� �������)
	while(got_any_nozero) {
		// ��������� ��� �����
		// ������ ������ ��� ������� �� �������� �������
		std::vector<std::fstream*> dec_files;
		for (int i = -9; i < 10; i++) {
			std::fstream* dec_file = new std::fstream;
			dec_file->open("tmp/dec_file" + std::to_string(i) + ".txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
			dec_files.push_back(dec_file);
		}

		got_any_nozero = false;
		merge_tape.seekg(merge_tape.beg);

		// ��� ������ ������ ������� ������� ������ � ��������������� �����
		std::string line;
		while (std::getline(merge_tape, line)) {
			if (line == "")
				continue;
			int current_file_num = (std::stoi(line));
			if (current_dec > 0)
				current_file_num /= std::pow(10, current_dec);
			current_file_num %= 10;
			if (current_file_num != 0)
				got_any_nozero = true;
			dec_files.at(current_file_num + 9)->write(line.c_str(), line.length());
			dec_files.at(current_file_num + 9)->write("\n", 1);
		}
		current_dec += 1;

		if (!got_any_nozero)
			break;
		// ��������� ��� ����� � ���� �����
		merge_tape.close();
		merge_tape.open(this->path_to_finish_tape, std::fstream::in | std::fstream::out | std::fstream::trunc);
		for (int i = 0; i < dec_files.size(); i++) {
			int tape_size = dec_files[i]->tellg();
			if (tape_size > 0) {
				dec_files[i]->seekg(dec_files[i]->beg);
				merge_tape << dec_files[i]->rdbuf();
			}
			// ��������� ����� ��� �� ������� �� ��������� �����
			dec_files.at(i)->close();
			delete dec_files.at(i);
		}
	}
	std::cout << "���������� ������� ���������." << std::endl;
}