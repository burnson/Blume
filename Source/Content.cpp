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
 
#include "Content.h"
#include "Elements.h"
#include "Page.h"
#include "Score.h"
#include "Viewer.h"

Content::Content(Document* document) : DocumentHandler(document)
{
  //Cancel if something went wrong.
  if(getDocument()->willCancelDocument)
    return;

  //Set the content as soon as we have it.
  getDocument()->content = this;
  
  getPages().Add() = new Page(getDocument());
  bbs::abstracts::Portfolio::Canvas* c = 
    getCanvases().Add() = new notation::Score::Page(getDocument(), *getScore());
  c->Dimensions = getContainer()->sizePage;
  getViewer()->positionPages(true);
  
  addAndMakeVisible(getPage(0));
}

Content::~Content()
{
  getPages().RemoveAndDeleteAll();
}

void Content::mouseMove(const juce::MouseEvent &e)
{
  if(!getDocument()->temporarilyHideHandles)
  {
    getDocument()->temporarilyHideHandles = true;
    getPage(0)->repaint();
  }
}

void Content::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::azure.darker(0.02f));
}

void Content::resized(void)
{
  getViewer()->positionPages(true);
}
