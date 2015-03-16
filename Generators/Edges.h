/**
 * @file    Fem.h
 * @brief
 * @author  Juan José García <juanjosegarciacan@gmail.com>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 * Do not distribute without further notice.
 */

#ifndef __NEUROLOTS_EDGE__
#define __NEUROLOTS_EDGE__

//Eigen

#include <Node.h>

#include <iostream>
#include <vector>

namespace neurolots
{


  class Edge
  {

    public:

      Edge( unsigned int node_, fem::NodePtr middleNode_ );
      ~Edge( void );

      //Getters
      unsigned int Node( void );
      fem::NodePtr MiddleNode( void );

    private:

      unsigned int _node;
      fem::NodePtr _middleNode;

  };

  class Edges
  {
    public:
      Edges( Eigen::Vector3f _center, float _radius );
      ~Edges( void );

      fem::NodePtr AddEdge( std::vector< fem::NodePtr >& nodes,
                            fem::NodePtr node0, fem::NodePtr node1 );
      void Clear( void );

    private:

      fem::NodePtr _MiddleNode( std::vector< fem::NodePtr >& nodes,
                                fem::NodePtr node0, fem::NodePtr node1);

      Eigen::Vector3f _center;
      float _radius;
      std::vector< std::vector< Edge >> _edges;

  };



} // end namespace neurolots

#endif // __NEUROLOTS_EDGE__

//EOF
