#include "NeuronMeshGenerator.h"

using namespace nsol;
using namespace std;

namespace neurolots
{

//CONSTRUCTORS AND DESTRUCTORS

  NeuronMeshGenerator::NeuronMeshGenerator( void )
  {

  }

  NeuronMeshGenerator::~NeuronMeshGenerator( void )
  {

  }

// PUBLIC METHODS

  NeuronMorphologyPtr NeuronMeshGenerator::ReadMorphology( const char *
                                                           file_name )
  {
    SwcReader r;
    return r.readFile(file_name);
  }

  void NeuronMeshGenerator::GenerateStructure( NeuronMorphologyPtr morpho,
                                               vector< float > & vertices,
                                               vector< unsigned int > & mesh )
  {
    vector< unsigned int >ids;
    ids.resize( _MaxId( morpho ));
    vertices.clear( );
    mesh.clear( );

    Neurites neurites = morpho->neurites( );
    NodePtr node = morpho->soma( )->nodes( )[0];

    ids[ node->id()-1 ] = 0;

    Vec3f pos = node->point( );
    vertices.push_back( pos.x( ));
    vertices.push_back( pos.y( ));
    vertices.push_back( pos.z( ));

    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );
      _GenerateStructure(section, vertices, mesh, ids);
    }

  }

  void NeuronMeshGenerator::GenerateStructure( NeuronMorphologyPtr morpho,
                                      std::vector< VectorizedNodePtr > & vNodes,
                                      vector< float > & vertices,
                                      vector< unsigned int > & mesh )
  {
    vertices.clear( );
    mesh.clear( );

    Neurites neurites = morpho->neurites( );
    NodePtr node = morpho->soma( )->nodes( )[0];

    VectorizedNodePtr vNode = vNodes[ node->id() - 1 ];

    Eigen::Vector3f pos = vNode->Position( );

    vNode->Primitive( new GeometricPrimitive( vertices.size() / 3 ));

    vertices.push_back( pos.x( ));
    vertices.push_back( pos.y( ));
    vertices.push_back( pos.z( ));



    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );
      _GenerateStructure(section, vNodes,vertices, mesh);
    }
  }

  void NeuronMeshGenerator::GenerateStructure(
                                        vector< VectorizedNodePtr > & vNodes,
                                        vector< float > & vertices,
                                        vector< unsigned int > & mesh )
    {
      vertices.clear( );
      mesh.clear( );

      for( int i=0; i < vNodes.size( ); i++ )
      {

        VectorizedNodePtr vNode = vNodes[i];

        int id1 = vertices.size() / 3;

        vNode->Primitive( new GeometricPrimitive( id1 ));
        Eigen::Vector3f pos = vNode->Position( );
        vertices.push_back( pos.x( ));
        vertices.push_back( pos.y( ));
        vertices.push_back( pos.z( ));


        if( vNode->Father() != nullptr )
        {
          int id0 = vNode->Father()->Primitive()->A() ;
          mesh.push_back( id0 );
          mesh.push_back( id1 );
        }

      }
    }

  void NeuronMeshGenerator::GenerateMeshQuads( NeuronMorphologyPtr morpho,
                                          vector< float > & vertices,
                                          vector< float > & centers,
                                          vector< float > & tangents,
                                          vector< unsigned int > & mesh)
  {
    vertices.clear( );
    centers.clear( );
    tangents.clear( );
    mesh.clear( );
    Eigen::Vector3f v( morpho->soma( )->center( ).x( ),
                       morpho->soma( )->center( ).y( ),
                       morpho->soma( )->center( ).z( ));

    Vec3f c = morpho->soma( )->center( );
    Nodes nodes = morpho->soma( )->nodes();
    float radius = 0.0f;
    for( unsigned int i = 0; i < nodes.size(); i++ )
    {
      radius += (nodes[i]->point() - c).norm();
    }
    radius /= nodes.size( );
    _CreateSphereQuads( v, radius, vertices, centers, tangents, mesh );

    //Dendritas

    Neurites neurites = morpho->neurites( );

    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );
      _GenerateMeshQuads( section, vertices, centers, tangents, mesh );
    }


  }

  void NeuronMeshGenerator::GenerateMeshQuads(
                                        vector< VectorizedNodePtr > & vNodes,
                                        vector< float > & vertices,
                                        vector< float > & centers,
                                        vector< float > & tangents,
                                        vector< unsigned int > & mesh )
  {
    vertices.clear( );
    centers.clear( );
    tangents.clear( );
    mesh.clear( );

    CalculateTangents( vNodes );
    CalculateGeometry( vNodes, vertices, centers, tangents);

    for ( int i = 0; i < vNodes.size(); i++ )
    {
      VectorizedNodePtr vNode = vNodes[i];
      if( vNode->Father( ) != nullptr )
      {
        _CreateQuadPipe( vNode->Father( )->Primitive( ), vNode->Primitive(), mesh );
      }
    }
  }

  void NeuronMeshGenerator::GenerateMeshTriangles( NeuronMorphologyPtr morpho,
                                           vector< float > & vertices,
                                           vector< float > & centers,
                                           vector< unsigned int > & mesh)
   {
     vertices.clear( );
     centers.clear( );
     mesh.clear( );
     Eigen::Vector3f v( morpho->soma( )->center( ).x( ),
                        morpho->soma( )->center( ).y( ),
                        morpho->soma( )->center( ).z( ));

     Vec3f c = morpho->soma( )->center( );
     Nodes nodes = morpho->soma( )->nodes();
     float radius = 0.0f;
     for( unsigned int i = 0; i < nodes.size(); i++ )
     {
       radius += (nodes[i]->point() - c).norm();
     }
     radius /= nodes.size( );
     _CreateSphereTriangles( v, radius, vertices, centers, mesh );

     //Dendritas

     Neurites neurites = morpho->neurites( );

     for( unsigned int i = 0; i < neurites.size( ); i++ )
     {
       SectionPtr section = neurites[i]->firstSection( );
       _GenerateMeshTriangles( section, vertices, centers, mesh );
     }


   }

  void NeuronMeshGenerator::GenerateMeshTriangles(
                                          vector< VectorizedNodePtr > & vNodes,
                                          vector< float > & vertices,
                                          vector< float > & centers,
                                          vector< unsigned int > & mesh)
    {
      vertices.clear( );
      centers.clear( );
      mesh.clear( );

      CalculateTangents( vNodes );
      vector< float > t;
      CalculateGeometry( vNodes, vertices, centers, t);

      for ( int i = 0; i < vNodes.size(); i++ )
      {
        VectorizedNodePtr vNode = vNodes[i];
        if( vNode->Father( ) != nullptr )
        {
          _CreateTrianglePipe( vNode->Father( )->Primitive( ),
                               vNode->Primitive(), mesh );
        }
//        _CreateSphereTriangles(pos,radius,vertices,centers,mesh);//BORRAR
      }

  }

  void NeuronMeshGenerator::GenerateSomaTriangles( nsol::SomaPtr soma,
                                 std::vector< VectorizedNodePtr > & firstNodes,
                                 std::vector< float >& vertices,
                                 std::vector< float >& centers,
                                 std::vector< unsigned int >& mesh )
  {



  }

  void NeuronMeshGenerator::VectorizeMorpho( NeuronMorphologyPtr morpho,
                                           vector< VectorizedNodePtr > & vNodes)
  {
    vNodes.clear( );
    Neurites neurites = morpho->neurites( );

    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );


      //////////////////////////////////////////////////////
      ///Hack Delete when nsol loaders are rigth
      section->firstSegment( section->firstSegment( )->next( ) );
      //////////////////////////////////////////////////////

      VectorizedNodePtr vNode = new VectorizedNode( );
      NodePtr node = section->firstSegment( )->begin( );
      Vec3f pos = node->point( );
      vNode->Position( Eigen::Vector3f( pos.x(), pos.y(), pos.z( )));
      vNode->Radius( node->radius( ));
      vNode->Id( vNodes.size( ));

