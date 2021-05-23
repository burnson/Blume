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

#ifndef bbsAbstracts
#define bbsAbstracts

#include "prim.h"

#include "bbsFont.h"
#include "bbsGlyph.h"
#include "bbsShapes.h"
#include "bbsText.h"

/**\brief Contains the abstract structures as well as helpers
which are particularly suited to the rendering of music symbols.*/
namespace bbs
{
  namespace abstracts
  {
    //Forward declarations
    class Portfolio;

    class Painter
    {
    public:
      /**\brief A structure containing various generic properties of the
      current raster engine state such as what color to fill with next.*/
      struct RasterState
      {
        prim::colors::RGB StrokeColor;
        prim::colors::RGB FillColor;
        
        RasterState() : 
          StrokeColor(prim::colors::Black),
          FillColor(prim::colors::Black) {}
      };

      /**\brief The transformation space and raster state stack, which
      keeps track of all transformations and raster state changes.**/
      prim::math::AffineTransform<RasterState> State;
    public:
      /**A Painter may have Properties associated with it. These comprise
      details that might be useful for the Painter device to know
      immediately, such as the name of a file, or how much compression to
      use, etc. The properties may be optional or required. If they are
      required, simply overload only the Paint method which uses them and
      leave out the other.*/
      class Properties
      {
      public:
        ///Constructor
        Properties(){}

        ///Virtual destructor so that memory is freed correctly.
        virtual ~Properties(){}

        /**\brief Returns a pointer to the requested derived class of
        this object if it exists.*/
        template <typename DerivedProperties>
        DerivedProperties* Interface(void)
        {
          return dynamic_cast<DerivedProperties*>(this);
        }
      };
    public:
      ///Constructor
      Painter(){}

      ///Destructor
      virtual ~Painter()
      {
        if(State.States()!=1)
        {
          prim::Console c;
          c += "Stack incorrectly collapsed; did you use UndoTransformation()?";
        }
      }

      /**\brief Allows the calling Portfolio object to determine whether
      an interface is of a particular derived class. \details Usually,
      information that is relevant to particular Painter should reside in
      its derived Properties class (filenames versus device contexts for
      screens).*/
      template <typename SpecificPainter> SpecificPainter* Interface(void)
      {
        return dynamic_cast<SpecificPainter*>(this);
      }

    protected:
      /**\brief When Painter is derived, the Paint() method is the entry
      point to the device-dependent specialization. \details This method
      is not called directly, rather the Portfolio's Create method will
      call this automatically. This method is protected to prevent the
      user from accidentally trying to call the Paint method without using
      the Create method in Portfolio.*/
      virtual void Paint(Portfolio* PortfolioToPaint,
        Properties* PortfolioProperties){PortfolioToPaint=0;
      PortfolioProperties=0;}

      //Friendship with Portfolio, so that it can call the Paint method.
      friend class Portfolio;

    public:
      //---------------//
      //DRAWING METHODS//
      //---------------//

      //Transformation
      virtual void Transform(const prim::math::StateMatrix<RasterState>& m)
      {
        m.State = State.TopState();
        State.Push(m);
      }

      virtual void UndoTransformation(prim::count NumberToUndo = 1)
      {
        for(prim::count i = 0; i < NumberToUndo; i++)
          State.Pop();
      }

      virtual void Translate(prim::math::Vector TranslateBy)
      {
        Transform(
          prim::math::StateMatrix<RasterState>::Translation(TranslateBy));
      }

      virtual void Scale(prim::number ScaleBy)
      {
        Transform(prim::math::StateMatrix<RasterState>::Scale(ScaleBy));
      }

      virtual void Scale(prim::math::Vector ScaleBy)
      {
        Transform(prim::math::StateMatrix<RasterState>::Scale(ScaleBy));
      }

      virtual void Rotate(prim::number RotateBy)
      {
        Transform(prim::math::StateMatrix<RasterState>::Rotation(RotateBy));
      }

