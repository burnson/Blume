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

#include "Representation.h"

#include "Elements.h"

Representation::Representation() {}

Representation::~Representation() {}

void Representation::createDefaultDocument(void)
{
  delete Root;
  Root = new Container;
  Section* s = new Section(0);
  s->segments = 4;
  s->scalarHeight = 1.0f;
  s->scalarAccelerando = 0.0f;
  Root->AddObject(s);
}

prim::XML::Element* Representation::CreateRootElement(prim::String& RootTagName)
{
  return new Container;
}

void Representation::WriteHeader(prim::String& XMLOutput)
{
  XMLOutput += "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}

void Representation::toString(prim::String& destination)
{
  destination.Clear();
  WriteToString(destination);
}

prim::XML::Parser::Error Representation::fromString(prim::String& source)
{
  return ParseDocument(source);
}

Representation::Container* Representation::getContainer(void)
{
  return dynamic_cast<Container*>(Root);
}
