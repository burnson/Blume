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

#include "Viewer.h"

#include "Content.h"
#include "Events.h"
#include "Globals.h"
#include "Page.h"
#include "Score.h"

void Viewer::positionPages(bool optimize)
{
  using namespace prim;
  using namespace math;
  
  //Make sure there are pages to position.
  if(!getPageCount())
    return;
  
  //Get the work area dimensions and focus point.
  Vector work_area((number)getContent()->getWidth(),
                   (number)getContent()->getHeight());
  Vector work_area_focus(
    work_area.x * focusScreenRelative.x, work_area.y * focusScreenRelative.y);
  
  //Get the current resolution of the desktop.
  const juce::Desktop& desktop = juce::Desktop::getInstance();
  juce::Rectangle<int> desktop_rectangle = desktop.getDisplays().getTotalBounds(false);
  Vector desktop_resolution((number)desktop_rectangle.getWidth(),
                            (number)desktop_rectangle.getHeight());
  
  //Calculate the screen's dots-per-inch.
  number dpi = desktop_resolution.Mag();
  //dpi /= globals()->inDisplayDiagonalSize;
  dpi = 96.0f;
  
  //Take into account the zoom.
  dpi *= percentageZoom;
  
  //Go through each page and calculate its pixel dimensions.
  List<Vector> pixelsPageDimensions;
  for(count i = 0; i < getCanvasCount(); i++)
  {
    Inches inPageDimensions = getCanvas(i)->Dimensions;
    pixelsPageDimensions.Add() = Vector(inPageDimensions.x * dpi, 
                                        inPageDimensions.y * dpi);
  }
  
  //Determine the index of the focus page.
  bool foundPointer = false;
  for(count i = 0; i < getPageCount(); i++)
  {
    if(getPage(i) == focusPage)
    {
      foundPointer = true;
      focusIndex = i;
      break;
    }
  }
  
  /*Use the old focus index, or the first or last page if the pointer was 
   invalid.*/
  if(!foundPointer)
  {
    if(focusIndex < 0)
    {
      //Totally out of bounds... just reset to the first page.
      focusIndex = 0;
      focusPage = getPages().first();
    }
    else if(focusIndex >= getPageCount())
    {
      //Totally out of bounds... just reset to the last page.
      focusIndex = getPageCount() - 1;
      focusPage = getPages().last();
    }
    else
    {
      //Index is OK, just look up new pointer.
      focusPage = getPage(focusIndex);
    }
  }
  
  //Calculate the focal pixel of the focal page.
  Vector page_focus_dimensions = pixelsPageDimensions[focusIndex];
  Vector page_focus(page_focus_dimensions.x * focusPageRelative.x,
                    page_focus_dimensions.y * focusPageRelative.y);
  
  //Compute the bias vector.
  Vector bias;
  
  //First compute the leading bias.
  for(count i = 0; i < focusIndex; i++)
  {
    bias.x -= pixelsPageDimensions[i].x + 
    globals()->pixelsHorizontalDistanceBetweenPages;
  }
  
  //Now add the focus bias.
  bias.x -= page_focus.x;
  bias.y -= page_focus.y;
  
  //Now add the screen bias.
  bias.x += work_area_focus.x;
  bias.y += work_area_focus.y;
  
  //Optimization if necessary.
  if(optimize)
  {
    prim::number edge = (prim::number)globals()->pixelsEdgeOfPageToDisplay;
    prim::number totalWidth = 0;
    prim::number maxHeight = 0;
    for(count i = 0; i < getPageCount(); i++)
    {
      totalWidth += pixelsPageDimensions[i].x;
      if(i)
        totalWidth += 
        (prim::number)globals()->pixelsHorizontalDistanceBetweenPages;
      maxHeight = Max(maxHeight, pixelsPageDimensions[i].y);
    }
    
    //Horizontal consideration...
    if(totalWidth <= getContent()->getWidth())
      bias.x = (number)getContent()->getWidth()
      / 2.0f - totalWidth / 2.0f;
    else if(bias.x > edge)
      bias.x = edge;
    else if(bias.x + totalWidth <
      (number)getContent()->getWidth() - edge)
        bias.x = (number)getContent()->getWidth() - edge - totalWidth;
    
    //Vertical consideration...
    if(maxHeight <= getContent()->getHeight())
      bias.y =
        (number)getContent()->getHeight() / 2.0f - maxHeight / 2.0f;
    else if(bias.y > edge)
      bias.y = edge;
    else if(bias.y + maxHeight < 
      (number)getContent()->getHeight() - edge)
      bias.y = (number)getContent()->getHeight() - edge - maxHeight;
  }
  
  //Now set the position of the pages.
  number left = 0;
  
  //Rounding error correction
  left += 0.5f;
  bias += 0.5f;
  
  left = (number)((int)left);
  bias.x = (number)((int)bias.x);
  bias.y = (number)((int)bias.y);
  
  for(count i = 0; i < getPageCount(); i++)
  {
    
    Vector pixelsTopLeft = Vector(left + bias.x, bias.y);
    Vector pixelsDimensions = pixelsPageDimensions[i];
    
    pixelsDimensions += 0.5f;
    pixelsDimensions.x = (number)((int)pixelsDimensions.x);
    pixelsDimensions.y = (number)((int)pixelsDimensions.y);
    
    getPage(i)->pixelsTopLeft = pixelsTopLeft;
    getPage(i)->pixelsDimensions = pixelsDimensions;
    
    juce::Rectangle<int> old_bounds = getPage(i)->getBounds();
    
    juce::Rectangle<int> new_bounds((int)pixelsTopLeft.x,
                               (int)pixelsTopLeft.y,
                               (int)pixelsDimensions.x,
                               (int)pixelsDimensions.y);
    
    if(old_bounds != new_bounds)
      getDocument()->pages[i]->setBounds(new_bounds);
    
    left += pixelsDimensions.x + 
      globals()->pixelsHorizontalDistanceBetweenPages;
  }
  
  return;
}

Viewer::Viewer(Document* document) : DocumentHandler(document), 
  percentageZoom(0.75f), focusPage(0), focusIndex(0)
{
  using namespace prim;
  using namespace math;
  focusPageRelative = Vector(0.5f, 0.25f);
  focusScreenRelative = Vector(0.5f, 0.5f);
}
