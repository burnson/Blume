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

#ifndef bbsShapes
#define bbsShapes

#include "primPath.h"
#include "primVector.h"

namespace bbs
{
  class StaffLines
  {
    static void DisplaceLineMeetsCurveOnAngle(prim::number Displacement,
      prim::math::Line& l_in, prim::math::Bezier& b_in, prim::math::Line& l_out,
      prim::math::Bezier& b_out)
    {
      using namespace prim;
      using namespace prim::math;

      /*Trivial case: line meets the curve on angle (the slope of the line
      equals (or is extremely close to) the slope of the curve initially).*/
      Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement);
      
      //Error correction: force the bezier curve to start where the line stops.
      b_out.GetControlPoints(p0, p1, p2, p3);
      p0 = l_out.b;
      b_out.SetControlPoints(p0, p1, p2, p3);
    }

    static void DisplaceLineMeetsCurveBelowAngle(prim::number Displacement,
      prim::math::Line& l_in, prim::math::Bezier& b_in, prim::math::Line& l_out,
      prim::math::Line& l2_out, prim::math::Bezier& b_out)
    {
      using namespace prim;
      using namespace prim::math;

      /*Gap case: There is a gap that needs to be filled by an additional line
      segment that has the same slope as the curve's initial tangent and 
      extends from the intersection with the line to the intersection with the
      curve's first point.*/
      Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement); 

