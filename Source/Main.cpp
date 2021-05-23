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

//--------------------//
//Visual Leak Detector//
//--------------------//
//#include "vld.h" //Memory-leak detection on Windows

//--------//
//Includes//
//--------//
#include "Main.h"

#include "Globals.h"
#include "Window.h"

///Initializes the application with a splash screen and a single window.
void Blume::initialise(const juce::String& commandLine)
{
  using namespace juce;

  //Create the application-wide global object.
  if(!globals)
  {
    globals = new Globals(this);
    new Window(new Document::Initialization());
  }
}

///Deletes all the application-wide objects.
void Blume::shutdown()
{
  delete globals;
}

///Removes all windows from view and goes through the quitting procedure.
void Blume::systemRequestedQuit(void)
{
  Window::closeAllWindows();
  //juce::JUCEApplication::quit();
}

///Occurs when the user attempts to run another instance of the application.
void Blume::anotherInstanceStarted(const juce::String& commandLine)
{
  //Create the application-wide global object.
  if(!globals)
    globals = new Globals(this);

  juce::StringArray droppedfiles;
  droppedfiles.addTokens(commandLine, true);
  int size = droppedfiles.size();
  if(size == 0)
  {
    new Window(new Document::Initialization());
    return;
  }

  for(int i = 0; i < size; i++)
    droppedfiles.set(i, droppedfiles[i].unquoted());

	for(int i = 0; i < droppedfiles.size(); i++)
  {
    juce::String j_file = droppedfiles[i];
    prim::String file = j_file.toRawUTF8();
    prim::String metadata;
    
    int startIndex = j_file.length() - 4;
    if(startIndex > 0)
    {
      juce::String extension = j_file.substring(startIndex).toLowerCase();
      if(extension == juce::String(".pdf"))
        abcd::PDF::RetrievePDFMetadataAsString(file, metadata);
      else if(extension == juce::String(".xml"))
        prim::File::ReadAsUTF8(file, metadata);
    }
      
    if(metadata.n())
      new Window(new Document::Initialization(file, metadata));
  }
}

//Start Blume using the JUCE macro.
START_JUCE_APPLICATION(Blume)
