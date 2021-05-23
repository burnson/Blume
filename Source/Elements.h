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


#ifndef Elements_h
#define Elements_h

#include "Representation.h"

//---------//
//Container//
//---------//
///The top-level node of the Representation XML structure.
struct Representation::Container : public prim::XML::Element
{
  //Data
  prim::String title;
  prim::math::Inches sizePage;
  prim::math::Inches sizeMainSection;
  prim::math::Inches offsetMainSection;
  prim::math::Inches sizeGrid;
  prim::math::Inches sizeSubgrid;
  prim::number scalarBeamSlant;
  
  ///Default constructor (does not create child elements).
  Container();

  //XML Callbacks
  prim::XML::Element* CreateChild(const prim::String& TagName);
  void Translate(void);
  bool Interpret(void);
};

//-------//
//Section//
//-------//
struct Representation::Section : public prim::XML::Element
{
  //Data
  Representation::Section* parentSection;
  prim::count parentSegment;
  prim::count segments;
  prim::number scalarHeight;
  prim::number scalarAccelerando;
  
  //Cached data
  prim::number cachedWidth;
  prim::number cachedHeight;
  prim::math::Vector cachedBottomLeft;
  prim::count cachedRecurseDepth;
  prim::number cachedExponentialScale;
  
  //Constructor
  Section(Section* parentSection);
  
  void Remove(void);
  
  //XML Callbacks
  prim::XML::Element* CreateChild(const prim::String& TagName);
  void Translate(void);
  bool Interpret(void);
};
#endif
