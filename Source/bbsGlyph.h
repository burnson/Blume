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

#ifndef bbsGlyph
#define bbsGlyph

#include "prim.h"

#include "bbsMappings.h"

namespace bbs
{
  //A glyph stores a path as though it were a character from a font.
  struct Glyph : public prim::Path
  {
    /**Describes a kerning pair. Kerning pairs are special combinations of 
    letters which use different spacing than the standard advance width to take 
    advantage of their geometry.*/
    struct Kerning
    {
      ///Character code of the following character.
      prim::unicode::UCS4 FollowingCharacter;

      ///Amount of horizontal space to expand or contract when the glyphs meet.
      prim::number HorizontalAdjustment;
    };

    ///Unicode character code assigned to this glyph.
    prim::unicode::UCS4 Character;
    
    ///Nominal advance-width for the character before kerning is applied.
    prim::number AdvanceWidth;
    
    ///Array of kerning pairs.
    prim::Array<Kerning> Kern;
    
    ///Stores the index at which this glyph is located in the GlyphIndexTable.
    prim::count CrossReferencedIndex;
    
    ///Stores the original index of the glyph within the device font.
    prim::count OriginalDeviceIndex;

    ///Default constructor zeroes fields.
    Glyph() : Character(0), AdvanceWidth(0), CrossReferencedIndex(0),
      OriginalDeviceIndex(0) {}

    ///Copies a Glyph from another object of the same type.
    Glyph& operator = (const Glyph& Other)
    {
      if(this != &Other)
      {
        Character = Other.Character;
        AdvanceWidth = Other.AdvanceWidth;
        Kern = Other.Kern;
        Components = Other.Components;
        OriginalDeviceIndex = Other.OriginalDeviceIndex;
        CrossReferencedIndex = Other.CrossReferencedIndex;
      }
      return *this;
    }

    void AppendToSVGString(prim::String& Destination, prim::String& Transform)
    {
      using namespace prim;
      Destination += "<path unicode=\"";
      Destination &= (integer)Character;
      Destination &= "\" advance-width=\"";
      Destination &= AdvanceWidth;
      Destination &= "\" d=\"";
      for(count i = 0; i < Components.n(); i++)
      {
        Path::Component& c = Components[i];
        Destination &= "M";
        Destination &= c.Curves[0].End.x;
        Destination -= c.Curves[0].End.y;
        for(count j = 1; j < c.Curves.n(); j++)
        {
          Path::Curve& cv = c.Curves[j];
          if(!cv.IsCurve)
          {
            Destination -= "L";
            Destination &= cv.End.x;
            Destination -= cv.End.y;
          }
          else
          {
            Destination -= "C";
            Destination &= cv.StartControl.x;
            Destination -= cv.StartControl.y;
            Destination -= cv.EndControl.x;
            Destination -= cv.EndControl.y;
            Destination -= cv.End.x;
            Destination -= cv.End.y;
          }
        }
      }
      Destination &= "\"";
      if(Transform != "")
      {
        Destination -= "transform=\"";
        Destination &= Transform;
        Destination &= "\"";
      }
      Destination &= "/>";
    }
  };

  class VectorFont
  {
    /**Remembers the height of the font. To access this value, call 
    GetHeight(). This value is not valid for symbol fonts, which are assumed
    to have a unit height.*/
    prim::number Height;
    
    ///The typographic ascender of the font.
    prim::number Ascender;
    
    ///The typographic descender of the font.
    prim::number Descender;
  
  public:
    /**Holds pointers to the glyphs that make up the font. A null pointer means
    that the character does not have a corresponding glyph and should be ignored
    or replaced with a missing character.*/
    prim::Array<Glyph*> GlyphTable;
    
    /**Holds pointers to the glyphs that make up the font using their index. All
    of the items in this array will point to valid glyph objects.*/
    prim::Array<Glyph*> GlyphIndexTable;

    /**Holds a pointer to the character mapping for text-based fonts. The
    pointer is null if the font is not a text-based font. Note it is possible
    for a font to be both text-based and symbol-based.*/
    bbs::TextMapping* TextMap;