      //Calculate the extension line.
      b_out.GetControlPoints(p0, p1, p2, p3);
      Line b_tangent(p0, p1);
      l_out.b = l_out.GetLineIntersection(b_tangent);
      l2_out.a = l_out.b;
      l2_out.b = p0;
    }

    static void DisplaceLineMeetsCurveAboveAngle(prim::number Displacement,
      prim::math::Line& l_in, prim::math::Bezier& b_in, prim::math::Line& l_out,
      prim::math::Bezier& b_out)
    {
      using namespace prim;
      using namespace prim::math;

      /*Overlap case: The displaced line overlaps with the displace curve. This
      problem reduces into finding the intersection of the displaced line and 
      curve and making that the join.*/
      Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement); 

      number t_intersection = b_out.FindLineIntersection(l_out);
      l_out.b = b_out.Value(t_intersection);
      b_out.Trim(t_intersection, (number)1.0);
    }

    static void DisplaceCurveMeetsCurveAndIntersects(prim::number Displacement,
      prim::math::Bezier& b1_in, prim::math::Bezier& b2_in,
      prim::math::Bezier& b1_out, prim::math::Bezier& b2_out)
    {
      using namespace prim;
      using namespace prim::math;

      b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
      b2_out = b2_in.MakeQuasiParallelCurve(Displacement);

      number Segmentation = 100.0;
      for(number i = 0; i < Segmentation; i++)
      {
        number t1 = i / Segmentation;
        number t2 = (i + (number)1.0) / Segmentation;
        Line TestLine(b1_out.Value(t1), b1_out.Value(t2));
        number t2_intersect = b2_out.FindLineIntersection(TestLine);
        Vector p1 = b2_out.Value(t2_intersect);
        Rectangle r(TestLine.a, TestLine.b);
        if(r.IsPointInside(p1))
        {
          Vector p0 = TestLine.a;
          Vector p2 = TestLine.b;
          number TestLineAngle = -TestLine.Angle();
          p0.Ang(p0.Ang() + TestLineAngle);
          p1.Ang(p1.Ang() + TestLineAngle);
          p2.Ang(p2.Ang() + TestLineAngle);
          number t1_intersect = (p1.x - p0.x) / (p2.x - p0.x) * (t2 - t1) + t1;
          b1_out.Trim(0, t1_intersect);
          b2_out.Trim(t2_intersect, (number)1.0);
          break;
        }
      }

      //Error correction: ensure curves join at a single point.
      Vector p0, p1, p2, p3, p4, p5, p6, p7;
      b1_out.GetControlPoints(p0, p1, p2, p3);
      b2_out.GetControlPoints(p4, p5, p6, p7);
      p4 = p3;
      b2_out.SetControlPoints(p4, p5, p6, p7);
    }

  public:
    /**Chooses an appropriate algorithm for calculating a displaced line-curve.
    Returns true if the extension line l_ext_out is used.*/
    static bool DisplaceLineMeetsCurve(prim::number Displacement,
      prim::math::Line& l_in, prim::math::Bezier& b_in, prim::math::Line& l_out,
      prim::math::Line& l_ext_out, prim::math::Bezier& b_out)
    {
      using namespace prim;
      using namespace prim::math;

      Vector p0, p1, p2, p3;
      b_in.GetControlPoints(p0, p1, p2, p3);
      Line b_tangent(p0, p1);
      number JoinAngle = l_in.Angle() - b_tangent.Angle();
      if(JoinAngle < -Pi)
        JoinAngle += TwoPi;
      if(JoinAngle >= Pi)
        JoinAngle -= TwoPi;

      if(Abs(JoinAngle) < 0.001)
      {
        DisplaceLineMeetsCurveOnAngle(Displacement, l_in, b_in, l_out, b_out);
        return false;
      }
      else if((JoinAngle > 0 && Displacement > 0)
        || (JoinAngle < 0 && Displacement <= 0))
      {
        DisplaceLineMeetsCurveBelowAngle(Displacement,
          l_in, b_in, l_out, l_ext_out, b_out);
        return true;
      }
      else
      {
        DisplaceLineMeetsCurveAboveAngle(Displacement,
          l_in, b_in, l_out, b_out);
        return false;
      }
    }

    /**Chooses an appropriate algorithm for calculating a displaced curve-line.
    Returns true if the extension line l_ext_out is used.*/
    static bool DisplaceCurveMeetsLine(prim::number Displacement,
      prim::math::Bezier& b_in, prim::math::Line& l_in,
      prim::math::Bezier& b_out, prim::math::Line& l_ext_out,
      prim::math::Line& l_out)
    {
      using namespace prim;
      using namespace prim::math;

      Vector p0, p1, p2, p3;
      b_in.GetControlPoints(p0, p1, p2, p3);

      Line l_swap(l_in.b, l_in.a);
      Bezier b_swap; b_swap.SetControlPoints(p3, p2, p1, p0);
      bool UseLineExtension = DisplaceLineMeetsCurve(
        -Displacement, l_swap, b_swap, l_out, l_ext_out, b_out);

      b_out.GetControlPoints(p0, p1, p2, p3);
      b_out.SetControlPoints(p3, p2, p1, p0);
      Swap(l_out.a, l_out.b);
      Swap(l_ext_out.a, l_ext_out.b);

      return UseLineExtension;
    }

    ///Chooses an appropriate algorithm for calculating a displace line-line.
    static void DisplaceLineMeetsLine(prim::number Displacement,
      prim::math::Line& l1_in, prim::math::Line& l2_in,
      prim::math::Line& l1_out, prim::math::Line& l2_out)
    {
      using namespace prim;
      using namespace prim::math;

      number JoinAngle = l1_in.Angle() - l2_in.Angle();

      if(Abs(JoinAngle) < 0.001)
      {
        l1_out = l1_in.MakeParallelLine(Displacement);
        l2_out = l2_in.MakeParallelLine(Displacement);

        //Error correction: ensure l1_out and l2_out join at a single point. 
        l2_out.a = l1_out.b;
      }
      else
      {
        l1_out = l1_in.MakeParallelLine(Displacement);
        l2_out = l2_in.MakeParallelLine(Displacement);

        Vector Intersection = l1_out.GetPolarLineIntersection(l2_out);
        l1_out.b = Intersection;
        l2_out.a = Intersection;
      }
    }

    /**Chooses an appropriate algorithm for calculating a displaced curve-curve.
    Returns true if the extension line l_ext_out is used.*/
    static bool DisplaceCurveMeetsCurve(prim::number Displacement,
      prim::math::Bezier& b1_in, prim::math::Bezier& b2_in,
      prim::math::Bezier& b1_out, prim::math::Line& l1_ext_out,
      prim::math::Line& l2_ext_out, prim::math::Bezier& b2_out)
    {
      using namespace prim;
      using namespace prim::math;

      Vector p0, p1, p2, p3, p4, p5, p6, p7;
      b1_in.GetControlPoints(p0, p1, p2, p3);
      b2_in.GetControlPoints(p4, p5, p6, p7);

      Line b1_tangent(p2, p3);
      Line b2_tangent(p4, p5);
      number JoinAngle = b1_tangent.Angle() - b2_tangent.Angle();
      if(JoinAngle < -Pi)
        JoinAngle += TwoPi;
      if(JoinAngle >= Pi)
        JoinAngle -= TwoPi;

      if(Abs(JoinAngle) < 0.001)
      {
        b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
        b2_out = b2_in.MakeQuasiParallelCurve(Displacement);
        
        //Error correction: ensure curves join at a single point.
        b1_out.GetControlPoints(p0, p1, p2, p3);
        b2_out.GetControlPoints(p4, p5, p6, p7);
        p4 = p3;
        b2_out.SetControlPoints(p4, p5, p6, p7);
        return false;
      }
      else if((JoinAngle > 0 && Displacement > 0)
        || (JoinAngle < 0 && Displacement <= 0))
      {
        b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
        b2_out = b2_in.MakeQuasiParallelCurve(Displacement);

        //Have to fill gap with two line extensions.
        b1_out.GetControlPoints(p0, p1, p2, p3);
        b2_out.GetControlPoints(p4, p5, p6, p7);
        Line b1_out_tangent(p2, p3);
        Line b2_out_tangent(p4, p5);
        Vector Intersection = 
          b1_out_tangent.GetLineIntersection(b2_out_tangent);
        l1_ext_out.a = p3;
        l1_ext_out.b = Intersection;
        l2_ext_out.a = Intersection;
        l2_ext_out.b = p4;
        return true;
      }
      else
      {
        DisplaceCurveMeetsCurveAndIntersects(Displacement,
          b1_in, b2_in, b1_out, b2_out);
        return false;
      }
    }

    static void DeriveParallelPath(prim::number Displacement,
      prim::Path& DestinationPath, prim::Path::Component& SourceComponent)
    {
      using namespace prim;
      using namespace prim::math;

      Path::Component& d = DestinationPath.Components.AddOne();
      Path::Component& s = SourceComponent;
      
      count s_Curves_n = s.Curves.n() - 2;
      for(count i = 1; i <= s_Curves_n; i++)
      {
        Vector& StartPoint = s.Curves[i - 1].End;
        Path::Curve& LeftCurve = s.Curves[i];
        Path::Curve& RightCurve = s.Curves[i + 1];
        if(!LeftCurve.IsCurve && !RightCurve.IsCurve)
        {
          Line l1(StartPoint, LeftCurve.End), l2(LeftCurve.End, RightCurve.End);
          Line l1_out, l2_out;
          StaffLines::DisplaceLineMeetsLine(Displacement, l1, l2,
            l1_out, l2_out);
          if(i == 1) d.AddCurve(l1_out.a);

          d.AddCurve(l1_out.b);
          if(i == s_Curves_n) d.AddCurve(l2_out.b);
        }
        else if(LeftCurve.IsCurve && !RightCurve.IsCurve)
        {
          Bezier b1; b1.SetControlPoints(StartPoint, LeftCurve.StartControl,
            LeftCurve.EndControl, LeftCurve.End);
          Line l1(LeftCurve.End, RightCurve.End);

          Bezier b_out;
          Line l_ext_out, l_out;
          bool UseExtension = StaffLines::DisplaceCurveMeetsLine(Displacement,
            b1, l1, b_out, l_ext_out, l_out);

          Vector p0, p1, p2, p3;
          b_out.GetControlPoints(p0, p1, p2, p3);
          if(i == 1) d.AddCurve(p0);

          d.AddCurve(b_out);
          if(UseExtension) d.AddCurve(l_ext_out.b);
          if(i == s_Curves_n) d.AddCurve(l_out.b);
        }
        else if(!LeftCurve.IsCurve && RightCurve.IsCurve)
        {
          Line l1(StartPoint, LeftCurve.End);
          Bezier b1; b1.SetControlPoints(LeftCurve.End, RightCurve.StartControl,
            RightCurve.EndControl, RightCurve.End);

          Line l_ext_out, l_out;
          Bezier b_out;
          bool UseExtension = StaffLines::DisplaceLineMeetsCurve(Displacement,
            l1, b1, l_out, l_ext_out, b_out);

          Vector p0, p1, p2, p3;
          b_out.GetControlPoints(p0, p1, p2, p3);
          if(i == 1) d.AddCurve(l_out.a);

          d.AddCurve(l_out.b);
          if(UseExtension) d.AddCurve(l_ext_out.b);
          if(i == s_Curves_n) d.AddCurve(b_out);
        }
        else
        {
          Bezier b1; b1.SetControlPoints(StartPoint, LeftCurve.StartControl,
            LeftCurve.EndControl, LeftCurve.End);
          Bezier b2; b2.SetControlPoints(LeftCurve.End,
            RightCurve.StartControl, RightCurve.EndControl, RightCurve.End);

          Bezier b1_out, b2_out;
          Line l_ext1_out, l_ext2_out;
          bool UseExtension = StaffLines::DisplaceCurveMeetsCurve(Displacement,
            b1, b2, b1_out, l_ext1_out, l_ext2_out, b2_out);

          Vector p0, p1, p2, p3;
          b1_out.GetControlPoints(p0, p1, p2, p3);
          if(i == 1) d.AddCurve(p0);

          d.AddCurve(b1_out);
          if(UseExtension)
          {
            d.AddCurve(l_ext1_out.b);
            d.AddCurve(l_ext2_out.b);
          }
          if(i == s_Curves_n) d.AddCurve(b2_out);
        }
      }
    }
  };

  struct Shapes
  {
    /**\brief Adds a rectangular Component to a Path by specifying the
    bottom-left and top-right corners as Vector objects.*/
    static void AddRectangle(prim::Path& p,
      prim::math::Vector BottomLeft,
      prim::math::Vector TopRight, bool isCounterClockwise = true)
    {
      using namespace prim;
      using namespace prim::math;
      p.AddComponent(BottomLeft);
      if(isCounterClockwise)
      {
        p.AddCurve(Vector(TopRight.x,BottomLeft.y));
        p.AddCurve(Vector(TopRight.x,TopRight.y));
        p.AddCurve(Vector(BottomLeft.x,TopRight.y));
        p.AddCurve(BottomLeft);
      }
      else
      {
        p.AddCurve(Vector(BottomLeft.x,TopRight.y));
        p.AddCurve(Vector(TopRight.x,TopRight.y));
        p.AddCurve(Vector(TopRight.x,BottomLeft.y));
        p.AddCurve(BottomLeft);
      }
    }

    ///Adds a rectangular Component to a Path by specifying a Rectangle.
    static void AddRectangle(prim::Path& p,
      prim::math::Rectangle r, bool isCounterClockwise = true)
    {
      AddRectangle(p, r.BottomLeft(), r.TopRight(), isCounterClockwise);
    }

    static void AddEllipse(prim::Path& p, prim::math::Vector Origin,
      prim::math::Vector Scale, prim::number Rotation,
      bool isCounterClockwise = true)
    {
      using namespace prim;
      using namespace prim::math;
      
      Bezier b1;
      Bezier b2;
      Bezier b3;
      Bezier b4;

      b1.Ellipse(Origin, Scale, Rotation, (count)1, isCounterClockwise);
      b2.Ellipse(Origin, Scale, Rotation, (count)2, isCounterClockwise);
      b3.Ellipse(Origin, Scale, Rotation, (count)3, isCounterClockwise);
      b4.Ellipse(Origin, Scale, Rotation, (count)4, isCounterClockwise);

      Vector v1;
      Vector v2;
      Vector v3;
      Vector v4;

      if(isCounterClockwise)
      {
        b1.GetControlPoints(v1, v2, v3, v4);
        p.AddComponent(v1);
        p.AddCurve(v2, v3, v4);

        b2.GetControlPoints(v1, v2, v3, v4);
        p.AddCurve(v2, v3, v4);

        b3.GetControlPoints(v1, v2, v3, v4);
        p.AddCurve(v2, v3, v4);

        b4.GetControlPoints(v1, v2, v3, v4);
        p.AddCurve(v2, v3, v4);
      }
      else
      {
        b4.GetControlPoints(v1, v2, v3, v4);
        p.AddComponent(v1);
        p.AddCurve(v2, v3, v4);

        b3.GetControlPoints(v1, v2, v3, v4);
        p.AddCurve(v2, v3, v4);

        b2.GetControlPoints(v1, v2 , v3, v4);
        p.AddCurve(v2, v3, v4);

        b1.GetControlPoints(v1, v2, v3, v4);
        p.AddCurve(v2, v3, v4);
      }
    }

    ///Adds a circular component to a path.
    static void AddCircle(prim::Path& p, prim::math::Vector Origin,
      prim::number Diameter, bool isCounterClockwise = true)
    {
      using namespace prim::math;
      AddEllipse(p, Origin, Vector(Diameter, Diameter), 0, isCounterClockwise);
    }

    /**Adds the outline of a non-zero thickness line to a Path. Additionally
    you can specify whether either the start or end caps are round
    (elliptical), and specify how elliptical the caps are.*/
    static void AddLine(prim::Path& p, prim::math::Vector Start,
      prim::math::Vector End, prim::number Thickness,
      bool isCounterClockwise = true, bool StartRoundCap = true,
      bool EndRoundCap = true, prim::number CapRelativeHeight = 1.0f)
    {
      using namespace prim;
      using namespace prim::math;

      Vector Delta = End - Start;
      number Angle = Delta.Ang();

      Vector t;

      t.Polar(Angle - HalfPi, Thickness / (number)2.0);

      Vector p0 = Start + t;
      Vector p1 = End + t;
      Vector p2 = End - t;
      Vector p3 = Start - t;

      Bezier Start1;
      Bezier Start2;
      Bezier End1;
      Bezier End2;

      Vector Scale(Thickness,Thickness*CapRelativeHeight);

      End1.Ellipse(End, Scale, Angle - HalfPi, 1, isCounterClockwise);
      End2.Ellipse(End, Scale, Angle - HalfPi, 2, isCounterClockwise);
      Start1.Ellipse(Start, Scale, Angle - HalfPi, 3,isCounterClockwise);
      Start2.Ellipse(Start, Scale, Angle - HalfPi, 4,isCounterClockwise);

      p.AddComponent(p0);

      if(!StartRoundCap && !EndRoundCap)
      {
        if(isCounterClockwise)
        {
          p.AddCurve(p1);
          p.AddCurve(p2);
          p.AddCurve(p3);
          p.AddCurve(p0);
        }
        else
        {
          p.AddCurve(p3);
          p.AddCurve(p2);
          p.AddCurve(p1);
          p.AddCurve(p0);
        }
      }
      else if(EndRoundCap && !StartRoundCap)
      {
        if(isCounterClockwise)
        {
          p.AddCurve(p1);
          p.AddCurve(End1);
          p.AddCurve(End2);
          p.AddCurve(p3);
          p.AddCurve(p0);
        }
        else
        {
          p.AddCurve(p3);
          p.AddCurve(p2);
          p.AddCurve(End2);
          p.AddCurve(End1);
          p.AddCurve(p0);
        }
      }
      else if(StartRoundCap && !EndRoundCap)
      {
        if(isCounterClockwise)
        {
          p.AddCurve(p1);
          p.AddCurve(p2);
          p.AddCurve(p3);
          p.AddCurve(Start1);
          p.AddCurve(Start2);
        }
        else
        {
          p.AddCurve(Start2);
          p.AddCurve(Start1);
          p.AddCurve(p2);
          p.AddCurve(p1);
          p.AddCurve(p0);
        }
      }
      else
      {
        if(isCounterClockwise)
        {
          p.AddCurve(p1);
          p.AddCurve(End1);
          p.AddCurve(End2);
          p.AddCurve(p3);
          p.AddCurve(Start1);
          p.AddCurve(Start2);
        }
        else
        {
          p.AddCurve(Start2);
          p.AddCurve(Start1);
          p.AddCurve(p2);
          p.AddCurve(End2);
          p.AddCurve(End1);
          p.AddCurve(p0);
        }
      }
    }

    ///Creates a rectangular Component to a Path with four lines.
    static void AddRectangleFromLines(prim::Path& p,
      prim::math::Vector BottomLeft, prim::math::Vector TopRight,
      prim::number Thickness)
    {
      prim::math::Vector BottomRight(TopRight.x, BottomLeft.y);
      prim::math::Vector TopLeft(BottomLeft.x, TopRight.y);
      AddLine(p, TopRight, BottomRight, Thickness);
      AddLine(p, BottomRight, BottomLeft, Thickness);
      AddLine(p, BottomLeft, TopLeft, Thickness);
      AddLine(p, TopLeft, TopRight, Thickness);
    }

    struct Music
    {
      static void AddWholeNote(prim::Path& p,
        prim::math::Vector HeadOrigin,
        prim::number SpaceHeight = (prim::number)1.0,
        prim::number RelativeWidth=1.7,
        prim::number HollowScale=0.55,
        prim::number HollowAngle=0.75*prim::math::Pi)
      {
        using namespace prim::math;
        Vector HeadScale(SpaceHeight*RelativeWidth,SpaceHeight);
        Vector HollowVectorScale = HeadScale;
        HollowVectorScale *= HollowScale;

        AddEllipse(p, HeadOrigin, HeadScale, 0, true);
        AddEllipse(p, HeadOrigin, HollowVectorScale,
          HollowAngle, false);
      }

      static void AddQuarterNote(prim::Path& p,
        prim::math::Vector HeadOrigin,
        prim::number SpaceHeight = (prim::number)1.0,
        bool MakeSingleOutline = true,
        prim::number RelativeStemHeight = (prim::number)4.0,
        prim::math::Vector* FlagPosition = 0,
        prim::number HeadTheta = (prim::number)20*prim::math::DegToRad,
        prim::number RelativeWidth = (prim::number)1.4,
        prim::number RelativeStemThickness = 0.1,
        prim::number RelativeStemCapHeight = (prim::number)0.8,
        bool isHollow = false,
        prim::number HollowScale = 0.48,
        prim::number HollowTheta =
          prim::math::HalfPi * (prim::number)0.4)
      {
        using namespace prim;
        using namespace prim::math;

        Vector HeadScale(SpaceHeight * RelativeWidth, SpaceHeight);
        number StemThickness = RelativeStemThickness * SpaceHeight;
        number StemHeight = RelativeStemHeight * SpaceHeight;

        //Create the hollow area if necessary.
        if(isHollow)
        {
          Vector HollowVectorScale = HeadScale * HollowScale;

          Vector Start;
          Start.Polar(HollowTheta + Pi,
            HollowVectorScale.x / (number)2.0);
          Start += HeadOrigin;

          Vector End;
          End.Polar(HollowTheta, HollowVectorScale.x / (number)2.0);
          End += HeadOrigin;

          AddLine(p, Start, End, HollowVectorScale.y,
            false, true, true, 1.0);
        }

        if(!MakeSingleOutline ||
          Abs(StemHeight) < HeadScale.y / (number)2.0)
        {
          /*Algorithm 1 - Draw an ellipse and overlay the stem as two
          different sub paths. Only draw a line if the height is non-
          zero, and draw on the right if it is positive, and on the
          left if it is negative.*/
          AddEllipse(p, HeadOrigin, HeadScale, HeadTheta, true);

          if(StemHeight > HeadScale.y / (number)2.0)
          {
            Vector Start = Ellipse::VerticalTangent(
              HeadScale.x / (number)2.0,
              HeadScale.y / (number)2.0, HeadTheta);

            StemHeight -= Start.y;

            Start.x = HeadOrigin.x + Start.x;
            Start.y = HeadOrigin.y + Start.y;

            Start.x -= StemThickness / (number)2.0;
            Vector End = Start;
            End.y += StemHeight;
            AddLine(p, Start, End, StemThickness, true, false, true,
              RelativeStemCapHeight);

            if(FlagPosition)
              *FlagPosition = Vector(
              End.x + StemThickness * (number)0.5, End.y);
          }
          else if(StemHeight < -HeadScale.y / (number)2.0)
          {
            Vector Start = Ellipse::VerticalTangent(
              HeadScale.x / (number)2.0,
              HeadScale.y / (number)2.0, HeadTheta);

            StemHeight += Start.y;

            Start.x = HeadOrigin.x - Start.x;
            Start.y = HeadOrigin.y - Start.y;

            Start.x += StemThickness / (number)2.0;
            Vector End = Start;
            End.y += StemHeight;
            AddLine(p, Start, End, StemThickness, true, false, true,
              RelativeStemCapHeight);
            if(FlagPosition)
              *FlagPosition = Vector(
              End.x - StemThickness * (number)0.5, End.y);
          }
        }
        else
        {
          /*Algorithm 2 - Calculate the outline exactly. Somewhat
          tedious and lots of math, but highly worth it for the
          ability to "ghost" outline notes.*/

          //Step 1 - Calculate the intersections.
          Bezier c1;
          Bezier c4;

          c1.Ellipse(HeadOrigin,HeadScale,HeadTheta,1,true);
          c4.Ellipse(HeadOrigin,HeadScale,HeadTheta,4,true);

          //Quadrant 1 intersection
          number t1 = Ellipse::VerticalIntersection(
            HeadScale.x / (number)2.0,
            HeadScale.y / (number)2.0,
            HeadTheta, StemThickness);

          //Quadrant 4 intersection
          number t4;
          number dummy;
          if(!c4.VerticalTangents(t4,dummy))
            return;

          //Step 2 - Declare the path segment variables.
          Bezier p1;
          Bezier p2;
          Bezier p3;
          Bezier p4;
          Vector l5_Start;
          Vector l5_End;
          Bezier p6;
          Bezier p7;
          Vector l8_Start;
          Vector l8_End;

          /*Step 3 - Create each part of the path, depending on
          whether the stem is on the right or left.*/
          if(StemHeight > (number)0.0)
          {
            p1.Ellipse(HeadOrigin, HeadScale, HeadTheta,
              (prim::count)1, true);
            p1.Trim(t1, (number)1.0); //Trim the curve.

            p2.Ellipse(HeadOrigin, HeadScale, HeadTheta,
              (prim::count)2, true);

            p3.Ellipse(HeadOrigin, HeadScale, HeadTheta,
              (prim::count)3, true);

            p4.Ellipse(HeadOrigin, HeadScale, HeadTheta,
              (prim::count)4, true);
            p4.Trim(0.0,t4); //Trim the curve.

            l5_Start = p4.Value((number)1.0);
            l5_End = l5_Start;
            l5_End.y = HeadOrigin.y + StemHeight;

            Vector p6origin(l5_End.x - StemThickness / (number)2.0,
              l5_End.y);
            Vector p6scale(StemThickness,
              StemThickness * RelativeStemCapHeight);
            p6.Ellipse(p6origin, p6scale, (number)0.0,
              (prim::count)1, true);

            p7.Ellipse(p6origin, p6scale, (number)0.0,
              (prim::count)2, true);

            l8_Start = l5_End;
            l8_Start.x -= StemThickness;
            l8_End = p1.Value(0);

            if(FlagPosition)
              *FlagPosition = l5_End;
          }
          else
          {
            p1.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi,
              (prim::count)1, true);
            p1.Trim(t1,1.0); //Trim the curve.

            p2.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi,
              (prim::count)2, true);

            p3.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi,
              (prim::count)3, true);

            p4.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi,
              (prim::count)4, true);
            p4.Trim((number)0.0,t4); //Trim the curve.

            l5_Start = p4.Value((number)1.0);
            l5_End = l5_Start;
            l5_End.y = HeadOrigin.y + StemHeight;

            Vector p6origin(l5_End.x + StemThickness / (number)2.0,
              l5_End.y);
            Vector p6scale(StemThickness,
              StemThickness * RelativeStemCapHeight);
            p6.Ellipse(p6origin, p6scale, (number)0.0,
              (prim::count)3, true);

            p7.Ellipse(p6origin, p6scale, (number)0.0,
              (prim::count)4, true);

            l8_Start = l5_End;
            l8_Start.x += StemThickness;
            l8_End = p1.Value((number)0.0);

            if(FlagPosition)
              *FlagPosition = l8_Start;
          }

          //Step 4 - Create a component path.
          p.AddComponent(l8_End);
          p.AddCurve(p1);
          p.AddCurve(p2);
          p.AddCurve(p3);
          p.AddCurve(p4);
          p.AddCurve(l5_End);
          p.AddCurve(p6);
          p.AddCurve(p7);
          p.AddCurve(l8_End);
        }
      }

      static void AddHalfNote(prim::Path& p,
        prim::math::Vector HeadOrigin,
        prim::number SpaceHeight = (prim::number)1.0,
        bool MakeSingleOutline = true,
        prim::number RelativeStemHeight = (prim::number)4.0,
        prim::math::Vector* FlagPosition = 0,
        prim::number HeadTheta = (prim::number)20*prim::math::DegToRad,
        prim::number RelativeWidth = (prim::number)1.4,
        prim::number RelativeStemThickness = 0.1,
        prim::number RelativeStemCapHeight = (prim::number)0.8,
        prim::number HollowScale = 0.48,
        prim::number HollowTheta =
          prim::math::HalfPi * (prim::number)0.4)
      {
        AddQuarterNote(p, HeadOrigin, SpaceHeight, MakeSingleOutline,
          RelativeStemHeight, FlagPosition, HeadTheta, RelativeWidth,
          RelativeStemThickness, RelativeStemCapHeight, true,
          HollowScale, HollowTheta);
      }

      ///Adds a convex slur to a path.
      static void AddSlur(prim::Path& p, prim::math::Vector a,
        prim::math::Vector b, prim::number inSpaceHeight,
        prim::number relativeArchHeight = 0.1f, 
        prim::number relativeArchWidth = 0.5f, 
        prim::number shMaxThickness = 0.2f,
        prim::number shMinThickness = 0.06f,
        prim::number relativeCapHeight = 2.0f)
      {
        using namespace prim;
        using namespace prim::math;

        number theta = a.Ang(b);
        number perpen = theta + math::HalfPi;
        number width = a.Mag(b);

        number shArchHeight = relativeArchHeight * (width / inSpaceHeight);
        if(shArchHeight > 2.5f)
          shArchHeight = 2.5f;
        if(shArchHeight < -2.5f)
          shArchHeight = -2.5f;

        shArchHeight *= inSpaceHeight;
        shMaxThickness *= inSpaceHeight;
        shMinThickness *= inSpaceHeight;

        Vector ghost_a1 = a;
        ghost_a1 += Vector(perpen) * shArchHeight;

        Vector c = ghost_a1;
        c += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector d = ghost_a1;
        d += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector ghost_a2 = a;
        ghost_a2 += Vector(perpen) * (shArchHeight + shMaxThickness);

        Vector e = ghost_a2;
        e += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector f = ghost_a2;
        f += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector g = a;
        g += Vector(a.Ang(c) + math::HalfPi) * shMinThickness;

        Vector h = b;
        h += Vector(b.Ang(d) - math::HalfPi) * shMinThickness;

        Vector i = (a + g) * 0.5f;
        Vector j = (b + h) * 0.5f;

        Bezier ellipse_bj;
        Bezier ellipse_jh;
        Bezier ellipse_gi;
        Bezier ellipse_ia;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 1, true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 2, true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 1, true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 2, true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0],bj[1],bj[2],bj[3]);
        ellipse_jh.GetControlPoints(jh[0],jh[1],jh[2],jh[3]);
        ellipse_gi.GetControlPoints(gi[0],gi[1],gi[2],gi[3]);
        ellipse_ia.GetControlPoints(ia[0],ia[1],ia[2],ia[3]);

        p.AddComponent(a);
        p.AddCurve(c, d, b);
        p.AddCurve(bj[1], bj[2], bj[3]);
        p.AddCurve(jh[1], jh[2], jh[3]);
        p.AddCurve(f, e, g);
        p.AddCurve(gi[1], gi[2], gi[3]);
        p.AddCurve(ia[1], ia[2], ia[3]);
      }

      struct SlurControlPoint
      {
        prim::math::Vector Start;
        prim::number degStartAngle;
        prim::number shStartDepth;
        prim::number shEndDepth;
        
        SlurControlPoint() : degStartAngle(0),
          shStartDepth(1.0f), shEndDepth(1.0f) {}

        SlurControlPoint(prim::math::Vector Start, prim::number degStartAngle,
          prim::number shStartDepth = 1.0f, prim::number shEndDepth = 1.0f)
        {
          SlurControlPoint::Start = Start;
          SlurControlPoint::degStartAngle = degStartAngle;
          SlurControlPoint::shStartDepth = shStartDepth;
          SlurControlPoint::shEndDepth = shEndDepth;
        }
      };

      static void AddSlur(prim::Path& p,
        prim::Array<SlurControlPoint>& ControlPoints,
        prim::number inSpaceHeight,
        prim::number shMaxThickness = 0.2f,
        prim::number shMinThickness = 0.06f,
        prim::number relativeCapHeight = 2.0f)
      {
        using namespace prim;
        using namespace prim::math;

        //Stop unreferenced formal parameter warning.
        inSpaceHeight += relativeCapHeight * 0.0f;

        //Make sure at least two control points are in the list.
        if(ControlPoints.n() < 2)
          return;

        //Begin the new path component.
        p.AddComponent();

        //Cached information
        Vector FirstPoint;

        //Trace upper part of slur hull.
        for(count i = 0; i < ControlPoints.n() - 1; i++)
        {
          SlurControlPoint a = ControlPoints[i];
          SlurControlPoint b = ControlPoints[i + 1];
          
          Vector a_Start = a.Start;
          Vector b_Start = b.Start;

          Vector a_Control = a_Start +
            Vector(a.degStartAngle, a.shStartDepth * inSpaceHeight, true);
          Vector b_Control = b_Start -
            Vector(b.degStartAngle, a.shEndDepth * inSpaceHeight, true);

          Vector p0, p1, p2, p3;
          p0 = a_Start + Vector(a.degStartAngle + 90.0f,
            shMinThickness * inSpaceHeight * 0.5f, true);
          p3 = b_Start + Vector(b.degStartAngle + 90.0f,
            shMinThickness * inSpaceHeight * 0.5f, true);

          p1 = a_Control + Vector(a.degStartAngle + 90.0f,
            shMaxThickness * inSpaceHeight * 0.5f, true);
          p2 = b_Control + Vector(b.degStartAngle + 90.0f,
            shMaxThickness * inSpaceHeight * 0.5f, true);

          if(i == 0)
          {
            FirstPoint = p0;
            p.AddCurve(p0);
          }

          p.AddCurve(p1, p2, p3);
        }

        //Add right end cap.
        /*
        Bezier ellipse_r1;
        Bezier ellipse_r2;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 1, true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 2, true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 1, true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 2, true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0],bj[1],bj[2],bj[3]);
        ellipse_jh.GetControlPoints(jh[0],jh[1],jh[2],jh[3]);
        ellipse_gi.GetControlPoints(gi[0],gi[1],gi[2],gi[3]);
        ellipse_ia.GetControlPoints(ia[0],ia[1],ia[2],ia[3]);

        p.AddComponent(a);
        p.AddCurve(c, d, b);
        p.AddCurve(bj[1], bj[2], bj[3]);
        p.AddCurve(jh[1], jh[2], jh[3]);
        */

        //Trace backwards on the lower part of the slur hull.
        for(count i = ControlPoints.n() - 2; i >= 0; i--)
        {
          SlurControlPoint a = ControlPoints[i];
          SlurControlPoint b = ControlPoints[i + 1];
          
          Vector a_Start = a.Start;
          Vector b_Start = b.Start;

          Vector a_Control = a_Start +
            Vector(a.degStartAngle, a.shStartDepth * inSpaceHeight, true);
          Vector b_Control = b_Start -
            Vector(b.degStartAngle, a.shEndDepth * inSpaceHeight, true);

          Vector p0, p1, p2, p3;
          p0 = a_Start - Vector(a.degStartAngle + 90.0f,
            shMinThickness * inSpaceHeight * 0.5f, true);
          p3 = b_Start - Vector(b.degStartAngle + 90.0f,
            shMinThickness * inSpaceHeight * 0.5f, true);

          p1 = a_Control - Vector(a.degStartAngle + 90.0f,
            shMaxThickness * inSpaceHeight * 0.5f, true);
          p2 = b_Control - Vector(b.degStartAngle + 90.0f,
            shMaxThickness * inSpaceHeight * 0.5f, true);

          if(i == ControlPoints.n() - 2)
            p.AddCurve(p3); //Temporary to create right cap.

          p.AddCurve(p2, p1, p0);
        }

        //Add left end cap.
        p.AddCurve(FirstPoint);
      }

 /*     ///Adds a convex slur to a path.
      static void AddPolygonSlur(prim::Path& p, prim::math::Vector a,
        prim::math::Vector b, prim::number inSpaceHeight,
        prim::number relativeArchHeight = 0.1f, 
        prim::number relativeArchWidth = 0.5f, 
        prim::number shMaxThickness = 0.2f,
        prim::number shMinThickness = 0.06f,
        prim::number relativeCapHeight = 2.0f)
      {
        using namespace prim;
        using namespace prim::math;

        number theta = a.Ang(b);
        number perpen = theta + math::HalfPi;
        number width = a.Mag(b);

        number shArchHeight = relativeArchHeight * (width / inSpaceHeight);
        if(shArchHeight > 2.5f)
          shArchHeight = 2.5f;
        if(shArchHeight < -2.5f)
          shArchHeight = -2.5f;

        shArchHeight *= inSpaceHeight;
        shMaxThickness *= inSpaceHeight;
        shMinThickness *= inSpaceHeight;

        Vector ghost_a1 = a;
        ghost_a1 += Vector(perpen) * shArchHeight;

        Vector c = ghost_a1;
        c += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector d = ghost_a1;
        d += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector ghost_a2 = a;
        ghost_a2 += Vector(perpen) * (shArchHeight + shMaxThickness);

        Vector e = ghost_a2;
        e += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector f = ghost_a2;
        f += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector g = a;
        g += Vector(a.Ang(c) + math::HalfPi) * shMinThickness;

        Vector h = b;
        h += Vector(b.Ang(d) - math::HalfPi) * shMinThickness;

        Vector i = (a + g) * 0.5f;
        Vector j = (b + h) * 0.5f;

        Bezier ellipse_bj;
        Bezier ellipse_jh;
        Bezier ellipse_gi;
        Bezier ellipse_ia;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 1, true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 2, true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 1, true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 2, true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0],bj[1],bj[2],bj[3]);
        ellipse_jh.GetControlPoints(jh[0],jh[1],jh[2],jh[3]);
        ellipse_gi.GetControlPoints(gi[0],gi[1],gi[2],gi[3]);
        ellipse_ia.GetControlPoints(ia[0],ia[1],ia[2],ia[3]);

        p.AddComponent(a);
        p.AddCurve(c, d, b);
        p.AddCurve(bj[1], bj[2], bj[3]);
        p.AddCurve(jh[1], jh[2], jh[3]);
        p.AddCurve(f, e, g);
        p.AddCurve(gi[1], gi[2], gi[3]);
        p.AddCurve(ia[1], ia[2], ia[3]);
      }*/

      ///Adds a grand staff brace to a path.
      static void AddBrace(prim::Path& p,
        prim::math::Vector Center,
        prim::number Height,
        prim::number RelativeWidth = (prim::number)0.1)
      {
        /*We're using a set of control points from a quadratic Bezier 
        curve taken from a font program, due to the complexity of the 
        shape.*/
        using namespace prim;
        using namespace prim::math;

        Vector v[44];

        //Even are anchors, odd are control points.
        v[0] =Vector(12.5f ,  12.5f);
        v[1] =Vector(25.0f ,  25.0f);
        v[2] =Vector(87.5f ,  87.5f);
        v[3] =Vector(150.0f,  150.0f);
        v[4] =Vector(100.0f,  325.0f);
        v[5] =Vector(50.0f ,  500.0f);
        v[6] =Vector(25.0f ,  650.0f);
        v[7] =Vector(0.0f  ,  800.0f);
        v[8] =Vector(75.0f ,  925.0f);
        v[9] =Vector(150.0f,  1050.0f);
        v[10]=Vector(175.0f,  1025.0f);
        v[11]=Vector(200.0f,  1000.0f);
        v[12]=Vector(125.0f,  925.0f);
        v[13]=Vector(50.0f ,  850.0f);
        v[14]=Vector(100.0f,  675.0f);
        v[15]=Vector(150.0f,  500.0f);
        v[16]=Vector(175.0f,  350.0f);
        v[17]=Vector(200.0f,  200.0f);
        v[18]=Vector(175.0f,  125.0f);
        v[19]=Vector(150.0f,  50.0f);
        v[20]=Vector(125.0f,  25.0f);
        v[21]=Vector(100.0f,  0.0f);
        v[22]=Vector(125.0f, -25.0f);
        v[23]=Vector(150.0f, -50.0f);
        v[24]=Vector(175.0f, -125.0f);
        v[25]=Vector(200.0f, -200.0f);
        v[26]=Vector(175.0f, -350.0f);
        v[27]=Vector(150.0f, -500.0f);
        v[28]=Vector(100.0f, -675.0f);
        v[29]=Vector(50.0f , -850.0f);
        v[30]=Vector(125.0f, -925.0f);
        v[31]=Vector(200.0f, -1000.0f);
        v[32]=Vector(175.0f, -1025.0f);
        v[33]=Vector(150.0f, -1050.0f);
        v[34]=Vector(75.0f , -925.0f);
        v[35]=Vector(0.0f  , -800.0f);
        v[36]=Vector(25.0f , -650.0f);
        v[37]=Vector(50.0f , -500.0f);
        v[38]=Vector(100.0f, -325.0f);
        v[39]=Vector(150.0f, -150.0f);
        v[40]=Vector(87.5f , -87.5f);
        v[41]=Vector(25.0f , -25.0f);
        v[42]=Vector(12.5f , -12.5f);
        v[43]=Vector(0.0f  ,  0.0f);

        //Transform into context space.
        for(count i = 0; i < 44; i++)
        {
          number mult  = Height / (number)2000.0 * (number)0.95;
          number multx = RelativeWidth / (number)0.1;
          v[i]   *= mult;
          v[i].x *= multx;
          v[i] += Center;
        }

        //Create the curve.
        p.AddComponent(v[0]);
        for(count i = 0; i <= 42; i+= 2)
        {
          Bezier b;
          Vector p0 = v[i];
          Vector p1 = v[i + 1];
          Vector p2 = v[Mod(i + 2, 44)];
          b.SetControlPoints(p0, p1, p2);
          p.AddCurve(b);
        }
      }
    };
  };
}
#endif
