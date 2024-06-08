#ifndef COURSE_PROJECT_ENUMS_H
#define COURSE_PROJECT_ENUMS_H

class enums
{
public:

    enum class allocator_types
    {
        GLOBAL_HEAP,
        SORTED_LIST,
        BUDDIE_SYSTEM,
        BOUNDARY_TAGS
    };

    enum class mode
    {
        in_memory_cache,
        file_system
    };

    enum class search_trees
    {
        b_tree
    };
};

#endif //COURSE_PROJECT_ENUMS_H
