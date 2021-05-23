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

#include "Events.h"

#include "Content.h"
#include "Elements.h"
#include "Interaction.h"
#include "Page.h"
#include "Viewer.h"

//-------//
//Tooltip//
//-------//
void Tooltip::showTip(int x, int y)
{
  x += 10;
  juce::Font f;
  int h = 20;
  int w = f.getStringWidth(text.Merge()) + 4;
  setBounds(x, y - h / 2, w, h);
  if(!isVisible())
    setVisible(true);
  else
    repaint();
}

void Tooltip::hideTip(void)
{
  setVisible(false);
}

void Tooltip::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::cornsilk);
  g.setColour(juce::Colours::saddlebrown);
  g.drawRect(0, 0, getWidth(), getHeight());
  g.setColour(juce::Colours::black);
  g.drawSingleLineText(text.Merge(), 2, 14);
}

//--------------------//
//Generic EventHandler//
//--------------------//

void EventHandler::stopEvent(void)
{
  *ptrEventHandler = 0;
}

EventHandler::EventHandler(Document* document) : DocumentHandler(document),
  isRunning(false), ptrEventHandler(0), page(0)
{
}

void EventHandler::beginEvent(Page* page, EventHandler** ptrCurrentEventHandler,
                              prim::integer beginX, prim::integer beginY)
{
  EventHandler::page = page;
  ptrEventHandler = ptrCurrentEventHandler;
  handler(beginX, beginY);
}


void EventHandler::cancel(void)
{
  stopEvent();
}

//Default event handlers do nothing...
void EventHandler::mouseDown(const juce::MouseEvent &e) {}
void EventHandler::mouseUp(const juce::MouseEvent &e) {}
void EventHandler::mouseMove(const juce::MouseEvent &e) {}
void EventHandler::mouseDrag(const juce::MouseEvent &e) {}
void EventHandler::mouseExit(const juce::MouseEvent &e) {}
void EventHandler::keyPress(const juce::KeyPress &k) {}

//----------//
//Drag Score//
//----------//

EventDragScore::EventDragScore(Document* document) : EventHandler(document) {}

void EventDragScore::handler(prim::integer beginX, prim::integer beginY)
{
  using namespace prim;

  oldZoom = getDocument()->viewer->percentageZoom;
  anchorX = beginX;
  anchorY = beginY;
  getDocument()->viewer->focusPage = page;
  page->setMouseCursor(
    juce::MouseCursor(juce::MouseCursor::DraggingHandCursor));
}

void EventDragScore::mouseDrag(const juce::MouseEvent &e)
{
  using namespace prim;
  using namespace math;
  
  getDocument()->viewer->focusPageRelative = 
    Vector((number)anchorX / (number)page->getWidth(),
           (number)anchorY / (number)page->getHeight());
  int displayX, displayY;
  juce::Point<int> jp = getDocument()->content->getMouseXYRelative();
  displayX = jp.getX();
  displayY = jp.getY();
  getDocument()->viewer->focusScreenRelative = 
    Vector((number)displayX / (number)getDocument()->content->getWidth(),
           (number)displayY / (number)getDocument()->content->getHeight());
  getViewer()->positionPages(false);
}

void EventDragScore::mouseUp(const juce::MouseEvent &e)
{
  getDocument()->viewer->percentageZoom = oldZoom;
  getViewer()->positionPages(true);
  page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
  stopEvent();
}

//-----------//
//Drag Handle//
//-----------//

EventDragHandle::EventDragHandle(Document* document) : EventHandler(document) {}

void EventDragHandle::handler(prim::integer beginX, prim::integer beginY)
{
  using namespace prim;
  
  anchorX = beginX;
  anchorY = beginY;
  
  if(interaction.type == Interaction::CreateSection ||
     interaction.type == Interaction::ChangeMainSectionSegments ||
     interaction.type == Interaction::SectionHeight ||
     interaction.type == Interaction::SectionAccelerandoLeft)
    page->setMouseCursor(juce::MouseCursor(
      juce::MouseCursor::UpDownResizeCursor));
  else if(interaction.type == Interaction::MainSectionWidth)
    page->setMouseCursor(juce::MouseCursor(
      juce::MouseCursor::LeftRightResizeCursor));
  else
    page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::CrosshairCursor));
  
  originalSegments = interaction.section->segments;
  
  tooltip = new Tooltip;
  getPage(0)->addChildComponent(tooltip);
}

