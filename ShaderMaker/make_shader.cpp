#include <iostream>
#include <sstream>
#include <vector>

using std::vector;
using std::string;
using std::istream;
using std::ostream;
using std::istringstream;
using std::ostringstream;

using std::move;
using std::forward;

struct Uniform
{
    string name;
    string type; // @temporary
};

class ContextGenerator
{
private:

    string            _name;
    vector< Uniform > _uniforms;

public:

    explicit ContextGenerator( string name )
        : _name { move( name ) }
    {
    }

    void add( Uniform uniform )
    {
        _uniforms.emplace_back( move( uniform ) );
    }

    void print( ostream& out ) const
    {
        const string className = "Renderer_" + _name;

        out <<
            "#include \"glhelp.h\"\n"
            "#include <glm/glm.hpp>\n"
            "#include <glm/gtx/transform.hpp>\n"
            "\n"
            "class " << className << "\n"
            "{\n"
            "private:\n"
            "   const GLuint programId;\n";
        
        for ( const Uniform& uniform : _uniforms )
            out << "    const GLuint " << uniform.name << ";\n";

        out <<
            "public:\n"
            "   glm::vec2 camPos { 0, 0 };\n"
            "   glm::vec2 camArea { 1, 1 };\n"
            "\n" // Constructor
            "   " << className << "()\n"
            "       : programId { load_shaders( \"vert.glsl\", \"frag.glsl\" ) }\n";

        for ( const Uniform& uniform : _uniforms )
            out << "        , " << uniform.name
                << " { glGetUniformLocation( programId, \"" << uniform.name << "\" ) }\n";

        out <<
            "   {\n"
            "       beginFrame();\n"
            // @TODO: setup shader with defaults
            "       endFrame();\n"
            "   }\n"
            "\n" // Destructor
            "   ~" << className << "()\n"
            "   {\n"
            "       glDeleteProgram( programId );\n"
            "   }\n"
            ;

        out << "};\n";
        out << std::endl;
    }
};

int main( int argc, char** argv )
{

}
