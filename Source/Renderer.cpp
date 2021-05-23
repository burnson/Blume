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

#include "Renderer.h"

void Renderer::Paint(bbs::abstracts::Portfolio* PortfolioToPaint,
                     bbs::abstracts::Painter::Properties* PortfolioProperties)
{
  //Get a pointer to the Renderer's properties.
  properties = PortfolioProperties->Interface<Properties>();

  //Give the painter methods access to the abstract canvas methods and members.
  properties->internalPointerToCanvas = 
    PortfolioToPaint->Canvases[properties->indexOfCanvas];

  //Paint the current canvas.
  PortfolioToPaint->Canvases[properties->indexOfCanvas]->Paint(this);

  //Set the properties pointer back to null to be safe.
  properties = 0;
}

void Renderer::ConvertPrimPathToJucePath(
  const prim::Path& ppath, juce::Path& jpath)
{
  using namespace prim;
  using namespace prim::math;

  const prim::Array<prim::Path::Component>& ppath_Components = ppath.Components;
  count ppath_Components_n = ppath_Components.n();

  for(count i = 0; i < ppath_Components_n; i++)
  {
    const prim::Path::Component& ppathc = ppath_Components[i];
    const prim::Array<prim::Path::Curve>& ppathcvs = ppathc.Curves;

    count ppathcvs_n = ppathc.Curves.n();
    if(!ppathcvs_n) continue;

    const Vector& StartPoint = ppathc.Curves[0].End;
    jpath.startNewSubPath((float)StartPoint.x, (float)StartPoint.y);
    
    for(count j = 1; j < ppathcvs_n; j++)
    {
      const prim::Path::Curve& ppathcv = ppathcvs[j];
      const Vector& ppathcv_End = ppathcv.End;

      if(!ppathcv.IsCurve)
        jpath.lineTo((float)ppathcv_End.x, (float)ppathcv_End.y);
      else
      {
        const Vector& ppathcv_StartControl = ppathcv.StartControl;
        const Vector& ppathcv_EndControl = ppathcv.EndControl;
        jpath.cubicTo(
          (float)ppathcv_StartControl.x, (float)ppathcv_StartControl.y,
          (float)ppathcv_EndControl.x, (float)ppathcv_EndControl.y,
          (float)ppathcv_End.x, (float)ppathcv_End.y);
      }
    }
  }
}

juce::Colour Renderer::ConvertPrimColorToJuceColor(prim::colors::RGB color)
{
  using namespace prim::colors;
  return juce::Colour(
                      (prim::byte)(prim::colors::Component::r(color) * 255.0f),
    (prim::byte)(prim::colors::Component::g(color) * 255.0f),
    (prim::byte)(prim::colors::Component::b(color) * 255.0f));
}

void Renderer::DrawPath(prim::Path& p, bool Stroke, bool Fill, bool ClosePath,
                        prim::number StrokeWidth)
{
  using namespace prim;
  using namespace prim::math;

  //Make sure that we are inside a valid paint event.
  if(!properties)
    return;

  //Create a JUCE path from the Prim path.
  juce::Path jp;
  ConvertPrimPathToJucePath(p, jp);

  //Determine the dimensions of the current canvas and the appropriate scale.
  Vector pageDimensions =
    Inches(properties->internalPointerToCanvas->Dimensions);
  number scaleToFitPage =
    (number)properties->componentContext->getWidth() / pageDimensions.x;

  //Calculate the affine transform so that the image is scaled to the page.
  StateMatrix<RasterState> m = State.Forwards();
  juce::AffineTransform jat(
    (float)m.a, (float)m.c, (float)m.e,
    (float)m.b, (float)m.d, (float)m.f);
  jat = jat.translated(0, (float)-pageDimensions.y);
  jat = jat.scaled((float)scaleToFitPage, (float)-scaleToFitPage);

  //Get the JUCE graphics context.
  juce::Graphics* g = properties->graphicsContext;

  //Fill path if necessary.
  if(Fill)
  {
    //Set the fill color.
    g->setColour(ConvertPrimColorToJuceColor(State.TopState().FillColor));

    //Fill the path.
    g->fillPath(jp, jat);
  }

  //Stroke path if necessary.
  if(Stroke)
  {
    //Set the stroke color.
    g->setColour(ConvertPrimColorToJuceColor(State.TopState().StrokeColor));

    //Determine the affine transform scaled stroke width.
    number scaledStrokeWidth = StrokeWidth * (number)jat.mat00;

    //Stroke the path.
    g->strokePath(jp, juce::PathStrokeType((float)scaledStrokeWidth), jat);
  }
}
