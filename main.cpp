#include "command/command_find_between_value.h"
#include "chain_of_responsibility/request_handler_with_command_chain.h"
#include "command/command_delete_collection.h"
#include "command/command_add_collection.h"
#include "command/command_delete_scheme.h"
#include "logger/client_logger_builder.h"
#include "command/command_delete_value.h"
#include "command/command_update_value.h"
#include "command/command_delete_pool.h"
#include "command/command_find_value.h"
#include "command/command_add_scheme.h"
#include "command/command_add_value.h"
#include "command/command_add_pool.h"
#include "containers/database.h"
#include <iostream>
#include <cstring>
#include <string>

#define FILESYSTEM "./FILESYSTEM"
#define SAVEFILE "./file_save.txt"

int is_leap_year(const int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int is_valid_time_ISO_8601(const char *dateString) {
    // 2024-02-22T18:09:22Z
    // YYYY-MM-DDTHH:MM:SSZ
    // 0123456789abcdefghij
    if (strlen(dateString) != 20) {
        return 0; 
    }

    for (int i = 0; i < 20; i++) {
        if (i == 4 || i == 7) {
            if (dateString[i] != '-') {
                return 0; 
            }
        }
        else if (i == 13 || i == 16) {
            if (dateString[i] != ':') {
                return 0; 
            }
        }
        else if (i == 10) {
            if (dateString[i] != 'T') {
                return 0;
            }
        }
        else if (i == 19) {
            if (dateString[i] != 'Z') {
                return 0;
            }
        }
        else {
            if (!isdigit(dateString[i])) {
                return 0;
            }
        }
    }

    int year, month, day, hour, minute, second;
    sscanf(dateString, "%d-%d-%dT%d:%d:%dZ", &year, &month, &day, &hour, &minute, &second);

    if (year < 1000 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31 ||
     hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        return 0; 
    }

    int daysInMonth[] = {31, 28 + is_leap_year(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (day > daysInMonth[month - 1]) {
        return 0; 
    }

    return 1; 
}

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::tm time_info = *std::localtime(&current_time);

    std::stringstream ss;
    ss << std::put_time(&time_info, "%Y-%m-%dT%H:%M:%SZ");
    
    return ss.str();
}

void restore(request_with_command_chain& _chain, std::string date_time)
{
    if (std::filesystem::exists(FILESYSTEM))
    {
        try
        {
            std::filesystem::remove_all(FILESYSTEM);
            logger_singleton::get_instance()->get_logger()->trace("directory FILESYSTEM has deleted");
        }
        catch (const std::exception &e)
        {
            logger_singleton::get_instance()->get_logger()->error("error deleting the directory FILESYSTEM");
        }
    }
    std::filesystem::create_directory(FILESYSTEM);
    enums::mode g = database::get_instance(3)->get_mode();
    database::clear();
    database::get_instance(3)->set_mode(enums::mode::file_system);
    std::ifstream new_file(SAVEFILE);
    std::string command_string;

    std::vector<std::string> requests = {};

    while (std::getline(new_file, command_string))
    {
        std::istringstream string_with_commands(command_string);
        std::string date;
        string_with_commands >> date;
        std::cout << command_string << std::endl;
        std::string date_time_activity_started = date_time;

        if(date_time_activity_started < date)
            break;
        requests.push_back(command_string);
    }
        
    file_save::clean_file();
    for(int i = 0; i < requests.size(); i++)
    {
        if (!_chain.handle(requests[i]))
        {
            throw std::logic_error("wrong command or smth goes bad");
        }
    }
    database::get_instance(3)->set_mode(g);
}

int main(int argc, char *argv[])
{
    argc = 3;
    argv[1] = "in_memory";
    argv[2] = "../kchau.txt";
    if (argc > 3 || argc < 2)
    {
        std::cout << argv[1] << std::endl;
        std::cout << "<program> <type: in_memory, files> <file_path:optional>" << std::endl;
        std::cout << "try again hm..";
        return 1;
    }

    logger_singleton::get_instance()->get_logger()->log("start work database", logger::severity::trace);

    size_t t = 3;
    database::get_instance(t)->set_t(t);

    request_with_command_chain _chain;
    _chain
            .add_handler(new command_add_pool())
            .add_handler(new command_add_scheme())
            .add_handler(new command_add_collection())
            .add_handler(new command_add_value())
            .add_handler(new command_update_value())
            .add_handler(new command_find_value())
            .add_handler(new command_find_between_value())
            .add_handler(new command_delete_value())
            .add_handler(new command_delete_collection())
            .add_handler(new command_delete_scheme())
            .add_handler(new command_delete_pool());


    std::string mode(argv[1]);

    std::cout << "print date in int64_t to restore from that moment (or 0 for actual version)" << std::endl;
    std::cout << "YYYY-MM-DDTHH:MM:SSZ" << std::endl;
    std::string s;
    getline(std::cin, s);
    if(!(s == "0"))
    {
        if(!is_valid_time_ISO_8601(s.c_str()))
        {
            std::cout << "try again hm...";
            return 1;
        }
        restore(_chain, s);
    }
    else
        restore(_chain, getCurrentTime());
    

    if(mode == "in_memory")
    {
        database::get_instance(t)->set_mode(enums::mode::in_memory_cache);
        logger_singleton::get_instance()->get_logger()->log("in memory cache", logger::severity::trace);
    }
    else if (mode == "files")
    {
        database::get_instance(t)->set_mode(enums::mode::file_system);
        logger_singleton::get_instance()->get_logger()->log("wrong mode operating", logger::severity::trace);
    }
    else
    {
        std::cerr << "Wrong first arg: expected in_memory/files" << std::endl;
        return 1;
    }

    std::vector<std::string> requests = {};

    std::string command_string;

    try {
        if(argc == 3)
        {
            std::ifstream input_file(argv[2]);

            if (!input_file.is_open())
            {
                std::cerr << "error with opening file" << std::endl;
                input_file.close();
                return 1;
            }

            while (std::getline(input_file, command_string))
            {
                std::cout << command_string << std::endl;
                std::string date_time_activity_started;

                date_time_activity_started = getCurrentTime();

                if (!_chain.handle(date_time_activity_started + " " + command_string))
                {
                    throw std::logic_error("wrong command or smth goes bad");
                }
                if(database::get_instance(t)->get_mode() == enums::mode::in_memory_cache)
                    requests.push_back(date_time_activity_started + " " + command_string);
            }
            input_file.close();
        }

        std::cout << "please enter a command: " << std::endl;

        while (std::getline(std::cin, command_string))
        {
            std::cout << command_string << std::endl;
            std::string date_time_activity_started;

            date_time_activity_started = getCurrentTime();

            if (command_string == "exit")
            {
                break;
            }
            if(command_string == "path")
            {
                std::getline(std::cin, command_string);
                std::ifstream input_file(command_string);

                if (!input_file.is_open())
                {
                    std::cerr << "error with opening file" << std::endl;
                    input_file.close();
                    return 1;
                }

                while (std::getline(input_file, command_string))
                {
                    std::cout << command_string << std::endl;
                    std::string date_time_activity_started;

                    date_time_activity_started = getCurrentTime();

                    if (!_chain.handle(date_time_activity_started + " " + command_string))
                    {
                        throw std::logic_error("wrong command or smth goes bad");
                    }
                    if(database::get_instance(t)->get_mode() == enums::mode::in_memory_cache)
                        requests.push_back(date_time_activity_started + " " + command_string);
                }
                input_file.close();
            }
            if (!_chain.handle(date_time_activity_started + " " + command_string))
            {
                std::cout << "You entered: " << command_string << std::endl;
                throw std::logic_error("wrong command or smth goes bad");
            }
            if(database::get_instance(t)->get_mode() == enums::mode::in_memory_cache)
                requests.push_back(date_time_activity_started + " " + command_string);
        }
        if(database::get_instance(t)->get_mode() == enums::mode::in_memory_cache)
        {
            std::string word;
            std::cout << "Do you want to save the data? Please write 'yes' or 'no':" << std::endl;
            std::cin >> word;

            if (word == "no")
            {
                std::cout << "Data will not be saved" << std::endl;
            }
            else if (word == "yes")
            {
                std::cout << "Data will be saved" << std::endl;
                for(int i = 0; i < requests.size(); i++)
                {
                    file_save::file_for_save(requests[i]);
                }   
            }
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    
    return 0;
}