void EventDragHandle::mouseDrag(const juce::MouseEvent &e)
{
  using namespace prim;
  using namespace math;
  
  Vector normal((number)e.x / (number)page->getWidth(),
         (number)e.y / (number)page->getHeight());
  
  normal.x -= 0.5f;
  normal.y -= 0.5f;
  
  normal.x = Max(Min(normal.x, (number)0.5f), (number)-0.5f);
  normal.y = Max(Min(normal.y, (number)0.5f), (number)-0.5f);
  
  normal.x *= getContainer()->sizePage.x;
  normal.y *= -getContainer()->sizePage.y;

  if(interaction.type == Interaction::MainSectionPosition)
  {
    Vector offset = normal;
    offset.y -= getContainer()->sizeMainSection.y * 0.5f;
    getContainer()->offsetMainSection.x = offset.x;
    getContainer()->offsetMainSection.y = offset.y;
  }
  else if(interaction.type == Interaction::MainSectionWidth)
  {
    Vector displace = normal - getContainer()->offsetMainSection;
    number w = Min(Max(Abs(displace.x) * (number)2.0f,
      (number)1.0f), getContainer()->sizePage.x);
    getContainer()->sizeMainSection.x = w;
    
    if(getDocument()->useInches)
    {
      getContainer()->sizeMainSection.x =
        (number)((int)(getContainer()->sizeMainSection.x * 10.0f)) / 10.0f;
      tooltip->text = getContainer()->sizeMainSection.x;
      tooltip->text &= " in";
    }
    else
    {
      getContainer()->sizeMainSection.x =
        (number)((int)(getContainer()->sizeMainSection.x * 25.4f)) / 25.4f;
      tooltip->text = (number)getContainer()->sizeMainSection.x * (number)2.54;
      tooltip->text &= " cm";
    }
    tooltip->showTip((int)anchorX, (int)anchorY);
  }
  else if(interaction.type == Interaction::SectionHeight)
  {
    Vector displace = normal - getContainer()->offsetMainSection;
    if(interaction.section != 
      getContainer()->GetChildOfType<Representation::Section>())
    {
      interaction.section->scalarHeight = Min((number)2.0f,
        Max((number)0.1f, displace.y / interaction.section->cachedHeight));
      interaction.section->scalarHeight =
        (number)((int)(interaction.section->scalarHeight * 100.0f)) / 100.0f;
      tooltip->text = (number)interaction.section->scalarHeight * (number)100.0;
      tooltip->text &= "%";
    }
    else
    {
      getContainer()->sizeMainSection.y =
        Min(getContainer()->sizePage.y * (number)0.5f,
          Max((number)0.5f, displace.y));
      if(getDocument()->useInches)
      {
        getContainer()->sizeMainSection.y =
          (number)((int)(getContainer()->sizeMainSection.y * 10.0f)) / 10.0f;
        tooltip->text = getContainer()->sizeMainSection.y;
        tooltip->text &= " in";
      }
      else
      {
        getContainer()->sizeMainSection.y =
          (number)((int)(getContainer()->sizeMainSection.y * 25.4f)) / 25.4f;
        tooltip->text = (number)getContainer()->sizeMainSection.y * 
          (number)2.54;
        tooltip->text &= " cm";
      }
    }
    tooltip->showTip((int)anchorX, (int)anchorY);
  }
  else if(interaction.type == Interaction::SectionAccelerandoLeft)
  {
    Vector displace = normal - getContainer()->offsetMainSection;
    number expscale = 1.0;//interaction.section->cachedExponentialScale;
    bool isUp = (interaction.section->cachedRecurseDepth % 2) == 1;
    
    displace.y -= interaction.section->cachedHeight *
      interaction.section->scalarHeight;
    number deltay = displace.y * -1.0f;
    number accel = deltay /
      (getContainer()->scalarBeamSlant * getContainer()->sizeMainSection.y *
       expscale);
    accel = Min((number)0.33f, Max((number)-0.33f, -accel));
    accel *= 100.0f;
    accel = (number)((int)accel) / 100.0f;
    if(accel < 0)
    {
      tooltip->text = "RL ";
      tooltip->text &= (-accel + 1.0f);
    }
    else if(accel > 0)
    {
      tooltip->text = "LR ";
      tooltip->text &= (accel + 1.0f);
    }
    else
    {
      tooltip->text = "Flat";
    }

    tooltip->showTip((int)anchorX, (int)anchorY);
    interaction.section->scalarAccelerando = (isUp ? 1.0f : -1.0f) * accel;
  }
  else if(interaction.type == Interaction::CreateSection)
  {
    Representation::Section* rs = 0;
    for(count i =
      interaction.section->CountChildrenOfType<Representation::Section>() - 1;
      i >= 0; i--)
    {
      Representation::Section* j = 
        interaction.section->GetChildOfType<Representation::Section>(i);
      if(j->parentSegment == interaction.segment)
      {
        rs = j;
        break;
      }
    }
   
    if(!rs)
    { 
      rs = new Representation::Section(interaction.section);
      rs->parentSegment = interaction.segment;
      interaction.section->AddObject(rs);
    }
    
    number dist = (number)e.y - (number)anchorY;
    count segs = Min((count)15,
      Max((count)(dist / -20.0f + 2.0f), (count)2));
    rs->segments = segs;
    tooltip->text = (integer)segs;
    tooltip->showTip((int)anchorX, (int)anchorY);
  }
  else if(interaction.type == Interaction::ChangeMainSectionSegments)
  {
    number dist = (number)e.y - (number)anchorY;
    count segs = Min((count)15, Max((count)(dist / -20.0f) + 
      originalSegments, (count)2));
    interaction.section->segments = segs;
    tooltip->text = (integer)segs;
    tooltip->showTip((int)anchorX, (int)anchorY);
  }
  else if(interaction.type == Interaction::DeleteSection)
  {
    number dist = (number)e.y - (number)anchorY;
    count segs = Min((count)15, Max((count)(dist / -20.0f) + 
      originalSegments, (count)2));
    interaction.section->segments = segs; 
    tooltip->text = (integer)segs;
    tooltip->showTip((int)anchorX, (int)anchorY);
  }
  
  getPage(0)->repaint();
}

