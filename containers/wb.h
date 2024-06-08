#ifndef COURSE_PROJECT_WB_H
#define COURSE_PROJECT_WB_H
#include <iostream>
#include <regex>
#include "util.h"
#include "error.h"
#include "../validate/errors.h"

//info about missing person
//id - int
//age - [0, 150]
//date-missing - date(string) DD.MM.YYYY
//status_found- bool (found/not found)
//city - string
//_info - string

class value
{   };

class value_in_memory_cache final : public value
{

public:

    int _id;
    int _age;
    std::string _date_missed;
    bool _status_found;
    std::string _city;
    std::string _info;

    static bool valid_params(int id,
                      int age,
                      const std::string& date_missed,
                      bool status_found,
                      const std::string& city,
                      const std::string& info)
    {
        if(age < 0 || age > 150)
            return false;
        if(!util::valid_date(date_missed))
            return false;
        if(city.size() > 50 || info.size() > 1000)
            return false;
        return true;
    }

    value_in_memory_cache(int id,
        int age,
        std::string& date_missed,
        bool status_found,
        std::string& city,
        std::string& info)
    {
        if(valid_params(id, age, date_missed, status_found, city, info))
        {
            _id = id;
            _age = age;
            _date_missed = date_missed;
            _status_found = status_found;
            _city = city;
            _info = info;
        }
        else
            throw value_error("invalid data to create a value instance");
    }

    //rule of five==========================

    ~value_in_memory_cache() { };

    value_in_memory_cache(const value_in_memory_cache& other)
    {
        if(valid_params(other._id, other._age, other._date_missed, other._status_found, other._city, other._info))
        {
            _id = other._id;
            _age = other._age;
            _date_missed = other._date_missed;
            _status_found = other._status_found;
            _city = other._city;
            _info = other._info;
        }
        else

            throw value_error("invalid data to create a value instance");
    }

    value_in_memory_cache& operator=(const value_in_memory_cache& other)
    {
        if (this != &other)
        {
            if(valid_params(other._id, other._age, other._date_missed, other._status_found, other._city, other._info))
            {
                _id = other._id;
                _age = other._age;
                _date_missed = other._date_missed;
                _status_found = other._status_found;
                _city = other._city;
                _info = other._info;
            }
            else
                throw value_error("invalid data to create a value instance");
        }

        return *this;
    }

    value_in_memory_cache(value_in_memory_cache&& other) noexcept
    {
        if (this != &other) {
            _id = other._id;
            _age = other._age;
            _date_missed = std::move(other._date_missed);
            _status_found = other._status_found;
            _city = std::move(other._city);
            _info = std::move(other._info);
        }
    }

    value_in_memory_cache& operator=(value_in_memory_cache&& other) noexcept
    {
        if (this != &other)
        {
            _id = other._id;
            _age = other._age;
            _date_missed = std::move(other._date_missed);
            _status_found = other._status_found;
            _city = std::move(other._city);
            _info = std::move(other._info);
        }
        return *this;
    }

    [[nodiscard]] std::string to_string() const
    {
        return std::to_string(_id) + " " +
        std::to_string(_age) + " " +
        _date_missed + " " +
        std::to_string(_status_found) + " " +
        _city + " " +
        _info + " ";
    }
    [[nodiscard]] std::string to_string_detailed() const
    {
        return "id: " + std::to_string(_id) + " " +
                "age: " + std::to_string(_age) + " " +
                "date_missed: " + _date_missed + " " +
                "status_found: " + std::to_string(_status_found) + " " +
                "city: " + _city + " " +
                "info: " + _info + " ";
    }
};



class key
{

public:

    int _id;

    explicit key(int id) :
            _id(id) {   }
};

#endif //COURSE_PROJECT_WB_Hи