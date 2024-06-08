#ifndef COURSE_PROJECT_REQUEST_WITH_COMMAND_H
#define COURSE_PROJECT_REQUEST_WITH_COMMAND_H
#include "../validate/errors.h"
#include "handler.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <ctime>
#include <sstream>
#include "../command/command.h"

class request_with_command
        : public handler
{

private:

    command* _target_action;

public:

    explicit request_with_command(command* target_action) :
            _target_action(target_action)
    {

    }

    ~request_with_command() override
    {
        delete _target_action;
    }

    request_with_command(request_with_command const& other) = delete;

    request_with_command(request_with_command&& other) = delete;

    request_with_command& operator=(request_with_command const& other) = delete;

    request_with_command& operator=(request_with_command const&& other) = delete;

public:

    bool handle(std::string const& request) final
    {
        if (!_target_action->can_execute(request))
        {
            if(handler::_next_handler == nullptr)
            {
                return false;
            }

            return handler::_next_handler->handle(request);
        }

        _target_action->execute(request);

        return true;
    }
};

#endif //COURSE_PROJECT_REQUEST_WITH_COMMAND_H
