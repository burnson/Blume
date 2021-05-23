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

#ifndef Globals_h
#define Globals_h

#include "Libraries.h"

//Required include since Globals lives in Blume.
#include "Main.h"

//Forward declaration...
struct Window;

///A class containing application-wide objects and default values.
struct Blume::Globals
{
  Blume* application;

  prim::number inDisplayDiagonalSize;
  prim::integer pixelsEdgeOfPageToDisplay;
  prim::integer pixelsHorizontalDistanceBetweenPages;
  
  prim::List<Window*> listWindows;

  juce::ApplicationCommandManager* commandManager;

  void addWindow(Window* window);
  
  static const int toolbarHeight;
  
  bool dontLetApplicationQuitBecauseOfNoWindowsOpen;
  
  Globals(Blume* ptrApplication);
  ~Globals()
  {
    //Delete the application command manager.
    delete commandManager;
  }
};

Blume::Globals* globals(void);

#endif
