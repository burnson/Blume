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

#ifndef Score_h
#define Score_h

#include "Document.h"

namespace notation
{
  struct Score : public bbs::abstracts::Portfolio, public DocumentHandler
  {
    Score(Document* Document);
    ~Score();

    struct Page : public Portfolio::Canvas, public DocumentHandler
    {
      Score* score;

      Page(Document* document, Score& score);
      
      void PaintSection(bbs::abstracts::Painter* Painter,
        Representation::Section* section,
        prim::number width, prim::number height, prim::number xOffset, bool up,
        prim::number beamSlant, prim::count recursion);

      void DrawGridlines(bbs::abstracts::Painter* Painter);
      
      void DrawHandles(bbs::abstracts::Painter* Painter);

      void Paint(bbs::abstracts::Painter* Painter);
    };
  };
}

#endif
