#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_H

#include <cstddef>
#include <utility>
#include <sstream>
#include <iomanip>

#include <string>

class allocator
{

public:
    
    using block_size_t = size_t;
    
    using block_pointer_t = void *;

public:
    
    virtual ~allocator() noexcept = default;

public:

    template<
        typename T,
        typename ...Args>
    inline static void construct(
        T *at,
        Args &&... constructor_arguments);
    
    template<
        typename T>
    inline static void destruct(
        T *at);

public:
    
    [[nodiscard]] virtual void *allocate(
        size_t value_size,
        size_t values_count) = 0;
    
    virtual void deallocate(
        void *at) = 0;
    
protected:

    static std::string get_block_dump(
        void* block,
        size_t size
    ) 
    {
        std::ostringstream str_stream(size > 0 ? " with data:" : "", std::ios::ate);
        str_stream << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < size; ++i)
        {
                str_stream << " 0x" << std::setw(2) << static_cast<unsigned int>(
                        *reinterpret_cast<unsigned char*>(block) + i);
        }
        
        return str_stream.str();
    }

};

template<
    typename T,
    typename ...Args>
inline void allocator::construct(
    T *at,
    Args &&... constructor_arguments)
{
    new (at) T(std::forward<Args>(constructor_arguments)...);
}

template<
    typename T>
inline void allocator::destruct(
    T *at)
{
    at->~T();
}

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_H