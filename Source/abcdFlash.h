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

#ifndef abcdFlash
#define abcdFlash

///Include the ActionScript 3.0 API.
#include "AS3.h"

//Tracing function.
extern "C" { void sztrace(char*); }

/**Contains supplied representations of the abstract classes. You can derive
these or create your own. The acronym stands for ABraCaDabra, or,
Another BBS Context Definition.*/
namespace abcd
{
  ///A simple Flash player painter.
  class Flash : public bbs::abstracts::Painter
  {
    friend class bbs::abstracts::Portfolio;
    
  public:
    ///Redirects trace output to /tmp/adl.trace when run under swfbridge.
    class Trace : public Stream
    {
    protected:
      ///Method to output incoming stream data to the console window.
      virtual void Out(const ascii* s)
      {
        sztrace(const_cast<char*>(s));
      }

    public:
      ///The virtual destructor
      virtual ~Trace() {};
    };

    ///Supplies the Flash class with additional Flash-specific information.
    struct Properties : public bbs::abstracts::Painter::Properties
    {
      ///The dimensions of the Flash stage.
      prim::math::Pixels Dimensions;
      
      ///The frame rate of the Flash player.
      prim::number FramesPerSecond;
      
      /**A dynamically updated value indicating the index of the current frame.
      This value begins at zero for the first time, and increases by one each
      time a frame is rendered.*/
      prim::count CurrentFrame;
      
      ///A dynamically updated value indicating the current time in seconds.
      prim::number CurrentTime;
      
      ///Holds the current data passed in by the Flash script.
      prim::String CurrentData;
      
      ///Current graphics context.
      AS3_Val GraphicsObject;
      
      Properties() : FramesPerSecond(30.0f), CurrentFrame(0), CurrentTime(0),
        GraphicsObject(0) {}
    };

protected:
    ///Properties of the Flash player.
    Properties* FlashProperties;
    
    ///Portfolio to paint.
    bbs::abstracts::Portfolio* Stage;
    
    ///Current fill color (upper 8 bits contain alpha).
    prim::colors::RGB ColorWithAlpha;
    
public:
    ///Default constructor for the Flash painter
    Flash() : FlashProperties(0), Stage(0), ColorWithAlpha(0xff000000) {}
    
protected:
    //---------//
    //Callbacks//
    //---------//
    
    static AS3_Val CallbackPaint(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);

      //Paint the canvases and each of their layers.
      for(prim::count i = 0; i < p->Stage->Canvases.n(); i++)
      {
        Portfolio::Canvas* c = p->Stage->Canvases[i];
        c->Paint(p);
        for(prim::count j = 0; j < c->Layers.n(); j++)
          c->Layers[j]->Paint(p);
      }
      
      return AS3_Null();
    }
    
    ///Callback to get the width of the stage.
    static AS3_Val CallbackGetWidth(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      return AS3_Number((double)p->FlashProperties->Dimensions.x);
    }
    
    ///Callback to get the height of the stage.
    static AS3_Val CallbackGetHeight(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      return AS3_Number((double)p->FlashProperties->Dimensions.y);
    }
    
    ///Callback to get the number of frames per second.
    static AS3_Val CallbackGetFramesPerSecond(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      return AS3_Number((double)p->FlashProperties->FramesPerSecond);
    }
    
    ///Callback to set the index of the current frame.
    static AS3_Val CallbackSetCurrentFrame(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      //Get the value passed by the caller.
      int CurrentFrameValue = 0;
      AS3_ArrayValue(Arguments, "IntType", &CurrentFrameValue);
      p->FlashProperties->CurrentFrame = (prim::count)CurrentFrameValue;
      
      return AS3_Null();
    }
    
    ///Callback to set the time of the current frame.
    static AS3_Val CallbackSetCurrentTime(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      //Get the value passed by the caller.
      double CurrentTimeValue = 0;
      AS3_ArrayValue(Arguments, "DoubleType", &CurrentTimeValue);
      p->FlashProperties->CurrentTime = (prim::number)CurrentTimeValue;
      
      return AS3_Null();
    }
    
    ///Callback to set the graphics context.
    static AS3_Val CallbackSetGraphics(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      //Get the value passed by the caller.
      AS3_ArrayValue(Arguments, "AS3ValType",
        &p->FlashProperties->GraphicsObject);
      
      return AS3_Null();
    }
    
