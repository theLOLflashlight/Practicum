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
#ifndef SMATH_CONSTANTS_H_
#define SMATH_CONSTANTS_H_

namespace smath
{
    ////////////////////////////////////////////////////////////
    // POSIX constants from cmath

    /** e */
    constexpr double E        = 2.71828182845904523540;

    /** log2(e) */
    constexpr double LOG2E    = 1.44269504088896340740;

    /** log10(e) */
    constexpr double LOG10E   = 0.43429448190325182765;

    /** ln(2) */
    constexpr double LN2      = 0.69314718055994530942;

    /** ln(10) */
    constexpr double LN10     = 2.30258509299404568402;

    /** pi */
    constexpr double PI       = 3.14159265358979323846;

    ////////////////////////////////////////////////////////////
    // Other mathematical constants

    /** Golden ratio */
    constexpr double PHI      = 1.61803398874989484820;

    /** tau (2pi) */
    constexpr double TAU      = 6.28318530717958647692;
}

#endif // SMATH_CONSTANTS_H_
