#ifndef COURSE_PROJECT_COMMAND_FIND_BETWEEN_VALUE_H
#define COURSE_PROJECT_COMMAND_FIND_BETWEEN_VALUE_H
#include "command.h"
#include "../chain__of_resposibility/request_with_command.h"
#include "../chain__of_resposibility/request_with_command_chain.h"

class command_find_between_value final :
        public command
{

private:

    std::string _pool_name;
    std::string _scheme_name;
    std::string _collection_name;
    std::string _date;

    key _id_min = key(0);
    key _id_max = key(0);
    bool _is_inclusive_lower = false;
    bool _is_inclusive_upper = false;

public:

    bool can_execute(std::string const& request)  override
    {
        logger_singleton::get_instance()->get_logger()->trace("start can_execute add value");

        std::istringstream string_with_commands(request);
        string_with_commands >> _date;
        std::string command;
        string_with_commands >> command;

        if (command == "FIND_BETWEEN_VALUE")
        {
            std::string pool_name;
            std::string scheme_name;
            std::string collection_name;
            int id_min, id_max;
            bool is_inclusive_lower, is_inclusive_upper;

            string_with_commands >> pool_name >> scheme_name >> collection_name >> id_min >> id_max >> is_inclusive_lower >> is_inclusive_upper;

            _pool_name = pool_name;
            _scheme_name = scheme_name;
            _collection_name = collection_name;
            _id_max = key(id_max);
            _id_min = key(id_min);
            _is_inclusive_lower = is_inclusive_lower;
            _is_inclusive_upper = is_inclusive_upper;

            return true;
        }
        else return false;
    }

    void execute(std::string const& request)  override
    {
        logger_singleton::get_instance()->get_logger()->trace("start execute find between value");
        std::vector<value*> vector = database::get_instance(3)->obtain_between_value(_pool_name, _scheme_name, _collection_name, _id_min, _id_max, _is_inclusive_lower, _is_inclusive_upper);

        if (database::get_instance(3)->get_mode() == enums::mode::file_system)
        {
            std::vector<value*> vector = database::get_instance(3)->obtain_between_value(_pool_name, _scheme_name, _collection_name, _id_min, _id_max, _is_inclusive_lower, _is_inclusive_upper);
            logger_singleton::get_instance()->get_logger()->trace("Vector: < start >");

            for(value* each : vector)
            {
                for (unsigned int i = _id_min._id; i < _id_max._id; ++i)
                {

                    std::string file_path = "./FILESYSTEM/" + _pool_name + "/" + _scheme_name + "/" + _collection_name + "/" + std::to_string(i) + ".txt";
                    std::ifstream input_file(file_path, std::ios::in);

                    if (!input_file.is_open())
                    {
                        continue;
                    }
                    else
                        input_file.close();
                    value_in_memory_cache value_memory = file_save::deserialization(_pool_name, _scheme_name, _collection_name, std::to_string(i), file_path);

                    logger_singleton::get_instance()->get_logger()->information("[find_value] " + value_memory.to_string_detailed());
                }
                
            }

            logger_singleton::get_instance()->get_logger()->trace("Vector: < end >");
            file_save::file_for_save(request);
        }
        else
        {
            key data_key_min(_id_min);
            key data_key_max(_id_max);

            std::vector<value*> vector = database::get_instance(3)->obtain_between_value(_pool_name, _scheme_name, _collection_name, _id_min, _id_max, _is_inclusive_lower, _is_inclusive_upper);
            logger_singleton::get_instance()->get_logger()->trace("Vector: < start >");

            for(value* each : vector)
            {
                value_in_memory_cache* value_memory = reinterpret_cast<value_in_memory_cache*>(each);
                logger_singleton::get_instance()->get_logger()->information("[find_value] name: " + value_memory->to_string_detailed());
            }
            logger_singleton::get_instance()->get_logger()->trace("Vector: < end >");
        }

        command* new_command = new command_find_between_value();
        request_with_command handler(new_command);


        logger_singleton::get_instance()->get_logger()->trace("finish execute add value");
    }
};

#endif //COURSE_PROJECT_COMMAND_FIND_BETWEEN_VALUE_H
