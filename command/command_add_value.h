#ifndef COURSE_PROJECT_COMMAND_ADD_VALUE_H
#define COURSE_PROJECT_COMMAND_ADD_VALUE_H
#include "command.h"
#include "../chain__of_resposibility/request_with_command.h"
#include "../chain__of_resposibility/request_with_command_chain.h"


class command_add_value final :
        public command
{

private:

    std::string _pool_name;
    std::string _scheme_name;
    std::string _collection_name;

    int _id;
    int _age;
    std::string _date_missed;
    bool _status_found;
    std::string _city;
    std::string _info;
    std::string _date;

    long _position = 0;
    long _size = 0;

public:

    bool can_execute(std::string const& request)  override
    {
        try {
            logger_singleton::get_instance()->get_logger()->trace("start can_execute add value");

            std::istringstream string_with_commands(request);
            string_with_commands >> _date;
            std::string command;
            string_with_commands >> command;

            if (command == "ADD_VALUE") {
                std::string pool_name;
                std::string scheme_name;
                std::string collection_name;

                string_with_commands >> pool_name >> scheme_name >> collection_name >> _id >> _age >> _date_missed
                                     >> _status_found >> _city >> _info;
                value_in_memory_cache val(_id, _age, _date_missed, _status_found, _city, _info);
                _pool_name = pool_name;
                _scheme_name = scheme_name;
                _collection_name = collection_name;
                return true;
            } else return false;
        }
        catch(std::exception& a)
        {
             logger_singleton::get_instance()->get_logger()->error(a.what());
             return false;
        }
    }

    void execute(std::string const& request)  override
    {
        logger_singleton::get_instance()->get_logger()->trace("start execute add value");
        file_save file;

        try {
            value_in_memory_cache val(_id, _age, _date_missed, _status_found, _city, _info);
            key _key(_id);

            if (database::get_instance(3)->get_mode() == enums::mode::file_system) {
                std::string command = _date + " ADD_VALUE";

                std::string value_file_name =
                        "./FILESYSTEM/" + _pool_name + "/" + _scheme_name + "/" + _collection_name + "/" + std::to_string(_key._id) + ".txt";

                std::ofstream file_other(value_file_name);
                if (file_other.is_open()) {
                    file_other << val.to_string() << std::endl;
                } else {
                    logger_singleton::get_instance()->get_logger()->error(
                            "[add_value] error opening the file: " + value_file_name);
                }

                logger_singleton::get_instance()->get_logger()->debug(
                        "[add_value] the value has been added successfully");
                file_save::file_for_save(request);
            }
            
            logger_singleton::get_instance()->get_logger()->trace("execute command add value, memory cache mode");
            database::get_instance(3)->add_value(_pool_name, _scheme_name, _collection_name, _key, &val);

            command* new_command = new command_add_value();

            request_with_command handler(new_command);

        }
        catch(const std::exception& error)
        {
            logger_singleton::get_instance()->get_logger()->error("[add_value] error with add value");
        }
        logger_singleton::get_instance()->get_logger()->trace("finish execute add value");
    }
};

#endif //COURSE_PROJECT_COMMAND_ADD_VALUE_H
