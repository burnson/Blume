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

#ifndef bbsFont
#define bbsFont

#include "primTypeface.h"
#include "primTypes.h"
#include "primText.h"

namespace bbs
{
  /**\brief Font stores information about a font program already lying around
  somewhere in memory. \details Fonts occupy a tricky classification in the
  Belle, Bonne, Sage library, as each Medium must implement them, but there is
  no generic representation, and each implementation is different. Thankfully,
  the number of representations of fonts is finite, and in fact only a few are
  in widespread use: TrueType, PostScript, and their "universal" descendent,
  OpenType. The prim::typeface namespace is devoted to providing APIs for each
  DescriptionLanguage mentioned above.

  Font, like Belle, Bonne, Sage, is platform-independent. What this means
  practically-speaking is that instead of reading a font from the system cache
  of loaded fonts, you instead supply the font file itself. This approach is
  useful because some output mediums, such as PDF, require the font program
  itself for embedding purposes. Operating systems typically do not give you
  direct access to the font program; rather, they supply a higher level API
  for interacting with it in rendering operations. Also, on most operating
  systems it is possible to load a font from an arbitrary file into its font
  cache. Many programs do this such as PDF readers. Thus, storing the actual
  font program is generally useful for even specialized applications.*/

  class Font
  {
  public:
    ///Stores the font's format-dependent information.
    prim::typeface::DescriptionLanguage* Program;

    ///Remembers the source data so that the font may be easily embedded.
    const prim::byte* ProgramData;

    ///Remember the source data's byte length.
    prim::count ProgramDataByteLength;

    /**\brief Reads in any font program from an array of bytes and attempts
    to decode it.*/
    Font(const prim::byte* PointerToProgram, prim::count ProgramLength) :
      Program(0)
    {
      using namespace prim;
      using namespace prim::typeface;

      ProgramData = PointerToProgram;
      ProgramDataByteLength = ProgramLength;

      prim::uint32 Version = *(prim::uint32*)PointerToProgram;
      Endian::ConvertObjectToBigEndian(Version);

      if(Version == 0x00010000)
      {
        OpenType* OpenTypeProgram;
        OpenTypeProgram =
          new OpenType(PointerToProgram, ProgramLength);
        Program = OpenTypeProgram;
      }
      else
      {
        String VersionFourLetters;
        VersionFourLetters.Append((const ascii*)PointerToProgram,4);

        if(VersionFourLetters=="true" || VersionFourLetters=="type1")
        {
          TrueType* TrueTypeProgram;
          TrueTypeProgram =
            new TrueType(PointerToProgram, ProgramLength);
          Program = TrueTypeProgram;
        }
      }
    }

    ///Destroys the program information associated with this Font.
    ~Font()
    {
      delete Program;
    }

    /**Returns the width of a string of text using some font. The units are
    in points strictly speaking. Practically though the value returned
    depends on whatever the units are of Scale. Notice this is merely an
    adaptation of the PDF text-drawing method, presented here as a no-op
    which simply finds what the width of the text would be based off of the
    information in the font. This method takes into account advance widths,
    the initial left-side bearing, and the kerning pairs.*/
    prim::number TextWidth(prim::String Text, prim::number Scale=1.0)
    {
      using namespace prim;
      using namespace prim::typeface;
      using namespace bbs;

      //Strip unsupported characters (use ASCII for now).
      Text::MakeASCIIPrintableString(Text);

      //Get a pointer to the OpenType font (otherwise exit)
      OpenType* OpenTypeFont = Program->Interface<OpenType>();
      if(!OpenTypeFont)
        return (number)0.0;

      //Determine the widths of each character.
      integer* WidthsList = new integer[Text.n()];
      integer* LSBList    = new integer[Text.n()];
      integer* KernList   = new integer[Text.n()];
      number   TotalWidth = 0;

      const ascii* TextCharacters=Text.Merge();
      for(count i=0;i<Text.n();i++)
      {
        //Find the glyphs.
        prim::int16 GlyphIndex =
          OpenTypeFont->CMAP.GlyphIndexFromCharacterCode(
          TextCharacters[i]);
        prim::int16 NextGlyphIndex =
          OpenTypeFont->CMAP.GlyphIndexFromCharacterCode(
          TextCharacters[i+1]);

        //Get the advance width and left-side bearing.
        WidthsList[i] = OpenTypeFont->HMTX.AdvanceWidths[GlyphIndex];
        LSBList[i] = OpenTypeFont->HMTX.LeftSideBearings[GlyphIndex];
        TotalWidth += (number)WidthsList[i];

        //Get the kern value.
        KernList[i] = OpenTypeFont->KERN.FindKerningPair(
          GlyphIndex, NextGlyphIndex);
        TotalWidth += KernList[i];

        //Adjust for the kerning value.
        WidthsList[i] += KernList[i];
      }

      //Account for the first LSB so that the width will be very accurate.
      number FirstLSB = (number)LSBList[0];

      //Get the units per em to determine the scale.
      number UnitsPerEm = (number)OpenTypeFont->HEAD.UnitsPerEm;

      delete [] WidthsList;
      delete [] LSBList;
      delete [] KernList;

      //Calculate the width in device units.
      return (TotalWidth - FirstLSB) * (Scale / UnitsPerEm);
    }
  };
}

#endif
