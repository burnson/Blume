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
 
#include "Page.h"

#include "Dialogs.h"
#include "Elements.h"
#include "Interaction.h"
#include "Renderer.h"
#include "Score.h"
#include "Viewer.h"

Page::Page(Document* document) : DocumentHandler(document),
  currentEvent(0), eventDragHandle(document), eventDragScore(document), eventZoomScore(document)
{
}

Page::~Page()
{
}

//-------//
//Helpers//
//-------//
prim::count Page::getPageIndex(void)
{
  for(prim::count i = 0; i < getPageCount(); i++)
    if(getPage(i) == this)
      return i;

  return -1; //This shouldn't happen!
}

void Page::paint(juce::Graphics& g)
{
  using namespace prim;
  Renderer::Properties properties;

  properties.graphicsContext = &g;
  properties.componentContext = this;
  properties.indexOfCanvas = getPageIndex();
  
  g.fillAll(juce::Colours::white);
  g.setColour(juce::Colours::black);
  g.setOpacity(0.1f);
  g.drawRect(0,0,getWidth(),getHeight());

  getScore()->Create<Renderer>(&properties);
}

bool Page::isSomePageInEvent(void)
{
  for(prim::count i = 0; i < getPageCount(); i++)
    if(getPage(i)->currentEvent)
      return true;
  return false;
}

void Page::cancelAllOtherPageEvents(Page* thisPage)
{
  for(prim::count i = 0; i < getPageCount(); i++)
  {
    Page* testPage = getPage(i);
    if(testPage != thisPage && testPage->currentEvent)
      testPage->currentEvent->cancel();
  }
}

//Gesture hooks
void Page::mouseDown(const juce::MouseEvent &e)
{
  cancelAllOtherPageEvents(this);
  if(currentEvent)
    currentEvent->mouseDown(e);
  gestureMouseDown(e);
}

void Page::mouseUp(const juce::MouseEvent &e)
{
  cancelAllOtherPageEvents(this);
  if(currentEvent)
    currentEvent->mouseUp(e);
  gestureMouseUp(e);
}

void Page::mouseMove(const juce::MouseEvent &e)
{
  //cancelAllOtherPageEvents(this);
  if(currentEvent)
    currentEvent->mouseMove(e);
  else
  {
    if(getDocument()->temporarilyHideHandles)
    {
      getDocument()->temporarilyHideHandles = false;
      repaint();
    }

    if(Interaction* i = isUnderHandle(e.x, e.y))
    {
      if(i->type == Interaction::ChangeMainSectionSegments ||
         i->type == Interaction::SectionHeight ||
         i->type == Interaction::SectionAccelerandoLeft)
        setMouseCursor(juce::MouseCursor(
          juce::MouseCursor::UpDownResizeCursor));
      else if(i->type == Interaction::MainSectionWidth)
        setMouseCursor(juce::MouseCursor(
          juce::MouseCursor::LeftRightResizeCursor));
      else if(i->type == Interaction::DeleteSection ||
        i->type == Interaction::CreateSection)
        setMouseCursor(juce::MouseCursor(
          juce::MouseCursor::PointingHandCursor));        
      else
        setMouseCursor(juce::MouseCursor(juce::MouseCursor::CrosshairCursor));    
          setMouseCursor(juce::MouseCursor(juce::MouseCursor::CrosshairCursor));
    }
    else
      setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
  }
  
  gestureMouseMove(e);
}

void Page::mouseDrag(const juce::MouseEvent &e)
{
  cancelAllOtherPageEvents(this);
  if(currentEvent)
    currentEvent->mouseDrag(e);
  gestureMouseDrag(e);
}

void Page::mouseExit(const juce::MouseEvent &e)
{
  cancelAllOtherPageEvents(this);
  if(currentEvent)
    currentEvent->mouseExit(e);
  gestureMouseExit(e);
}

void Page::mapEvent(EventHandler& EventType, prim::integer beginX, 
                    prim::integer beginY)
{
  if(!currentEvent)
  {
    currentEvent = &EventType;
    currentEvent->beginEvent(this, &currentEvent, beginX, beginY);
  }
}

Interaction* Page::isUnderHandle(prim::integer x, prim::integer y)
{
  prim::math::Vector normal((prim::number)x / (prim::number)getWidth(),
                            (prim::number)y / (prim::number)getHeight());
  
  normal.x -= 0.5f;
  normal.y -= 0.5f;
  
  normal.x *= getContainer()->sizePage.x;
  normal.y *= -getContainer()->sizePage.y;
  
  Interaction* bestin = 0;
  prim::number bestradius = 100.0f;
  for(prim::count i = 0; i < getInteractions().n(); i++)
  {
    Interaction* in = getInteractions()[i];
    prim::number d = prim::math::Dist(in->position.x, in->position.y,
                                      normal.x, normal.y);
    if(d <= in->radius * 2.5f && d < bestradius)
    {
      bestin = in;
      bestradius = d;
    }
  }
  
  return bestin;
}

