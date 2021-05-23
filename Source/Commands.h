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

#ifndef Commands_h
#define Commands_h

#include "Document.h"

#include "Globals.h"

class Commands : public juce::ApplicationCommandTarget, public DocumentHandler
{
public:
  Commands(Document* document) : DocumentHandler(document) {}
  juce::ApplicationCommandTarget* getNextCommandTarget(void){return 0;}

  void getAllCommands(juce::Array<juce::CommandID>& commands);

  juce::String getCommandNameFromID(const juce::CommandID id);
  static const juce::String getHelpTopic(int id);
  
  void getShortcuts(const juce::CommandID id, 
    juce::ApplicationCommandInfo& info);

  void getCommandInfo(const juce::CommandID id,
    juce::ApplicationCommandInfo& info);

  bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info);

  //--------//
  //Commands//
  //--------//
  /*If you add or change commands, make sure to edit these places:
      - The enum CommandIDs below
      - In Commands.cpp: getAllCommands, getCommandNameFromID, getShortcuts,
      getCommandInfo, and perform.
      - In Menu.cpp: add the menu item in getMenuForIndex

    If you change the text name of a menu item be sure to change its identifier
    name here, so that things stay consistent. For this a global search and 
    replace with matching case and word is fine.
    
    Also be sure to present them in menu order in all of these places.*/
  enum CommandIDs
  {
    FileNewWindow        = 0x10100,
    FileOpen             = 0x10200,
    FileCloseWindow      = 0x10300,
    FileSave             = 0x10400,
    FileSaveAs           = 0x10500,
    FileSaveAsXML        = 0x10600,
    FileRevertToOriginal = 0x10700,
    FileQuit             = 0x10800,
    
    ViewUseCentimeters   = 0x20100,
    ViewUseInches        = 0x20200,
    ViewShowCoarseGrid   = 0x20300,
    ViewShowFineGrid     = 0x20400,
    
    PageLandscape        = 0x30100,
    PagePortrait         = 0x30200,
    PageLetter           = 0x30300,
    PageA4               = 0x30400,
    PageB4               = 0x30500,
    PageA3               = 0x30600,
    PageTabloid          = 0x30700,
    PageB3               = 0x30800,
    PageCustomSize       = 0x30900
  };
};

#endif