    /**Holds a pointer to the character mapping for symbol-based fonts. The
    pointer is null if the font is not a symbol-based font. Note it is possible
    for a font to be both text-based and symbol-based.*/
    bbs::SymbolMapping* SymbolMap;

    /**Converts SVG path construction information to a Path object. 
    Specifically, it parses the info in "d" attribute of the <path> and
    retraces the path in a prim::Path object.*/
    static void ConvertSVGDataToPath(prim::String& Data, prim::Path& p)
    {
      using namespace prim;
      using namespace prim::math;

      //First remove all existing subpaths.
      p.Clear();

      //Ensure that only the lower ASCII set is being used in the data stream.
      if(Data.ByteLength() != Data.CharacterLength())
        return;

      //Make a contiguous block of randomly accessible characters.
      const ascii* Text = Data.Merge();

      String CurrentToken;
      List<String> TokenList;

      while(*Text)
      {       
        if((*Text >= 'A' && *Text <= 'Z') || (*Text >= 'a' && *Text <= 'z'))
        {
          //Processing a path-construction command.

          //Flush token.
          if(CurrentToken != "")
          {
            TokenList.Add() = CurrentToken;
            CurrentToken = "";
          }

          //Flush path-construction token.
          TokenList.Add().Append((unicode::UCS4)*Text);
        }
        else if(*Text == ' ' && CurrentToken != "")
        {
          //Processing white space.

          //Flush token.
          if(CurrentToken != "")
          {
            TokenList.Add() = CurrentToken;
            CurrentToken = "";
          }
        }
        else
        {
          //Assume processing a number.
          CurrentToken.Append((unicode::UCS4)*Text);
        }

        Text++;
      }

      //Flush final token.
      if(CurrentToken != "")
        TokenList.Add() = CurrentToken;

      //Now process the list of tokens.
      for(count i = 0; i < TokenList.n(); i++)
      {
        if(TokenList[i] == "M" && i + 2 < TokenList.n())
        {
          Vector v1(TokenList[i + 1].ToNumber(), TokenList[i + 2].ToNumber());
          p.AddComponent(v1);
        }
        else if(TokenList[i] == "C" && i + 6 < TokenList.n())
        {
          Vector v1(TokenList[i + 1].ToNumber(), TokenList[i + 2].ToNumber());
          Vector v2(TokenList[i + 3].ToNumber(), TokenList[i + 4].ToNumber());
          Vector v3(TokenList[i + 5].ToNumber(), TokenList[i + 6].ToNumber());
          p.AddCurve(v1, v2, v3);
        }
        else if(TokenList[i] == "L" && i + 2 < TokenList.n())
        {
          Vector v1(TokenList[i + 1].ToNumber(), TokenList[i + 2].ToNumber());
          p.AddCurve(v1);
        }
      }
    }

    ///Returns the bounding box of the whole font.
    prim::math::Rectangle GetBoundingBox(
      bool IgnorePrivateUseCharacters = false)
    {
      prim::math::Rectangle BoundingBoxAllPaths;
      bool Initialized = false;
      for(prim::count i = 0; i < GlyphTable.n(); i++)
      {
        if(IgnorePrivateUseCharacters && i == (prim::unicode::UCS4)0xE000)
        {
          i = 0xF8FF;
          continue;
        }
        if(!GlyphTable[i])
          continue;
        if(Initialized)
        {
          BoundingBoxAllPaths = BoundingBoxAllPaths + 
            GlyphTable[i]->GetBoundingBox();
        }
        else
        {
          BoundingBoxAllPaths = GlyphTable[i]->GetBoundingBox();
          Initialized = true;
        }
      }
      return BoundingBoxAllPaths;
    }