      //Raster States
      virtual void StrokeColor(prim::colors::RGB Color)
      {
        RasterState& m = State.TopState();
        m.StrokeColor = Color;
      }

      virtual void FillColor(prim::colors::RGB Color)
      {
        RasterState& m = State.TopState();
        m.FillColor = Color;
      }

      //-----//
      //Paths//
      //-----//
      
      /**Draws one or all of a path's contexts. If the stroke width is greater
      than 0, the path is stroked. If the stroke width is exactly zero, the path
      is filled. If the stroke width is negative, the path is filled and then
      stroked with a thickness of negative the stroke width. By default, the
      context index is set to -1 to indicate that all contexts in the path will
      be drawn. Setting a valid context index will cause just that context to be
      drawn. If the path has no contexts, then the raw path data will be drawn
      instead (same as drawing a path with a single identity context).*/
      virtual void Draw(prim::Path& p, prim::number StrokeWidth = 0.0f,
        prim::count ContextIndex = -1)
      {
        p.Components.n();
        StrokeWidth = 0.0f;
        ContextIndex = -1;
      }
      
      virtual void DrawPath(prim::Path& p,
        bool Stroke=true, bool Fill=false, bool ClosePath=false,
        prim::number StrokeWidth=0.0) = 0;

      virtual void DrawStrokedLine(prim::math::Vector p1, prim::math::Vector p2,
        prim::number StrokeWidth = 0)
      {
        prim::Path p;
        p.AddComponent(p1);
        p.AddCurve(p2);
        DrawPath(p, true, false, false, StrokeWidth);
      }

      virtual void DrawLine(prim::math::Vector p1, prim::math::Vector p2,
        prim::number StrokeWidth = 0)
      {
        prim::Path p;
        bbs::Shapes::AddLine(p, p1, p2, StrokeWidth, true, false, false);
        DrawPath(p, false, true, true);
      }

      //Text
      virtual prim::math::Rectangle DrawVectorText(prim::String& Text, 
        bbs::Text::Unformatted& Style, bool OnlyReturnBoundingBox = false);

      virtual prim::number DrawVectorText(
        bbs::VectorFont* FontToUse,
        prim::String Text,
        prim::math::Vector TranslateBy,
        prim::number ScaleBy = 1.0f,
        prim::Justification JustificationType = prim::Justifications::Left,
        prim::number WrapWidth = 0.0f,
        bool ReturnWidthOnly = false,
        bool DisableAutoLigatures = false);

      virtual prim::number DrawText(
        bbs::Font* FontToUse,
        prim::String Text,
        prim::math::Vector Position,
        prim::number Scale=(prim::number)1.0,
        prim::Justification JustificationType =
          prim::Justifications::Left,
        prim::number WrapWidth=0.0, bool ReturnWidthOnly=false)
      {
        //Stop unreferenced formal parameter warnings.
        FontToUse = 0;
        Text = "";
        Position = prim::math::Vector();
        Scale = (prim::number)0.0;
        JustificationType = prim::Justifications::Left;
        WrapWidth = (prim::number)0.0;
        ReturnWidthOnly = false;
        return (prim::number)0.0;
      }

      virtual prim::number DrawSymbol(bbs::VectorFont* FontToUse,
        prim::unicode::UCS4 Symbol,
        prim::math::Vector Position,
        prim::number Scale=1.0,
        prim::Justification JustificationType =
          prim::Justifications::Left,
        prim::number WrapWidth=0.0, bool ReturnWidthOnly=false)
      {
        prim::String SymbolText;
        SymbolText.Append(Symbol);
        return DrawVectorText(FontToUse,SymbolText,Position,Scale,
          JustificationType,WrapWidth,ReturnWidthOnly);
      }
      
      //------//
      //Images//
      //------//
      virtual void DrawJPEG(prim::String Filename, prim::number Width = 1.0f,
        prim::number Height = 0.0f) {Filename = ""; Width = Height = 0.0f;}
    };

