#ifndef COURSE_PROJECT_DATABASE_H
#define COURSE_PROJECT_DATABASE_H
#include "pool.h"
#include "../enums/enums.h"
#include "../tree/b_tree.h"
#include "../comparator/comparer.h"
#include "../chain_of_responsibility/request_handler_with_command_chain.h"
#include "../chain_of_responsibility/request_handler_with_command.h"
#include "../chain_of_responsibility/handler.h"
#include "../validate/errors.h"

class database
{

private:

    static database* _instance;
    allocator* _allocator_database;

    size_t _t;
    enums::mode _mode;

public:

    b_tree<std::string, pool> *_database_entrypoint;

    static database* get_instance(size_t t)
    {
        if (_instance == nullptr)
        {
            _instance = new database(t);
        }

        return _instance;
    }
    static void clear()
    {
        size_t t_par = _instance->_t;
        if(_instance != nullptr)
            delete _instance;
        _instance = new database(t_par);
    }

public:

    database(size_t t) :
            _t(t),
            _database_entrypoint(new b_tree<std::string, pool>(t, key_comparer()))
    {
        _instance = this;
    }

public:

    [[nodiscard]] size_t get_t() const
    {
        return _t;
    }

    void set_t(size_t t)
    {
        _t = t;
    }

public:

    void add_pool(std::string const& pool_name) const
    {
        try
        {
            _database_entrypoint->insert(pool_name, pool(_t));
        }
        catch(const std::exception& e)
        {
            throw std::logic_error("can't add pool");
        }
    }

    void delete_pool(std::string const& pool_name) const
    {
        try
        {
            _database_entrypoint->dispose(pool_name);
        }
        catch(const std::exception& e)
        {
            throw std::logic_error("can't delete pool");
        }
    }

    [[nodiscard]] pool& find_pool(std::string const& pool_name) const
    {
        try
        {
            return const_cast<pool &>(_database_entrypoint->obtain(pool_name));
        }
        catch(const std::exception& e)
        {
            throw std::logic_error("can't find pool");
        }
    }

public:

