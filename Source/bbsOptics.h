/*
  ==============================================================================

   This file is part of the Belle, Bonne, Sage library
   Copyright 2007-2010 by Andi Brae

  ------------------------------------------------------------------------------

   Belle, Bonne, Sage can be redistributed and/or modified under the terms of
   the GNU Lesser General Public License, as published by the
   Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   Belle, Bonne, Sage is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with Belle, Bonne, Sage; if not, visit www.gnu.org/licenses or write:
   
   Free Software Foundation, Inc.
   59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ==============================================================================
*/

#ifndef bbsOptics
#define bbsOptics

#include "prim.h"

namespace prim
{
extern Profiler primProfiles;
}

namespace bbs
{
  struct Optics
  {
    /**Calculates the conservative distance two paths must be to not collide.
    This calculation is based on their bounding boxes and is useful for
    determining a starting point for a finer optical-based collision detection
    algorithm.*/
    static prim::number CalculateMinimumNonCollidingDistance(
      const prim::Path& p, const prim::Path& q)
    {
      using namespace prim;
      using namespace prim::math;

      Rectangle r = p.GetBoundingBox();
      Rectangle s = q.GetBoundingBox();
      
      return (Dist(r.Width(), r.Height()) + Dist(s.Width(), s.Height())) * 0.5f;
    }

    /**Recursively bisects for the closest non-colliding distance of two paths.
    The first path is the stationary anchor, the other is the floater which
    moves on a line from the origin to the polar coordinate consisting of an
    angle and a minimum non-colliding distance. The latter should be calculated
    with CalculateMinimumNonCollidingDistance. If left zero, it will be
    automatically calculated.*/
    static prim::number CalculateClosestNonCollidingDistanceAtAngle(
      const prim::Path& Anchor, const prim::Path& Floater,
      prim::number ThetaRadians, prim::math::Vector AnchorCenter,
      prim::number MinimumNonCollidingDistance = 0,
      bool UseConvexHulls = true, prim::count Iterations = 10)
    {
      using namespace prim;
      using namespace prim::math;

      _profile(primProfiles);
 
      if(!MinimumNonCollidingDistance)
        MinimumNonCollidingDistance = 
          CalculateMinimumNonCollidingDistance(Anchor, Floater);

      number NearDistance = 0, FarDistance = MinimumNonCollidingDistance;
      number BestDistance = FarDistance;
      
      Vector Near, Far(ThetaRadians, MinimumNonCollidingDistance, false);
        
      //Translate the line on which the floater travels.
      Near += AnchorCenter;
      Far += AnchorCenter;
      
      //Create convex hulls if necessary.
      prim::math::PolygonPath AnchorPolygon, FloaterPolygon;
      prim::math::PolygonPath AnchorHull, FloaterHull;
      if(UseConvexHulls)
      {
        Anchor.GetPolygonPathOutline(AnchorPolygon);
        Floater.GetPolygonPathOutline(FloaterPolygon);
        AnchorPolygon.CreateConvexHull(AnchorHull);
        FloaterPolygon.CreateConvexHull(FloaterHull);
      }

      /*If the maxima intersects, then return 0 since the floater collides no
      matter what.*/
      if((UseConvexHulls && AnchorHull.IntersectsOutline(FloaterHull, Far)) ||
        (!UseConvexHulls && Anchor.IntersectsOutline(Floater, Far)))
      {
        _endprofile(primProfiles);
        return 0;
      }

      for(count i = 0; i < Iterations; i++)
      {
        Vector Mid = (Near + Far) * 0.5f;
        number MidDistance = (NearDistance + FarDistance) * 0.5f;
        if((UseConvexHulls && !AnchorHull.IntersectsOutline(FloaterHull, Mid))
          || (!UseConvexHulls && !Anchor.IntersectsOutline(Floater, Mid)))
        {
          //Move closer.
          Far = Mid;
          BestDistance = FarDistance = MidDistance;
        }
        else
        {
          //Move further away.
          Near = Mid;
          NearDistance = MidDistance;
        }
      }

      _endprofile(primProfiles);
      return BestDistance;
    }
  };
}
#endif
