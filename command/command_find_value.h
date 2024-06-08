#ifndef COURSE_PROJECT_COMMAND_FIND_VALUE_H
#define COURSE_PROJECT_COMMAND_FIND_VALUE_H
#include "command.h"
#include "../chain__of_resposibility/request_with_command.h"
#include "../chain__of_resposibility/request_with_command_chain.h"

class command_find_value final :
        public command
{

private:

    std::string _pool_name;
    std::string _scheme_name;
    std::string _collection_name;
    std::string _date;
    key _key = key(0);

public:

    bool can_execute(std::string const& request)  override
    {
        logger_singleton::get_instance()->get_logger()->trace("start can_execute add value");

        std::istringstream string_with_commands(request);
        string_with_commands >> _date;
        std::string command;
        string_with_commands >> command;

        if (command == "FIND_VALUE")
        {
            std::string pool_name;
            std::string scheme_name;
            std::string collection_name;
            int id;

            string_with_commands >> pool_name >> scheme_name >> collection_name >> id;

            _pool_name = pool_name;
            _scheme_name = scheme_name;
            _collection_name = collection_name;
            _key = key(id);

            return true;
        }
        else return false;
    }

    void execute(std::string const& request)  override
    {
        logger_singleton::get_instance()->get_logger()->trace("start execute add value");
        file_save file;

        if (database::get_instance(3)->get_mode() == enums::mode::file_system)
        {
            auto new_command = database::get_instance(3)->obtain_value(_pool_name, _scheme_name, _collection_name, _key);
            auto* result = reinterpret_cast<command_find_value*>(new_command);

            std::string value_file_name =
                        "./FILESYSTEM/" + _pool_name + "/" + _scheme_name + "/" + _collection_name + "/" + std::to_string(_key._id) + ".txt";

            value_in_memory_cache value_memory = file.deserialization(_pool_name, _scheme_name, _collection_name, std::to_string(_key._id), value_file_name);

            logger_singleton::get_instance()->get_logger()->information("[find_value] name: " + value_memory.to_string_detailed());

            file_save::file_for_save(request);
        }
        else
        {
            logger_singleton::get_instance()->get_logger()->trace("execute command update value, memory cache mode");
            auto* value_memory = reinterpret_cast<value_in_memory_cache*>(database::get_instance(3)->obtain_value(_pool_name, _scheme_name, _collection_name, _key));

            logger_singleton::get_instance()->get_logger()->information("[find_value] name: " + value_memory->to_string_detailed());
        }

        command* new_command = new command_find_value();
        request_with_command handler(new_command);


        logger_singleton::get_instance()->get_logger()->trace("finish execute add value");
    }
};

#endif //COURSE_PROJECT_COMMAND_FIND_VALUE_H
