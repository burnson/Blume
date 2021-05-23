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

#ifndef abcdSimulation
#define abcdSimulation

#include "bbs.h"

namespace abcd
{
  /**The Simulation painter is a Painter accepting input but yielding no output.
  The purpose of Simulation is to be able to retrieve what would be the results 
  of a drawing command very quickly without actually performing the drawing
  operation. This is useful especially in complex drawing commands such as text
  operations which may return the width or bounding box of the drawn text. For
  example, the bounding box of a multiline text drawing operation can not be 
  predicted without doing the calculations based on the advance widths, kerning, 
  line width, etc. Since these calculations constitute a majority of that 
  operation's code, it is better to simply indicate to the operation that at the
  end of the calculations, the drawing will not commence. The Simulation painter 
  ensures that the calculations are performed accurately without having to go 
  through the actual drawing. The nice thing about Simulation is that you do not
  have to create it using Portfolio::Create<Simulation>. Since it has no output, 
  you can safely create your own Simulation object and call its methods. You 
  could also in theory substitute Simulation for one of the other Painter
  classes if you wanted to do a dry run to test how quickly the non-drawing
  portion of your application is working.*/
  struct Simulation : public bbs::abstracts::Painter
  {
    //-----------//
    //Paint Event//
    //-----------//
    virtual void Paint(bbs::abstracts::Portfolio* PortfolioToPaint,
      bbs::abstracts::Painter::Properties* PortfolioProperties)
    {
      using namespace prim;
      PortfolioProperties = 0; //Stop unused parameter warning.
      for(count i = 0; i < PortfolioToPaint->Canvases.n(); i++)
        PortfolioToPaint->Canvases[i]->Paint(this);
    }

    //--------------//
    //Transformation//
    //--------------//
    virtual void Transform(const prim::math::StateMatrix<RasterState>& m)
    {
      //Stop unused parameter warning.
      prim::math::AffineTransform<RasterState> at;
      at.Push(m);
    }
    
    virtual void UndoTransformation(prim::count NumberToUndo = 1)
    {
      NumberToUndo = 0; //Stop unused parameter warning.
    }

    //-------------//
    //Raster States//
    //-------------//
    virtual void StrokeColor(prim::colors::RGB Color)
    {
      Color = 0; //Stop unused parameter warning.
    }
    
    virtual void FillColor(prim::colors::RGB Color)
    {
      Color = 0; //Stop unused parameter warning.
    }

    //-----//
    //Paths//
    //-----//
    virtual void DrawPath(prim::Path& p,
      bool Stroke = true, bool Fill = false, bool ClosePath = false,
      prim::number StrokeWidth = 0.0)
    {
      p.Components.n();
      Stroke = false;
      Fill = false;
      ClosePath = false;
      StrokeWidth = (prim::number)0.0;
    }

    virtual void DrawLine(prim::math::Vector p1, prim::math::Vector p2,
      prim::number StrokeWidth = 0)
    {
      //Stop unused parameter warnings.
      p1 = prim::math::Vector();
      p2 = prim::math::Vector();
      StrokeWidth = (prim::number)0.0;
    }

    //----//
    //Text//
    //----//
    virtual prim::number DrawText(bbs::Font* FontToUse,  prim::String Text,
      prim::math::Vector Position, prim::number Scale = 1.0, 
      prim::Justification JustificationType =  prim::Justifications::Left,
      prim::number WrapWidth = 0.0, bool ReturnWidthOnly = false)
    {
      Position = prim::math::Vector();
      JustificationType = prim::Justifications::Left;
      WrapWidth = (prim::number)0.0;
      ReturnWidthOnly = false;
      
      if(FontToUse)
        return FontToUse->TextWidth(Text, Scale);
      else
        return 0;
    }

    virtual prim::math::Rectangle DrawVectorText(prim::String& Text, 
      bbs::Text::Unformatted& Style, bool OnlyReturnBoundingBox = false)
    {
      OnlyReturnBoundingBox = false;
      return bbs::abstracts::Painter::DrawVectorText(Text, Style, true);
    }

    virtual prim::number DrawVectorText(
      bbs::VectorFont* FontToUse,
      prim::String Text,
      prim::math::Vector TranslateBy,
      prim::number ScaleBy = 1.0f,
      prim::Justification JustificationType = prim::Justifications::Left,
      prim::number WrapWidth = 0.0f,
      bool ReturnWidthOnly = false,
      bool DisableAutoLigatures = false)
    {
      return bbs::abstracts::Painter::DrawVectorText(FontToUse, Text, 
        TranslateBy, ScaleBy, JustificationType, WrapWidth, ReturnWidthOnly,
        DisableAutoLigatures);
    }
  };
}
#endif
