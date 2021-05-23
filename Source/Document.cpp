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

#include "Document.h"

#include "Interaction.h"
#include "Representation.h"
#include "Score.h"
#include "Viewer.h"
#include "Window.h"

Document::Initialization::Initialization()
{
  createdFromEmptyDocument = true;
}

Document::Initialization::Initialization(const prim::String& sourceFile,
  const prim::String& data)
{
  createdFromEmptyDocument = false;
  metadata = data;
  sourceFilename = sourceFile;
}

Document::Document(Document::Initialization* init) :
  willCancelDocument(false),
  temporarilyHideHandles(false),
  useInches(false),
  showGrid(false),
  showFineGrid(false),
  content(0),
  initialization(init),
  window(0),
  score(0),
  viewer(0),
  representation(new Representation)
{
  score = new notation::Score(this);
  viewer = new Viewer(this);

  if(init->metadata.n())
    representation->fromString(init->metadata);
  else
    representation->createDefaultDocument();
  filename = init->sourceFilename;
}

Document::~Document()
{
  delete initialization;
  interactions.RemoveAndDeleteAll();
}

DocumentHandler::DocumentHandler(Document* document)
{
  DocumentHandler::document = document;
}

Document* DocumentHandler::getDocument()
{
  return document;
}

prim::count DocumentHandler::getCanvasCount(void)
{
  return document->score->Canvases.n();
}

prim::List<bbs::abstracts::Portfolio::Canvas*>&
  DocumentHandler::getCanvases(void)
{
  return document->score->Canvases;
}

bbs::abstracts::Portfolio::Canvas* DocumentHandler::getCanvas(prim::count i)
{
  return document->score->Canvases[i];
}

void DocumentHandler::setDocumentTitle(prim::String title)
{
  prim::String windowTitle = "Blume - ";
  windowTitle &= title;
  getWindow()->setName(windowTitle.Merge());
}

Representation::Container* DocumentHandler::getContainer(void)
{
  return document->representation->getContainer();
}
