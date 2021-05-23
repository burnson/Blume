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

#ifndef Document_h
#define Document_h

#include "Libraries.h"

//Forward includes...
#include "Representation.h"

//Forward declarations...
struct Content;
struct Interaction;
struct Page;
struct Window;
struct Viewer;

namespace notation {struct Score;}

/**The Document class stores all information related to a single window. It 
contains pointers to all the objects held by a window, and, most importantly, a
window and its document are inextricably tied for their lifetimes, so you can
not change the pointer held by the window to the document. You access the 
information in Document by deriving a class from DocumentHandler. By doing this
your derived class immediately will have access to every nook and cranny of the
Blume Document Object Model. Be careful to not cache pointers to any Document
objects indefinitely, as they may change (i.e. during editing). If you decide to
add something to the Document class, make sure to delete it in the proper order
in the Document destructor. For creating the object, add a createObject method
in the Document class and add a getObject method for others to access. Be sure
to make the pointer private so that it is not possible for any other part of the
program to make accidental changes.*/
struct Document
{
public:
  ///A structure to hold initialization information for the Document.
  struct Initialization
  {
    bool createdFromEmptyDocument;

    prim::String sourceFilename;
    prim::String metadata;

    Initialization();
    Initialization(const prim::String& sourceFile, const prim::String& data);
  };

  bool willCancelDocument;
  
  bool temporarilyHideHandles;
  bool useInches;
  bool showGrid;
  bool showFineGrid;
  
  prim::String filename;
  
  Content* content;
  Initialization* initialization;
  Window* window;
  notation::Score* score;
  Viewer* viewer;
  prim::List<Page*> pages;
  Representation* representation;
  prim::List<Interaction*> interactions;

  Document(Initialization* initialization);
  ~Document();
};

///This base class is added to other classes that need access to Document.
class DocumentHandler
{
private: 
  Document* document; //Deriving classes must use getDocument()
public:
  //-----------//
  //Constructor//
  //-----------//
  DocumentHandler(Document* document);
  
  ///Provides access to the relevant window's Document.
  Document* getDocument(void);
  
  //Other accessors
  Content* getContent(void){return document->content;}
  Document::Initialization* getInitialization(void)
    {return document->initialization;}
  notation::Score* getScore(void){return document->score;}
  Viewer* getViewer(void){return document->viewer;}
  Window* getWindow(void){return document->window;}
  
  prim::count getPageCount(void){return document->pages.n();}
  prim::List<Page*>& getPages(void){return document->pages;}
  Page* getPage(prim::count i){return document->pages[i];}
  
  prim::count getCanvasCount(void);
  prim::List<bbs::abstracts::Portfolio::Canvas*>& getCanvases(void);
  bbs::abstracts::Portfolio::Canvas* getCanvas(prim::count i);
  
  Representation* getRepresentation(void){return document->representation;}
  Representation::Container* getContainer(void);
  
  void setDocumentTitle(prim::String title);
  
  prim::List<Interaction*>& getInteractions(void)
    {return document->interactions;}
};
#endif