//      std::cout << "Nodo " << node->id() << "\nPosicion: " << node->point().x()
//                << " " << node->point().y() << " " << node->point().z()
//                << std::endl;



      vNodes.push_back( vNode );

      _VectorizeMorpho(section, vNodes, vNode);
    }
  }

  void NeuronMeshGenerator::VectorizeMorpho( NeuronMorphologyPtr morpho,
                                       vector< VectorizedNodePtr >& vNodes,
                                       vector< VectorizedNodePtr >& firstNodes )
  {
    vNodes.clear( );
    firstNodes.clear( );
    Neurites neurites = morpho->neurites( );

    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );

      //////////////////////////////////////////////////////
      ///Hack Delete when nsol loaders are rigth
      section->firstSegment( section->firstSegment( )->next( ) );
      //////////////////////////////////////////////////////

      VectorizedNodePtr vNode = new VectorizedNode( );
      NodePtr node = section->firstSegment( )->begin( );
      Vec3f pos = node->point( );
      vNode->Position( Eigen::Vector3f( pos.x(), pos.y(), pos.z( )));
      vNode->Radius( node->radius( ));
      vNode->Id( vNodes.size( ));

//      std::cout << "Nodo " << node->id() << "\nPosicion: " << node->point().x()
//                << " " << node->point().y() << " " << node->point().z()
//                << std::endl;



      vNodes.push_back( vNode );
      firstNodes.push_back( vNode );

      _VectorizeMorpho(section, vNodes, vNode);
    }
  }

  void NeuronMeshGenerator::CalculateTangents(
                                    std::vector< VectorizedNodePtr >& vNodes )
  {
    for ( int i = 0; i < vNodes.size(); i++ )
    {
      VectorizedNodePtr vNode = vNodes[i];
      VectorizedNodePtr vNodeFather = vNode->Father( );
      vector< VectorizedNodePtr > childNodes = vNode->Childs();

      Eigen::Vector3f tangent( 0.0f, 0.0f, 0.0f );



      if ( childNodes.size( ) == 0 ) // End node
      {
//        cout << "Tipo: Terminal" << std::endl;
        vNodeFather = vNode->Father() ;
        tangent = ( vNode->Position( ) - vNodeFather->Position( )).normalized( );
      }
      else
      {

        if ( vNode->Bifurcation( )) // Bifurcation Node
        {
//          cout << "Tipo: Bifurcacion" << std::endl;
//          std::cout << "Nodo " << i << "\nPosicion: " << vNode->Position().x()
//                                       << " " << vNode->Position().y() << " "
//                                       << vNode->Position().z() << std::endl;
          for ( int j = 0; j < childNodes.size(); j++ )
          {

//            Eigen::Vector3f p1 = childNodes[j]->Position( );
//            std::cout << "Posicion de hijo " << j << ": " << p1.x() << " "
//                      << p1.y() << " " << p1.z() << std::endl;
//            Eigen::Vector3f t = childNodes[j]->Position( ) - vNode->Position( );
//            std::cout << "Tangente00: " << t.x() << " " << t.y() << " "
//                                                   << t.z() << std::endl;
            tangent += ( childNodes[j]->Position( ) - vNode->Position( ) ).normalized( );
          }
          tangent.normalize( );

          if( vNodeFather != nullptr )
          {
            tangent  += ( vNode->Position( ) - vNodeFather->Position( )).normalized( );
            tangent.normalize( );
          }
//          std::cout << "Tangente: " << tangent.x() << " " << tangent.y() << " "
//                              << tangent.z() << std::endl;

        }
        else // Normal Mode
        {
//          cout << "Tipo: Estandar" << std::endl;
          tangent = ( childNodes[0]->Position( ) - vNode->Position( ) ).normalized( );
          if( vNodeFather != nullptr )
          {
            tangent  += ( vNode->Position( ) - vNodeFather->Position( )).normalized( );
            tangent.normalize( );
          }
        }
      }

//      std::cout << "Tangente: " << tangent.x() << " " << tangent.y() << " "
//                                    << tangent.z() << std::endl;


      vNode->Tangent(tangent);

    }
  }

  void NeuronMeshGenerator::CalculateGeometry(
                                          vector< VectorizedNodePtr >& vNodes,
                                          vector< float >& vertices,
                                          vector< float >& centers,
                                          vector< float >& tangents )
  {

    for ( int i = 0; i < vNodes.size(); i++ )
    {

      Eigen::Vector3f exe( 0.0f, 0.0f, 1.0f );

      Eigen::Vector3f va( 0.0f, 1.0f, 0.0f );
      Eigen::Vector3f vb( -1.0f, 0.0f, 0.0f );
      Eigen::Vector3f vc( 0.0f, -1.0f, 0.0f );
      Eigen::Vector3f vd( 1.0f, 0.0f, 0.0f );

      Eigen::Vector3f center;
      Eigen::Vector3f tangent;
      Eigen::Vector3f position;

      Eigen::Quaternion< float > q;


      VectorizedNodePtr vNode = vNodes[i];

      center = vNode->Position( );
      tangent = vNode->Tangent();


      //std::cout << "Tangente de nodo i :" << tangent.x() << " " << tangent.y()
      //          << " " << tangent.z() << std::endl;

      q.setFromTwoVectors(exe,tangent);

      int a = vertices.size( ) / 3;
      position = q._transformVector( va ) * vNode->Radius( ) + center;
      vertices.push_back( position.x( ));
      vertices.push_back( position.y( ));
      vertices.push_back( position.z( ));

      int b = vertices.size( ) / 3;
      position = q._transformVector( vb ) * vNode->Radius( ) + center;
      vertices.push_back( position.x( ));
      vertices.push_back( position.y( ));
      vertices.push_back( position.z( ));

      int c = vertices.size( ) / 3;
      position = q._transformVector( vc ) * vNode->Radius( ) + center;
      vertices.push_back( position.x( ));
      vertices.push_back( position.y( ));
      vertices.push_back( position.z( ));

      int d = vertices.size( ) / 3;
      position = q._transformVector( vd ) * vNode->Radius( ) + center;
      vertices.push_back( position.x( ));
      vertices.push_back( position.y( ));
      vertices.push_back( position.z( ));


      for (unsigned int j = 0; j < 4; j++ )
      {
        centers.push_back( center.x( ));
        centers.push_back( center.y( ));
        centers.push_back( center.z( ));

        tangents.push_back( tangent.x( ));
        tangents.push_back( tangent.y( ));
        tangents.push_back( tangent.z( ));
      }

      vNode->Primitive( new GeometricPrimitive( a, b, c, d ));
    }
  }

