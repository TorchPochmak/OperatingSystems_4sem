#include <cstring>
// #include <mutex>

#include "allocator_sorted_list.h"


allocator_sorted_list::~allocator_sorted_list()
{
    std::string function = "Destructor\n";
    debug_with_guard(get_typename() + "start" + function);
    logger * _logger = get_logger();
    deallocate_with_guard(_trusted_memory);
    if (_logger != nullptr) _logger->debug(get_typename() + "end" + function);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    std::string function = "Move constructor\n";
    logger * log = other.get_logger();
    if (log != nullptr) log->debug(get_typename() + "start" + function);

    if (_trusted_memory != nullptr) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if (log != nullptr) log->debug(get_typename() + "end" + function);
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    std::string function = "Move operator\n";
    logger * log = other.get_logger();
    if (log != nullptr) log->debug(get_typename() + "start" + function);

    if (this == &other)
    {
        debug_with_guard(get_typename() + "end" + function);
        return *this;
    }

    if (_trusted_memory != nullptr) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if (log != nullptr) log->debug(get_typename() + "end" + function);
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    std::string function = "Constructor\n";

    if (logger != nullptr) logger->debug(get_typename() + "start" + function);

    auto meta_size = sizeof(size_t) + sizeof(allocator *) + sizeof(class logger *) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex);
    auto block_meta_size = sizeof(size_t) + sizeof(void*);

    if (space_size < block_meta_size + sizeof(void*))
    {
        std::string space_error = " Not enough space for allocator\n";
        if (logger != nullptr) logger->error(get_typename() + space_error);
        throw std::logic_error(space_error);
    }
    auto result_size = space_size + meta_size;
    
    try
    {
        if (parent_allocator != nullptr) _trusted_memory = parent_allocator -> allocate(result_size, 1);
        else _trusted_memory = :: operator new(result_size);
    }
    catch(std::bad_alloc const &ex)
    {
        std::string error = " Bad alloc while allocating trusted memory\n";
        if (logger != nullptr) logger->error(get_typename() + error);
        throw ex;
    }

    unsigned char * memory = reinterpret_cast<unsigned char *>(_trusted_memory);

    *reinterpret_cast<allocator**>(memory) = parent_allocator;
    memory += sizeof(allocator*);

    *reinterpret_cast<class logger**>(memory) = logger;
    memory += sizeof(class logger*);

    *reinterpret_cast<size_t*>(memory) = space_size;
    memory += sizeof(size_t);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory) = allocate_fit_mode;
    memory += sizeof(allocator_with_fit_mode::fit_mode);

    *reinterpret_cast<void**>(memory) = memory + sizeof(void*) + sizeof(std::mutex);
    memory += sizeof(void*);

    allocator::construct(reinterpret_cast<std::mutex *>(memory));
    memory += sizeof(std::mutex);

    *reinterpret_cast<void**>(memory) = nullptr;
    memory += sizeof(void*);
    *reinterpret_cast<size_t*>(memory) = space_size;

    debug_with_guard(get_typename() + "end" + function);
}


[[nodiscard]] void *allocator_sorted_list::allocate(size_t value_size, size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    std::string function = "Allocate\n";
    debug_with_guard(get_typename() + "start" + function);

    auto requested_size = value_size * values_count;
    if (requested_size < sizeof(void*))
    {
        requested_size = sizeof(void*);
        warning_with_guard(get_typename() + " Requested size has been changed\n");
    }
    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    auto meta_size = sizeof(size_t) + sizeof(allocator*);
    auto result_size = meta_size + requested_size;

    size_t available_memory = 0;

    void * block = nullptr;
    void * prev_target = nullptr;
    void * next_target = nullptr;
    size_t prev_size = 0;

    void * current = get_first_available_block();
    void * previous = nullptr;

    while (current != nullptr)
    {
        size_t current_block_size = get_available_block_size(current);
        available_memory += current_block_size;
        if (current_block_size == 0) break;
        if (current_block_size >= result_size)
        {
            if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit && block == nullptr)
            {
                block = current;
                prev_target = previous;
                next_target = get_available_block_next_block_address(current);
                prev_size = current_block_size;
            }
            else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit)
            {
                if (current_block_size < prev_size || prev_size == 0)
                {
                    block = current;
                    prev_target = previous;
                    next_target = get_available_block_next_block_address(current);
                    prev_size = current_block_size;
                }
            }
            else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
            {
                if (current_block_size > prev_size)
                {
                    block = current;
                    prev_target = previous;
                    next_target = get_available_block_next_block_address(current);
                    prev_size = current_block_size;
                }
            }
        }
        previous = current;
        current = get_available_block_next_block_address(current);
    }
    if (block == nullptr)
    {
        error_with_guard(get_typename() + " Cannot allocate block\n");
        throw std::bad_alloc();
    }

    auto blocks_sizes_difference = get_available_block_size(block) - requested_size;
    if (blocks_sizes_difference > 0 && blocks_sizes_difference < meta_size)
    {
        warning_with_guard(get_typename() + " Requested space size was changed\n");
        requested_size += blocks_sizes_difference;
        result_size = requested_size + meta_size;
    }
    else if (blocks_sizes_difference > 0) // осталось пространство
    {
        void ** new_next = reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + result_size);
        *reinterpret_cast<size_t*>(new_next + 1) = blocks_sizes_difference - sizeof(void*) - sizeof(size_t);

        if (new_next + blocks_sizes_difference != next_target) *new_next = next_target; // если справа занятый
        else merge_blocks(new_next, 0, next_target); // если справа свободный

        if (prev_target != nullptr)
        {
            void **next_block_adress = reinterpret_cast<void **>(prev_target);
            *next_block_adress = new_next;
        }
        else set_first_available_block(new_next);
    }

    void ** prev_adress = reinterpret_cast<void**>(block);
    prev_adress = nullptr;
    
    size_t * prev_size_ptr = reinterpret_cast<size_t*>(prev_adress + 1);
    prev_size_ptr = nullptr;

    size_t * size = reinterpret_cast<size_t*>(block);
    *size = requested_size;

    *reinterpret_cast<allocator**>(size + 1) = this;

    void * result_block = reinterpret_cast<unsigned char *>(block) + meta_size;

    information_with_guard(get_typename() + " Available memory: " + std::to_string(available_memory));

    debug_with_guard(get_typename() + "end" + function);
    return result_block;
}