void Page::ClickAndDrag(prim::integer x, prim::integer y)
{
  getDocument()->temporarilyHideHandles = true;
  
  if(Interaction* handle = isUnderHandle(x, y))
  {
    eventDragHandle.interaction = *handle;
    mapEvent(eventDragHandle, x, y);
  }
  else
    mapEvent(eventDragScore, x, y);
}

void Page::DoubleClick(prim::integer x, prim::integer y)
{
  if(Interaction* handle = isUnderHandle(x, y))
  {
    switch(handle->type)
    {
    case Interaction::MainSectionPosition:
      getContainer()->offsetMainSection = prim::math::Inches(0, 0); break;
      
    case Interaction::MainSectionWidth:
      getContainer()->sizeMainSection.x = getContainer()->sizePage.x *
        (prim::number)(9.0 / 11.0); break;
        
    case Interaction::SectionHeight:
      if(getContainer()->GetChildOfType<Representation::Section>() ==
        handle->section)
        getContainer()->sizeMainSection.y = getContainer()->sizePage.y * 0.25f;
      else
        handle->section->scalarHeight = 0.6f;
      break;
      
    case Interaction::SectionAccelerandoLeft:
      {
        Representation::Section* parentSection = handle->section;
        prim::number v = -parentSection->scalarAccelerando;
        new ValueChooser("Change Accelerando",
          "Enter new value (-0.33 to 0.33):", v, -0.33f, 0.33f);
        if(ValueChooserComponent::valueIsValid)
        {
          parentSection->scalarAccelerando =
            -ValueChooserComponent::lastValueReturned;
        }
      }     
      break;
      
    case Interaction::CreateSection:
      {
        Representation::Section* parentSection = handle->section;
        prim::count parentSegment = handle->segment;
        new ValueChooser("Create Section", "Enter segments (2-100):",
          2, 2, 100);
        if(ValueChooserComponent::valueIsValid)
        {
          Representation::Section* rs =
            new Representation::Section(parentSection);
          rs->parentSegment = parentSegment;
          rs->segments = (prim::count)ValueChooserComponent::lastValueReturned;
          parentSection->AddObject(rs);
        }
      }
      break;
      
    case Interaction::ChangeMainSectionSegments:
      {
        Representation::Section* section = handle->section;
        new ValueChooser("Change Segments", "Enter segments (2-100):",
          2, 2, 100);
        if(ValueChooserComponent::valueIsValid)
        {
          section->segments = (prim::count)
            ValueChooserComponent::lastValueReturned;
        }
      }
      break;
      
    case Interaction::DeleteSection:
      handle->section->Remove();
      break;
    }
    repaint();
  }
}

void Page::DoubleClickAndDrag(prim::integer x, prim::integer y)
{
  mapEvent(eventZoomScore, x, y);
}

void Page::DoubleClickAndHold(prim::integer x, prim::integer y)
{
  mapEvent(eventZoomScore, x, y);
}

void Page::ClickAndHold(prim::integer x, prim::integer y)
{
  //mapEvent(eventMakeExportSelection, x, y);
}

void Page::TripleClick(prim::integer x, prim::integer y)
{
  using namespace prim::math;
  
  getViewer()->focusPage = this;
  getViewer()->percentageZoom = 0.75f;
  getViewer()->focusPageRelative = Vector(0.5f, 0.25f);
  getViewer()->focusScreenRelative = Vector(0.5f, 0.5f);
  getViewer()->positionPages(true);
}

void Page::Click(prim::integer x, prim::integer y)
{
  using namespace prim;
  using namespace math;

  //Get the position in terms of inches.
  Inches inDimensions = getCanvas(getPageIndex())->Dimensions;
  number scale = inDimensions.x / (number)getWidth();
  Vector inPosition((number)x,(number)getHeight() - (number)y);
  inPosition *= scale;
}

void Page::Pausing(prim::integer x, prim::integer y)
{
  getDocument()->temporarilyHideHandles = true;
  repaint();
}