// PRIVATE METHODS

  void NeuronMeshGenerator::_GenerateStructure( nsol::SectionPtr section,
                                                vector< float > & vertices,
                                                vector< unsigned int > & mesh,
                                                vector< unsigned int > & ids )
  {
    SegmentPtr segment = section->firstSegment( );

    while( segment != nullptr)
    {
      int id0 = ids[ segment->begin( )->id( )-1 ];
      int id1 = vertices.size( )/3;

      NodePtr node = segment->end( );

      ids[ node->id( )-1 ] = id1;

      Vec3f pos = node->point( );
      vertices.push_back( pos.x( ));
      vertices.push_back( pos.y( ));
      vertices.push_back( pos.z( ));

      mesh.push_back( id0 );
      mesh.push_back( id1 );

      segment = segment->next();
    }

    Sections childs = section->children();
    for( unsigned int i = 0; i < childs.size(); i++ )
    {
      _GenerateStructure( childs[i], vertices, mesh, ids );

    }

  }

  void NeuronMeshGenerator::_GenerateStructure( nsol::SectionPtr section,
                                           vector< VectorizedNodePtr > & vNodes,
                                           vector< float > & vertices,
                                           vector< unsigned int > & mesh )
    {
      SegmentPtr segment = section->firstSegment( );

      while( segment != nullptr)
      {
        int id0 = vNodes[ segment->begin()->id( ) - 1 ]->Primitive( )->A();

        int id1 = vertices.size( ) / 3;

        NodePtr node = segment->end( );

        VectorizedNodePtr vNode = vNodes[ node->id() - 1 ];

        Eigen::Vector3f pos = vNode->Position( );

        vNode->Primitive( new GeometricPrimitive( id1 ));

        vertices.push_back( pos.x( ));
        vertices.push_back( pos.y( ));
        vertices.push_back( pos.z( ));

        mesh.push_back( id0 );
        mesh.push_back( id1 );

        segment = segment->next();
      }

      Sections childs = section->children();
      for( unsigned int i = 0; i < childs.size(); i++ )
      {
        _GenerateStructure( childs[i], vNodes, vertices, mesh );

      }

    }

  void NeuronMeshGenerator::_GenerateMeshQuads( SectionPtr section,
                                           vector< float > & vertices,
                                           vector< float > & centers,
                                           vector< float > & tangents,
                                           vector< unsigned int > & mesh )
  {
    SegmentPtr segment = section->firstSegment( );

    while( segment != nullptr)
    {
      NodePtr node = segment->end( );

      Eigen::Vector3f v( node->point( ).x( ),
                         node->point( ).y( ),
                         node->point( ).z( ));
      float radius = node->radius( );
      _CreateSphereQuads( v, radius, vertices, centers, tangents, mesh );

     segment = segment->next();
     }

     Sections childs = section->children();
     for( unsigned int i = 0; i < childs.size(); i++ )
     {
       _GenerateMeshQuads( childs[i], vertices, centers, tangents, mesh );
     }
  }

  void NeuronMeshGenerator::_GenerateMeshTriangles( SectionPtr section,
                                             vector< float > & vertices,
                                             vector< float > & centers,
                                             vector< unsigned int > & mesh )
    {
      SegmentPtr segment = section->firstSegment( );

      while( segment != nullptr)
      {
        NodePtr node = segment->end( );

        Eigen::Vector3f v( node->point( ).x( ),
                           node->point( ).y( ),
                           node->point( ).z( ));
        float radius = node->radius( );
        _CreateSphereTriangles( v, radius, vertices, centers, mesh );

       segment = segment->next();
       }

       Sections childs = section->children();
       for( unsigned int i = 0; i < childs.size(); i++ )
       {
         _GenerateMeshTriangles( childs[i], vertices, centers, mesh );
       }
    }

  void NeuronMeshGenerator::_VectorizeMorpho( SectionPtr section,
                                          vector< VectorizedNodePtr > & vNodes,
                                          VectorizedNodePtr vFatherNode )
  {
    SegmentPtr segment = section->firstSegment( );

    while( segment != nullptr)
    {
      VectorizedNodePtr vNode = new VectorizedNode( );

      NodePtr node = segment->end( );
      Vec3f pos = node->point( );

//      std::cout << "Nodo " << node->id() << "\nPosicion: " << node->point().x()
//                      << " " << node->point().y() << " " << node->point().z()
//                      << std::endl;

      vNode->Position( Eigen::Vector3f( pos.x(), pos.y(), pos.z( )));
      vNode->Radius( node->radius( ));
      vNode->Id( vNodes.size() );
      vNode->Father( vFatherNode);
      vNodes.push_back( vNode );

      vFatherNode->AddChild( vNode );
      vFatherNode = vNode;
      segment = segment->next();
    }

    Sections childSections = section->children();
    for( unsigned int i = 0; i < childSections.size(); i++ )
    {

      _VectorizeMorpho( childSections[i], vNodes, vFatherNode );

    }
  }

  unsigned int NeuronMeshGenerator::_NumNodes( NeuronMorphologyPtr morpho )
  {
    unsigned int numNodes = 0;
    Neurites neurites = morpho->neurites( );
    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );
      numNodes += _NumNodes(section);

    }

    return numNodes;
  }

  unsigned int NeuronMeshGenerator::_NumNodes( nsol::SectionPtr section )
  {
      unsigned int numNodes = 0;
      SegmentPtr segment = section->firstSegment( );

      while( segment != nullptr)
      {
        numNodes++;
        segment = segment->next();
      }

      Sections childs = section->children();

      for( unsigned int i = 0; i < childs.size(); i++ )
      {
        numNodes += _NumNodes( childs[i] );

      }

      return numNodes;
  }

  unsigned int NeuronMeshGenerator::_MaxId( nsol::NeuronMorphologyPtr morpho )
  {
    unsigned int maxId = 0;
    Neurites neurites = morpho->neurites( );
    for( unsigned int i = 0; i < neurites.size( ); i++ )
    {
      SectionPtr section = neurites[i]->firstSection( );

      unsigned int id = _MaxId(section);
      if(id > maxId)
        maxId = id;
    }

    return maxId;
  }

  unsigned int NeuronMeshGenerator::_MaxId( nsol::SectionPtr section )
  {
      unsigned int maxId = 0;
      SegmentPtr segment = section->firstSegment( );

      while( segment != nullptr)
      {
        unsigned int id = segment->end( )->id( );
        if( id > maxId )
          maxId = id;
        segment = segment->next();
      }

      Sections childs = section->children();

      for( unsigned int i = 0; i < childs.size(); i++ )
      {
        unsigned int id = _MaxId( childs[i] );
        if( id > maxId )
          maxId = id;
      }

      return maxId;
  }

  void NeuronMeshGenerator::_CreateSphereQuads( Eigen::Vector3f center,
                                           float radius,
                                           vector< float > & vertices,
                                           vector< float > & centers,
                                           vector< float > & tangents,
                                           vector< unsigned int > & mesh )
  {

    Eigen::Vector3f aux = center + Eigen::Vector3f( 0.0, 0.0, radius );
    int id0 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( 0.0, 0.0, -radius );
    int id1 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( radius, 0.0, 0.0 );
    int id2 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( -radius, 0.0, 0.0 );
    int id3 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( 0.0, radius, 0.0 );
    int id4 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( 0.0, -radius, 0.0 );
    int id5 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    for( int i = 0; i < 6; i++ )
    {
      centers.push_back( center.x( ));
      centers.push_back( center.y( ));
      centers.push_back( center.z( ));

      tangents.push_back( 0.0f );
      tangents.push_back( 0.0f );
      tangents.push_back( 0.0f );
    }

    mesh.push_back(id4); mesh.push_back(id0); mesh.push_back(id2);
    mesh.push_back(id5);

    mesh.push_back(id4); mesh.push_back(id3); mesh.push_back(id0);
    mesh.push_back(id5);

    mesh.push_back(id4); mesh.push_back(id1); mesh.push_back(id3);
    mesh.push_back(id5);

    mesh.push_back(id4); mesh.push_back(id2); mesh.push_back(id1);
    mesh.push_back(id5);


  }

  void NeuronMeshGenerator::_CreateSphereTriangles( Eigen::Vector3f center,
                                                float radius,
                                                vector< float > & vertices,
                                                vector< float > & centers,
                                                vector< unsigned int > & mesh )
  {

    Eigen::Vector3f aux = center + Eigen::Vector3f( 0.0, 0.0, radius );
    int id0 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( 0.0, 0.0, -radius );
    int id1 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( radius, 0.0, 0.0 );
    int id2 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( -radius, 0.0, 0.0 );
    int id3 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( 0.0, radius, 0.0 );
    int id4 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    aux = center + Eigen::Vector3f( 0.0, -radius, 0.0 );
    int id5 = vertices.size( ) / 3;
    vertices.push_back( aux.x( )); vertices.push_back( aux.y( )); vertices.push_back( aux.z( ));

    for( int i = 0; i < 6; i++ )
    {
      centers.push_back( center.x( ));
      centers.push_back( center.y( ));
      centers.push_back( center.z( ));
    }

    mesh.push_back(id4); mesh.push_back(id0); mesh.push_back(id2);
    mesh.push_back(id5); mesh.push_back(id2); mesh.push_back(id0);

    mesh.push_back(id4); mesh.push_back(id3); mesh.push_back(id0);
    mesh.push_back(id5); mesh.push_back(id0); mesh.push_back(id3);

    mesh.push_back(id4); mesh.push_back(id1); mesh.push_back(id3);
    mesh.push_back(id5); mesh.push_back(id3); mesh.push_back(id1);

    mesh.push_back(id4); mesh.push_back(id2); mesh.push_back(id1);
    mesh.push_back(id5); mesh.push_back(id1); mesh.push_back(id2);

  }

  void NeuronMeshGenerator::_CreateTrianglePipe( GeometricPrimitivePtr geom0,
                           GeometricPrimitivePtr geom1,
                           vector< unsigned int > & mesh )
  {
    //AB
    mesh.push_back( geom0->A() );
    mesh.push_back( geom0->B() );
    mesh.push_back( geom1->A() );

    mesh.push_back( geom0->B() );
    mesh.push_back( geom1->B() );
    mesh.push_back( geom1->A() );

    //BC
    mesh.push_back( geom0->B() );
    mesh.push_back( geom0->C() );
    mesh.push_back( geom1->B() );

    mesh.push_back( geom0->C() );
    mesh.push_back( geom1->C() );
    mesh.push_back( geom1->B() );

    //CD
    mesh.push_back( geom0->C() );
    mesh.push_back( geom0->D() );
    mesh.push_back( geom1->C() );

    mesh.push_back( geom0->D() );
    mesh.push_back( geom1->D() );
    mesh.push_back( geom1->C() );

    //DA
    mesh.push_back( geom0->D() );
    mesh.push_back( geom0->A() );
    mesh.push_back( geom1->D() );

    mesh.push_back( geom0->A() );
    mesh.push_back( geom1->A() );
    mesh.push_back( geom1->D() );
  }

  void NeuronMeshGenerator::_CreateQuadPipe( GeometricPrimitivePtr geom0,
                             GeometricPrimitivePtr geom1,
                             vector< unsigned int > & mesh )
  {
    //AB
    mesh.push_back( geom0->A() );
    mesh.push_back( geom0->B() );
    mesh.push_back( geom1->A() );
    mesh.push_back( geom1->B() );

    //BC
    mesh.push_back( geom0->B() );
    mesh.push_back( geom0->C() );
    mesh.push_back( geom1->B() );
    mesh.push_back( geom1->C() );

    //CD
    mesh.push_back( geom0->C() );
    mesh.push_back( geom0->D() );
    mesh.push_back( geom1->C() );
    mesh.push_back( geom1->D() );

    //DA
    mesh.push_back( geom0->D() );
    mesh.push_back( geom0->A() );
    mesh.push_back( geom1->D() );
    mesh.push_back( geom1->A() );
  }
} // end namespace neurolots

// EOF
