// Andrew Meckling
#pragma once

#include <array>
#include <functional> // std::reference_wrapper

// Trivial std array wrapper.
template< typename T, size_t N >
struct ArrayBase
{
    std::array< T, N > _array;
};

// Specialized std array wrapper for std reference_wrapper types.
template< typename T, size_t N >
struct ArrayBase< std::reference_wrapper< T >, N >
{
    union { // Don't construct. (reference_wrapper has no default ctor.)
        std::array< std::reference_wrapper< T >, N > _array;
    };

    // Deleted by default because of union.
    ArrayBase() { }
};
