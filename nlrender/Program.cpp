#include "Program.h"

#include <iostream>
#include <fstream>

namespace neurolots
{

  Program::Program( TProgram type_, const std::string& path_ )
    : _type( type_ )
    , id_( 0 )
    , inVertex_( -1 )
    , inCenter_( -1 )
    , inTangent_( -1 )
    , vshader_( 0 )
    , teshader_( 0 )
    , tcshader_( 0 )
    , fshader_( 0 )
    , uProy_( -1 )
    , uView_( -1 )
    , uModel_( -1 )
    , uColor_( -1 )
    , uCameraPos_( -1 )
    , uDesp_( -1 )
    , uLod_( -1 )
    , uTng_( -1 )
    , uMaxDist_( -1 )
    , maxDist_( 500.0f )
    , lod_( 1.0f )
    , tng_( 0.0f )
    , _path( path_ )
  {
  }

  Program::~Program( void )
  {
  }

  GLuint Program::_LoadShader( const std::string& fileName_, GLenum type_ )
  {
    std::ifstream file;
    file.open( fileName_.c_str( ), std::ios::in );
    if( !file )
      return 0;

    //Calculo de la longitud del fichero
    file.seekg( 0, std::ios::end );
    unsigned int fileLen = file.tellg( );
    file.seekg( std::ios::beg );
    //Lectura del fichero
    char *source=new char[ fileLen + 1 ];
    int i = 0;
    while( file.good( ))
    {
      source[ i ] = file.get( );
      if( !file.eof( )) i++;
      else fileLen = i;
    }
    source[ fileLen ] = '\0';
    file.close( );

    //Creacion y compilacion del Shader
    GLuint shader;
    shader = glCreateShader( type_ );
    glShaderSource( shader, 1, ( const GLchar ** ) & source,
                    ( const GLint* ) & fileLen );

    glCompileShader( shader );
    delete source;

    //Comprobacion compilacion correcta
    GLint compiled;
    glGetShaderiv( shader, GL_COMPILE_STATUS, & compiled );
    if( !compiled )
    {
      GLint logLen;
      glGetShaderiv( shader, GL_INFO_LOG_LENGTH, & logLen );

      char *logString = new char[ logLen ];
      glGetShaderInfoLog( shader, logLen, NULL, logString );
      std::cout << "Error: " << logString << std::endl;
      delete logString;

      glDeleteShader( shader );
      return 0;
    }
    return shader;
  }

