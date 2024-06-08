#ifndef COURSE_PROJECT_VALIDATE_PARAMETERS_H
#define COURSE_PROJECT_VALIDATE_PARAMETERS_H
#include <iostream>
#include <cstring>

class validate_parameters
{
//private:
//
//    std::string _t_string;
//
//public:
//
//    std::string get_t_string()
//    {
//        return _t_string;
//    }
//
//    validate_parameters(std::string t_string) :
//            _t_string(t_string)
//    {
//
//    }

public:

    static bool validate_file_system_or_memory_cache(std::string const &p)
    {
        for (char symbol : p)
        {
            if (symbol == '1' || symbol == '0')
            {
                return true;
            }
        }

        return false;
    }

    static bool validate_file_for_file_system(const std::string &input_value)
    {
        for (char symbol : input_value)
        {
            if (!std::isalnum(symbol) && symbol != '_')
            {
                return false;
            }
        }

        return true;
    }

    static std::string& validate_path(std::string &sub_path)
    {
        for (char symbol : sub_path)
        {
            if (std::isalnum(symbol) && symbol == '_')
            {
                return sub_path;
            }
        }
        throw std::logic_error("path has invalid symbols");
    }

    static bool validate_input_file_path(std::string &input_file_path)
    {
        std::string extension = ".txt";

        if (input_file_path.length() >= extension.length() &&
            input_file_path.compare(input_file_path.length() - extension.length(), extension.length(), extension) == 0)
        {
            return true;
        }

        return false;
    }

public:

    validate_parameters(validate_parameters const &other) = delete;

    validate_parameters(validate_parameters const &&other) = delete;

    validate_parameters& operator=(validate_parameters const &other) = delete;

    validate_parameters& operator=(validate_parameters const &&other) = delete;

    ~validate_parameters() = default;
};

#endif //COURSE_PROJECT_VALIDATE_PARAMETERS_H
