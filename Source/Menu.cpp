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

#include "Menu.h"

#include "Commands.h"
#include "Globals.h"

//-----------------------//
//Top Level Menu Creation//
//-----------------------//

Menu::Menu(/*Document* document*/)
{
  //Initialize document pointer.
  //Menu::document = document;

  //Get a pointer to the application command manager.
  juce::ApplicationCommandManager* acm = globals()->commandManager;

  /*Tells the menu bar model that it should watch the application command 
  manager for changes, and send change messages accordingly.*/
  setApplicationCommandManagerToWatch(acm);
}

juce::StringArray Menu::getMenuBarNames(void)
{
  const char* topLevelMenus[4] = {"File", "View", "Page", 0};
  return juce::StringArray(topLevelMenus);
}

juce::PopupMenu Menu::getMenuForIndex(int index, 
  const juce::String& name)
{
  using namespace juce;

  PopupMenu menu;
  juce::ApplicationCommandManager* acm = globals()->commandManager;

  if(name == String("File"))
  {
    menu.addCommandItem(acm, Commands::FileNewWindow);
    menu.addSeparator();
    menu.addCommandItem(acm, Commands::FileCloseWindow);
    menu.addSeparator();
    menu.addCommandItem(acm, Commands::FileSave);
    menu.addCommandItem(acm, Commands::FileSaveAs);
    //menu.addCommandItem(acm, Commands::FileSaveAsXML);
    menu.addSeparator();
    menu.addCommandItem(acm, Commands::FileQuit);
  }
  else if(name == String("View"))
  {
    menu.addCommandItem(acm, Commands::ViewShowCoarseGrid);
    menu.addCommandItem(acm, Commands::ViewShowFineGrid);
    menu.addSeparator();  
    menu.addCommandItem(acm, Commands::ViewUseCentimeters);
    menu.addCommandItem(acm, Commands::ViewUseInches);
  }
  else if(name == String("Page"))
  {
    menu.addCommandItem(acm, Commands::PageLandscape);
    menu.addCommandItem(acm, Commands::PagePortrait);
    menu.addSeparator();
    menu.addCommandItem(acm, Commands::PageLetter);
    menu.addCommandItem(acm, Commands::PageA4);
    menu.addCommandItem(acm, Commands::PageB4);
    menu.addCommandItem(acm, Commands::PageA3);
    menu.addCommandItem(acm, Commands::PageTabloid);
    menu.addCommandItem(acm, Commands::PageB3);
  }
  
  return menu;
}
