/**
 * @file    NeuronMesh.cpp
 * @brief
 * @author  Juan José García <juanjose.garcia@urjc.es>
 * @date
 * @remarks Copyright (c) 2015 GMRV/URJC. All rights reserved.
 * Do not distribute without further notice.
 */

#include "NeuronMesh.h"

using namespace std;

namespace nlrender
{

  NeuronMesh::NeuronMesh( const nsol::NeuronMorphologyPtr& morpho_ )
  {
    _morpho = nsol::Simplifier::Instance( )
      ->simplify( morpho_, nsol::Simplifier::DIST_NODES_RADIUS );
    _morpho = nsol::Simplifier::Instance( )
      ->simplify( _morpho, nsol::Simplifier::ANGLE_MIN,
                  (( float ) M_PI) * 0.5f );
    _init( );
  }

  NeuronMesh::~NeuronMesh( void )
  {
    if ( _vbos.size( ) > 0 )
    {
      glDeleteBuffers( (GLsizei)_vbos.size( ), _vbos.data( ));
    }
    _vbos.clear( );

    if ( _vao > 0 )
    {
      glDeleteVertexArrays( 1, &_vao );
    }
  }

  void NeuronMesh::regenerate( const float& alphaRadius_,
                               const std::vector< float >& alphaNeurites_ )
  {
    std::vector< float > vertices;
    std::vector< float > centers;
    std::vector< float > tangents;
    std::vector< unsigned int > mesh;

    nlgenerator::NeuronMeshGenerator::GenerateMesh( _morpho, alphaRadius_,
                                                    alphaNeurites_,
                                                    vertices, centers,
                                                    tangents, mesh,
                                                    _somaEnd );
    // VAO Generation
    glBindVertexArray( _vao );

    _size = ( unsigned int ) mesh.size( );

    glBindBuffer( GL_ARRAY_BUFFER, _vbos[0]);
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * vertices.size( ),
                  vertices.data( ), GL_STATIC_DRAW );


    glBindBuffer( GL_ARRAY_BUFFER, _vbos[ 1 ]);
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * centers.size( ),
                  centers.data( ), GL_STATIC_DRAW );


    glBindBuffer( GL_ARRAY_BUFFER, _vbos[ 2 ]);
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * tangents.size( ),
                  tangents.data( ), GL_STATIC_DRAW );


    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _vbos[ 3 ]);
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( int ) * _size,
                  mesh.data( ), GL_STATIC_DRAW );


    glBindVertexArray( 0 );

    vertices.clear( );
    centers.clear( );
    tangents.clear( );
    mesh.clear( );
  }

  void NeuronMesh::paintSoma( void ) const
  {
    glBindVertexArray(_vao);
    glPatchParameteri( GL_PATCH_VERTICES, 3 );
    glDrawElements( GL_PATCHES, _somaEnd, GL_UNSIGNED_INT, 0 );
  }

  void NeuronMesh::paintNeurites( void ) const
  {
    glBindVertexArray(_vao);
    glPatchParameteri( GL_PATCH_VERTICES, 4 );
    glDrawElements( GL_PATCHES,  _size -_somaEnd, GL_UNSIGNED_INT,
                 (void *) (_somaEnd * sizeof(unsigned int)));
  }


  void NeuronMesh::writeOBJ( const std::string& fileName_,
                             nlgeometry::Vertices& vertices_,
                             const nlgeometry::Facets& facets_ ) const
  {
    ofstream outStream(fileName_.c_str());
    if(!outStream.is_open())
    {
      std::cerr <<  fileName_ << ": Error creating the file" << std::endl;
      return;
    }

    for ( unsigned int i = 0; i < ( unsigned int ) vertices_.size( ); i++ )
    {
      vertices_[i]->id( ) = i + 1;
      outStream << "v " << vertices_[i]->position( ).x( ) << " "
                << vertices_[i]->position( ).y( ) << " "
                << vertices_[i]->position( ).z( ) << "\n";
    }
    for( nlgeometry::VertexPtr vertex: vertices_ )
    {
      outStream << "vn " << vertex->normal( ).x( ) << " "
                << vertex->normal( ).y( ) << " "
                << vertex->normal( ).z( ) << "\n";
    }
    for( nlgeometry::FacetPtr facet: facets_ )
    {
      outStream << "f " << facet->vertex0( )->id( ) << "//"
                <<  facet->vertex0( )->id( ) << " " << facet->vertex1( )->id( )
                << "//" <<  facet->vertex1( )->id( ) << " "
                << facet->vertex2( )->id( )<< "//" <<  facet->vertex2( )->id( )
                << "\n";
    }

    outStream.close();
    std::cout << "File " << fileName_ << " saved" << " with "
              << vertices_.size() << " vertices and " << facets_.size()
              << " facets." << std::endl;

  }

  void NeuronMesh::_init( void )
  {
    std::vector< float > vertices;
    std::vector< float > centers;
    std::vector< float > tangents;
    std::vector< unsigned int > mesh;

    nlgenerator::NeuronMeshGenerator::GenerateMesh( _morpho, vertices,
                                                    centers, tangents,
                                                    mesh, _somaEnd );
    // VAO Generation
    glGenVertexArrays( 1, &_vao );
    glBindVertexArray( _vao );

    _size = ( unsigned int ) mesh.size( );

    _vbos.resize( 4 );
    glGenBuffers(4,_vbos.data( ));

    glBindBuffer( GL_ARRAY_BUFFER, _vbos[0]);
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * vertices.size( ),
                  vertices.data( ), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, _vbos[ 1 ]);
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * centers.size( ),
                  centers.data( ), GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 1 );

    glBindBuffer( GL_ARRAY_BUFFER, _vbos[ 2 ]);
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * tangents.size( ),
                  tangents.data( ), GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 2 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _vbos[ 3 ]);
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( int ) * _size,
                  mesh.data( ), GL_STATIC_DRAW );


    glBindVertexArray( 0 );

    vertices.clear( );
    centers.clear( );
    tangents.clear( );
    mesh.clear( );
  }

} // end namespace nlrender

// EOF