std::string allocator_sorted_list::get_block_info(void * block) const noexcept
{
    // состояние блока
    unsigned char * bytes = reinterpret_cast<unsigned char *>(block);
    size_t size = get_occupied_block_size(bytes - sizeof(size_t) - sizeof(allocator*));
    std::string bytes_array = "";
    for (block_size_t i = 0; i < size; ++i)
    {
        bytes_array += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        bytes_array += ' ';
    }
    return bytes_array;
}

void allocator_sorted_list::deallocate(void *at)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    std::string function = "Deallocate\n";

    debug_with_guard(get_typename() + "start" + function);

    std::string block_info_array = get_block_info(at);
    debug_with_guard(get_typename() + "\n" + block_info_array);

    size_t meta_size = sizeof(allocator*) + sizeof(size_t);

    size_t available_memory = 0;

    void * block = reinterpret_cast<unsigned char *>(at) - meta_size;
    size_t block_size = get_occupied_block_size(block);
    
    if (get_occupied_block_allocator(block) != this)
    {
        std::string error = " Block doesn't belong to this allocator";
        error_with_guard(get_typename() + error);
        throw std::logic_error(error);
    }

    void * current_available = get_first_available_block();
    void * previous_available = nullptr;
    void * next_available = nullptr;
      
    while (current_available != nullptr) // идем по списку свободных
    { 
        available_memory += get_available_block_size(current_available);
        void * current_occupied;
        if ((previous_available == nullptr && current_available != get_first_block()) || current_available == get_first_block()) current_occupied = get_first_block();
        else current_occupied = reinterpret_cast<unsigned char *>(previous_available) + get_available_block_size(previous_available) + sizeof(void*) + sizeof(size_t);

        while (current_occupied != current_available)
        {
            size_t occupied_size = get_occupied_block_size(current_occupied);

            if (current_occupied == block) break;
            current_occupied = reinterpret_cast<unsigned char *>(current_occupied) + sizeof(size_t) + sizeof(allocator*) + occupied_size;
        }
        if (current_occupied == block) break; // нашли блок
        previous_available = current_available;
        current_available = get_available_block_next_block_address(current_available);
    }
    if (block == get_first_block()) // если блок в начале
    {
        if (reinterpret_cast<unsigned char *>(block) + block_size + meta_size == current_available) // справа свободный
        {
            merge_blocks(block, 1, current_available);
        }
        else // справа занятый
        {
            size_t * old_size_ptr = reinterpret_cast<size_t*>(block);
            old_size_ptr = nullptr;
            void ** new_adress_ptr = reinterpret_cast<void**>(block);
            *new_adress_ptr = current_available;
            *reinterpret_cast<size_t*>(new_adress_ptr + 1) = block_size;
        }
        set_first_available_block(block);

        debug_with_guard(get_typename() + "end" + function);
        return;
    }
    if (current_available == reinterpret_cast<unsigned char *>(block) +
    sizeof(size_t) + sizeof(allocator*) + block_size && current_available != nullptr) // если справа свободный
    {
        // сначала мерж с правым
        merge_blocks(block, 1, current_available);

        //меняем указатель у левого
        if (previous_available != nullptr)
        {
            *reinterpret_cast<void**>(previous_available) = block;

            if (reinterpret_cast<unsigned char *>(previous_available) + sizeof(size_t) + 
            sizeof(void*) + get_available_block_size(previous_available) == block) // если слева свободный
            {
                // мержим с левым
                merge_blocks(previous_available, 0, block);
            }
        }
        else set_first_available_block(block);
    }
    else if (current_available == reinterpret_cast<unsigned char *>(block) + sizeof(size_t) + 
    sizeof(allocator*) + block_size && current_available == nullptr) // если справа конец
    {
        size_t * old_size = reinterpret_cast<size_t*>(block);
        old_size = nullptr;
        void ** new_adress = reinterpret_cast<void**>(block);
        *new_adress = nullptr;
        *reinterpret_cast<size_t*>(new_adress + 1) = block_size;

        //меняем указатель у левого
        *reinterpret_cast<void**>(previous_available) = block;

        if (reinterpret_cast<unsigned char *>(previous_available) + sizeof(size_t) + 
        sizeof(void*) + get_available_block_size(previous_available) == block) // если слева свободный
        {
            merge_blocks(previous_available, 0, block);
        }
    }
    else // если справа занятый
    {
        size_t * old_size = reinterpret_cast<size_t*>(block);
        old_size = nullptr;
        void ** new_adress = reinterpret_cast<void**>(block);

        *new_adress = current_available;
        *reinterpret_cast<size_t*>(new_adress + 1) = block_size;
        //меняем указатель у левого
        if (previous_available != nullptr)
        {
            *reinterpret_cast<void**>(previous_available) = block;
            if (reinterpret_cast<unsigned char *>(previous_available) + sizeof(size_t) + 
            sizeof(void*) + get_available_block_size(previous_available) == block) // если слева свободный
            {
                merge_blocks(previous_available, 0, block);
            }
        }
        else set_first_available_block(block);
    }
    
    information_with_guard(get_typename() + " Available memory: " + std::to_string(available_memory));

    debug_with_guard(get_typename() + "end" + function);
}


inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t)) = mode;
}

inline allocator_with_fit_mode::fit_mode allocator_sorted_list::get_fit_mode()
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t));
}

inline allocator *allocator_sorted_list::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

inline logger *allocator_sorted_list::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<allocator **>(_trusted_memory) + 1);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "[Allocator Sorted List]";
}

void *allocator_sorted_list::get_first_available_block() const noexcept
{
    return *reinterpret_cast<void **>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode));
}

allocator::block_size_t allocator_sorted_list::get_available_block_size(void *block_address) const noexcept
{
    return *reinterpret_cast<allocator::block_size_t *>(reinterpret_cast<void **>(block_address) + 1);
}

void *allocator_sorted_list::get_available_block_next_block_address(void *block_address) const noexcept
{
    return *reinterpret_cast<void **>(block_address);
}

allocator::block_size_t allocator_sorted_list::get_occupied_block_size(void *block_address) const noexcept
{
    return *reinterpret_cast<allocator::block_size_t *>(block_address);
}

void allocator_sorted_list::set_first_available_block(void * first_available_block) const noexcept
{
    void ** first_block = reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
    *first_block = first_available_block;
}

void * allocator_sorted_list::get_first_block() const noexcept
{
    return reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*) + sizeof(std::mutex);
}

void allocator_sorted_list::clear_available_block(void * block) const noexcept
{
    void ** adress = reinterpret_cast<void**>(block);
    size_t * size = reinterpret_cast<size_t*>(adress + 1);
    adress = nullptr;
    size = nullptr;
}

void allocator_sorted_list::merge_blocks(void * first, int type, void * second) noexcept
{
    void * next_adress = get_available_block_next_block_address(second);
    size_t second_size = get_available_block_size(second);
    size_t first_size;
    if (type == 0) // свободный
    {
        first_size = get_available_block_size(first);
    }
    else // занятый
    {
        first_size = get_occupied_block_size(first);
        size_t * old_size = reinterpret_cast<size_t*>(first);
        old_size = nullptr;
        allocator ** alc = reinterpret_cast<allocator**>(old_size + 1);
        alc = nullptr;
    }
    void ** new_adress_ptr = reinterpret_cast<void**>(first);
    *new_adress_ptr = next_adress;
    *reinterpret_cast<size_t*>(new_adress_ptr + 1) = first_size + second_size + sizeof(void*) + sizeof(size_t);
    clear_available_block(second);
}

allocator * allocator_sorted_list::get_occupied_block_allocator(void * block) const noexcept
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char *>(block) + sizeof(size_t));
}

std::mutex &allocator_sorted_list::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex *>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(void*) + sizeof(allocator_with_fit_mode::fit_mode));
}
