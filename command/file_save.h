#ifndef COURSE_PROJECT_FILE_SAVE_H
#define COURSE_PROJECT_FILE_SAVE_H
#include <iostream>
#include <fstream>
#include "../logger/logger_singleton.h"
#include "../containers/database.h"

class file_save
{

private:

public:

    file_save() = default;

public:

    file_save(file_save const& other) = delete;

    file_save(file_save const&& other) = delete;

    file_save& operator=(file_save const& other) = delete;

    file_save& operator=(file_save const&& other) = delete;

public:

    static void file_for_save(std::string const& request)
    {
        logger_singleton::get_instance()->get_logger()->trace("hello");
        std::string _file_name = "file_save.txt";

        std::cout << _file_name << std::endl;

        std::ofstream file_save(_file_name, std::ios::app);

        if (!file_save.is_open())
        {
            logger_singleton::get_instance()->get_logger()->error("error with opening file for saving data");
        }

        file_save << request << std::endl;
        file_save.close();
    }

    static value_in_memory_cache deserialization(std::string const& pool_name, std::string const& scheme_name,
                                          std::string const& collection_name, std::string const& key,
                                          std::string& path)
    {
        logger_singleton::get_instance()->get_logger()->trace("start deserialization method");
        logger_singleton::get_instance()->get_logger()->trace("file name: " + path);

        std::ifstream file_save(path);

        if (!file_save.is_open())
        {
            logger_singleton::get_instance()->get_logger()->error("error with opening file for saving data");
            throw std::logic_error("error with opening file");
        }

        std::string ab;
        getline(file_save, ab);

        std::istringstream iss(ab);

        int id;
        int age;
        std::string date_missed;
        bool status_found;
        std::string city;
        std::string info;

        iss >> id >> age >> date_missed >> status_found >> city >> info;

        value_in_memory_cache value_memory(id, age, date_missed, status_found, city, info);

        return value_memory;
    }

    static void clean_file()
    {
        std::ofstream ofs;
        std::string _file_name = "file_save.txt";
        ofs.open(_file_name, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }


};

#endif //COURSE_PROJECT_FILE_SAVE_H
