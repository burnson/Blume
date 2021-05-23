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

#ifndef Dialogs_h
#define Dialogs_h

#include "Libraries.h"

struct ValueChooserComponent : public juce::Component,
  public juce::TextEditorListener
{
  juce::TextEditor* textEditor;
  juce::Label* labelInfo;
  static prim::number lastValueReturned;
  static bool valueIsValid;
  prim::number min;
  prim::number max;
  
  void parentHierarchyChanged(void);
  void textEditorReturnKeyPressed(juce::TextEditor &editor);
  void textEditorEscapeKeyPressed(juce::TextEditor &editor);
  void textEditorTextChanged(juce::TextEditor &editor);
  void textEditorFocusLost(juce::TextEditor &editor);
  void paint(juce::Graphics& g);
  
  ValueChooserComponent(const juce::String& label,
    prim::number initialValue, prim::number minValue, prim::number maxValue);
  ~ValueChooserComponent();
};

struct ValueChooser : public juce::DialogWindow
{
  void closeButtonPressed(void);
  ValueChooser(const juce::String& title, const juce::String& label, 
    prim::number initialValue, prim::number minValue, prim::number maxValue);
};

#endif
