/*
 * Copyright (C) 2013-2015 Morwenn
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

 /*
 * This file is the work of theLOLflashlight's GitHub account.
 */

#ifndef SMATH_DECIMAL_H_
#define SMATH_DECIMAL_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cstdint>
#include <type_traits>

namespace smath
{
    ////////////////////////////////////////////////////////////
    // Trigonometric functions

    struct decimal
    {
        enum e_sign : bool { positive, negative };

        //constexpr decimal(double x)
        //    : sign ( x >= 0 ? positive : negative )
        //    , exponent (  )
        //{
        //}

        const e_sign   sign;
        const uint16_t exponent;
        const uint64_t significand;
    };

    /**
    * @brief Sine function
    * @param x Angle in radians
    */


    #include <static_math/decimal.inl>
}

#endif // SMATH_DECIMAL_H_
