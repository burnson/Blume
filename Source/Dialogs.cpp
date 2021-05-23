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

#include "Dialogs.h"

prim::number ValueChooserComponent::lastValueReturned = 0;
bool ValueChooserComponent::valueIsValid = false;

void ValueChooserComponent::textEditorReturnKeyPressed(juce::TextEditor &editor)
{
  prim::number n = (prim::number)editor.getText().getDoubleValue();
  if(n < min || n > max)
  {
    juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    return;
  }
  ValueChooserComponent::lastValueReturned = n;
  ValueChooserComponent::valueIsValid = true;
  getParentComponent()->exitModalState(1);
}

void ValueChooserComponent::textEditorEscapeKeyPressed(juce::TextEditor &editor)
{
  getParentComponent()->exitModalState(0);  
}
  
ValueChooserComponent::ValueChooserComponent(const juce::String& label,
  prim::number initialValue, prim::number minValue, prim::number maxValue)
{
  min = minValue;
  max = maxValue;
  int w = 250, h = 30;
  int tw = 50, th = h - 4;
  juce::Font f;
  tw = w - f.getStringWidth(label) - 15;
  setSize(w, h);
  textEditor = new juce::TextEditor;
  labelInfo = new juce::Label("Info", label);
  labelInfo->setBounds(0, 0, w, h);
  addAndMakeVisible(textEditor);
  addAndMakeVisible(labelInfo);   
  textEditor->setMultiLine(false);
  textEditor->addListener(this);
  textEditor->setSize(tw, th);
  textEditor->setBounds(250 - tw, h / 2 - th / 2, tw, th);
  prim::String s = initialValue;
  textEditor->setText(s.Merge());
  textEditor->setHighlightedRegion(juce::Range<int>(0, (int)s.n()));
  textEditor->setWantsKeyboardFocus(true);
  ValueChooserComponent::valueIsValid = false;
  ValueChooserComponent::lastValueReturned = 0;
}

ValueChooserComponent::~ValueChooserComponent()
{
  delete textEditor;
}

void ValueChooserComponent::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::gainsboro);
}

void ValueChooserComponent::textEditorTextChanged(juce::TextEditor &editor) {}

void ValueChooserComponent::textEditorFocusLost(juce::TextEditor &editor)
{
  textEditor->grabKeyboardFocus();
}

void ValueChooserComponent::parentHierarchyChanged(void)
{
  textEditor->grabKeyboardFocus();
}

ValueChooser::ValueChooser(const juce::String& title, const juce::String& label,
  prim::number initialValue, prim::number minValue, prim::number maxValue) :
  juce::DialogWindow(title, juce::Colours::lightgrey, true, true)
{
  setTitleBarButtonsRequired(4, true);
  setUsingNativeTitleBar(true);
  setContentOwned(new ValueChooserComponent(label, initialValue, minValue,
    maxValue), true);
  centreWithSize(250, 30);
  setVisible(true);
  runModalLoop();
  delete this;
}

void ValueChooser::closeButtonPressed(void)
{
  exitModalState(0);
}
