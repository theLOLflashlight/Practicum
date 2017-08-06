
#include "String.h"
#include <string>

inline std::ostream& operator <<( std::ostream& out, const String& str )
{
    for ( char c : str )
        out.put( c );
    return out;
}

inline std::istream& operator >>( std::istream& in, String& str )
{
    std::string buffer;
    if ( in >> buffer )
        str = { buffer.c_str(), buffer.length() };
    return in;
}
