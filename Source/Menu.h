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

#ifndef Menu_h
#define Menu_h

#include "Libraries.h"

//#include "Document.h"

class Menu : public juce::MenuBarModel
{
  //Document* document;
public:
  //----------------------//
  //Constructor/Destructor//
  //----------------------//
  Menu(/*Document* document*/);

  //---------------//
  //Menubar Targets//
  //---------------//
  juce::StringArray getMenuBarNames(void);
  juce::PopupMenu getMenuForIndex(int index, const juce::String& name);
  void menuItemSelected(int id, int topLevelMenuIndex){}
};

#endif
