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

#ifndef Page_h
#define Page_h

#include "Libraries.h"

//Base classes...
#include "Gestures.h"

//Forward declarations...
#include "Events.h"

struct Page :
  public juce::Component, 
  //public juce::FileDragAndDropTarget,
  public GestureHandler,
  public DocumentHandler
{
  //-----------//
  //Cached Data//
  //-----------//
  prim::math::Vector pixelsTopLeft;
  prim::math::Vector pixelsDimensions;

  //----------------------//
  //Constructor/Destructor//
  //----------------------//
  Page(Document* document);
  virtual ~Page();

  //-----------//
  //Paint Event//
  //-----------//
  void paint(juce::Graphics& g);

  //---------------//
  //Events/Gestures//
  //---------------//
  EventHandler* currentEvent;

  EventDragHandle eventDragHandle;
  EventDragScore eventDragScore;
  EventZoomScore eventZoomScore;
  
  //-------//
  //Helpers//
  //-------//
  prim::count getPageIndex(void);
  bool isSomePageInEvent(void);
  void cancelAllOtherPageEvents(Page* thisPage);
  Interaction* isUnderHandle(prim::integer x, prim::integer y);

  //------------//
  //Event Mapper//
  //------------//
  void mapEvent(EventHandler& EventType, prim::integer beginX = 0, 
    prim::integer beginY = 0);

  //-------------//
  //Gesture Hooks//
  //-------------//
  void mouseDown(const juce::MouseEvent &e);
  void mouseUp(const juce::MouseEvent &e);
  void mouseMove(const juce::MouseEvent &e);
  void mouseDrag(const juce::MouseEvent &e);
  void mouseExit(const juce::MouseEvent &e);
  
  //--------------------------//
  //Gesture Context Resolution//
  //--------------------------//
  virtual void Click(prim::integer x, prim::integer y);
  virtual void DoubleClick(prim::integer x, prim::integer y);
  virtual void TripleClick(prim::integer x, prim::integer y);
  virtual void ClickAndHold(prim::integer x, prim::integer y);
  virtual void DoubleClickAndHold(prim::integer x, prim::integer y);
  virtual void TripleClickAndHold(prim::integer x, prim::integer y){}
  virtual void ClickAndDrag(prim::integer x, prim::integer y);
  virtual void DoubleClickAndDrag(prim::integer x, prim::integer y);
  virtual void TripleClickAndDrag(prim::integer x, prim::integer y){}
  virtual void Pausing(prim::integer x, prim::integer y);

  //--------------//
  //Export to JPEG//
  //--------------//
  /*bool exportToJPEG(prim::String filename, 
    prim::math::Rectangle relativeSubsection,
    prim::number dpi = 300.0f);*/
    
  //-------------//
  //Drag and Drop//
  //-------------//
  /*virtual bool isInterestedInFileDrag(const juce::StringArray &files);
  virtual void filesDropped(const juce::StringArray &files, int x, int y);*/
};

#endif
