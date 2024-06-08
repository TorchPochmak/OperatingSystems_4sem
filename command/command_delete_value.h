#ifndef COURSE_PROJECT_COMMAND_DELETE_VALUE_H
#define COURSE_PROJECT_COMMAND_DELETE_VALUE_H
#include "command.h"
#include "../chain__of_resposibility/request_with_command.h"
#include "../chain__of_resposibility/request_with_command_chain.h"

class command_delete_value final :
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

        if (command == "DELETE_VALUE")
        {
            std::string pool_name;
            std::string scheme_name;
            std::string collection_name;
            int id;
            string_with_commands >> pool_name >> scheme_name >> collection_name >> id;
            _key = key(id);

            _pool_name = pool_name;
            _scheme_name = scheme_name;
            _collection_name = collection_name;

            return true;
        }
        else return false;
    }

    void execute(std::string const& request)  override
    {
        logger_singleton::get_instance()->get_logger()->trace("start execute add value");
        database::get_instance(3)->delete_value(_pool_name, _scheme_name, _collection_name, _key);

        if (database::get_instance(3)->get_mode() == enums::mode::file_system)
        {
            std::string value_file_name = "./FILESYSTEM/" + _pool_name + "/" + _scheme_name + "/" + _collection_name + "/" + std::to_string(_key._id) + ".txt";

            if (std::remove(value_file_name.c_str()) != 0)
            {
                logger_singleton::get_instance()->get_logger()->error("error deleting the file: " + value_file_name);
            }
            else
            {
                logger_singleton::get_instance()->get_logger()->trace("file has deleted successfully: " + value_file_name);
                std::cout << "file has deleted successfully: " << value_file_name << std::endl;
            }

            file_save file;
            file.file_for_save(request);
        }
        command* new_command = new command_delete_value();
        request_with_command handler(new_command);


        logger_singleton::get_instance()->get_logger()->trace("finish execute add value");
    }


};

#endif //COURSE_PROJECT_COMMAND_DELETE_VALUE_H
