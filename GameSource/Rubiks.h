#pragma once

#include <array>

enum Facelet : char
{
    U1, U2, U3, U4, U5, U6, U7, U8, U9,
    R1, R2, R3, R4, R5, R6, R7, R8, R9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9,
    D1, D2, D3, D4, D5, D6, D7, D8, D9,
    L1, L2, L3, L4, L5, L6, L7, L8, L9,
    B1, B2, B3, B4, B5, B6, B7, B8, B9,
};

enum Corner : char
{
    URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB,
};

struct CornerCubie
{
    Corner c : 4;
    char   o : 2;

    constexpr CornerCubie( Corner c = URF, int o = 0 )
        : c( c )
        , o( o % 3 )
    {
    }
};

enum Edge : char
{
    UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR,
};

struct EdgeCubie
{
    Edge e : 5;
    char o : 1;
    char oA : 1;

    constexpr EdgeCubie( Edge e = UR, int o = 0, int oA = 0 )
        : e( e )
        , o( o % 2 )
        , oA( oA % 2 )
    {
    }
};

namespace moves
{
    enum CubeMove : signed char
    {
        U, R, F, D, L, B,
        U2, R2, F2, D2, L2, B2,
        Ui, Ri, Fi, Di, Li, Bi,
        u, r, f, d, l, b,
        u2, r2, f2, d2, l2, b2,
        ui, ri, fi, di, li, bi,
    };
}

constexpr std::array< Facelet, 54 > face_permutation[] {
    { U3,U6,U9,U2,U5,U8,U1,U4,U7,F1,F2,F3,R4,R5,R6,R7,R8,R9,L1,L2,L3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,D7,D8,D9,B1,B2,B3,L4,L5,L6,L7,L8,L9,R1,R2,R3,B4,B5,B6,B7,B8,B9 },
    { U1,U2,B7,U4,U5,B4,U7,U8,B1,R3,R6,R9,R2,R5,R8,R1,R4,R7,F1,F2,U3,F4,F5,U6,F7,F8,U9,D1,D2,F3,D4,D5,F6,D7,D8,F9,L1,L2,L3,L4,L5,L6,L7,L8,L9,D9,B2,B3,D6,B5,B6,D3,B8,B9 },
    { U1,U2,U3,U4,U5,U6,R1,R4,R7,D3,R2,R3,D2,R5,R6,D1,R8,R9,F3,F6,F9,F2,F5,F8,F1,F4,F7,L3,L6,L9,D4,D5,D6,D7,D8,D9,L1,L2,U9,L4,L5,U8,L7,L8,U7,B1,B2,B3,B4,B5,B6,B7,B8,B9 },
    { U1,U2,U3,U4,U5,U6,U7,U8,U9,R1,R2,R3,R4,R5,R6,B7,B8,B9,F1,F2,F3,F4,F5,F6,R7,R8,R9,D3,D6,D9,D2,D5,D8,D1,D4,D7,L1,L2,L3,L4,L5,L6,F7,F8,F9,B1,B2,B3,B4,B5,B6,L7,L8,L9 },
    { F1,U2,U3,F4,U5,U6,F7,U8,U9,R1,R2,R3,R4,R5,R6,R7,R8,R9,D1,F2,F3,D4,F5,F6,D7,F8,F9,B9,D2,D3,B6,D5,D6,B3,D8,D9,L3,L6,L9,L2,L5,L8,L1,L4,L7,B1,B2,U7,B4,B5,U4,B7,B8,U1 },
    { L7,L4,L1,U4,U5,U6,U7,U8,U9,R1,R2,U1,R4,R5,U2,R7,R8,U3,F1,F2,F3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,R9,R6,R3,D7,L2,L3,D8,L5,L6,D9,L8,L9,B3,B6,B9,B2,B5,B8,B1,B4,B7 },
};

constexpr std::array< CornerCubie, 8 > corner_permutation[] {
    { CornerCubie{ UBR, 0 }, { URF, 0 }, { UFL, 0 }, { ULB, 0 }, { DFR, 0 }, { DLF, 0 }, { DBL, 0 }, { DRB, 0 } },
    { CornerCubie{ DFR, 2 }, { UFL, 0 }, { ULB, 0 }, { URF, 1 }, { DRB, 1 }, { DLF, 0 }, { DBL, 0 }, { UBR, 2 } },
    { CornerCubie{ UFL, 1 }, { DLF, 2 }, { ULB, 0 }, { UBR, 0 }, { URF, 2 }, { DFR, 1 }, { DBL, 0 }, { DRB, 0 } },
    { CornerCubie{ URF, 0 }, { UFL, 0 }, { ULB, 0 }, { UBR, 0 }, { DLF, 0 }, { DBL, 0 }, { DRB, 0 }, { DFR, 0 } },
    { CornerCubie{ URF, 0 }, { ULB, 1 }, { DBL, 2 }, { UBR, 0 }, { DFR, 0 }, { UFL, 2 }, { DLF, 1 }, { DRB, 0 } },
    { CornerCubie{ URF, 0 }, { UFL, 0 }, { UBR, 1 }, { DRB, 2 }, { DFR, 0 }, { DLF, 0 }, { ULB, 2 }, { DBL, 1 } },
};