    ///Callback to set the graphics context.
    static AS3_Val CallbackSetData(void* Painter, AS3_Val Arguments)
    {
      //Get a handle to the current painter.
      Flash* p = reinterpret_cast<Flash*>(Painter);
      
      //Get the value passed by the caller.
      char* DataString = 0;
      AS3_ArrayValue(Arguments, "StrType", &DataString);
      p->FlashProperties->CurrentData &= DataString;
      
      return AS3_Null();
    }

    ///This method is called within Flash and never returns.
    virtual void Paint(bbs::abstracts::Portfolio* PortfolioToPaint,
      bbs::abstracts::Painter::Properties* PortfolioProperties)
    {
      //Include the prim namespace.
      using namespace prim;
      using namespace prim::math;
      using namespace bbs::abstracts;

      //Ensure that we have a valid Properties object pointer.
      FlashProperties = PortfolioProperties->Interface<Properties>();
      if(!FlashProperties)
        return;
      
      //Set the stage so this object can be referenced during static callbacks.
      if((Stage = PortfolioToPaint) == 0)
        return;
      
      //Create ActionScript function callbacks.
      AS3_Val PaintMethod =
        AS3_Function((void*)this, CallbackPaint);
      AS3_Val GetWidthMethod =
        AS3_Function((void*)this, CallbackGetWidth);
      AS3_Val GetHeightMethod =
        AS3_Function((void*)this, CallbackGetHeight);
      AS3_Val GetFramesPerSecondMethod =
        AS3_Function((void*)this, CallbackGetFramesPerSecond);
      AS3_Val SetCurrentFrameMethod =
        AS3_Function((void*)this, CallbackSetCurrentFrame);
      AS3_Val SetCurrentTimeMethod =
        AS3_Function((void*)this, CallbackSetCurrentTime);
      AS3_Val SetGraphicsMethod =
        AS3_Function((void*)this, CallbackSetGraphics);
      AS3_Val SetDataMethod =
        AS3_Function((void*)this, CallbackSetData);
        
      //Create an ActionScript key-value object containing the functions.
      AS3_Val Functions = AS3_Object(
        "Paint: AS3ValType,"
        "GetPixelWidth: AS3ValType,"
        "GetPixelHeight: AS3ValType,"
        "GetFramesPerSecond: AS3ValType,"
        "SetCurrentFrame: AS3ValType,"
        "SetCurrentTime: AS3ValType,"
        "SetGraphics: AS3ValType,"
        "SetData: AS3ValType", //Be careful about strings and commas here!
        PaintMethod,
        GetWidthMethod,
        GetHeightMethod,
        GetFramesPerSecondMethod,
        SetCurrentFrameMethod,
        SetCurrentTimeMethod,
        SetGraphicsMethod,
        SetDataMethod);
      
      //Release the function callbacks that were temporarily created.
      AS3_Release(PaintMethod);
      AS3_Release(GetWidthMethod);
      AS3_Release(GetHeightMethod);
      AS3_Release(GetFramesPerSecondMethod);
      AS3_Release(SetCurrentFrameMethod);
      AS3_Release(SetCurrentTimeMethod);
      AS3_Release(SetGraphicsMethod);
      AS3_Release(SetDataMethod);
      
      //Initialize the library--this function does not return.
      AS3_LibInit(Functions);
    }

    //---------------//
    //Drawing Methods//
    //---------------//

    //--------------//
    //Transformation//
    //--------------//

    virtual void Transform(const prim::math::StateMatrix<RasterState>& m)
    {
      using namespace prim;
      using namespace prim::math;
    }

    virtual void UndoTransformation(prim::count NumberToUndo = 1)
    {
      using namespace prim;
      using namespace prim::math;
    }

    //-------------//
    //Raster States//
    //-------------//

    virtual void StrokeColor(prim::colors::RGB Color)
    {
      using namespace prim::colors;
      
      bbs::abstracts::Painter::StrokeColor(Color);
    }

    virtual void FillColor(prim::colors::RGB Color)
    {
      using namespace prim::colors;

      bbs::abstracts::Painter::FillColor(Color);
      ColorWithAlpha = Color;
    }
    