  void Program::_ShaderInit( void )
  {

    id_ = glCreateProgram( );

    std::string vname;
    std::string tcname;
    std::string tename;
    std::string fname;

    switch ( _type )
    {
      case LINES:
        vname = _path;
        vname.append( "/vshader.glsl" );
        fname = _path;
        fname.append( "/fshader.glsl" );

        std::cout << vname << std::endl;
        std::cout << fname << std::endl;

        vshader_ = _LoadShader( vname, GL_VERTEX_SHADER );
        fshader_ = _LoadShader( fname, GL_FRAGMENT_SHADER );

        glAttachShader( id_, vshader_ );
        glAttachShader( id_, fshader_ );

        glBindAttribLocation( id_, 0, "inVertex" );
        break;

      case TRIANGLES:
        vname = _path;
        vname.append( "/vshader.glsl" );
        tcname = _path;
        tcname.append( "/tcshader.glsl" );
        tename = _path;
        tename.append( "/teshader.glsl" );
        fname = _path;
        fname.append( "/fshader.glsl" );

        std::cout << "Triangles shaders: " << std::endl;
        std::cout << "\t" << vname << std::endl;
        std::cout << "\t" << tcname << std::endl;
        std::cout << "\t" << tename << std::endl;
        std::cout << "\t" << fname << std::endl;

        vshader_ = _LoadShader( vname, GL_VERTEX_SHADER );
        tcshader_ = _LoadShader( tcname, GL_TESS_CONTROL_SHADER );
        teshader_ = _LoadShader( tename, GL_TESS_EVALUATION_SHADER );
        fshader_ = _LoadShader( fname, GL_FRAGMENT_SHADER );

        glAttachShader( id_, vshader_ );
        glAttachShader( id_, tcshader_ );
        glAttachShader( id_, teshader_ );
        glAttachShader( id_, fshader_ );

        glBindAttribLocation( id_, 0, "inVertex" );
        glBindAttribLocation( id_, 1, "inCenter" );
        break;

      case QUADS:
        vname = _path;
        vname.append( "/vshader.glsl" );
        tcname = _path;
        tcname.append( "/tcshader.glsl" );
        tename = _path;
        tename.append( "/teshader.glsl" );
        fname = _path;
        fname.append( "/fshader.glsl" );

        std::cout << "Quads shaders: " << std::endl;
        std::cout << "\t" << vname << std::endl;
        std::cout << "\t" << tcname << std::endl;
        std::cout << "\t" << tename << std::endl;
        std::cout << "\t" << fname << std::endl;

        //comopilacion de shaders
        vshader_ = _LoadShader( vname, GL_VERTEX_SHADER );
        tcshader_ = _LoadShader( tcname, GL_TESS_CONTROL_SHADER );
        teshader_ = _LoadShader( tename, GL_TESS_EVALUATION_SHADER );
        fshader_ = _LoadShader( fname, GL_FRAGMENT_SHADER );

        glAttachShader( id_, vshader_ );
        glAttachShader( id_, tcshader_ );
        glAttachShader( id_, teshader_ );
        glAttachShader( id_, fshader_ );

        glBindAttribLocation( id_, 0, "inVertex" );
        glBindAttribLocation( id_, 1, "inCenter" );
        glBindAttribLocation( id_, 2, "inTangent" );
        break;

    }

    glLinkProgram( id_ );
    //Comprobacion de lincado
    int linked;
    glGetProgramiv( id_, GL_LINK_STATUS, &linked );
    if( !linked )
    {
      GLint logLen;
      glGetProgramiv( id_, GL_INFO_LOG_LENGTH, &logLen );

      char * logString = new char[ logLen ];
      glGetProgramInfoLog( id_, logLen, NULL, logString );
      std::cout << "Error: " << logString << std::endl;
      delete logString;

      glDeleteProgram( id_ );
      id_ = 0;
      return;
    }

    switch ( _type )
    {
      case LINES:
        uProy_ = glGetUniformLocation( id_, "proy" );
        uView_ = glGetUniformLocation( id_, "view" );
        uModel_ = glGetUniformLocation( id_, "model" );
        uColor_ = glGetUniformLocation( id_, "color" );
        inVertex_ = glGetAttribLocation( id_, "inVertex" );
        break;

      case TRIANGLES:
        uProy_ = glGetUniformLocation( id_, "proy" );
        uView_ = glGetUniformLocation( id_, "view" );
        uModel_ = glGetUniformLocation( id_, "model" );
        uColor_ = glGetUniformLocation( id_, "color" );
        uCameraPos_ = glGetUniformLocation( id_, "cameraPos" );
        uLod_ = glGetUniformLocation( id_, "lod" );
        uTng_ = glGetUniformLocation( id_, "tng" );
        uMaxDist_ = glGetUniformLocation( id_, "maxDist" );
        //Atributos de entrada a shaders
        inVertex_ = glGetAttribLocation( id_, "inVertex" );
        inCenter_ = glGetAttribLocation( id_, "inCenter" );
        break;

      case QUADS:
        //Variables uniform
        uProy_ = glGetUniformLocation( id_, "proy" );
        uView_ = glGetUniformLocation( id_, "view" );
        uModel_ = glGetUniformLocation( id_, "model" );
        uColor_ = glGetUniformLocation( id_, "color" );
        uCameraPos_ = glGetUniformLocation( id_, "cameraPos" );
        uLod_ = glGetUniformLocation( id_, "lod" );
        uTng_ = glGetUniformLocation( id_, "tng" );
        uMaxDist_ = glGetUniformLocation( id_, "maxDist" );
        //Atributos de entrada a shaders
        inVertex_ = glGetAttribLocation( id_, "inVertex" );
        inCenter_ = glGetAttribLocation( id_, "inCenter" );
        inTangent_ = glGetAttribLocation( id_, "inTangent" );
        break;
    }
  }

  void Program::Init( void )
  {
    _ShaderInit( );
  }

  void Program::lod( float _lod )
  {
    lod_ = _lod;
  }

  void Program::tng( float _tng )
  {
    tng_ = _tng;
  }

  void Program::maxDist( float _maxDist )
  {
    maxDist_ = _maxDist;
  }

  Program::TProgram Program::type( void )
  {
    return _type;
  }

  GLuint Program::id( void )
  {
    return id_;
  }

  GLint Program::inVertex( void )
  {
    return inVertex_;
  }

  GLint Program::inCenter( void )
  {
    return inCenter_;
  }

  GLint Program::inTangent( void )
  {
    return inTangent_;
  }

  GLuint Program::vshader( void )
  {
    return vshader_;
  }

  GLuint Program::teshader( void )
  {
    return teshader_;
  }

  GLuint Program::tcshader( void )
  {
    return tcshader_;
  }

  GLuint Program::fshader( void )
  {
    return fshader_;
  }

  GLint Program::uProy( void )
  {
    return uProy_;
  }

  GLint Program::uView( void )
  {
    return uView_;
  }

  GLint Program::uModel( void )
  {
    return uModel_;
  }

  GLint Program::uColor( void )
  {
    return uColor_;
  }

  GLint Program::uCameraPos( void )
  {
    return uCameraPos_;
  }

  GLint Program::uDesp( void )
  {
    return uDesp_;
  }

  GLint Program::uLod( void )
  {
    return uLod_;
  }

  GLint Program::uTng( void )
  {
    return uTng_;
  }

  GLint Program::uMaxDist( void )
  {
    return uMaxDist_;
  }

  float Program::lod( void )
  {
    return lod_;
  }

  float Program::tng( void )
  {
    return tng_;
  }

  float Program::maxDist( void )
  {
    return maxDist_;
  }

} // end namespace neurolots

//EOF