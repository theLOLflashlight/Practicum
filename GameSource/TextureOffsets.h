#pragma once

struct TextureOffset
{
    char x : 4;
    char y : 4;
};

constexpr TextureOffset FLOOR_OFFSETS[]
{
    //////////// N E S W
    { 5, 0 }, // 0 0 0 0
    { 6, 1 }, // 0 0 0 1
    { 3, 0 }, // 0 0 1 0
    { 2, 0 }, // 0 0 1 1
    { 4, 1 }, // 0 1 0 0
    { 5, 1 }, // 0 1 0 1
    { 0, 0 }, // 0 1 1 0
    { 1, 0 }, // 0 1 1 1
    { 3, 2 }, // 1 0 0 0
    { 2, 2 }, // 1 0 0 1
    { 3, 1 }, // 1 0 1 0
    { 2, 1 }, // 1 0 1 1
    { 0, 2 }, // 1 1 0 0
    { 1, 2 }, // 1 1 0 1
    { 0, 1 }, // 1 1 1 0
    { 1, 1 }  // 1 1 1 1
};

constexpr size_t FLOOR_OFFSETS_COUNT = sizeof( FLOOR_OFFSETS ) / sizeof( TextureOffset );

constexpr TextureOffset WALL_OFFSETS[]
{
    //////////// N E S W
    { 1, 1 }, // 0 0 0 0 *duplicate of (1 0 0 0)
    { 2, 2 }, // 0 0 0 1 *duplicate of (1 0 0 1)
    { 0, 1 }, // 0 0 1 0 *duplicate of (1 0 1 0)
    { 2, 0 }, // 0 0 1 1
    { 0, 2 }, // 0 1 0 0 *duplicate of (1 1 0 0)
    { 1, 0 }, // 0 1 0 1
    { 0, 0 }, // 0 1 1 0
    { 4, 0 }, // 0 1 1 1
    { 1, 1 }, // 1 0 0 0
    { 2, 2 }, // 1 0 0 1
    { 0, 1 }, // 1 0 1 0
    { 5, 1 }, // 1 0 1 1
    { 0, 2 }, // 1 1 0 0
    { 4, 2 }, // 1 1 0 1
    { 3, 1 }, // 1 1 1 0
    { 4, 1 }  // 1 1 1 1
};

constexpr size_t WALL_OFFSETS_COUNT = sizeof( WALL_OFFSETS ) / sizeof( TextureOffset );

constexpr TextureOffset PIT_OFFSETS[]
{
    //////////// NE NW N E S W
    { 4, 0 }, //  0 0  0 0 0 0  * same for all
    { 2, 0 }, //  0 0  0 0 0 1  * same for all
    { 4, 0 }, //  0 0  0 0 1 0  * same for all
    { 2, 0 }, //  0 0  0 0 1 1  * same for all
    { 0, 0 }, //  0 0  0 1 0 0  * same for all
    { 1, 0 }, //  0 0  0 1 0 1  * same for all
    { 0, 0 }, //  0 0  0 1 1 0  * same for all
    { 1, 0 }, //  0 0  0 1 1 1  * same for all
    { 4, 1 }, //  0 0  1 0 0 0  * same for all
    { 5, 1 }, //  0 0  1 0 0 1
    { 4, 1 }, //  0 0  1 0 1 0  * same for all
    { 5, 1 }, //  0 0  1 0 1 1
    { 7, 1 }, //  0 0  1 1 0 0
    { 6, 0 }, //  0 0  1 1 0 1
    { 7, 1 }, //  0 0  1 1 1 0
    { 6, 0 }, //  0 0  1 1 1 1

    //////////// NE NW N E S W
    { 4, 0 }, //  0 1  0 0 0 0  *
    { 2, 0 }, //  0 1  0 0 0 1  *
    { 4, 0 }, //  0 1  0 0 1 0  *
    { 2, 0 }, //  0 1  0 0 1 1  *
    { 0, 0 }, //  0 1  0 1 0 0  *
    { 1, 0 }, //  0 1  0 1 0 1  *
    { 0, 0 }, //  0 1  0 1 1 0  *
    { 1, 0 }, //  0 1  0 1 1 1  *
    { 4, 1 }, //  0 1  1 0 0 0  *
    { 2, 1 }, //  0 1  1 0 0 1
    { 4, 1 }, //  0 1  1 0 1 0  *
    { 2, 1 }, //  0 1  1 0 1 1
    { 7, 1 }, //  0 1  1 1 0 0
    { 7, 0 }, //  0 1  1 1 0 1
    { 7, 1 }, //  0 1  1 1 1 0
    { 7, 0 }, //  0 1  1 1 1 1

    //////////// NE NW N E S W
    { 4, 0 }, //  1 0  0 0 0 0  *
    { 2, 0 }, //  1 0  0 0 0 1  *
    { 4, 0 }, //  1 0  0 0 1 0  *
    { 2, 0 }, //  1 0  0 0 1 1  *
    { 0, 0 }, //  1 0  0 1 0 0  *
    { 1, 0 }, //  1 0  0 1 0 1  *
    { 0, 0 }, //  1 0  0 1 1 0  *
    { 1, 0 }, //  1 0  0 1 1 1  *
    { 4, 1 }, //  1 0  1 0 0 0  *
    { 5, 1 }, //  1 0  1 0 0 1
    { 4, 1 }, //  1 0  1 0 1 0  *
    { 5, 1 }, //  1 0  1 0 1 1
    { 0, 1 }, //  1 0  1 1 0 0
    { 5, 0 }, //  1 0  1 1 0 1
    { 0, 1 }, //  1 0  1 1 1 0
    { 5, 0 }, //  1 0  1 1 1 1

    //////////// NE NW N E S W
    { 4, 0 }, //  1 1  0 0 0 0  *
    { 2, 0 }, //  1 1  0 0 0 1  *
    { 4, 0 }, //  1 1  0 0 1 0  *
    { 2, 0 }, //  1 1  0 0 1 1  *
    { 0, 0 }, //  1 1  0 1 0 0  *
    { 1, 0 }, //  1 1  0 1 0 1  *
    { 0, 0 }, //  1 1  0 1 1 0  *
    { 1, 0 }, //  1 1  0 1 1 1  *
    { 4, 1 }, //  1 1  1 0 0 0  *
    { 2, 1 }, //  1 1  1 0 0 1
    { 4, 1 }, //  1 1  1 0 1 0  *
    { 2, 1 }, //  1 1  1 0 1 1
    { 0, 1 }, //  1 1  1 1 0 0
    { 1, 1 }, //  1 1  1 1 0 1
    { 0, 1 }, //  1 1  1 1 1 0
    { 1, 1 }, //  1 1  1 1 1 1
};

constexpr size_t PIT_OFFSETS_COUNT = sizeof( PIT_OFFSETS ) / sizeof( TextureOffset );