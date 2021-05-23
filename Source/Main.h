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

#ifndef Main_h
#define Main_h

#include "Libraries.h"

///The class deriving from JUCEApplication which starts Blume.
class Blume : public juce::JUCEApplication
{
public:
  //Internal forward declarations...
  struct Globals; //Globals lives inside Blume.
  Globals* globals;

  //-----------------------//
  //Initialization/Shutdown//
  //-----------------------//
  Blume() : globals(0) {}
  ~Blume(){}
  void initialise (const juce::String& commandLine);
  void shutdown();

  //------------------//
  //Application Events//
  //------------------//
  void systemRequestedQuit(void);
  void anotherInstanceStarted(const juce::String& commandLine);
  
  //---------------------//
  //Application Constants//
  //---------------------//
  const juce::String getApplicationName(void)
  {
    return "Blume";
  }

  const juce::String getApplicationVersion(void)
  {
    return "1.0";
  }

  //--------------------------//
  //Multiple Instance Checking//
  //--------------------------//
  bool moreThanOneInstanceAllowed(void)
  {
    /*Disable instance checking on Mac for now due to JUCE bug, but on Windows
    and Linux, prevent multiple instances from running and instead open a new
    window. Note that Mac OS X tends to not allow this anyway, so JUCE doesn't 
    really need to take care of it.*/
    return prim::OS::MacOSX();
  }
};

#endif
