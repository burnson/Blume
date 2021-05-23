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

#include "Gestures.h"

//Default gesture handlers do nothing...
void GestureHandler::Click(prim::integer x, prim::integer y){}
void GestureHandler::DoubleClick(prim::integer x, prim::integer y){}  
void GestureHandler::TripleClick(prim::integer x, prim::integer y){}
void GestureHandler::ClickAndHold(prim::integer x, prim::integer y){}
void GestureHandler::DoubleClickAndHold(prim::integer x, prim::integer y){}
void GestureHandler::TripleClickAndHold(prim::integer x, prim::integer y){}
void GestureHandler::ClickAndDrag(prim::integer x, prim::integer y){}
void GestureHandler::DoubleClickAndDrag(prim::integer x, prim::integer y){}
void GestureHandler::TripleClickAndDrag(prim::integer x, prim::integer y){}
void GestureHandler::Pausing(prim::integer x, prim::integer y){}

void GestureHandler::Holding(prim::integer x, prim::integer y)
{    
  if(recentClickCount == 1)
    ClickAndHold(x,y);
  else if(recentClickCount == 2)
    DoubleClickAndHold(x,y);
  else
    TripleClickAndHold(x,y);
}

void GestureHandler::Dragging(prim::integer x, prim::integer y)
{
  if(recentClickCount == 1)
    ClickAndDrag(x,y);
  else if(recentClickCount == 2)
    DoubleClickAndDrag(x,y);
  else
    TripleClickAndDrag(x,y);
}


GestureHandler::GestureHandler() : recentClickCount(0), 
    waitingForMouseUpTimeout(false),
    waitingForGestureTimeout(false), isHolding(false), isDragging(false) {}

GestureHandler::~GestureHandler()
{ 
  stopTimer();
}


void GestureHandler::timerCallback(void)
{
  stopTimer(); //Stop the timer from recurring.
  
  if(waitingForMouseUpTimeout)
  {
    //A hold event is taking place.
    waitingForMouseUpTimeout = false;
    isHolding = true;
    Holding(recentX,recentY);
  }
  else if(waitingForGestureTimeout)
  {
    waitingForGestureTimeout = false;
    //The gesture has ended.
    recentClickCount=0;
  }
  else if(waitingForMouseHold && mouseTracker.n() >= 1)
  {
    //Calculate the deviation.
    MouseTrack* gmt1 = &mouseTracker.first();
    MouseTrack* gmt2 = &mouseTracker.last();

    prim::number DeltaX = (prim::number)(gmt2->x - gmt1->x);
    prim::number DeltaY = (prim::number)(gmt2->y - gmt1->y);
    prim::number Deviation = DeltaX * DeltaX + DeltaY * DeltaY;
    const prim::number DeviationLimit = 10.0; //CONSTANT
    if(Deviation <= DeviationLimit * DeviationLimit)
    {
      waitingForMouseHold = false;
      Pausing(mouseTracker.last().x,
        mouseTracker.last().y);
    }
    else
      startTimer((int)(1.5f * 1000.0f)); //CONSTANT
  }
}

void GestureHandler::gestureMouseDown(const juce::MouseEvent &e)
{
  /*Step 1: Clear the mouse tracker and stop the timer for the tracker if it 
   was running.*/
  
  if(recentClickCount==0)
    if(isTimerRunning())
      stopTimer();
  
  mouseTracker.RemoveAll();
  waitingForMouseHold = false;
  
  //Step 2: Increment the click counter.
  recentClickCount++;
  
  //Step 3: Handle the most recent click.
  switch(recentClickCount)
  {
    case 1:
      firstX=e.x;
      firstY=e.y;
      Click(firstX,firstY);
      break;
    case 2:
      DoubleClick(firstX,firstY);
      break;
    case 3:
      TripleClick(firstX,firstY);
  }
  
  /*Step 3: Start a timeout for the mouse up event.
   case a) Mouse up happens before timeout -- second timeout is started.
   i) Second timeout occurs after up -- gesture is over.
   ii)Mouse goes down before second timeout -- click is added.
   case b) Hold/drag takes place until mouse up, and then the gesture 
   finishes with mouse up.*/
  if(waitingForGestureTimeout)
  {
    //We've added another click and the gesture is continuing.
    waitingForGestureTimeout = false;
  }
  
  startTimer(1400); //CONSTANT (Note: anything marked CONSTANT can be changed.)
  waitingForMouseUpTimeout = true;
}

void GestureHandler::gestureMouseUp(const juce::MouseEvent &e)
{
  if(waitingForMouseUpTimeout)
  {
    waitingForMouseUpTimeout = false;
    
    //Start the second timer.
    startTimer(400); //CONSTANT 
    waitingForGestureTimeout = true;
  }
  if(isHolding)
  {
    //Entire gesture is over.
    isHolding=false;
    recentClickCount=0;
  }
  
  isDragging=false;
}

void GestureHandler::gestureMouseExit(const juce::MouseEvent &e)
{
  if(recentClickCount == 0)
  {
    if(isTimerRunning())
      stopTimer();
    mouseTracker.RemoveAll();
    waitingForMouseHold=false;
  }
}

void GestureHandler::gestureMouseMove(const juce::MouseEvent &e)
{
  /*Mouse tracking must be very passive. We will check to see if the user has 
   clicked recently first to avoid clashing with other gestures.*/
  juce::ModifierKeys mouse;
  
  recentX = e.x;
  recentY = e.y;
  
  if(recentClickCount == 0 && 
     !waitingForGestureTimeout && 
     !waitingForMouseUpTimeout)
  {
    waitingForMouseHold=true;
    
    //If a timer hasn't been started yet then start the timer.
    if(!isTimerRunning())
      startTimer((int)(1.5f * 1000.0f)); //CONSTANT (change this)
    
    MouseTrack* gmt = &mouseTracker.Add();
    gmt->x = e.x;
    gmt->y = e.y;
    gmt->time = 
    (prim::number)juce::Time::highResolutionTicksToSeconds(
                                                           juce::Time::getHighResolutionTicks());
    
    //Remove any events that are past the interval.
    for(prim::count i = 0; i < mouseTracker.n() - 1; i++)
    {
      if((gmt->time - mouseTracker[i].time) > 1.5) //CONSTANT
      {
        mouseTracker.Remove(i);
        i--; //Decrement to offset the anticipated increment.
      }
      else
      {
        //Calculate the deviation.
        prim::number DeltaX = (prim::number)(gmt->x - mouseTracker[i].x);
        prim::number DeltaY = (prim::number)(gmt->y - mouseTracker[i].y);
        prim::number Deviation = DeltaX * DeltaX + DeltaY * DeltaY;
        const prim::number DeviationLimit = 10.0; //CONSTANT
        
        if(Deviation > DeviationLimit*DeviationLimit)
        {
          mouseTracker.Remove(i);
          i--;
          startTimer((int)(1.5f * 1000.0f)); //CONSTANT
        }
      }
    }
  }
}

void GestureHandler::gestureMouseDrag(const juce::MouseEvent &e)
{
  if(isDragging || e.getDistanceFromDragStart() > 10) //CONSTANT
  {
    isDragging = true;
    
    if(waitingForMouseUpTimeout)
    {
      waitingForMouseUpTimeout = false;
      isHolding = true;
      Dragging(firstX, firstY);
    }
  }
}