void EventDragHandle::mouseUp(const juce::MouseEvent &e)
{
  page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
  delete tooltip;
  stopEvent();
}

//----------//
//Zoom Score//
//----------//

EventZoomScore::EventZoomScore(Document* document) : 
  EventHandler(document), oldZoom(1.0) {}

void EventZoomScore::handler(prim::integer beginX, prim::integer beginY)
{
  using namespace prim;
  using namespace math;

  oldZoom = getDocument()->viewer->percentageZoom;
  anchorX = beginX;
  anchorY = beginY;
  getDocument()->viewer->focusPage = page;
  getDocument()->viewer->focusPageRelative = 
    Vector((number)anchorX / (number)page->getWidth(),
           (number)anchorY / (number)page->getHeight());
  int displayX, displayY;
  juce::Point<int> jp = getDocument()->content->getMouseXYRelative();
  displayX = jp.getX();
  displayY = jp.getY();
  getDocument()->viewer->focusScreenRelative = 
    Vector((number)displayX / (number)getDocument()->content->getWidth(),
           (number)displayY / (number)getDocument()->content->getHeight());
       
  page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::UpDownResizeCursor));
}

void EventZoomScore::mouseDrag(const juce::MouseEvent &e)
{
  using namespace prim;
  using namespace math;
  
  number dist = (number)e.getDistanceFromDragStartY();
  number newZoom = oldZoom * exp(-dist / 100.0f);

  //Clamp the zoom.
  if(newZoom < 1.0f / 2.0f)
    newZoom = 1.0f / 2.0f;

  if(newZoom > 20000.0f)
    newZoom = 20000.0f;
  
  getDocument()->viewer->percentageZoom = newZoom;
  getViewer()->positionPages(true);
}

void EventZoomScore::mouseUp(const juce::MouseEvent &e)
{
  getViewer()->positionPages(true);
  page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
  stopEvent();
}


