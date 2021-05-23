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

#ifndef Gestures_h
#define Gestures_h

#include "Libraries.h"

struct MouseTrack
{
  prim::integer x;
  prim::integer y;
  prim::number time;
};

class GestureHandler : public juce::Timer
{
protected:
  //Gesture types

  //Clicks:     0    1    2    3
  //
  //Release          x    x    x

  //Hold/Pause  x    x    x    x

  //Drag/Draw        x    x    x

  //Event handlers
  virtual void Click(prim::integer x, prim::integer y);
  virtual void DoubleClick(prim::integer x, prim::integer y);
  virtual void TripleClick(prim::integer x, prim::integer y);
  virtual void ClickAndHold(prim::integer x, prim::integer y);
  virtual void DoubleClickAndHold(prim::integer x, prim::integer y);
  virtual void TripleClickAndHold(prim::integer x, prim::integer y);
  virtual void ClickAndDrag(prim::integer x, prim::integer y);
  virtual void DoubleClickAndDrag(prim::integer x, prim::integer y);
  virtual void TripleClickAndDrag(prim::integer x, prim::integer y);
  virtual void Pausing(prim::integer x, prim::integer y);

private:

  void Holding(prim::integer x, prim::integer y);
  void Dragging(prim::integer x, prim::integer y);
  //------//
  //States//
  //------//
  prim::integer recentClickCount;

  ///If time expires here, then either a drag/hold event is taking place.
  bool waitingForMouseUpTimeout;
  bool waitingForGestureTimeout;
  bool waitingForMouseHold;
  bool isHolding;
  bool isDragging;

  prim::integer recentX,recentY;
  prim::integer firstX,firstY;

  //Mouse movement tracking
  prim::List<MouseTrack> mouseTracker;

protected:

  ///Constructor
  GestureHandler();

  ///Destructor
  virtual ~GestureHandler();


private:

  ///The event for the timer
  virtual void timerCallback(void);

protected:
  void gestureMouseDown(const juce::MouseEvent &e);
  void gestureMouseUp(const juce::MouseEvent &e);
  void gestureMouseExit(const juce::MouseEvent &e);
  void gestureMouseMove(const juce::MouseEvent &e);
  void gestureMouseDrag(const juce::MouseEvent &e);
};

#endif
