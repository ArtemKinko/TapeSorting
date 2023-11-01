#include <iostream>
#include "Tape.h"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    if (argc >= 3) {
        Tape* tape = new Tape(argv[1], argv[2]);
        //bool is_loaded_config = tape->load_config_from_file("config_test.txt");
        bool is_loaded_config = tape->load_config_from_file(argc == 4 ? argv[3] : "");
        if (is_loaded_config)
            tape->sort_tape();
    }
    else {
        std::cout << "Недостаточное количество параметров. Запуск в режиме по-умолчанию. Попробуйте: <./program start_tape.txt finish_tape.txt config_test.txt>\n";
        Tape* tape = new Tape("start_tape.txt", "finish_tape.txt");
        bool is_loaded_config = tape->load_config_from_file("config_test.txt");
        if (is_loaded_config)
            tape->sort_tape();
    }
}
