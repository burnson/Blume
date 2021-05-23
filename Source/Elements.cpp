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

#include "Elements.h"

//---------//
//Container//
//---------//
Representation::Container::Container() : prim::XML::Element("blume")
{
  using namespace prim;
  using namespace prim::math;
  
  title = "Untitled";
  sizePage = Millimeters(297.0f, 210.0f);
  sizeMainSection = Inches(9.0f, 1.4f);
  offsetMainSection = Inches(0.0f, 0.0f);
  sizeGrid = Millimeters(50.0f, 50.0f);
  sizeSubgrid = Millimeters(10.0f, 10.0f);
  scalarBeamSlant = 1.0f;
}

prim::XML::Element* Representation::Container::CreateChild(
  const prim::String& TagName)
{
  if(TagName == "section")
    return new Section(0);
  else
    return 0; 
}

void Representation::Container::Translate(void)
{
  using namespace prim;
  using namespace prim::XML;

  Attributes.RemoveAll();
  AddAttribute("title", title);
  AddAttribute("page-width", sizePage.x);
  AddAttribute("page-height", sizePage.y);
  AddAttribute("initial-width", sizeMainSection.x);
  AddAttribute("initial-height", sizeMainSection.y);
  AddAttribute("offset-x", offsetMainSection.x);
  AddAttribute("offset-y", offsetMainSection.y);
  AddAttribute("grid-width", sizeGrid.x);
  AddAttribute("grid-height", sizeGrid.y);
  AddAttribute("subgrid-width", sizeSubgrid.x);
  AddAttribute("subgrid-height", sizeSubgrid.y);
  AddAttribute("beam-slant", scalarBeamSlant);
  
  //for(count i = Objects.n() - 1; i >= 0; i--)
  //  if(Objects[i]->IsText())
  //    Objects.Remove(i);
  
  Element::Translate();
}

bool Representation::Container::Interpret(void)
{
  using namespace prim;
  using namespace prim::math;
  using namespace prim::XML;

  for(count i = Attributes.n() - 1; i >= 0; i--)
  {
    prim::String Name = Attributes[i].Name;
    prim::String Value = Attributes[i].Value;
    if(Name == "title")
    {
      title = Value;
      continue;
    }
    
    number v = StringToNumber(Value);
   
    if(Name == "page-width")
      sizePage.x = v;
    else if(Name == "page-height")
      sizePage.y = v;
    else if(Name == "initial-width")
      sizeMainSection.x = v;
    else if(Name == "initial-height")
      sizeMainSection.y = v;
    else if(Name == "offset-x")
      offsetMainSection.x = v;
    else if(Name == "offset-y")
      offsetMainSection.y = v;
    else if(Name == "grid-width")
      sizeGrid.x = v;
    else if(Name == "grid-height")
      sizeGrid.y = v;
    else if(Name == "subgrid-width")
      sizeSubgrid.x = v;
    else if(Name == "subgrid-height")
      sizeSubgrid.y = v;
    else if(Name == "beam-slant")
      scalarBeamSlant = v;
  }
  
  Element::Interpret();
  return true;
}

//-------//
//Section//
//-------//
Representation::Section::Section(Section* parent) :
  prim::XML::Element("section")
{
  parentSection = parent;
  parentSegment = 0;
  segments = 5;
  scalarHeight = 0.6f;
  scalarAccelerando = 0.0f;
}

prim::XML::Element* Representation::Section::CreateChild(
  const prim::String &TagName)
{
  if(TagName == "section")
    return new Section(this);
  else
    return 0;
}

void Representation::Section::Translate(void)
{
  using namespace prim;
  using namespace prim::math;
  using namespace prim::XML;

  Attributes.RemoveAll();
  
  AddAttribute("parent-segment", (integer)parentSegment);
  AddAttribute("segments", (integer)segments);
  AddAttribute("height-scalar", scalarHeight);
  AddAttribute("accelerando-scalar", scalarAccelerando);
  
  //for(count i = Objects.n() - 1; i >= 0; i--)
  //  if(Objects[i]->IsText())
  //    Objects.Remove(i);
  
  Element::Translate();
}

bool Representation::Section::Interpret(void)
{
  using namespace prim;
  using namespace prim::math;
  using namespace prim::XML;
  
  for(count i = Attributes.n() - 1; i >= 0; i--)
  {
    prim::String Name = Attributes[i].Name;
    number v = StringToNumber(Attributes[i].Value);

    if(Name == "parent-segment")
      parentSegment = (count)v;
    else if(Name == "segments")
      segments = (count)v;
    else if(Name == "height-scalar")
      scalarHeight = v;
    else if(Name == "accelerando-scalar")
      scalarAccelerando = v;
  }
  
  Element::Interpret();

  return true;
}

void Representation::Section::Remove(void)
{
  if(!parentSection)
    return;
    
  for(prim::count i = parentSection->Objects.n() - 1; i >= 0; i--)
  {
    if(parentSection->Objects[i]->IsElement() == this)
    {
      parentSection->Objects.RemoveAndDelete(i);
      break;
    }
  }
}