    ///Calculates the width of a string of text based on a given font size.
    prim::number GetTextWidth(const prim::String& Text, 
      prim::number FontSize = 1.0f)
    {
      prim::number Width = 0;
      for(prim::count i = 0; i < Text.n(); i++)
      {
        prim::unicode::UCS4 c = Text[i];
        prim::unicode::UCS4 next_c = 0;
        if(i < Text.n() - 1)
          next_c = Text[i + 1];

        Glyph* g = 0, *next_g = 0;
        if((prim::count)c < GlyphTable.n())
          g = GlyphTable[(prim::count)c];
        if((prim::count)next_c < GlyphTable.n())
          next_g = GlyphTable[(prim::count)next_c];

        if(!g)
          continue;

        Width += g->AdvanceWidth;

        if(!next_g)
          continue;
        
        for(prim::count j = 0; j < g->Kern.n(); j++)
        {
          if(g->Kern[j].FollowingCharacter == next_c)
          {
            Width += g->Kern[j].HorizontalAdjustment;
            break;
          }
        }
      }
      
      return Width * FontSize;
    }

    /**Returns a pointer to new glyph at the correponding character. The
    character code is automatically filled in inside the glyph.*/
    Glyph* AddGlyph(prim::unicode::UCS4 CharacterCode)
    {
      //Get the character code of the new glyph.
      prim::count Character = (prim::count)CharacterCode;

      //Increase the table size to accomodate the new glyph.
      if(Character + 1 > GlyphTable.n())
        GlyphTable.n(Character + 1);

      //Default glyph index is at the end.
      prim::count NewGlyphIndex = GlyphIndexTable.n();
      
      /*If there was a previous glyph then the new one will overwrite, so delete
      the old glyph.*/
      if(Glyph* OldGlyph = GlyphTable[Character])
      {
        for(prim::count i = GlyphIndexTable.n() - 1; i >= 0; i--)
        {
          if(GlyphIndexTable[i] == OldGlyph)
          {
            NewGlyphIndex = i;
            break;
          }
        }
        delete OldGlyph;
      }
      else
        GlyphIndexTable.AddOne();

      //Create the new glyph.
      Glyph* NewGlyph = new Glyph;
      GlyphTable[Character] = NewGlyph;
      GlyphIndexTable[NewGlyphIndex] = NewGlyph;
      NewGlyph->Character = CharacterCode;
      NewGlyph->CrossReferencedIndex = NewGlyphIndex;
      
      return NewGlyph;
    }

    /**Calculates and returns the height of the font by calculating the font's 
    bounding box and returning the Y value of the top side. To recalculate the
    value, set the recalculate parameter to true. Note that you do not need to
    set recalculate to true on the first call, since the first time calculation
    is detected automatically.*/
    prim::number GetHeight(bool Recalculate = false)
    {
      if(!Recalculate && Height != 0)
        return Height;

      return Height = GetBoundingBox().Height();
    }
    
    prim::number GetFontHeight(bool Recalculate = false)
    {
      return GetHeight(Recalculate);
    }
    
    prim::number GetAscender(void)
    {
      return Ascender;
    }
    
    prim::number GetDescender(void)
    {
      return Descender;
    }

    ///Removes all the glyphs from the font.
    void RemoveAllGlyphs(void)
    {
      for(prim::count i = 0; i < GlyphTable.n(); i++)
        delete GlyphTable[i];
      GlyphTable.n(0);
      GlyphIndexTable.n(0);
    }

    ///Destroys the VectorFont.
    ~VectorFont()
    {
      delete TextMap;
      delete SymbolMap;
      RemoveAllGlyphs();
    }

