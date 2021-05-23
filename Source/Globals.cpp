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

#include "Globals.h"

#include "Window.h"

//---------//
//Constants//
//---------//
const int Blume::Globals::toolbarHeight = 44;

///Initializes application-wide globals with their appropriate defaults.
Blume::Globals::Globals(Blume* ptrApplication) : 
  application(ptrApplication),
  inDisplayDiagonalSize(24.0f),
  pixelsEdgeOfPageToDisplay(5),
  pixelsHorizontalDistanceBetweenPages(5),
  commandManager(0),
  dontLetApplicationQuitBecauseOfNoWindowsOpen(false)
{
  //Set the pointer to this globals class so other initialization can use it.
  application->globals = this;

  //Create an application command manager and register commands for it.
  commandManager = new juce::ApplicationCommandManager;
  commandManager->registerAllCommandsForTarget(application->getInstance());
}

Blume::Globals* globals(void)
{
  Blume* blume = 
    dynamic_cast<Blume*>(juce::JUCEApplication::getInstance());
  return blume->globals;
}

void Blume::Globals::addWindow(Window* window)
{
  listWindows.Add() = window;
}
