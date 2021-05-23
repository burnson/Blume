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

#ifndef Viewer_h
#define Viewer_h

#include "Document.h"

struct Page;

struct Viewer : public DocumentHandler
{
  struct LayoutType
  {
    ///Pages sit side by side and the user drags them across the screen.
    static const prim::count SideBySide = 1;
  };

  ///Zoom level normalized around one. 1.0 = 100%, 0.5 = 50%, etc.
  prim::number percentageZoom;

  ///A pointer to the current page.
  Page* focusPage;
  
  ///Used in case focusPage is no longer valid.
  prim::count focusIndex;

  ///On a 0.0 to 1.0 normalized scale.
  prim::math::Vector focusPageRelative;

  ///On a 0.0 to 1.0 normalized scale.
  prim::math::Vector focusScreenRelative;

  /**Uses the information stored in the data members of this class to position 
  the pages on the screen. With optimize on, it intelligently moves the pages
  so that the most page fits on the screen. By definition it does not remove
  anything from the screen, and it may only increase the amount of page seen on
  the screen. This commonly occurs when the user drags the page "too far" and
  leaves a lot of blank space which could otherwise show more information.*/
  void positionPages(bool optimize = false);

  Viewer(Document* document);
};

#endif
