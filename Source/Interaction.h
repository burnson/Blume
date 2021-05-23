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

#ifndef Interaction_h
#define Interaction_h

#include "Document.h"

#include "Elements.h"

struct Interaction
{
  prim::math::Vector position;
  
  Representation::Section* section;
  prim::count segment;
  
  enum InteractionType
  {
    MainSectionPosition,
    MainSectionWidth,
    SectionHeight,
    SectionAccelerandoLeft,
    CreateSection,
    DeleteSection,
    ChangeMainSectionSegments
  };
  
  InteractionType type;
  
  prim::number radius;
  
  bool drawCross;
  bool drawCircle;
  bool drawRectangle;

  Interaction(prim::math::Vector p, Representation::Section* s,
    InteractionType t, prim::number r, bool cross = false,
    bool circle = false, bool rectangle = false, prim::count seg = 0);
    
  Interaction();
};
#endif