    /**\brief The Portfolio class is an abstract representation of a
    collection of a group of Canvas elements, akin to a musical score which
    contains pages of music. \details It is the highest level of notational
    representation in the library, from which all other representations
    derive. The fundamental perk of Belle, Bonne, Sage is that you can
    derive different notation schemes from Portfolio using the base class
    Painter to do all of the drawing. You can then choose an arbitrary
    output by deriving a Painter. Derivations of these classes are to be
    assigned to the abcd namespace ("another bbs context definition", or its
    pet name "abracadabra"). Therefore, notation is never dependent on its
    final representation or vice-versa. This library is dedicated to Gardner
    Read, who once said contrary to the widely held belief: "Music is not
    universal." It is to this seemingly paradoxical statement, that this
    library owes its intention. One can never hope to create a notation (or
    music for that matter) that suits everyone, but one can create a tool
    for allowing new notations to be easily incorporated with existing
    ones.*/
    class Portfolio
    {
    public:
      //Forward declarations
      class Canvas;

      ///A list of canvases which this portfolio comprises.
      prim::List<Canvas*> Canvases;

      /**\brief The magical Create method which takes no arguments, merely
      a templated type, and manifests the portfolio in the 'painted' form.
      \details It requests the Painter to paint, which in the process of
      doing so calls the Paint method in the Canvas. Since all these
      methods are virtual, the nitty-gritty becomes completely
      transparent. If you need to supply a Properties object to the
      Painter, then call this method and pass in the relevant Properties
      object.*/
      template <typename PainterType> void
        Create(Painter::Properties* Properties=0)
      {
        PainterType* PainterSpecificPtr = new PainterType;
        PainterSpecificPtr->Paint(this, Properties);
        delete PainterSpecificPtr;
      }

      ///Default constructor
      Portfolio(){}

      ///Virtual destructor so that memory is freed correctly.
      virtual ~Portfolio(){}
    public:
      class Canvas
      {
      public:
        ///Dimensions of the canvas in inches
        prim::math::Points Dimensions;

        //Forward declaration
        class Layer;

        ///A list of canvases which this portfolio comprises.
        prim::List<Layer*> Layers;

        ///Constructor
        Canvas()
        {
          //Set the default page dimensions to US Letter 8.5 x 11 in.
          using namespace prim;
          Dimensions = math::Inches((number)8.5,(number)11);
        }

        ///Virtual destructor so that memory is freed correctly.
        virtual ~Canvas(){}

        /**\brief Returns a pointer to the requested derived class of
        this object if it exists.*/
        template <typename DerivedCanvas>
        DerivedCanvas* Interface(void)
        {
          return dynamic_cast<DerivedCanvas*>(this);
        }

        /**The entry point for painting within the Canvas. The single
        argument is a pointer to an abstract painter. By definition all
        notation representations should be able to use the generic
        Painter class to commit all drawing operations, though it is
        possible to glean information about the specific Painter via the
        Painter::Interface method.*/
        virtual void Paint(Painter* Painter){Painter=0;}

        /**\brief The Layer class may be implemented optionally and it
        serves to provide another level of drawing abstraction. \details
        For example, a score might have the music done in the first
        layer and then annotations done on the second layer. In terms of
        drawing order the default Canvas layer is always drawn first,
        and then the layers inside here are drawn on top.*/
        class Layer
        {
        public:
          ///Constructor
          Layer() {}

          ///Virtual destructor so that memory is freed correctly.
          virtual ~Layer(){}

          /**\brief The same as the Canvas::Paint method, except that
          the Layer paint methods are always called after the Canvas
          has painted, so that the Layers paint on top of whatever was
          committed by the Canvas.*/
          virtual void Paint(Painter* Painter){Painter=0;}
        };
      };
    };
  }
}

#endif