    ///Saves the VectorFont to a string containing SVG path data.
    void SaveToSVGString(prim::String& SVG, prim::count Columns = 8,
      prim::count pxThumbnailSize = 100)
    {
      using namespace prim;
      using namespace prim::math;

      //Empty the output string.
      SVG = "";

      //Count the number of glyphs.
      count Glyphs = 0;
      for(count i = 0; i < GlyphTable.n(); i++)
      {
        if(GlyphTable[i])
          Glyphs++;
      }

      //Calculate the number of rows.
      count Rows = Glyphs / Columns;
      if(Glyphs % Columns)
        Rows++;

      //Get the bounding box of all the glyphs.
      Rectangle BoundingBoxOfAllGlyphs = GetBoundingBox();
      number unitsThumbnailSize = Max(BoundingBoxOfAllGlyphs.Width(), 
        BoundingBoxOfAllGlyphs.Height());

      //Write the header information
      integer w = Columns * pxThumbnailSize + 2;
      integer h = Rows * pxThumbnailSize + 2; 
      SVG = "<?xml version=\"1.0\" standalone=\"no\"?>"
        "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
        "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">";
      SVG += "<svg width=\"";
      SVG &= w;
      SVG &= "\" height=\"";
      SVG &= h;
      SVG &= "\" version=\"1.1\"";
      SVG -= "xmlns=\"http://www.w3.org/2000/svg\">";

      SVG++;
      SVG += "<!--Kerning Information-->";
      for(count i = 0; i < GlyphTable.n(); i++)
      {
        if(!GlyphTable[i])
          continue;

        for(count j = 0; j < GlyphTable[i]->Kern.n(); j++)
        {
          prim::unicode::UCS4 Left = GlyphTable[i]->Character;
          prim::unicode::UCS4 Right = GlyphTable[i]->Kern[j].FollowingCharacter;
          prim::number Adjustment = GlyphTable[i]->Kern[j].HorizontalAdjustment;
          SVG += "<kern left=\"";
          SVG &= (prim::integer)Left;
          SVG &= "\" right=\"";
          SVG &= (prim::integer)Right;
          SVG &= "\" horizontal-adjustment=\"";
          SVG &= Adjustment;
          SVG &= "\"/>";
        }
      }

      SVG++;
      SVG += "<!--Path data for each glyph-->";

      number PercentageToMoveX = 0;
      number PercentageToMoveY = 0;
      if(BoundingBoxOfAllGlyphs.Width() < BoundingBoxOfAllGlyphs.Height())
      {
        PercentageToMoveX = 1.0f - (BoundingBoxOfAllGlyphs.Width() / 
          BoundingBoxOfAllGlyphs.Height());
        PercentageToMoveX *= 0.5f;
      }
      else
      {
        PercentageToMoveY = 1.0f - (BoundingBoxOfAllGlyphs.Height() / 
          BoundingBoxOfAllGlyphs.Width());
        PercentageToMoveY *= 0.5f;
      }

      //Draw thumbnails of each glyph in their own little box.
      for(count i = 0, actual_i = -1; i < GlyphTable.n(); i++)
      {
        if(!GlyphTable[i])
          continue;
        else
          actual_i++;

        prim::count Row = actual_i / Columns;
        prim::count Column = actual_i % Columns;
        AffineMatrix m;
        

        number dx = PercentageToMoveX * BoundingBoxOfAllGlyphs.Height();
        number dy = PercentageToMoveY * BoundingBoxOfAllGlyphs.Width();

        m += AffineMatrix::Translation(Vector(
          (number)(Column * pxThumbnailSize),
          (number)((Row + 1) * pxThumbnailSize)));
        m += AffineMatrix::Scale(pxThumbnailSize / unitsThumbnailSize);
        m += AffineMatrix::Scale(Vector(1.0f, -1.0f));
        m += AffineMatrix::Translation(Vector(dx, dy));
        m += AffineMatrix::Translation(Vector(
          -BoundingBoxOfAllGlyphs.BottomLeft().x,
          -BoundingBoxOfAllGlyphs.BottomLeft().y));
        
        String Transform = "matrix(";
        Transform &= m.a;
        Transform -= m.b;
        Transform -= m.c;
        Transform -= m.d;
        Transform -= m.e;
        Transform -= m.f;
        Transform &= ")";;
        GlyphTable[i]->AppendToSVGString(SVG, Transform);
      }

      SVG++;
      SVG += "<!--Annotations (grid lines, character index, etc.)-->";

      //Draw some grid lines.
      for(count i = 0; i <= Rows; i++)
      {
        number x1 = 0;
        number x2 = (number)(Columns * pxThumbnailSize);
        number y1 = (number)(i * pxThumbnailSize);
        number y2 = y1;
        SVG += "<line x1=\""; SVG &= x1; SVG &= "\" y1=\""; SVG &= y1; 
        SVG &= "\" x2=\""; SVG &= x2; SVG &= "\" y2=\""; SVG &= y2;
        SVG &= "\" style=\"stroke:rgb(0,0,0);stroke-width:1\"/>";
      }

      for(count i = 0; i <= Columns; i++)
      {
        number x1 = (number)(i * pxThumbnailSize);
        number x2 = x1;
        number y1 = 0;
        number y2 = (number)(Rows * pxThumbnailSize);
        SVG += "<line x1=\""; SVG &= x1; SVG &= "\" y1=\""; SVG &= y1; 
        SVG &= "\" x2=\""; SVG &= x2; SVG &= "\" y2=\""; SVG &= y2;
        SVG &= "\" style=\"stroke:rgb(0,0,0);stroke-width:1\"/>";
      }

      //Draw the unicode character indexes for each glyph.
      for(count i = 0, actual_i = -1; i < GlyphTable.n(); i++)
      {
        if(!GlyphTable[i])
          continue;
        else
          actual_i++;

        prim::count Row = actual_i / Columns;
        prim::count Column = actual_i % Columns;
        number x1 = (number)(Column * pxThumbnailSize + 2);
        number y1 = (number)(Row * pxThumbnailSize + 14);
        SVG += "<text x=\""; SVG &= x1; SVG &= "\" y=\""; SVG &= y1; 
        SVG &= "\" fill=\"blue\">";
        SVG &= (integer)GlyphTable[i]->Character;
        SVG &= "</text>";
      }

      SVG += "</svg>";
    }

