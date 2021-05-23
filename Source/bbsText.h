/*
  ==============================================================================

   This file is part of the Belle, Bonne, Sage library
   Copyright 2007-2010 by Andi Brae

  ------------------------------------------------------------------------------

   Belle, Bonne, Sage can be redistributed and/or modified under the terms of
   the GNU Lesser General Public License, as published by the
   Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   Belle, Bonne, Sage is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with Belle, Bonne, Sage; if not, visit www.gnu.org/licenses or write:
   
   Free Software Foundation, Inc.
   59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ==============================================================================
*/

#ifndef bbsText
#define bbsText

#include "prim.h"
#include "bbsGlyph.h"

namespace bbs
{
  struct Text
  {
    /**Determines where and how to draw unformatted text. Single line and 
    multiline text is supported with full justification available. When 
    DrawVectorText is called with unformatted text, it modifies the structure 
    after drawing to update it to the position of the next paragraph, so you can 
    make multiple paragraphs using one call per paragraph.*/
    struct Unformatted
    {
      ///Font to use.
      bbs::VectorFont* Font;

      ///Position of text from which justification is relative.
      prim::math::Vector unitsPosition;

      /**The size of the font in the units of the parent space.*/
      prim::number unitsFontSize;

      /**Anchor of text bounding box relative to position on a [-1, 1] scale.
      For example, if the text is anchored to <0, 0> then the position vector 
      indicates the center of the text. If the text is anchored <-1, -1>, then 
      the bottom-left corner of the text meets the position vector (text goes up
      and to the right). Values outside of the range [-1, 1] are also 
      acceptable. For a symbol font, leave the justification set to <-1, -1>
      (in other words glyphs generally occupy the space in quadrant 1.*/
      prim::math::Vector relativeAnchor;

      /**Justification of the text within the bounding box. If the line width
      is set to zero (for drawing a single line of text), then this has no
      effect, and you should use the anchor vector to accomplish justification
      relative to the position vector.*/
      prim::Justification Justification;

      /**The maximum width of the line in the units of the parent space. If this
      value is zero then the text will occupy a single line.*/
      prim::number unitsLineWidth;

      /**Controls the amount of space between two adjacent lines. The actual
      height of the line is equivalent to this value multiplied by the font 
      height. The height of the font is equivalent to the average of the heights 
      of all the capitalized letters A through Z multiplied by the font size.
      For symbol fonts, there are never multiple lines, so this value is 
      irrelevant.*/
      prim::number percentageLineSpacing;

      /**Amount by which the first line of the text is indented. This only
      applies for multiline text (where line width has been set to something
      other than zero). To indent single line text, just change the position
      vector.*/
      prim::number unitsIndentation;

      ///Auto-substitutes the most common latin ligatures when available.
      bool UseLatinLigatures;

      /**Auto-substitutes left and right single or double quotation marks. This
      only occurs when the glyphs are available in the font.*/
      bool UseDirectionalQuotationMarks;

      ///Auto-substitutes en-dashes and em-dashes when available.
      bool UseDashes;

      /**Calculates the total line height in the units of the parent space. This
      is equivalent to the height of the capital letters in the parent space 
      multiplied by the line spacing percentage.*/
      prim::number CalculateTotalLineHeight(void)
      {
        if(!Font)
          return 0;
        else
        {
          return Font->GetFontHeight(false) * unitsFontSize * 
            percentageLineSpacing;
        }
      }

      ///Default constructor.
      Unformatted() : Font(0), unitsPosition(0, 0), unitsFontSize(1.0f),
        relativeAnchor(-1.0f, -1.0f), Justification(prim::Justifications::Left), 
        unitsLineWidth(0), percentageLineSpacing(1.0f), unitsIndentation(0),
        UseLatinLigatures(true), UseDirectionalQuotationMarks(true), 
        UseDashes(true) {}

      ///Constructor that allows you to specify the unformatted text's style.
      Unformatted(
        VectorFont* Font,
        prim::math::Vector unitsPosition,
        prim::number unitsFontSize = 1.0f,
        prim::number unitsLineWidth = 0,
        prim::math::Vector relativeAnchor = prim::math::Vector(-1.0f, -1.0f), 
        prim::Justification Justification = prim::Justifications::Left,
        prim::number percentageLineSpacing = 1.0f,
        prim::number unitsIndentation = 0,
        bool UseLatinLigatures = true,
        bool UseDirectionalQuotationMarks = true,
        bool UseDashes = true)
      {
        Unformatted::Font = Font;
        Unformatted::unitsPosition = unitsPosition;
        Unformatted::unitsFontSize = unitsFontSize;
        Unformatted::unitsLineWidth = unitsLineWidth;
        Unformatted::relativeAnchor = relativeAnchor;
        Unformatted::Justification = Justification;
        Unformatted::percentageLineSpacing = percentageLineSpacing;
        Unformatted::unitsIndentation = unitsIndentation;
        Unformatted::UseLatinLigatures = UseLatinLigatures;
        Unformatted::UseDirectionalQuotationMarks = 
          UseDirectionalQuotationMarks;
        Unformatted::UseDashes = UseDashes;
      }
    };
  };
}
#endif
