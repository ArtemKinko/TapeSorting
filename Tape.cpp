#include "Tape.h"


Tape::Tape(std::string path_to_tape, std::string path_to_finish_tape) {
	this->tape_position = 0;
	this->path_to_tape = path_to_tape;
	this->path_to_finish_tape = path_to_finish_tape;
	this->tape.open(path_to_tape, std::fstream::in | std::fstream::out);

	// переписываем все данные с ленты на временную с добавлением пробелов
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

// метод для проверки ячейки, число ли она
bool Tape::is_line_digit(std::string line) {
	for (int i = 0; i < line.size(); i++)
		if (!(std::isdigit(line[i]) || line[i] == '-' || line[i] == ' ')) {
			return false;
		}
	return true;
}

// метод для загрузки первичного файла конфигурации
bool Tape::load_config_from_file(std::string path_to_config) {
	std::ifstream config_file;
	config_file.open(path_to_config, std::ios_base::in);


	// закладки в файле - ячейки, с которыми нужно будет взаимодействовать
	std::vector<unsigned int> bookmarks;
	std::vector<bool> is_write_commands;
	std::vector<int> bookmark_nums;

	// текущая команда
	std::string line;

	// флаг перемещения ленты на n строк следующей командой
	// 0 - нет перемещения следующей командой
	// -1 - перемещение налево следующей командой
	// 1 - перемещение направо следующей командой
	short swap_mode = 0;

	// флаг записи элемента
	bool is_next_to_write = false;

	// макимальная правая ячейка, к которой обращаемся, согласно конфигурации
	long max_tape_position = 0;

	// просматриваем команды из файла
	while (std::getline(config_file, line))
	{
		// команда перемещения ленты вправо
		if (line == "<")
			swap_mode = 1;
		// команда перемещения ленты влево
		else if (line == ">")
			swap_mode = -1;
		// команда для записи в ячейку
		else if (line == "I") {
			is_next_to_write = true;
		}

		// команда для чтения из ячейки
		else if (line == "O") {
			bookmarks.push_back(this->tape_position);
			bookmark_nums.push_back(0);
			is_write_commands.push_back(false);
		}
		else {
			// проверяем, число ли следующая команда
			bool is_digit = this->is_line_digit(line);
			

			// если предыдущая команда - знак перемещения, и текущая - цифра, перемещаем ленту
			if (swap_mode != 0 && is_digit) {
				this->tape_position = this->tape_position + (std::stoi(line) * swap_mode);
				max_tape_position = std::max(this->tape_position, max_tape_position);
				is_next_to_write = false;
				swap_mode = 0;
				
				// проверяем предельный сдвиг ленты
				if (this->tape_position < 0) {
					std::cout << "Предельный сдвиг ленты (влево). Невозможно выполнить конфигурацию.\n";
					return false;
				}
			}
			// предыдущая команда - запись, текущая - цифра, записываем в ячейку
			else if (is_next_to_write && is_digit) {
				bookmarks.push_back(this->tape_position);
				bookmark_nums.push_back(std::stoi(line));
				is_write_commands.push_back(true);
				is_next_to_write = false;

			}
			else {
				// число без предыдущей команды
				std::cout << "Неверный синтаксис. Найдено число без предыдущей команды записи/сдвига ленты. Невозможно выполнить конфигурацию.\n";
				return false;
			}
			
		}
	}

	int number_of_lines = 0;
	std::vector<std::streampos>* bookmark_pos = new std::vector<std::streampos>(bookmarks.size());
	// считываем ленту и смотрим количество ячеек, расставляем закладки
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
			std::cout << "Лента содержит строковые ячейки. Невозможно выполнить конфигурацию.\n";
			return false;
		}
	}

	// проверяем максимально смещение вправо
	if (max_tape_position >= (number_of_lines)) {
		std::cout << "Предельный сдвиг ленты (вправо). Невозможно выполнить конфигурацию.\n";
		this->tape.close();
		return false;
	}
	else {
		// обрабатываем все команды вставки и чтения
		this->space_tape.clear();
		for (int i = 0; i < bookmarks.size(); i++) {
			// выводим значение ячейки в консоль
			if (!is_write_commands[i]) {
				this->space_tape.seekg(bookmark_pos->at(i));
				std::getline(this->space_tape, line);
				std::cout << "Значение ячейки: " << line << "\n";
				this->space_tape.clear();
			}
			// заменяем значение ячейки
			else {
				std::string spaces = "";
				for (int i = 0; i < line.length(); i++) {
					spaces += " ";
				}
				this->space_tape.seekg(bookmark_pos->at(i));
				this->space_tape.write(spaces.c_str(), spaces.length());
				this->space_tape.seekg(bookmark_pos->at(i));
				this->space_tape.write(std::to_string(bookmark_nums[i]).c_str(), std::to_string(bookmark_nums[i]).length());
				std::cout << "Записали в ячейку число: " << bookmark_nums[i] << std::endl;
			}
		}
	}

	std::cout << "Конфигурация успешно применена.\n";
	return true;
}

// поразрядная сортировка ленты с пробелами
void Tape::sort_tape() {
	// флаг для проверки, обработки всех чисел
	bool got_any_nozero = true;
	int current_dec = 0;

	// временная лента между поразрядным сравнением
	std::fstream merge_tape;
	merge_tape.open(this->path_to_finish_tape, std::fstream::in | std::fstream::out | std::fstream::trunc);
	this->space_tape.seekg(this->space_tape.beg);
	merge_tape << this->space_tape.rdbuf();

	// обрабатываем поразрядно (с меньших)
	while(got_any_nozero) {
		// открываем все файлы
		// вектор файлов для каждого из значений разряда
		std::vector<std::fstream*> dec_files;
		for (int i = -9; i < 10; i++) {
			std::fstream* dec_file = new std::fstream;
			dec_file->open("tmp/dec_file" + std::to_string(i) + ".txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
			dec_files.push_back(dec_file);
		}

		got_any_nozero = false;
		merge_tape.seekg(merge_tape.beg);

		// для каждой ячейки относим текущий разряд к соответсвующему файлу
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
		// совмещаем все файлы в одну ленту
		merge_tape.close();
		merge_tape.open(this->path_to_finish_tape, std::fstream::in | std::fstream::out | std::fstream::trunc);
		for (int i = 0; i < dec_files.size(); i++) {
			int tape_size = dec_files[i]->tellg();
			if (tape_size > 0) {
				dec_files[i]->seekg(dec_files[i]->beg);
				merge_tape << dec_files[i]->rdbuf();
			}
			// закрываем файлы для их очистки на следующем цикле
			dec_files.at(i)->close();
			delete dec_files.at(i);
		}
	}
	std::cout << "Сортировка успешно выполнена." << std::endl;
}