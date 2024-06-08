#ifndef COURSE_PROJECT_COLLECTION_H
#define COURSE_PROJECT_COLLECTION_H
#include "wb.h"
#include "../enums/enums.h"
#include "../tree/associative_container.h"
#include "../tree/b_tree.h"
#include "../comparator/comparer.h"
#include "allocator.h"
#include "allocator_guardant.h"
#include "allocator_with_fit_mode.h"
#include "allocator_global_heap.h"
#include "allocator_sorted_list.h"
#include "allocator_boundary_tags.h"
#include "allocator_buddies_system.h"
#include "../flyweight/flyweight_string.h"
#include "../flyweight/flyweight_fabric.h"

class collection
{

private:

    associative_container<key, value*> *_data;
    allocator* _allocator_for_data_base;
    allocator_with_fit_mode::fit_mode _fit_mode;
    size_t _t;
    enums::allocator_types _type;

public:

    explicit collection(allocator* allocator_for_data_base, size_t t, allocator_with_fit_mode::fit_mode fit_mode, enums::allocator_types type) :
            _t(t),
            _fit_mode(fit_mode),
            _type(type),
            _allocator_for_data_base(allocator_for_data_base),
            _data(new b_tree<key, value*>(t, key_comparer()))
    {

    }

public:

    void add_value(key& key_collection, value* value_collection) const
    {
        try
        {
            _data->insert(key_collection, value_collection);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error(e.what());
        }
    }

    void add_value(key id, value_in_memory_cache* val)
    {
        auto *value_memory = reinterpret_cast<value_in_memory_cache *>(
                _allocator_for_data_base->allocate(sizeof(value_in_memory_cache), 1));

        auto date_missing = *(string_flyweight_factory::get_instance().get_string_flyweight(val->_date_missed)->get_value());
        auto city = *(string_flyweight_factory::get_instance().get_string_flyweight(val->_city)->get_value());
        auto info = *(string_flyweight_factory::get_instance().get_string_flyweight(val->_info)->get_value());

        allocator::construct(value_memory, val->_id, val->_age, date_missing, val->_status_found, city, info);

        try
        {
            _data->insert(id, value_memory);
        }
        catch (const std::exception& e)
        {
            _allocator_for_data_base->deallocate(value_memory);
            throw std::logic_error(e.what());
        }
    }

    void update_value(key id, value_in_memory_cache* val) const
    {

        auto *value_memory = reinterpret_cast<value_in_memory_cache *>(
                _allocator_for_data_base->allocate(sizeof(value_in_memory_cache), 1));

        auto date_missing = *(string_flyweight_factory::get_instance().get_string_flyweight(val->_date_missed)->get_value());
        auto city = *(string_flyweight_factory::get_instance().get_string_flyweight(val->_city)->get_value());
        auto info = *(string_flyweight_factory::get_instance().get_string_flyweight(val->_info)->get_value());

        allocator::construct(value_memory, val->_id, val->_age, date_missing, val->_status_found, city, info);

        try
        {
            _data->update(id, dynamic_cast<value*>(value_memory));
        }
        catch (const std::exception& e)
        {
            _allocator_for_data_base->deallocate(value_memory);
            throw std::logic_error(e.what());
        }
    }

    void delete_value(key const &key_collection) const
    {
        try
        {
            _data->dispose(key_collection);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error(e.what());
        }
    }

    [[nodiscard]] value* find_value(key const &key_collection) const
    {
        try
        {
            return _data->obtain(key_collection);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error(e.what());
        }
    }

    [[nodiscard]] std::vector<value*> obtain_between(key min_bound, key max_bound,
                                bool is_inclusive_lower, bool is_inclusive_upper) const
    {
        try
        {
            b_tree<key, value *> *tree = reinterpret_cast<b_tree<key, value *> *>(_data);
            std::vector<associative_container<key, value *>::key_value_pair> res = tree->obtain_between(min_bound,
                                                                                                        max_bound,
                                                                                                        is_inclusive_lower,
                                                                                                        is_inclusive_upper);
            std::vector<value *> result;

            for (auto each: res)
            {
                result.push_back(each.value);
            }

            return result;
        }
        catch (const std::exception& e)
        {
            throw std::logic_error(e.what());
        }
    }


public:

    ~collection()
    {
        delete _data;
    }

    collection(collection const &other) :
            _data(new b_tree<key, value*>(*dynamic_cast<b_tree<key, value*>*>(other._data))),
            _allocator_for_data_base(other._allocator_for_data_base)
    {
        _t = other._t;
    }

    collection &operator=(collection const &other)
    {
        if (this != &other)
        {
            _t = other._t;

            delete this->_data;

            if (this->_allocator_for_data_base != other._allocator_for_data_base)
            {
                this->_allocator_for_data_base = other._allocator_for_data_base;
            }

            this->_data = new b_tree<key, value*>(*dynamic_cast<b_tree<key, value*>*>(other._data));;
        }

        return *this;
    }

    collection(collection &&other) noexcept
    {
        _t = other._t;
        this->_data = other._data;
        other._data = nullptr;

        this->_allocator_for_data_base = other._allocator_for_data_base;
        other._allocator_for_data_base = nullptr;
    }

    collection &operator=(collection &&other) noexcept
    {
        if (this != &other)
        {
            _t = other._t;
            delete this->_data;
            this->_data = other._data;
            other._data = nullptr;

            delete this->_allocator_for_data_base;
            this->_allocator_for_data_base = other._allocator_for_data_base;
            other._allocator_for_data_base = nullptr;
        }

        return *this;
    }
};

#endif //COURSE_PROJECT_COLLECTION_H