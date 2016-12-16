// Andrew Meckling
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

GLuint load_shaders( const char* vertexFilePath,
                     const char* fragmentFilePath );


template< typename T >
constexpr GLenum gl_type_constant( T = {} );

template<>
constexpr GLenum gl_type_constant< GLbyte >( GLbyte )
{
    return GL_BYTE;
}

template<>
constexpr GLenum gl_type_constant< GLubyte >( GLubyte )
{
    return GL_UNSIGNED_BYTE;
}

template<>
constexpr GLenum gl_type_constant< GLshort >( GLshort )
{
    return GL_SHORT;
}

template<>
constexpr GLenum gl_type_constant< GLushort >( GLushort )
{
    return GL_UNSIGNED_SHORT;
}

template<>
constexpr GLenum gl_type_constant< GLint >( GLint )
{
    return GL_INT;
}

template<>
constexpr GLenum gl_type_constant< GLuint >( GLuint )
{
    return GL_UNSIGNED_INT;
}

template<>
constexpr GLenum gl_type_constant< GLfloat >( GLfloat )
{
    return GL_FLOAT;
}

template<>
constexpr GLenum gl_type_constant< GLdouble >( GLdouble )
{
    return GL_DOUBLE;
}


inline void glUniform( GLint location, const glm::mat4& mat )
{
    glUniformMatrix4fv( location, 1, GL_FALSE, (float*) &mat );
}

inline void glUniform( GLint location, const glm::vec4& vec )
{
    glUniform4fv( location, 1, (float*) &vec );
}

inline void glUniform( GLint location, const glm::vec3& vec )
{
    glUniform3fv( location, 1, (float*) &vec );
}

inline void glUniform( GLint location, const glm::vec2& vec )
{
    glUniform2fv( location, 1, (float*) &vec );
}

inline void glUniform( GLint location, float num )
{
    glUniform1f( location, num );
}

inline void glUniform( GLint location, int num )
{
    glUniform1i( location, num );
}

// Code grabbed from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
inline GLuint load_shaders( const char* vertex_file_path, const char* fragment_file_path )
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader( GL_VERTEX_SHADER );
    GLuint FragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream( vertex_file_path, std::ios::in );
    if ( VertexShaderStream.is_open() )
    {
        std::string Line = "";
        while ( getline( VertexShaderStream, Line ) )
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else
    {
        printf( "Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path );
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream( fragment_file_path, std::ios::in );
    if ( FragmentShaderStream.is_open() )
    {
        std::string Line = "";
        while ( getline( FragmentShaderStream, Line ) )
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf( "Compiling shader : %s\n", vertex_file_path );
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource( VertexShaderID, 1, &VertexSourcePointer, NULL );
    glCompileShader( VertexShaderID );

    // Check Vertex Shader
    glGetShaderiv( VertexShaderID, GL_COMPILE_STATUS, &Result );
    glGetShaderiv( VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength );
    if ( InfoLogLength > 0 )
    {
        std::vector<char> VertexShaderErrorMessage( InfoLogLength+1 );
        glGetShaderInfoLog( VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[ 0 ] );
        printf( "%s\n", &VertexShaderErrorMessage[ 0 ] );
    }



    // Compile Fragment Shader
    printf( "Compiling shader : %s\n", fragment_file_path );
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource( FragmentShaderID, 1, &FragmentSourcePointer, NULL );
    glCompileShader( FragmentShaderID );

    // Check Fragment Shader
    glGetShaderiv( FragmentShaderID, GL_COMPILE_STATUS, &Result );
    glGetShaderiv( FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength );
    if ( InfoLogLength > 0 )
    {
        std::vector<char> FragmentShaderErrorMessage( InfoLogLength+1 );
        glGetShaderInfoLog( FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[ 0 ] );
        printf( "%s\n", &FragmentShaderErrorMessage[ 0 ] );
    }



    // Link the program
    printf( "Linking program\n" );
    GLuint ProgramID = glCreateProgram();
    glAttachShader( ProgramID, VertexShaderID );
    glAttachShader( ProgramID, FragmentShaderID );
    glLinkProgram( ProgramID );

    // Check the program
    glGetProgramiv( ProgramID, GL_LINK_STATUS, &Result );
    glGetProgramiv( ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength );
    if ( InfoLogLength > 0 )
    {
        std::vector<char> ProgramErrorMessage( InfoLogLength+1 );
        glGetProgramInfoLog( ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[ 0 ] );
        printf( "%s\n", &ProgramErrorMessage[ 0 ] );
    }


    glDetachShader( ProgramID, VertexShaderID );
    glDetachShader( ProgramID, FragmentShaderID );

    glDeleteShader( VertexShaderID );
    glDeleteShader( FragmentShaderID );

    return ProgramID;
}