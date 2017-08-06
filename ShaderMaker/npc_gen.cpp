
#include "String.h"

#include <string>
#include <fstream>

#include "libxl.h"
#include <experimental/generator>

using namespace libxl;

using std::ofstream;
using std::experimental::generator;

#define FILEPATH "C:/Users/Andrew Meckling/Documents/Visual Studio 2015/Projects/Practicum/"

struct Npc
{
    std::string name;
    std::string textureUnit;
    int x, y;
};

generator< Npc > enumerate_npcs( Book* book )
{
    if ( Sheet* sheet = book->getSheet( 0 ) )
    {
        for ( int r = 1; r < sheet->lastRow(); ++r )
        {
            Npc npc;
            npc.name = sheet->readStr( r, 0 );
            npc.textureUnit = sheet->readStr( r, 1 );
            npc.x = sheet->readNum( r, 2 );
            npc.y = sheet->readNum( r, 3 );
            co_yield npc;
        }
    }
}

int main( int argc, char** argv )
{
    ofstream outfile { "GameSource/Npc.h" };
    outfile << "#pragma once\n\n#include \"Player.h\"\n";

    Book* book = xlCreateBook();
    if ( book->load( FILEPATH "Npc.xls" ) )
    {
        for ( Npc npc : enumerate_npcs( book ) )
        {
            outfile << String( "\n"
                "const Texture %_TEX {\n"
                "    %, Rect { %*16, %*16, 16, 16 },\n"
                "    TEXTURE_SIZE[ % ]\n"
                "};\n" ).format( npc.name, npc.textureUnit,
                    npc.x, npc.y, npc.textureUnit );
        }
    }
    book->release();
}