/*Some old code from another application that can do selection-based exports,
and drag-and-drop file opening:

bool Page::exportToJPEG(prim::String filename, 
  prim::math::Rectangle exportRectangle,
  prim::number exportResolutionDPI)
{
  //Context resolution goes here...
  using namespace juce;

  //Set the properties correctly.
  Renderer::Properties exportProperties;
  for(prim::count i=0;i<getDisplay()->typefaces.n();i++)
    exportProperties.typefaces.Add() = getDisplay()->typefaces[i];

  for(prim::count i = 0; i < getPageCount();i++)
    if(getPage(i) == this)
      exportProperties.indexOfCanvas = i;

  //Fix the export rectangle.
  prim::number x1,y1,x2,y2;
  x1 = exportRectangle.BottomLeft().x;
  y1 = exportRectangle.BottomLeft().y;
  x2 = exportRectangle.TopRight().x;
  y2 = exportRectangle.TopRight().y;

  if(x1 < 0.0f)
    x1 = 0.0f;
  if(y1 < 0.0f)
    y1 = 0.0f;
  if(x2 > 1.0f)
    x2 = 1.0f;
  if(y2 > 1.0f)
    y2 = 1.0f;

  exportRectangle.BottomLeft(prim::math::Vector(x1,y1));
  exportRectangle.TopRight(prim::math::Vector(x2,y2));

  //Calculate the export dimensions.
  prim::math::Inches inDimensions = 
    getScore()->Canvases[exportProperties.indexOfCanvas]->Dimensions;
  prim::number exportWidth = exportResolutionDPI * inDimensions.x;
  prim::number exportHeight = exportResolutionDPI * inDimensions.y;
  
  //Create a fake component to export to.
  Component exportComponent;
  exportComponent.setBufferedToImage(false);
  exportComponent.setSize((int)exportWidth,(int)exportHeight);

  //Create an image buffer to store the whole page in.
  if((int)exportWidth == 0 || (int)exportHeight == 0)
    return false;

  Image exportImage(Image::RGB,
    (int)exportWidth,(int)exportHeight,false);
  
  //Create a graphics context for the image buffer and fill it white.
  Graphics exportGraphics(exportImage);
  exportGraphics.fillAll(Colours::white);

  //Set the contexts in the properties.
  exportProperties.graphicsContext = &exportGraphics;
  exportProperties.componentContext = &exportComponent;
  
  //Export the whole page, and then get the subsection after.
  getScore()->Create<Renderer>(&exportProperties);

  //Calculate the subsection dimensions.
  prim::number subsectionWidth = exportWidth * exportRectangle.Width();
  prim::number subsectionHeight = exportHeight * exportRectangle.Height();
    
  //Create the subsection.
  if((int)subsectionWidth == 0 || (int)subsectionHeight == 0)
    return false;

  Image exportSubsection(Image::RGB, 
    (int)subsectionWidth, 
    (int)subsectionHeight, 
    false);

  //Copy the subsection from the whole page to the new subsection image.
  int x,y,w,h;
  int ls = 0, ps = 0;
  x = (int)(exportRectangle.BottomLeft().x * exportWidth);
  y = (int)(exportRectangle.BottomLeft().y * exportHeight);
  w = exportSubsection.getWidth();
  h = exportSubsection.getHeight();
  
  const juce::uint8* sourcePixelData = 
    exportImage.lockPixelDataReadOnly(x,y,w,h,ls,ps);

  exportSubsection.setPixelData(0,0,
    exportSubsection.getWidth(),exportSubsection.getHeight(),
    sourcePixelData, ls);

  exportImage.releasePixelDataReadOnly(sourcePixelData);

  //Export a JPEG.
  JPEGImageFormat jpeg;
  jpeg.setQuality(1.0f);
  File outputFile(String(filename.Merge()));
  if(outputFile.deleteFile())
  {
    FileOutputStream outputFileStream(outputFile);
    if(jpeg.writeImageToStream(exportSubsection, outputFileStream))
      return true;
    else
      return false;
  }
  else
    return false;
}

bool Page::isInterestedInFileDrag(const juce::StringArray &files)
{
  using namespace prim;
  for(count i=0;i<files.size();i++)
  {
    int startIndex = files[i].length() - 4;
    
    if(startIndex > 0)
    {
      juce::String extension = files[i].substring(startIndex).toLowerCase();
      if(extension == juce::String(".pdf") || extension == juce::String(".xml"))
        return true;
    }
  }
  return false;
}

void Page::filesDropped(const juce::StringArray &files, int x, int y)
{
  using namespace prim;
  for(count i = 0; i < files.size(); i++)
  {
    juce::String j_file = files[i];
    String file = j_file.toUTF8();
    String metadata;

    int startIndex = j_file.length() - 4;
    if(startIndex > 0)
    {
      juce::String extension = j_file.substring(startIndex).toLowerCase();
      if(extension == juce::String(".pdf"))
        abcd::PDF::RetrievePDFMetadataAsString(file, metadata);
      else if(extension == juce::String(".xml"))
        File::ReadAsUTF8(file, metadata);
    }
    
    if(metadata.n() != 0)
    {
      Window* new_ccw = new Window(
        new Document::Initialization(metadata, file));
    }
  }
}
*/