    private:
    
    //--------------------------//
    //Bezier Curve Approximation//
    //--------------------------//
    
    /*-----------------------------------------------------------------------*\
    | Approximate cubic curves with quadratic curves based on a               |
    | simplified version of the MidPoint algorithm by Helen Triolo            |
    |                                                                         |
    | Retrieved June 7, 2010 from:                                            |
    | http://www.timotheegroleau.com/Flash/articles/cubic_bezier_in_flash.htm |
    |                                                                         |
    | By Timoethee Groleau                                                    |
    | Bezier_lib.as - v1.2 - May 19, 2002                                     |
    | http://www.timoetheegroleau.com                                         |
    | "Feel free to use and modify the library file as much as you want."     |
    |                                                                         |
    | Assuming from author's release statement that code is LGPL licensable   |
    \*-----------------------------------------------------------------------*/
    
    ///Return the middle of a segment defined by two points.
    prim::math::Vector GetMiddle(prim::math::Vector p0, prim::math::Vector p1)
    {
      return prim::math::Vector((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
    }
    
    /**Return a point on a segment [p0, p1] whose distance from p0 is a ratio of
    the length [p0, p1].*/
    prim::math::Vector GetPointOnSegment(prim::math::Vector p0,
      prim::math::Vector p1, prim::number DistanceRatio)
    {
      return prim::math::Vector(
        p0.x + (p1.x - p0.x) * DistanceRatio,
        p0.y + (p1.y - p0.y) * DistanceRatio);
    }
    
    /**Takes four cubic p-control points and returns eight quadratic
    control points in the q-array.*/
    void ApproximateCubicBezier(
      prim::math::Vector p0,
      prim::math::Vector p1,
      prim::math::Vector p2,
      prim::math::Vector p3,
      prim::Array<prim::math::Vector>& q)
    {
      using namespace prim;
      using namespace prim::math;
      
      static const number _3_4  = 3.0f / 4.0f;
      static const number _3_8  = 3.0f / 8.0f;
      static const number _1_16 = 1.0f / 16.0f;
      
      //Calculates the useful base points.
      Vector PA = GetPointOnSegment(p0, p1, _3_4);
      Vector PB = GetPointOnSegment(p3, p2, _3_4);
      
      //Get 1/16 of the [P3, P0] segment.
      number dx = (p3.x - p0.x) * _1_16;
      number dy = (p3.y - p0.y) * _1_16;
      
      //Calculates control point 1.
      Vector Pc_1 = GetPointOnSegment(p0, p1, _3_8);
      
      //Calculates control point 2.
      Vector Pc_2 = GetPointOnSegment(PA, PB, _3_8);
      Pc_2.x -= dx;
      Pc_2.y -= dy;
      
      //Calculates control point 3.
      Vector Pc_3 = GetPointOnSegment(PB, PA, _3_8);
      Pc_3.x += dx;
      Pc_3.y += dy;
      
      //Calculates control point 4.
      Vector Pc_4 = GetPointOnSegment(p3, p2, _3_8);
      
      //Calculates the 3 anchor points.
      Vector Pa_1 = GetMiddle(Pc_1, Pc_2);
      Vector Pa_2 = GetMiddle(PA, PB);
      Vector Pa_3 = GetMiddle(Pc_3, Pc_4);

      //Store the four quadratic subsegments.
      q.n(8);
      q[0] = Pc_1;
      q[1] = Pa_1;
      q[2] = Pc_2;
      q[3] = Pa_2;
      q[4] = Pc_3;
      q[5] = Pa_3;
      q[6] = Pc_4;
      q[7] = p3;
    }
    
    protected:
    
    //-----//
    //Paths//
    //-----//
    virtual void Draw(prim::Path& p, prim::number StrokeWidth = 0.0f,
      prim::count ContextIndex = -1)
    {
      using namespace prim;
      using namespace prim::math;

      if(StrokeWidth != 0.f)
      {
        //Abs(StrokeWidth)
      }

      //beginFill: color
      int color = (int)(ColorWithAlpha & 0xffffff);
      double alpha = (number)((ColorWithAlpha & 0xff000000) >> 24) / 255.0f;
      AS3_Val colorParameters = AS3_Array("IntType, DoubleType", color, alpha);
      AS3_CallS("beginFill", FlashProperties->GraphicsObject, colorParameters);
      AS3_Release(colorParameters);

      Vector PreviousPoint;
      
      double h = FlashProperties->Dimensions.y;
      
      for(count i = 0; i < p.Components.n(); i++)
      {
        Path::Component& comp = p.Components[i];
        Path::Curve& start = comp.Curves[0];
        
        //moveTo: start.End.x, start.End.y;
        double mx = start.End.x, my = h - start.End.y;
        AS3_Val mParameters = AS3_Array("DoubleType, DoubleType", mx, my);
        AS3_CallS("moveTo", FlashProperties->GraphicsObject, mParameters);
        AS3_Release(mParameters);
        PreviousPoint = start.End;
        
        for(count j = 1; j < comp.Curves.n(); j++)
        {
          Path::Curve& curv = comp.Curves[j];
          if(!curv.IsCurve)
          {
            //lineTo: curv.End.x, curv.End.y
            double lx = curv.End.x, ly = h - curv.End.y;
            AS3_Val lParameters = AS3_Array("DoubleType, DoubleType", lx, ly);
            AS3_CallS("lineTo", FlashProperties->GraphicsObject, lParameters);
            AS3_Release(lParameters);
            PreviousPoint = curv.End;
          }
          else
          {
            //curveto: curv.StartControl.x, curv.StartControl.y
            //curv.EndControl.x, curv.EndControl.y
            //curv.End.x, curv.End.y
            
            //Approximate cubic Bezier since Flash only has quadratic curves.
            Array<Vector> q;
            ApproximateCubicBezier(PreviousPoint, curv.StartControl,
              curv.EndControl, curv.End, q);
            
            //Draw each of the four quadratic curves.
            for(count k = 0; k < 8; k += 2)
            {
              double c0x = q[k].x, c0y = h - q[k].y,
                c1x = q[k + 1].x, c1y = h - q[k + 1].y;

              AS3_Val cParameters = AS3_Array(
                "DoubleType, DoubleType, DoubleType, DoubleType",
                c0x, c0y, c1x, c1y);
                
              AS3_CallS("curveTo", FlashProperties->GraphicsObject, cParameters);
              AS3_Release(cParameters);
            }
            PreviousPoint = curv.End;
          }
        }
      }
      
      //endFill:
      AS3_Val endfillParameters = AS3_Array("");
      AS3_CallS("endFill", FlashProperties->GraphicsObject, endfillParameters);
      AS3_Release(endfillParameters);

      /*//Stroking and Filling
      if(StrokeWidth > 0)
        t += "S"; //Stroke only.
      else if(StrokeWidth == 0)
        t += "f"; //Fill only.
      else if(StrokeWidth < 0)
        t += "B"; //Fill and stroke.
      else
        t += "n"; //"No-op"
      */
      

      /*//Contexts
      count numContexts = p.Contexts.n();
      if(numContexts)
      {
        for(count i = 0; i < numContexts; i++)
        {
          if(ContextIndex == -1 || i == ContextIndex)
          {
            const prim::math::StateMatrix<RasterState>& m = p.Contexts[i];
            Transform(m);
            Rasterize(t);
            UndoTransformation();
          }
        }
      }
      else
        Rasterize(t);
      */
      
      //Draw a rectangle.
      /*double x = FlashProperties->CurrentFrame, y = x, w = 40, h = 40;
      AS3_Val Parameters = AS3_Array(
        "DoubleType, DoubleType, DoubleType, DoubleType", x, y, w, h);
      AS3_CallS("drawRect", GraphicsObject, Parameters);
      AS3_Release(Parameters);*/
    }

    virtual void DrawPath(prim::Path& p,
      bool Stroke = true, bool Fill = false, bool ClosePath = false,
      prim::number StrokeWidth = 0.0f)
    {
      using namespace prim;
      using namespace prim::math;
      StrokeWidth = Abs(StrokeWidth);
      if(!Stroke)
        StrokeWidth = 0.0f;
      if(Fill)
        StrokeWidth = -StrokeWidth;
      DrawPath(p, StrokeWidth);
    }
  };
}

#endif