//---------------------//
//Make Export Selection//
//---------------------//
/*
EventMakeExportSelection::Overlay::Overlay(EventMakeExportSelection* parent)
{
  Overlay::parent = parent;
}

EventMakeExportSelection::EventMakeExportSelection(Document* document) : 
  EventHandler(document), isDragAndDrop(false), overlay(0) {}

void EventMakeExportSelection::Overlay::paint(juce::Graphics &g)
{
  g.setColour(juce::Colours::blanchedalmond.darker(0.05f));
  g.setOpacity(0.2f);
  g.fillRect(0,0,getWidth(),getHeight());
  g.setColour(juce::Colours::blanchedalmond.darker(0.25f));
  g.setOpacity(0.2f);
  g.drawRect(0,0,getWidth(),getHeight(),2);
  juce::String reminder;
  if(parent->isDragAndDrop)
    reminder = "Drag to an external application or the desktop.";
  else
    reminder = "Select the area you want to export.";
  
  g.setColour(juce::Colours::black);
  g.drawText(reminder,0,0,getWidth(),getHeight(),
    juce::Justification::centred, false);
}

void EventMakeExportSelection::Overlay::mouseDrag(const juce::MouseEvent &e)
{
  using namespace prim;
  using namespace math;

  prim::number srcX = (prim::number)getX();
  prim::number srcY = (prim::number)getY();
  prim::number destX = (prim::number)(getX() + getWidth());
  prim::number destY = (prim::number)(getY() + getHeight());
  if(destX == srcX)
    destX++;
  if(destY == srcY)
    destY++;

  prim::number w = (prim::number)parent->page->getWidth();
  prim::number h = (prim::number)parent->page->getHeight();

  Rectangle subsection(Vector(srcX / w, srcY / h),Vector(destX / w, destY / h));
  String filename;
  
  juce::String timestamp;
  {
    juce::Time t = juce::Time::getCurrentTime();
    timestamp << t.toString(true,true,false,false);
    prim::String randomNumbers;
    randomNumbers.AppendInteger(math::Rand(0,999),3,false);
    timestamp << "-";
    timestamp << randomNumbers.Merge();
    timestamp = timestamp.replaceCharacter(' ','.');
    timestamp = timestamp.replaceCharacter(':','.');
    timestamp = timestamp.replaceCharacter('\'','.');
    timestamp = timestamp.replaceCharacter(',','.');
    timestamp = timestamp.replace(juce::String(".."),juce::String("."),false);
    timestamp = timestamp.replace(juce::String(".."),juce::String("."),false);
    timestamp = timestamp.replace(juce::String(".."),juce::String("."),false);
    timestamp << ".jpg";
  }
  filename = juce::File::getSpecialLocation(
    juce::File::tempDirectory).getFullPathName().toUTF8();
  filename.Append((unicode::UCS4)juce::File::separator);
  filename &= timestamp.toUTF8();

  parent->page->exportToJPEG(filename, subsection);

  juce::StringArray fileArray;
  fileArray.add(juce::String(filename.Merge()));
  juce::DragAndDropContainer::performExternalDragDropOfFiles(fileArray, true);

  juce::String j_filename = filename.Merge();
  juce::File f(j_filename);
  //f.deleteFile();

  parent->page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
  parent->stopEvent();
  parent->overlay = 0;
  parent->isDragAndDrop = false;
  delete this;
}

void EventMakeExportSelection::handler(prim::integer beginX, 
                                       prim::integer beginY)
{
  anchorX = beginX;
  anchorY = beginY;
  delete overlay;
  overlay = new Overlay(this);
  
  overlay->setBounds((int)anchorX,(int)anchorY,1,1);
  page->addAndMakeVisible(overlay);
  page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::CrosshairCursor));
}

void EventMakeExportSelection::mouseDrag(const juce::MouseEvent &e)
{
  prim::integer srcX = anchorX;
  prim::integer srcY = anchorY;
  prim::integer destX = e.x;
  prim::integer destY = e.y;
  isDragAndDrop = false;
  if(destX == srcX)
    destX++;
  if(destY == srcY)
    destY++;

  prim::math::Ascending(srcX,destX);
  prim::math::Ascending(srcY,destY);

  prim::integer max_w = page->getWidth();
  prim::integer max_h = page->getHeight();

  if(srcX < 0)
    srcX = 0;
  if(srcY < 0)
    srcY = 0;
  if(destX > max_w)
    destX = max_w;
  if(destY > max_h)
    destY = max_h;

  overlay->setBounds((int)srcX,(int)srcY,(int)(destX-srcX),(int)(destY-srcY));
}

void EventMakeExportSelection::mouseDown(const juce::MouseEvent &e)
{
  delete overlay;
  overlay = 0;
  page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
  stopEvent();
}

void EventMakeExportSelection::mouseUp(const juce::MouseEvent &e)
{
  if(overlay->getWidth() < 10 || overlay->getHeight() < 10)
  {
    delete overlay;
  overlay = 0;
    page->setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
    stopEvent();  
  }
  else
  {
    isDragAndDrop = true;
    overlay->repaint();
  }
}

EventMakeExportSelection::~EventMakeExportSelection()
{
  delete overlay;
}
*/