    ///Opens the VectorFont from a string containing SVG-saved information.
    void OpenFromSVGString(prim::String& Input, bool MergeIntoExisting = false);

    ///Saves the VectorFont to an SVG file for viewing and later reloading.
    void SaveToSVGFile(const prim::ascii* Filename, prim::count Columns = 8,
      prim::count pxThumbnailSize = 100)
    {
      prim::String Output;
      SaveToSVGString(Output, Columns, pxThumbnailSize);
      prim::File::Replace(Filename, Output);
    }

    ///Opens the VectorFont from an SVG file saved using SaveToSVGFile().
    void OpenFromSVGFile(const prim::ascii* Filename)
    {
      prim::String Input;
      prim::File::ReadAsUTF8(Filename, Input);
      OpenFromSVGString(Input);
    }
    
    ///Attempts to load a font from a data block using the FreeType library.
    prim::String OpenFromFontData(const prim::byte* ByteArray,
      prim::count LengthInBytes);
    
    ///Attempts to load a font file using the FreeType library.
    prim::String OpenFromFontFile(const prim::ascii* Filename)
    {
      prim::String Result;
      prim::String FileStr = Filename;
      if(prim::OS::MacOSX() &&
        (FileStr.Suffix(6) == ".dfont" || FileStr.Suffix(5) == ".suit"))
          Result = OpenFromFontData((const prim::byte*)Filename, -1);
      else        
      {
        prim::byte* ByteArray = 0;
        prim::count LengthInBytes = prim::File::Read(Filename, ByteArray);
        Result = OpenFromFontData(ByteArray, LengthInBytes);
        delete ByteArray;
      }
      return Result;
    }
    
    ///Generic font file loading method for both native SVG and standard types.
    prim::String Open(const prim::ascii* Filename)
    {
      prim::String FileStr = Filename;
      if(FileStr.Suffix(4) == ".svg")
        OpenFromSVGFile(Filename);
      else
        return OpenFromFontFile(Filename);
      return "";
    }


    ///Default constructor is a font with no glyphs.
    VectorFont() : Height(0), Ascender(0), Descender(0), TextMap(0),
      SymbolMap(0) {}

    ///Constructor that opens a font file by pathname.
    VectorFont(const prim::ascii* Filename) : Height(0), Ascender(0),
      Descender(0), TextMap(0), SymbolMap(0)
    {
      Open(Filename);
    }

