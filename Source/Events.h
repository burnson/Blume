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
 
#ifndef Events_h
#define Events_h

#include "Document.h"

#include "Interaction.h"

//Forward Declarations
struct Page;
struct Interaction;

struct Tooltip : public juce::Component
{
  prim::String text;
  
  void showTip(int x, int y);
  void hideTip(void);
  void paint(juce::Graphics& g);
};

class EventHandler : public DocumentHandler
{
private:
  bool isRunning;

protected:
  EventHandler** ptrEventHandler;
  Page* page;

  virtual void stopEvent(void);

public:
  EventHandler(Document* document);

  void beginEvent(
    Page* page,
    EventHandler** ptrCurrentEventHandler,
    prim::integer beginX, prim::integer beginY);

  virtual void handler(prim::integer beginX, prim::integer beginY) = 0;
  virtual void cancel(void);

  virtual void mouseDown(const juce::MouseEvent &e);
  virtual void mouseUp(const juce::MouseEvent &e);
  virtual void mouseMove(const juce::MouseEvent &e);
  virtual void mouseDrag(const juce::MouseEvent &e);
  virtual void mouseExit(const juce::MouseEvent &e);
  virtual void keyPress(const juce::KeyPress &k);
  virtual ~EventHandler() {}
};

class EventDragHandle : public EventHandler
{
public:
  Interaction interaction;
  prim::integer anchorX;
  prim::integer anchorY;
  prim::count originalSegments;
  Tooltip* tooltip;

  EventDragHandle(Document* document);
  virtual void handler(prim::integer beginX, prim::integer beginY);
  virtual void mouseDrag(const juce::MouseEvent &e);
  virtual void mouseUp(const juce::MouseEvent &e);
  virtual ~EventDragHandle() {}
};

class EventDragScore : public EventHandler
{
  prim::number oldZoom;
  prim::integer anchorX;
  prim::integer anchorY;
public:
  EventDragScore(Document* document);
  virtual void handler(prim::integer beginX, prim::integer beginY);
  virtual void mouseDrag(const juce::MouseEvent &e);
  virtual void mouseUp(const juce::MouseEvent &e);
  virtual ~EventDragScore() {}
};

class EventZoomScore : public EventHandler
{
  prim::number oldZoom;
  prim::integer anchorX;
  prim::integer anchorY;
public:
  EventZoomScore(Document* document);
  virtual void handler(prim::integer beginX, prim::integer beginY);
  virtual void mouseDrag(const juce::MouseEvent &e);
  virtual void mouseUp(const juce::MouseEvent &e);
  virtual ~EventZoomScore() {}
};
#endif
