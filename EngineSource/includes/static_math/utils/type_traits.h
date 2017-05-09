/*
 * Copyright (C) 2013-2014 Morwenn
 *
 * static_math is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * static_math is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef SMATH_UTILS_TYPE_TRAITS_H_
#define SMATH_UTILS_TYPE_TRAITS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <type_traits>

namespace smath
{
    ////////////////////////////////////////////////////////////
    // Size traits

    template<typename T, typename U>
    using greater_of = std::conditional_t<
        sizeof(T) >= sizeof(U),
        T,
        U
    >;

    template<typename T, typename U>
    using lesser_of = std::conditional_t<
        sizeof(T) <= sizeof(U),
        T,
        U
    >;

    ////////////////////////////////////////////////////////////
    // Value type of

    template< typename T >
    struct value_type_of
    {
        using type = std::conditional_t<
                         std::is_fundamental< T >::value,
                         T, typename T::value_type >;
    };

    template< typename T >
    using value_type_of_t = typename value_type_of< T >::type;

    ////////////////////////////////////////////////////////////
    // Float matching

    template< typename T >
    struct _similar_float
    {
        using type = float;
    };

    template< typename T >
    struct similar_float
        : _similar_float< std::decay_t< T > >
    {
    };

    template< typename T >
    using similar_float_t = typename similar_float< T >::type;

    template< template< typename... > class T, typename U, typename... Rest >
    struct _similar_float< T< U, Rest... > >
    {
        using type = T< similar_float_t< U >, Rest... >;
    };

    template<>
    struct _similar_float< void >
    {
        using type = void;
    };

    // Matches double

    template<>
    struct _similar_float< double >
    {
        using type = double;
    };

    template<>
    struct _similar_float< long >
    {
        using type = double;
    };

    template<>
    struct _similar_float< unsigned long >
    {
        using type = double;
    };

    // Matches long double

    template<>
    struct _similar_float< long double >
    {
        using type = long double;
    };

    template<>
    struct _similar_float< long long >
    {
        using type = long double;
    };

    template<>
    struct _similar_float< unsigned long long >
    {
        using type = long double;
    };

    /*template< typename T >
    struct template_type1
    {
        using value_type = T;
        T value;
    };

    template< typename T, typename U >
    struct template_type2a
    {
        using value_type = T;
        T value;
    };

    template< typename T, typename U >
    struct template_type2b
    {
        using value_type = U;
        U value;
    };

    template< typename T, typename U, typename V >
    struct template_type3a
    {
        using value_type = T;
        T value;
    };

    template< typename T, typename U, typename V >
    struct template_type3b
    {
        using value_type = U;
        U value;
    };

    template< typename T, typename U, typename V >
    struct template_type3c
    {
        using value_type = V;
        V value;
    };

    similar_float_t< char > x = { 1.0 };
    float y = { 1.0 };
    static_assert( std::is_same< decltype( x ), decltype( y ) >::value, "" );

    similar_float_t< template_type1< int > > x1 = { 1.0 };
    template_type1< float > y1 = { 1.0 };
    static_assert( std::is_same< decltype( x1 ), decltype( y1 ) >::value, "" );

    similar_float_t< template_type2b< int, long long > > x2b = { 1.0 };
    template_type2b< int, long double > y2b = { 1.0 };
    static_assert( std::is_same< decltype( x2b ), decltype( y2b ) >::value, "" );

    similar_float_t< template_type3b< int, short, long > > x3b = { 1.0 };
    template_type3b< int, float, long > y3b = { 1.0 };
    static_assert( std::is_same< decltype( x3b ), decltype( y3b ) >::value, "" );

    similar_float_t< template_type3c< int, long, long > > x3c = { 1.0 };
    template_type3c< int, long, double > y3c = { 1.0 };
    static_assert( std::is_same< decltype( x3c ), decltype( y3c ) >::value, "" );*/

}

#endif // SMATH_UTILS_TYPE_TRAITS_H_
