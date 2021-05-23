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

#include "Window.h"

#include "Content.h"
#include "Globals.h"

//----------------------//
//Constructor/Destructor//
//----------------------//

Window::Window(Document::Initialization* initialization)
  : DocumentWindow("", juce::Colours::lightgrey,
    juce::DocumentWindow::allButtons, true),
    Commands(new Document(initialization))
{
  //Set this as the document window.
  getDocument()->window = this;
  
  //Set the document title.
  prim::String title = "Untitled ";
  static prim::integer titleIndex = 0;
  title &= ++titleIndex;
  
  if(!initialization->sourceFilename)
    setDocumentTitle(title);
  else
  {
    juce::File f(initialization->sourceFilename.Merge());
    setDocumentTitle(f.getFileNameWithoutExtension().toRawUTF8());
  }
  
  //Add the window to the application window list.
  globals()->listWindows.Add() = this;

  //Register the menu commands.
  globals()->commandManager->clearCommands();
  globals()->commandManager->registerAllCommandsForTarget(this);

  //Create a key listener on the content component to listen for key mappings.
  addKeyListener(globals()->commandManager->getKeyMappings());

  //Add the menu bar to the content component.
#ifdef JUCE_MAC
  setMacMainMenu(this);
#else
  setMenuBar(this);
#endif

  //Create the content component for the window.
  new Content(getDocument());
  getDocument()->content->setSize(800, 600);

  //Set the content component for this window.
  setContentOwned(getDocument()->content, true);

  //Do cascading windows.
  {
    int baseWidth = 850;
    int baseHeight = 600;
    int baseShift = 30;
    if(prim::OS::MacOSX())
      baseShift = 22;
    int baseStartX = 20;
    int baseStartY = 50;
    int baseXArea = 300;
    int baseYArea = 150;
    static int baseIndex = 0;
    int newX, newY;
    newX = baseStartX + (baseIndex * baseShift) % (baseXArea - baseStartX);
    newY = baseStartY + (baseIndex * baseShift) % (baseYArea - baseStartY);
    baseIndex++;
    setBounds(newX, newY, baseWidth, baseHeight);
  }
    
  //Set window properties.
  setResizable(true, false);
  setUsingNativeTitleBar(true);
  setDropShadowEnabled(prim::OS::MacOSX());
  setVisible(true);
}

Window::~Window()
{
#ifdef JUCE_MAC
  setMacMainMenu(0);
#else
  setMenuBar(0);
#endif
}

//------//
//Events//
//------//

void Window::closeButtonPressed(void)
{
  close();
}

//-----------------//
//Window Management//
//-----------------//

void Window::close(void)
{
  using namespace prim;

  /*  
  //See if the user needs to save.
  if(forceSaveDialog)
  {
    //Select global namespace to get File and not prim::File.
    ::File file(getDocument());
    file.makePDFScore();
  }
  else if(getScore()->settings.hasBeenEdited && !skipSaveDialog)
  {
    //Bring the window to the foreground.
    toFront(true);

    //Ask whether or not to save changes.
    juce::String Notice = "Changes have been made to the score. ";
    Notice << "Would you like to save them?";
    if(Alerts::showOkCancelBox(juce::AlertWindow::QuestionIcon,
      juce::String("Save Changes"),
      juce::String(Notice),
      juce::String("Save Changes"),juce::String("Discard Changes")))
    {
      ::File file(getDocument());
      file.makePDFScore();
    }
  }
  */

  //Remove the window from the list.
  List<Window*>& listWindows = globals()->listWindows;
  for(count i = 0; i < listWindows.n(); i++)
  {
    if(listWindows[i] == this)
    {
      listWindows.Remove(i);            
      break;
    }
  }
  
  //Delete the window.
  delete this;
  
#ifdef JUCE_MAC
  //Refresh the Mac OS X main menu.
  if(globals()->listWindows.n() > 0)
  {
    globals()->listWindows[globals()->listWindows.n() - 1]->grabKeyboardFocus();
    Window* nextWindow = dynamic_cast<Window*>(
      juce::TopLevelWindow::getActiveTopLevelWindow());
    if(nextWindow)
      setMacMainMenu(nextWindow);
  }
#endif
  
  //Quit application if no windows are open.
  if(globals()->dontLetApplicationQuitBecauseOfNoWindowsOpen)
    return;
  if(!listWindows.n())
  {
    #ifdef JUCE_MAC
        setMacMainMenu(0);
    #else
        setMenuBar(0);
    #endif
    juce::JUCEApplication::getInstance()->quit();
  }
}

void Window::closeAllWindows(void)
{
  //Close each window in the application list of windows.
  while(globals()->listWindows.n())
    globals()->listWindows.first()->close();
}