    /**Substitutes a string of find text with a string of replacement text if
    the given character exists in the font.*/
    void SubstituteCharacters(prim::String& Text, const prim::ascii* FindText, 
      prim::unicode::UCS4 CharacterCodeToSubstitute, 
      prim::String& ReplacementText)
    {
      if((prim::count)CharacterCodeToSubstitute >= GlyphTable.n())
        return;
      if(!GlyphTable[(prim::count)CharacterCodeToSubstitute])
        return;
      Text.Replace(FindText, ReplacementText);
    }

    /**Substitutes a string of find text with a single character if it exists in
    the font.*/
    void SubstituteCharacters(prim::String& Text, 
      const prim::ascii* FindText, 
      prim::unicode::UCS4 CharacterCodeToSubstitute)
    {
      prim::String SubstituteText;
      SubstituteText.Append(CharacterCodeToSubstitute);
      SubstituteCharacters(Text, FindText, CharacterCodeToSubstitute,
        SubstituteText);
    }

    /**Converts the combinations ff, fi, fl, ffi, ffl, ft, and st to ligatures.
    Each will only be converted if the ligatures actually exist in the font.*/
    void MakeLatinLigatures(prim::String& Text)
    {
      SubstituteCharacters(Text, "ff", 0xFB00);
      SubstituteCharacters(Text, "fi", 0xFB01);
      SubstituteCharacters(Text, "fl", 0xFB02);
      SubstituteCharacters(Text, "ffi", 0xFB03);
      SubstituteCharacters(Text, "ffl", 0xFB04);
      prim::String LongSAndT;
      LongSAndT.Append((prim::unicode::UCS4)0x017F, 
        (prim::unicode::UCS4)0x0074);
      SubstituteCharacters(Text, LongSAndT, 0xFB00);
      //SubstituteCharacters(Text, "st", 0xFB06); //This is uncommon.
    }

    /**Directionalizes quotation marks. Method makes a guess as to where these
    should occur based on context.*/
    void MakeDirectionalQuotationMarks(prim::String& Text)
    {
      prim::String RightQuoteSpace;
      RightQuoteSpace.Append(8217, 32);
      SubstituteCharacters(Text, "' ", 8217, RightQuoteSpace);
      prim::String CommaRightQuote = ",";
      CommaRightQuote.Append(8217);
      SubstituteCharacters(Text, ",'", 8217, CommaRightQuote);
      prim::String PeriodRightQuote = ".";
      PeriodRightQuote.Append(8217);
      SubstituteCharacters(Text, ".'", 8217, PeriodRightQuote);
      SubstituteCharacters(Text, "'", 8217);

      prim::String DoubleRightQuoteSpace;
      DoubleRightQuoteSpace.Append(8221, 32);
      SubstituteCharacters(Text, "\" ", 8221, DoubleRightQuoteSpace);
      prim::String CommaDoubleRightQuote = ",";
      CommaDoubleRightQuote.Append(8221);
      SubstituteCharacters(Text, ",\"", 8221, CommaDoubleRightQuote);
      prim::String PeriodDoubleRightQuote = ".";
      PeriodDoubleRightQuote.Append(8221);
      SubstituteCharacters(Text, ".\"", 8221, PeriodDoubleRightQuote);
      SubstituteCharacters(Text, "\"", 8220);
    }

    ///Turns " - " into an en-dash and "--" into an em-dash.
    void MakeDashes(prim::String& Text)
    {
      SubstituteCharacters(Text, " - ", 8211);
      SubstituteCharacters(Text, " -- ", 8212);
      SubstituteCharacters(Text, "--", 8212);
    }

    prim::number GetKerningPairValue(prim::unicode::UCS4 Left, 
      prim::unicode::UCS4 Right)
    {
      Glyph* g = 0;
      if((prim::count)Left < GlyphTable.n())
        g = GlyphTable[(prim::count)Left];

      if(!g)
        return 0;

      for(prim::count i = 0; i < g->Kern.n(); i++)
        if(g->Kern[i].FollowingCharacter == Right)
          return g->Kern[i].HorizontalAdjustment;

      return 0;
    }
  };
}

#endif
