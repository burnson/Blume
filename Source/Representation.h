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

#ifndef Representation_h
#define Representation_h

#include "Libraries.h"

struct Representation : protected prim::XML::Document
{
  //Forward declarations...
  struct Container;
  struct Placement;
  struct SATB;
  struct Section;

  ///Default constructor which does not create elements.
  Representation();

  ///Destructor to clean up and release memory.
  ~Representation();

  /**Creates a document with all the appropriate objects with their default
  values.*/
  void createDefaultDocument(void);

  ///Virtual overload which instantiates the Container.
  prim::XML::Element* CreateRootElement(prim::String& RootTagName);

  ///Virtual overload which writes the appropriate header to the XML data.
  void WriteHeader(prim::String& XMLOutput);

  ///Outputs the representation to an XML string.
  void toString(prim::String& destination);

  ///Parses the representation from an XML string.
  prim::XML::Parser::Error fromString(prim::String& source);

  ///Gets a pointer to the container object.
  Container* getContainer(void);
};

#endif
