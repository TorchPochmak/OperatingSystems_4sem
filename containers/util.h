#include <iostream>
#include <regex>
#ifndef COURSE_PROJECT_UTIL_H
#define COURSE_PROJECT_UTIL_H

#endif //COURSE_PROJECT_UTIL_H


struct util {
    static bool is_leap_year(unsigned int year)
    {
        if (year % 4 != 0)
        {
            return false;
        }
        else if (year % 100 != 0)
        {
            return true;
        }
        else if (year % 400 == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    static bool valid_date(const std::string& date)
    {
        std::regex pattern("\\d{2}\\.\\d{2}\\.\\d{4}");
        bool match =  std::regex_match(date, pattern);
        if(match)
        {
            unsigned int days = (date[0] - '0') * 10 + (date[1] - '0');
            unsigned int month = (date[3] - '0') * 10 + (date[4] - '0');
            unsigned int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + (date[9] - '0');
            bool is_leap = is_leap_year(year);
            if(month < 1 || month > 12)
                return false;
            if(is_leap && month == 2)
                if(days < 1 || days > 29)
                    return false;
            if(!is_leap && month == 2)
                if(days < 1 || days > 28)
                    return false;

            if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
                if(days < 1 || days > 31)
                    return false;
                else
                    return true;
            else
                if(days < 1 || days > 30)
                    return false;
                else
                    return true;
        }
        else
            return false;
    }
};