    void add_scheme(std::string const& pool_name, std::string const& scheme_name) const
    {
        try
        {
            find_pool(pool_name).add_scheme(scheme_name);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    void delete_scheme(std::string const& pool_name, std::string const& scheme_name) const
    {
        try
        {
            find_pool(pool_name).remove_scheme(scheme_name);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    [[nodiscard]] scheme& find_scheme(std::string const& pool_name, std::string const& scheme_name) const
    {
        try
        {
            return const_cast<scheme &>(find_pool(pool_name).find_scheme(scheme_name));
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

public:

    void add_collection(std::string const& pool_name, std::string const& scheme_name,
                        std::string const& collection_name, enums::allocator_types
                         type, allocator_with_fit_mode::fit_mode fit_mode)
    {
        try
        {
            scheme &scheme_db = const_cast<scheme &>(find_pool(pool_name).find_scheme(scheme_name));

            switch (type)
            {
                case enums::allocator_types::BOUNDARY_TAGS:
                    _allocator_database = new allocator_boundary_tags(3000, nullptr, nullptr, fit_mode);
                    break;

                case enums::allocator_types::BUDDIE_SYSTEM:
                    _allocator_database = new allocator_buddies_system(24, nullptr, nullptr, fit_mode);
                    break;

                case enums::allocator_types::SORTED_LIST:
                    _allocator_database = new allocator_sorted_list(3000, nullptr, nullptr, fit_mode);
                    break;

                case enums::allocator_types::GLOBAL_HEAP:
                    _allocator_database = new allocator_global_heap();
                    break;
            }

            scheme_db.add_collection(collection_name, type, fit_mode, _allocator_database);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    void delete_collection(std::string const& pool_name, std::string const& scheme_name,
                           std::string const& collection_name) const
    {
        try
        {
            find_scheme(pool_name, scheme_name).remove_collection(collection_name);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    [[nodiscard]] collection& find_collection(std::string const& pool_name, std::string const& scheme_name,
                                std::string const& collection_name) const
    {
        try
        {
            return const_cast<collection&>((find_scheme(pool_name, scheme_name)).find_collection(collection_name));
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }


public:


    void add_value(
            std::string const& pool_name,
            std::string const& scheme_name,
            std::string const& collection_name,
            key id,
            value_in_memory_cache* val) const
    {
        try
        {
            find_pool(pool_name).find_scheme(scheme_name)
                    .find_collection(collection_name)
                    .add_value(id, val);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (const collection_error &e)
        {
            std::string error = "collection '" + collection_name + "' not found in scheme '" + scheme_name + "' in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    // path_file, start_value_bytes, size_value
    void update_value(
            std::string const& pool_name,
            std::string const& scheme_name,
            std::string const& collection_name,
            key id,
            value_in_memory_cache* val) const
    {
        try
        {
            find_pool(pool_name).find_scheme(scheme_name)
                    .find_collection(collection_name)
                    .update_value(id, val);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (const collection_error &e)
        {
            std::string error = "collection '" + collection_name + "' not found in scheme '" + scheme_name + "' in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }


    [[nodiscard]] value* obtain_value(
            std::string const& pool_name,
            std::string const& scheme_name,
            std::string const& collection_name,
            key key_value) const
    {
        try
        {
            return find_pool(pool_name).find_scheme(scheme_name)
                    .find_collection(collection_name)
                    .find_value(key_value);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (const collection_error &e)
        {
            std::string error = "collection '" + collection_name + "' not found in scheme '" + scheme_name + "' in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }


    void delete_value(
            std::string const& pool_name,
            std::string const& scheme_name,
            std::string const& collection_name,
            key id) const
    {
        try
        {
            find_pool(pool_name).find_scheme(scheme_name)
                    .find_collection(collection_name)
                    .delete_value(id);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (const collection_error &e)
        {
            std::string error = "collection '" + collection_name + "' not found in scheme '" + scheme_name + "' in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    std::vector<value*> obtain_between_value(
            std::string const& pool_name,
            std::string const& scheme_name,
            std::string const& collection_name,
            key &lower_bound,
            key &upper_bound,
            bool lower_bound_inclusive,
            bool upper_bound_inclusive) const
    {
        try
        {
            return find_pool(pool_name).find_scheme(scheme_name)
                                .find_collection(collection_name)
                                .obtain_between(lower_bound, upper_bound,
                                                lower_bound_inclusive,
                                                upper_bound_inclusive);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (const collection_error &e)
        {
            std::string error = "collection '" + collection_name + "' not found in scheme '" + scheme_name + "' in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

    std::vector<value*> obtain_between_value(
            std::string const& pool_name,
            std::string const& scheme_name,
            std::string const& collection_name,
            int lower_bound,
            int upper_bound,
            bool lower_bound_inclusive,
            bool upper_bound_inclusive) const
    {
        try
        {
            return find_pool(pool_name).find_scheme(scheme_name)
                    .find_collection(collection_name)
                    .obtain_between(key(lower_bound), key(upper_bound),
                                    lower_bound_inclusive,
                                    upper_bound_inclusive);
        }
        catch (const pool_error &e)
        {
            std::string error = "pool '" + pool_name + "' not found";
            throw std::logic_error(error);
        }
        catch (const scheme_error &e)
        {
            std::string error = "scheme '" + scheme_name + "' not found in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (const collection_error &e)
        {
            std::string error = "collection '" + collection_name + "' not found in scheme '" + scheme_name + "' in pool '" + pool_name + "'";
            throw std::logic_error(error);
        }
        catch (std::exception const &e)
        {
            throw std::logic_error(e.what());
        }
    }

public:

    ~database()
    {
        delete _database_entrypoint;
    }

    database(database const &other) noexcept = delete;

    database(database const &&other) noexcept = delete;

    database &operator=(database &other) = delete;

    database &operator=(database &&other) = delete;

public:

    enums::mode get_mode()
    {
        return this->_mode;
    }

    enums::mode set_mode(enums::mode mode)
    {
        _mode = mode;
        return _mode;
    }

public:

};

database* database::_instance = nullptr;


#endif //COURSE_PROJECT_DATABASE_H