constexpr std::array< EdgeCubie, 12 > edge_permutation[] {
    { EdgeCubie{ UB, 0, 1 }, { UR, 0, 1 }, { UF, 0, 1 }, { UL, 0, 1 }, { DR, 0, 0 }, { DF, 0, 0 }, { DL, 0, 0 }, { DB, 0, 0 }, { FR, 0, 0 }, { FL, 0, 0 }, { BL, 0, 0 }, { BR, 0, 0 } },
    { EdgeCubie{ FR, 0, 1 }, { UF, 0, 0 }, { UL, 0, 0 }, { UB, 0, 0 }, { BR, 0, 1 }, { DF, 0, 0 }, { DL, 0, 0 }, { DB, 0, 0 }, { DR, 0, 1 }, { FL, 0, 0 }, { BL, 0, 0 }, { UR, 0, 1 } },
    { EdgeCubie{ UR, 0, 0 }, { FL, 1, 1 }, { UL, 0, 0 }, { UB, 0, 0 }, { DR, 0, 0 }, { FR, 1, 1 }, { DL, 0, 0 }, { DB, 0, 0 }, { UF, 1, 1 }, { DF, 1, 1 }, { BL, 0, 0 }, { BR, 0, 0 } },
    { EdgeCubie{ UR, 0, 0 }, { UF, 0, 0 }, { UL, 0, 0 }, { UB, 0, 0 }, { DF, 0, 1 }, { DL, 0, 1 }, { DB, 0, 1 }, { DR, 0, 1 }, { FR, 0, 0 }, { FL, 0, 0 }, { BL, 0, 0 }, { BR, 0, 0 } },
    { EdgeCubie{ UR, 0, 0 }, { UF, 0, 0 }, { BL, 0, 1 }, { UB, 0, 0 }, { DR, 0, 0 }, { DF, 0, 0 }, { FL, 0, 1 }, { DB, 0, 0 }, { FR, 0, 0 }, { UL, 0, 1 }, { DL, 0, 1 }, { BR, 0, 0 } },
    { EdgeCubie{ UR, 0, 0 }, { UF, 0, 0 }, { UL, 0, 0 }, { BR, 1, 1 }, { DR, 0, 0 }, { DF, 0, 0 }, { DL, 0, 0 }, { BL, 1, 1 }, { FR, 0, 0 }, { FL, 0, 0 }, { UB, 1, 1 }, { DB, 1, 1 } },
};

struct Permutation;

Permutation get_permutation( moves::CubeMove mov );

struct Permutation
{
    union {
        std::array< Facelet, 54 > facelets;
        struct {
            std::array< Facelet, 9 > U, R, F, D, L, B;
        };
    };

    std::array< CornerCubie, 8 > cornerCubies = {
        URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB,
    };

    std::array< EdgeCubie, 12 > edgeCubies = {
        UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR,
    };

    constexpr Permutation()
        : U { U1, U2, U3, U4, U5, U6, U7, U8, U9 }
        , R { R1, R2, R3, R4, R5, R6, R7, R8, R9 }
        , F { F1, F2, F3, F4, F5, F6, F7, F8, F9 }
        , D { D1, D2, D3, D4, D5, D6, D7, D8, D9 }
        , L { L1, L2, L3, L4, L5, L6, L7, L8, L9 }
        , B { B1, B2, B3, B4, B5, B6, B7, B8, B9 }
    {
    }

    constexpr Permutation(
        const std::array< Facelet, 54 >&    faces,
        const std::array< CornerCubie, 8 >& corners,
        const std::array< EdgeCubie, 12 >&  edges )
        : facelets( faces )
        , cornerCubies( corners )
        , edgeCubies( edges )
    {
    }

    Permutation( const moves::CubeMove& mov )
        : Permutation( get_permutation( mov ) )
    {
    }

    Permutation& operator *=( const Permutation& perm );
};

Permutation operator *( const Permutation& lhs, const Permutation& rhs )
{
    auto x = lhs.facelets.begin();
    auto y = rhs.cornerCubies.begin();
    auto z = rhs.edgeCubies.begin();
    auto& a = rhs.facelets;
    auto& b = lhs.cornerCubies;
    auto& c = lhs.edgeCubies;

    auto corner = [&]( CornerCubie cc ) -> CornerCubie {
        return { b[ cc.c ].c, b[ cc.c ].o + cc.o };
    };

    auto edge = [&]( EdgeCubie ec ) -> EdgeCubie {
        return { c[ ec.e ].e, c[ ec.e ].o + ec.o, c[ ec.e ].oA + ec.oA };
    };

    return {
        std::array< Facelet, 54 > {
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
            a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ], a[ *x++ ],
        },
        std::array< CornerCubie, 8 > {
            corner( *y++ ), corner( *y++ ), corner( *y++ ), corner( *y++ ),
            corner( *y++ ), corner( *y++ ), corner( *y++ ), corner( *y++ ),
        },
        std::array< EdgeCubie, 12 > {
            edge( *z++ ), edge( *z++ ), edge( *z++ ), edge( *z++ ),
            edge( *z++ ), edge( *z++ ), edge( *z++ ), edge( *z++ ),
            edge( *z++ ), edge( *z++ ), edge( *z++ ), edge( *z++ ),
        }
    };
}

Permutation& Permutation::operator *=( const Permutation& perm )
{
    return *this = *this * perm;
}

Permutation operator *( const moves::CubeMove& lhs, const moves::CubeMove& rhs )
{
    return Permutation( lhs ) * Permutation( rhs );
}

Permutation get_permutation( moves::CubeMove mov )
{
    int m = (int) mov % moves::U2;

    Permutation base {
        face_permutation[ m ],
        corner_permutation[ m ],
        edge_permutation[ m ]
    };

    auto permutation = base;

    for ( int i = mov; i > moves::B; i -= moves::U2 )
        permutation *= base;

    return permutation;
}