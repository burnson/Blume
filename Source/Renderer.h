/*
 ==============================================================================
 
 This file is part of Blume
 Copyright 2010 Andi Brae
 
 ------------------------------------------------------------------------------
 
 Blume can be redistributed and/or modified under the terms of the
 GNU General Public License, as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.
 
 Blume is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with Blume; if not, visit www.gnu.org/licenses or write to
 the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA
 
 ==============================================================================
 */

#ifndef Renderer_h
#define Renderer_h

#include "Libraries.h"

/*The Renderer.h and .cpp files contain an implementation of BBS using JUCE 
components as the target device. In just 150 lines of code, we can neatly map 
the abstract BBS graphics calls to the JUCE graphics engine.*/
struct Renderer : public bbs::abstracts::Painter
{
  struct Properties : public bbs::abstracts::Painter::Properties
  {
    juce::Graphics* graphicsContext;
    juce::Component* componentContext;
    prim::count indexOfCanvas;

  protected:
    bbs::abstracts::Portfolio::Canvas* internalPointerToCanvas;

  public:
    Properties() : graphicsContext(0), componentContext(0), indexOfCanvas(-1), 
      internalPointerToCanvas(0) {}

    friend struct Renderer;
  };
public:
  ///Contains the JUCE-specific pointers to the Graphics and Component objects.
  Properties* properties;

  ///Constructor initializes the renderer.
  Renderer() : properties(0) {}

  ///Calls the paint event of the current canvas being painted.
  virtual void Paint(bbs::abstracts::Portfolio* PortfolioToPaint,
    bbs::abstracts::Painter::Properties* PortfolioProperties);

  ///Converts a Prim path to a JUCE path.
  void ConvertPrimPathToJucePath(const prim::Path& ppath, juce::Path& jpath);

  ///Converts a Prim color to a JUCE color.
  juce::Colour ConvertPrimColorToJuceColor(prim::colors::RGB color);

  ///Maps the Prim path object to a JUCE path object and draws it.
  virtual void DrawPath(prim::Path& p,
    bool Stroke = true, bool Fill = false, bool ClosePath = false,
    prim::number StrokeWidth = 0.0);
};

#endif
