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

#include "prim.h"
#include "bbs.h"

#ifndef USING_FREETYPE_EXTENSIONS
  #define USING_FREETYPE_EXTENSIONS 0
#endif

#if USING_FREETYPE_EXTENSIONS

#include <ft2build.h>
#include FT_FREETYPE_H 

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/ftadvanc.h>


///Internal callback move-to used by OpenFromFontData and FreeType
static int FreeTypeCallbackMoveTo(const FT_Vector* To, void* Glyph)
{
  using namespace prim;
  bbs::Glyph* g = reinterpret_cast<bbs::Glyph*>(Glyph);
  g->AddComponent(math::Vector((number)To->x, (number)To->y));
  return 0;
}

///Internal callback line-to used by OpenFromFontData and FreeType
static int FreeTypeCallbackLineTo(const FT_Vector* To, void* Glyph)
{
  using namespace prim;
  bbs::Glyph* g = reinterpret_cast<bbs::Glyph*>(Glyph);
  g->AddCurve(math::Vector((number)To->x, (number)To->y));
  return 0;
}

///Internal callback conic-to used by OpenFromFontData and FreeType
static int FreeTypeCallbackConicTo(const FT_Vector* Control,
  const FT_Vector* To, void* Glyph)
{
  using namespace prim;
  bbs::Glyph* g = reinterpret_cast<bbs::Glyph*>(Glyph);
  g->AddCurve(
    math::Vector((number)Control->x, (number)Control->y),
    math::Vector((number)To->x, (number)To->y));
  return 0;
}
  
///Internal callback cubic-to used by OpenFromFontData and FreeType
static int FreeTypeCallbackCubicTo(const FT_Vector* Control1,
  const FT_Vector* Control2, const FT_Vector* To, void* Glyph)
{
  using namespace prim;
  bbs::Glyph* g = reinterpret_cast<bbs::Glyph*>(Glyph);
  g->AddCurve(
    math::Vector((number)Control1->x, (number)Control1->y),
    math::Vector((number)Control2->x, (number)Control2->y),
    math::Vector((number)To->x, (number)To->y));
  return 0;
}

prim::String bbs::VectorFont::OpenFromFontData(const prim::byte* ByteArray,
  prim::count LengthInBytes)
{
  //Initialize the vector font.
  RemoveAllGlyphs();
  
  //Initialize the FreeType library.
  FT_Library Library;
  int ErrorCode = 0;
  if( (ErrorCode = FT_Init_FreeType(&Library)) )
    return "Could not initialize font service.";
  
  //Open the first font face out of the collection.
  FT_Face Face;
  if(LengthInBytes == -1)
    ErrorCode = FT_New_Face(Library, (const prim::ascii*)ByteArray, 0, &Face);
  else
    ErrorCode = FT_New_Memory_Face(Library,
      &ByteArray[0], LengthInBytes, 0, &Face);

  if(ErrorCode == FT_Err_Unknown_File_Format)
    return "The font format is unknown.";
  else if(ErrorCode)
    return "The font file is invalid.";
  
  //Select a symbol character map if it exists.
  prim::count TotalCMaps = Face->num_charmaps;
  for(prim::count i = 0; i < TotalCMaps; i++)
  {
    FT_Encoding enc = Face->charmaps[i]->encoding;
    if(enc == FT_ENCODING_MS_SYMBOL)
      FT_Select_Charmap(Face, enc);
  }
  
  //Initialize the callback methods which construct the outlines.
  FT_Outline_Funcs OutlineCallbacks;
  OutlineCallbacks.shift = 0;
  OutlineCallbacks.delta = 0;
  OutlineCallbacks.move_to = (FT_Outline_MoveToFunc)FreeTypeCallbackMoveTo;
  OutlineCallbacks.line_to = (FT_Outline_LineToFunc)FreeTypeCallbackLineTo;
  OutlineCallbacks.conic_to = (FT_Outline_ConicToFunc)FreeTypeCallbackConicTo;
  OutlineCallbacks.cubic_to = (FT_Outline_CubicToFunc)FreeTypeCallbackCubicTo;
  
  //Determine the normalization scalar.
  prim::number NormalizingScalar = 1.0f;
  prim::number UnitsPerEM = (prim::number)Face->units_per_EM;
  if(UnitsPerEM)
    NormalizingScalar /= UnitsPerEM;
    
  //Retrieve and normalize the height, ascender, and descender.
  Height = (prim::number)Face->height * NormalizingScalar;
  Ascender = (prim::number)Face->ascender * NormalizingScalar;
  Descender = (prim::number)Face->descender * NormalizingScalar;

  //Load in each character found in the character map.
  FT_ULong CharacterCode = 0;
  FT_UInt GlyphIndex = 0;
  CharacterCode = FT_Get_First_Char(Face, &GlyphIndex);
  prim::count NumberNotLoaded = 0;
  prim::count NumberNotInOutlineFormat = 0;
  prim::count NumberGetGlyphFails = 0;
  prim::count NumberDecomposeFails = 0;
  prim::count TotalTried = 0;
  
  do
  {
    TotalTried++;
    
    /*Attempt to load the glyph given by the glyph index. Note: from
    FT_LOAD_NO_SCALE, FT_LOAD_NO_BITMAP is implied. FT_LOAD_IGNORE_TRANSFORM is
    not necessary if FT_Set_Transform is not called? Not sure.*/
    if( (ErrorCode = FT_Load_Glyph(Face, GlyphIndex,
      FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM)) )
    {
      NumberNotLoaded++;
      continue;
    }
    
    //Make sure that the glyph is in an outline format.
    if(Face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
    {
      NumberNotInOutlineFormat++;
      continue;
    }
    
    //Attempt to grab the glyph so that its outline can be processed.
    FT_Glyph Glyph;
    if( (ErrorCode = FT_Get_Glyph(Face->glyph, &Glyph)) )
    {
      NumberGetGlyphFails++;
      continue;
    }
    
    //Cast the glyph to an outline glyph to access the outline.
    FT_Outline *Outline = &reinterpret_cast<FT_OutlineGlyph>(Glyph)->outline;
    
    //New Glyph to store the converted result.
    bbs::Glyph& ConvertedGlyph = *AddGlyph(CharacterCode);
    
    //Set the character code.
    ConvertedGlyph.Character = CharacterCode;
    
    //Remember the glyph index.
    ConvertedGlyph.OriginalDeviceIndex = (prim::count)GlyphIndex;
    
    //Set the advance width.
    FT_Fixed Advance = 0;
    FT_Get_Advance(Face, GlyphIndex,
      FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM, &Advance);
    ConvertedGlyph.AdvanceWidth = (prim::number)Advance * NormalizingScalar;
    
    //Walk through the outline and convert it to the native Glyph format.
    if( (ErrorCode = FT_Outline_Decompose(
      Outline, &OutlineCallbacks, (void*)&ConvertedGlyph)) )
      NumberDecomposeFails++;
    
    //Scale the glyph to a unit point.
    ConvertedGlyph *= NormalizingScalar;
    
    //Free the memory associated with the glyph.
    FT_Done_Glyph(Glyph);
    
    //Get the next character code and exit if there are no more.
  } while((CharacterCode = FT_Get_Next_Char(
      Face, CharacterCode, &GlyphIndex)) );
      
  /*Determine all of the kernings. Currently this is a brute-force check in
  which all pairwise possibilities are examined. FreeType does not appear to
  have any direct way to determine which glyphs are kerned against which.*/
  prim::Console c;
  prim::count TotalGlyphsStored = GlyphIndexTable.n();
  prim::count KerningsFound = 0;
  for(prim::count i = 0; i < TotalGlyphsStored; i++)
  {
    Glyph* Left = GlyphIndexTable[i];
    for(prim::count j = 0; j < TotalGlyphsStored; j++)
    {
      Glyph* Right = GlyphIndexTable[j];
      
      //Retrieve the kerning from the font.
      FT_Vector KerningVector;
      FT_Get_Kerning(Face, (FT_UInt)Left->OriginalDeviceIndex,
        (FT_UInt)Right->OriginalDeviceIndex, FT_KERNING_UNSCALED,
        &KerningVector);
        
      //If a non-zero kerning is found then add it to the kerning array.
      if(KerningVector.x)
      {
        KerningsFound++;
        Glyph::Kerning Kern;
        Kern.FollowingCharacter = Right->Character;
        Kern.HorizontalAdjustment = (prim::number)KerningVector.x;
        Left->Kern.Add(Kern);
      }
    }
  }
  
  //Return detailed information on what failed.
  if(NumberNotLoaded > 0 || NumberNotInOutlineFormat > 0 ||
    NumberGetGlyphFails > 0 || NumberDecomposeFails > 0)
  {
    using namespace prim;
    String e = "Out of ";
    e &= (integer)TotalTried;
    e &= " glyphs: ";
    
    e &= (integer)NumberNotLoaded;
    e &= " could not be loaded, ";
    
    e &= (integer)NumberNotInOutlineFormat;
    e &= " were not in outline format, ";
    
    e &= (integer)NumberGetGlyphFails;
    e &= " could not be converted to outline glyphs, and ";
    
    e &= (integer)NumberDecomposeFails;
    e &= " failed to generate path segments.";
    
    return e;
  }
  
  //Return empty string for success.
  return "";
}
#else
prim::String bbs::VectorFont::OpenFromFontData(const prim::byte* ByteArray,
  prim::count LengthInBytes)
{
  return "Could not load font data. FreeType extension not compiled.";
}
#endif

void bbs::VectorFont::OpenFromSVGString(prim::String& Input,
  bool MergeIntoExisting)
{
  using namespace prim;

  //First remove all existing paths if not merging.
  if(!MergeIntoExisting)
    RemoveAllGlyphs();

  XML::Document SVGDocument;
  SVGDocument.ParseDocument(Input);
  
  const List<XML::Object*> ObjectList = SVGDocument.Root->GetObjects();
  for(count i = 0; i < ObjectList.n(); i++)
  {
    if(XML::Element* e = ObjectList[i]->IsElement())
    {
      if(e->GetName() == "path")
      {
        Glyph CurrentGlyph;
        bool GlyphIsValid = false;

        const List<XML::Element::Attribute> AttributeList = 
          e->GetAttributes();
        for(count j = 0; j < AttributeList.n(); j++)
        {
          if(AttributeList[j].Name == "d")
          {
            //Process the path information.                
            ConvertSVGDataToPath(AttributeList[j].Value, CurrentGlyph);
            GlyphIsValid = true;
          }
          else if(AttributeList[j].Name == "unicode")
          {
            //Get the Unicode codepoint.
            CurrentGlyph.Character = 
              (unicode::UCS4)AttributeList[j].Value.ToInteger();
          }
          else if(AttributeList[j].Name == "advance-width")
          {
            //Get the advance width.
            CurrentGlyph.AdvanceWidth = AttributeList[j].Value.ToNumber();
          }
          else
          {
            //Ignore unwanted attributes.
          }              
        }

        //If the glyph is valid then add it to the GlyphBasedFont.
        if(GlyphIsValid)
          *AddGlyph(CurrentGlyph.Character) = CurrentGlyph;
      }
    }
  }

  //Add the kerning pairs.
  for(count i = 0; i < ObjectList.n(); i++)
  {
    if(XML::Element* e = ObjectList[i]->IsElement())
    {
      if(e->GetName() == "kern")
      {
        prim::unicode::UCS4 Left = 0;
        prim::unicode::UCS4 Right = 0;
        prim::number HorizontalAdjustment = 0;

        const List<XML::Element::Attribute> AttributeList = 
          e->GetAttributes();
        for(count j = 0; j < AttributeList.n(); j++)
        {
          if(AttributeList[j].Name == "left")
          {
            //Get the Unicode codepoint of the left character.
            Left = (prim::unicode::UCS4)AttributeList[j].Value.ToInteger();
          }
          else if(AttributeList[j].Name == "right")
          {
            //Get the Unicode codepoint of the right character.
            Right = (prim::unicode::UCS4)AttributeList[j].Value.ToInteger();
          }
          else if(AttributeList[j].Name == "horizontal-adjustment")
          {
            //Get the horizontal adjustment of the kern.
            HorizontalAdjustment = AttributeList[j].Value.ToNumber();
          }
          else
          {
            //Ignore unwanted attributes.
          }
        }

        //Create a kerning pair from the information.
        if((prim::count)Left < GlyphTable.n() && GlyphTable[(prim::count)Left])
        {
          Glyph::Kerning KerningPair;
          KerningPair.FollowingCharacter = Right;
          KerningPair.HorizontalAdjustment = HorizontalAdjustment;
          GlyphTable[(prim::count)Left]->Kern.Add(KerningPair);
        }
      }
    }
  }
}

prim::number bbs::abstracts::Painter::DrawVectorText(
        bbs::VectorFont* FontToUse,
        prim::String Text,
        prim::math::Vector TranslateBy,
        prim::number ScaleBy,
        prim::Justification JustificationType,
        prim::number WrapWidth,
        bool ReturnWidthOnly,
        bool DisableAutoLigatures)
{
  if(!FontToUse)
    return 0;
    
  //Stop unused parameter warning.
  WrapWidth = (prim::number)0.0;

  //Apply the ligatures to the text.
  if(!DisableAutoLigatures)
    FontToUse->MakeLatinLigatures(Text);

  //Get the width of the text.
  prim::number Width = FontToUse->GetTextWidth(Text, ScaleBy);
  if(ReturnWidthOnly)
    return Width;

  //Set justification.
  if(JustificationType == prim::Justifications::Center)
    TranslateBy.x += FontToUse->GetTextWidth(Text, ScaleBy) * -0.5f;
  else if(JustificationType == prim::Justifications::Right)
    TranslateBy.x -= FontToUse->GetTextWidth(Text, ScaleBy);

  //Transform to starting position.
  Translate(TranslateBy);
  Scale(ScaleBy);

  //Track the advancement of the character sequence.
  prim::number Advance = 0;

  //Draw each letter as a vector-based path.
  for(prim::count i = 0; i < Text.n(); i++)
  {
    //Get the glyph of the character.
    bbs::Glyph* Character = 0;
    prim::count SingleCharacter = (prim::count)Text[i];
    if(SingleCharacter < FontToUse->GlyphTable.n())
      Character = FontToUse->GlyphTable[SingleCharacter];

    //Make sure the character exists in the font.
    if(!Character)
      continue;

    //Go to the character's position and draw it.
    Translate(prim::math::Vector(Advance, 0));
    DrawPath(*Character, false, true);
    UndoTransformation();

    //Advance position.
    Advance += Character->AdvanceWidth;

    //Adjust for kerning.
    bbs::Glyph* NextCharacter = 0;
    if(i < Text.n() - 1)
    {
      prim::count SingleCharacter = (prim::count)Text[i + 1];
      if(SingleCharacter < FontToUse->GlyphTable.n())
        NextCharacter = FontToUse->GlyphTable[SingleCharacter];
    }
    if(!NextCharacter)
      continue;
    for(prim::count j = 0; j < Character->Kern.n(); j++)
    {
      if(Character->Kern[j].FollowingCharacter == NextCharacter->Character)
      {
        Advance += Character->Kern[j].HorizontalAdjustment;
        break;
      }
    }
  }

  //Undo the first two transformations.
  UndoTransformation(2);

  return Width;
}

prim::math::Rectangle bbs::abstracts::Painter::DrawVectorText(
  prim::String& Text,
  bbs::Text::Unformatted& Style, 
  bool OnlyReturnBoundingBox)
{
  //Make a bounding box.
  prim::math::Rectangle BoundingBox;

  //Make sure the font and text are valid.
  if(!Text.n() || !Style.Font)
    return BoundingBox;

  //Apply character substitutions.
  if(Style.UseLatinLigatures)
    Style.Font->MakeLatinLigatures(Text);
  if(Style.UseDirectionalQuotationMarks)
    Style.Font->MakeDirectionalQuotationMarks(Text);
  if(Style.UseDashes)
    Style.Font->MakeDashes(Text);

  //This method does not respect tab or carraige returns, so eliminate them.
  prim::String Tab; Tab.Append(9);
  prim::String LF; LF.Append(10);
  prim::String CR; CR.Append(13);
  prim::String SPLFSP; SPLFSP.Append(32, 10, 32);
  Text.Replace(Tab, "");
  Text.Replace(CR, "");

  //Determine whether to draw multi-line text or single line text.
  prim::count DummyLF = 0;
  bool DrawMultilineText = Style.unitsLineWidth > 0;
  if(!DrawMultilineText)
    Text.Replace(LF, ""); //Only break line in multiline text.
  else if(Text.Find(LF, DummyLF))
    Text.Replace(LF, SPLFSP); //Force each line feed to be a "word".

  //Typesetting information
  prim::Array<prim::String> WordsAndSpaces;
  prim::Array<prim::number> WordAndSpaceKerning;
  prim::Array<prim::number> WordAndSpaceWidth;
  prim::Array<prim::String> TextPerLine;
  prim::Array<prim::number> WidthPerLine;
    
  //Calculate the width and height of the text area.
  prim::number TextAreaWidth = 0, TextAreaHeight = 0;
  if(DrawMultilineText)
  {
    TextAreaWidth = Style.unitsLineWidth;
    
    const prim::ascii* NextLetter = Text.Merge();
    const prim::ascii* BeginningOfCurrentWord = NextLetter;
    bool IteratingThroughInitialWhitespace = true;
    bool CapturingWordNotSpace = false;

    //Break the text up into strings of words and spaces.
    while(*NextLetter)
    {
      const prim::ascii* CurrentLetter = NextLetter;
      prim::unicode::UCS4 Code = prim::unicode::UTF8::Decode(NextLetter);

      if(!IteratingThroughInitialWhitespace && 
        CapturingWordNotSpace == (Code == 32))
      {
        CapturingWordNotSpace = !CapturingWordNotSpace;
        prim::String CurrentWord(BeginningOfCurrentWord,
          (prim::count)(CurrentLetter - BeginningOfCurrentWord));
        WordsAndSpaces.Add(CurrentWord);
        BeginningOfCurrentWord = CurrentLetter;
      }
      else if(IteratingThroughInitialWhitespace && Code != 32)
      {
        IteratingThroughInitialWhitespace = false;
        CapturingWordNotSpace = true;
        if(BeginningOfCurrentWord != CurrentLetter)
        {
          /*There is some initial whitespace which will be treated as though
          it were its own word. Initial whitespace is special in that it has
          the ability to, if it is long enough, push the beginning of the text
          to the second line. Whitespace in between words is not capable of
          doing this.*/
          prim::String InitialWhitespace(BeginningOfCurrentWord,
            (prim::count)(CurrentLetter - BeginningOfCurrentWord));
          WordsAndSpaces.Add(InitialWhitespace);
          BeginningOfCurrentWord = CurrentLetter;
        }
      }

      if(!*NextLetter)
      {
        //Add the last word to the list.
        prim::String CurrentWord(BeginningOfCurrentWord,
          (prim::count)(NextLetter - BeginningOfCurrentWord));
        WordsAndSpaces.Add(CurrentWord);
      }
    }

    WordAndSpaceKerning.n(WordsAndSpaces.n());
    WordAndSpaceWidth.n(WordsAndSpaces.n());
    for(prim::count i = 0; i < WordsAndSpaces.n(); i++)
    {
      WordAndSpaceWidth[i] = Style.Font->GetTextWidth(WordsAndSpaces[i],
        Style.unitsFontSize);

      if(i == 0)
      {
        WordAndSpaceKerning[i] = 0;
        continue;
      }
      prim::String& LeftString = WordsAndSpaces[i - 1];
      prim::String& RightString = WordsAndSpaces[i];
      prim::unicode::UCS4 Left = LeftString[LeftString.n() - 1];
      prim::unicode::UCS4 Right = RightString[0];

      WordAndSpaceKerning[i] = Style.Font->GetKerningPairValue(Left, Right) *
        Style.unitsFontSize;
    }

    /*Determine which words fall on what lines. Note that at least one word must
    fall on each line.*/
    {
      prim::number CurrentLineWidth = Style.unitsIndentation + 
        WordAndSpaceWidth[0];
      prim::String CurrentString = WordsAndSpaces[0];
      prim::number MaximumWidth = Style.unitsLineWidth;
      prim::number TestWordWidth = 0;
      for(prim::count i = 1; i < WordAndSpaceWidth.n(); i++)
      {
        TestWordWidth = WordAndSpaceWidth[i] + WordAndSpaceKerning[i];
        bool IsSpace = (WordsAndSpaces[i][0] == 32);
        bool IsLineFeed = (WordsAndSpaces[i][0] == 10);
        if(!IsSpace && (IsLineFeed || 
          TestWordWidth + CurrentLineWidth > MaximumWidth))
        {
          
          TextPerLine.Add(CurrentString);

          CurrentString = WordsAndSpaces[i];
          CurrentLineWidth = WordAndSpaceWidth[i]; //Ignore kerning.
          
          if(IsLineFeed)
            i++; //Skip the mandantory space following the line feed.
        }
        else
        {
          CurrentString &= WordsAndSpaces[i];
          CurrentLineWidth += TestWordWidth;
        }
      }
      TextPerLine.Add(CurrentString);
    }

    //Trim the whitespace at the end of each line.
    for(prim::count i = 0; i < TextPerLine.n(); i++)
    {
      prim::String& CurrentLine = TextPerLine[i];
      for(prim::count j = CurrentLine.n() - 1; j >= 0; j--)
      {
        if(CurrentLine[j] != 32 && CurrentLine[j] != 10)
        {
          CurrentLine.TrimEnd(CurrentLine.n() - 1 - j);
          break;
        }
      }
      WidthPerLine.Add(Style.Font->GetTextWidth(CurrentLine, 
        Style.unitsFontSize));
    }

    //Calculate the text area height.
    TextAreaHeight = Style.Font->GetFontHeight() * Style.unitsFontSize * 
      Style.percentageLineSpacing * (prim::number)TextPerLine.n();
  }
  else
  {
    //Single line only calculation.
    TextAreaWidth = Style.Font->GetTextWidth(Text, Style.unitsFontSize);
    TextAreaHeight = Style.Font->GetFontHeight() * Style.unitsFontSize;
  }

  //Calculate the bounding box given the width and height of the text area.
  prim::math::Vector BottomLeft = Style.relativeAnchor + 
    prim::math::Vector(1.0f, 1.0f);
  BottomLeft *= -0.5f;
  BottomLeft.x *= TextAreaWidth;
  BottomLeft.y *= TextAreaHeight;
  prim::math::Vector TopRight = BottomLeft + 
    prim::math::Vector(TextAreaWidth, TextAreaHeight);
  BottomLeft += Style.unitsPosition;
  TopRight += Style.unitsPosition;
  BoundingBox = prim::math::Rectangle(BottomLeft, TopRight);

  //Draw the text.
  if(DrawMultilineText && !OnlyReturnBoundingBox)
  {
    //Transform to starting position.
    Translate(BoundingBox.BottomLeft());
    Translate(prim::math::Vector(0, 
      -Style.Font->GetBoundingBox().BottomLeft().y * Style.unitsFontSize));
    Scale(Style.unitsFontSize);
    prim::number LineCount = 0;
    for(prim::count Line = TextPerLine.n() - 1; Line >= 0; Line--)
    {
      prim::String& Text = TextPerLine[Line];
      prim::number Indentation = 0;
      if(Line == 0)
        Indentation = Style.unitsIndentation / Style.unitsFontSize;
      Translate(prim::math::Vector(Indentation, LineCount * 
        Style.Font->GetFontHeight() * Style.percentageLineSpacing));

      //Count spaces if doing full justification.
      prim::number NumberOfSpaces = 0;
      const prim::ascii* Letter = Text.Merge();
      if(Style.Justification == prim::Justifications::Full)
        while(*Letter)
          if(prim::unicode::UTF8::Decode(Letter) == 32)
            NumberOfSpaces++;

      prim::number ExtraWidthPerSpace = 0;
      if(NumberOfSpaces > 0)
      {
        if(Line != 0)
        {
          ExtraWidthPerSpace = (Style.unitsLineWidth - WidthPerLine[Line]) / 
            NumberOfSpaces / Style.unitsFontSize;
        }
        else
        {
          ExtraWidthPerSpace = (Style.unitsLineWidth - WidthPerLine[Line] -
            Style.unitsIndentation) / NumberOfSpaces / Style.unitsFontSize;
        }
      }

      //---------Same as Single Line Algorithm Except For Insertions----------//
      //Track the advancement of the character sequence.
      prim::number Advance = 0;

      //----------Center and Right Justification----------//
      if(Style.Justification == prim::Justifications::Center)
        Advance = (Style.unitsLineWidth * 0.5f - WidthPerLine[Line] * 0.5f) /
          Style.unitsFontSize;
      else if(Style.Justification == prim::Justifications::Right)
        Advance = (Style.unitsLineWidth - WidthPerLine[Line]) /
          Style.unitsFontSize;
      //--------------------------------------------------//

      //Draw each letter as a vector-based path.
      for(prim::count i = 0; i < Text.n(); i++)
      {
        //Get the glyph of the character.
        bbs::Glyph* CurrentCharacter = 0;
        prim::count SingleCharacter = (prim::count)Text[i];
        if(SingleCharacter < Style.Font->GlyphTable.n())
          CurrentCharacter = Style.Font->GlyphTable[SingleCharacter];

        //Make sure the character exists in the font.
        if(!CurrentCharacter)
          continue;

        //Go to the character's position and draw it.
        Translate(prim::math::Vector(Advance, 0));
        DrawPath(*CurrentCharacter, false, true);
        UndoTransformation();

        //Advance position.
        Advance += CurrentCharacter->AdvanceWidth;

        //Adjust for kerning.
        bbs::Glyph* NextCharacter = 0;
        if(i < Text.n() - 1)
        {
          prim::count SingleCharacter = (prim::count)Text[i + 1];
          if(SingleCharacter < Style.Font->GlyphTable.n())
            NextCharacter = Style.Font->GlyphTable[SingleCharacter];
        }
        if(!NextCharacter)
          continue;
        for(prim::count j = 0; j < CurrentCharacter->Kern.n(); j++)
        {
          if(CurrentCharacter->Kern[j].FollowingCharacter == 
            NextCharacter->Character)
          {
            Advance += CurrentCharacter->Kern[j].HorizontalAdjustment;
            break;
          }
        }
        //-------------Full Justification-----------//
        if(LineCount != 0 && CurrentCharacter->Character == 32 && 
          Style.Justification == prim::Justifications::Full)
            Advance += ExtraWidthPerSpace;
        //------------------------------------------//
      }
      //----------------------------------------------------------------------//
      UndoTransformation();
      LineCount++;
    }

    //Undo the first three transformations.
    UndoTransformation(3);
  }
  else if(!OnlyReturnBoundingBox)
  {
    //Transform to starting position.
    Translate(BoundingBox.BottomLeft());
    Scale(Style.unitsFontSize);

    //Track the advancement of the character sequence.
    prim::number Advance = 0;

    //Draw each letter as a vector-based path.
    for(prim::count i = 0; i < Text.n(); i++)
    {
      //Get the glyph of the character.
      bbs::Glyph* Character = 0;
      prim::count SingleCharacter = (prim::count)Text[i];
      if(SingleCharacter < Style.Font->GlyphTable.n())
        Character = Style.Font->GlyphTable[SingleCharacter];

      //Make sure the character exists in the font.
      if(!Character)
        continue;

      //Go to the character's position and draw it.
      Translate(prim::math::Vector(Advance, 0));
      DrawPath(*Character, false, true);
      UndoTransformation();

      //Advance position.
      Advance += Character->AdvanceWidth;

      //Adjust for kerning.
      bbs::Glyph* NextCharacter = 0;
      if(i < Text.n() - 1)
      {
        prim::count SingleCharacter = (prim::count)Text[i + 1];
        if(SingleCharacter < Style.Font->GlyphTable.n())
          NextCharacter = Style.Font->GlyphTable[SingleCharacter];
      }
      if(!NextCharacter)
        continue;
      for(prim::count j = 0; j < Character->Kern.n(); j++)
      {
        if(Character->Kern[j].FollowingCharacter == NextCharacter->Character)
        {
          Advance += Character->Kern[j].HorizontalAdjustment;
          break;
        }
      }
    }

    //Undo the first two transformations.
    UndoTransformation(2);
  }

  if(!DrawMultilineText)
  {
    prim::number DescenderBias = 
      -Style.Font->GetBoundingBox().BottomLeft().y * Style.unitsFontSize;
    BoundingBox = prim::math::Rectangle(BoundingBox.Left(), 
      BoundingBox.Bottom() - DescenderBias, BoundingBox.Right(), 
      BoundingBox.Top() - DescenderBias);
  }
  return BoundingBox;
}

//---------------------//
//Doxygen Documentation//
//---------------------//

//---------//
//Home Page//
//---------//
/**
  \mainpage Belle, Bonne, Sage Reference
  \section intro_welcome Welcome
  Thank you for trying out <i>Belle, Bonne, Sage</i>&mdash;the
  &ldquo;beautiful, good, wise&rdquo; vector-graphics
  library dedicated solely to music notation! This manual comprises a C++
  and Lua programming reference and several tutorials. See the
  <a href="../index.html">main page</a> for more prose about the goals of the
  project.
  
  \section building Building
  <ul>
  <li> \ref buildsystem : how the build system works</li>
  <li> \ref buildsource : how to build the library and applications from the soure code</li>
  </ul>

  \section bellebonnesage_tutorials Belle, Bonne, Sage
  \subsection console_apps Console Applications
  These are C++ examples that run in a console or terminal window and output
  text and files. Here are the tutorials for the C++ part of the library:
  <ul>
  <li>  \ref tutorial1  </li>
  <li>  \ref tutorial2  </li>
  <li>  \ref tutorial3  </li>
  </ul>
  \subsection windowed_apps Windowed Applications
  These are C++ examples that combine <i>Belle, Bonne, Sage</i> and
  <a href="http://rawmaterialsoftware.com/juce.php">JUCE</a> to provide a
  graphical user interface.
  Here you can find out more about what these applications do:
  <ul>
  <li> \ref blume : a tool for geometric <i>accel.</i> and <i>decel.</i> rhythmic notation </li>
  <li> \ref choralecomposer : an environment for studying four-part music theory </li>
  </ul>
  
  \section lune_reference Lune
  Lune is a Lua-based interpreter for quickly prototyping music scores.
  Here are the tutorials for <i>Lune</i>:
  <ul>
  <li> \ref lune : an introduction to Lune</li>
  <li> \ref lua : a brief introduction to Lua programming</li>
  </ul>
*/

//------------//
//Build System//
//------------//
/** \page buildsystem System Design




*/

//------------//
//Build Source//
//------------//
/** \page buildsource Compiling from the Source

\section Prerequisites

<i>Belle, Bonne, Sage</i> has a few prerequisites that you should know about.

\subsection Compiler

\subsubsection compiler_windows Windows XP, Vista, or 7

On Windows, the Visual C++ 2005 or 2008 compiler. The 2010 edition is not
directly supported yet, but the solutions should upconvert from a Visual C++
2005 or 2008 project and compile without issue. The free Express Editions are
fine: http://www.microsoft.com/express/product/

Tip: if you are student at a university, you can download the bells-and-whistles
editions for free via the DreamSpark program (http://dreamspark.com).

In order to compile the user interfaces on XP, Vista, or 7, you will need
"Microsoft Windows SDK for Windows 7 and .NET Framework 3.5 SP1" or later
installed:
http://www.microsoft.com/downloads/details.aspx?FamilyID=c17ba869-9671-4330-a63e-1fd44e0e2505&displaylang=en
(If this link breaks in the future, search the internet for:
"download windows 7 sdk")

After you install the SDK, you need to add directories to the search paths in
Visual Studio. To do this, go to Tools -> Options, then in the left pane, select
Visual C++ Directories. Here you need to add paths to the SDK's libraries,
include, and source.

\subsubsection compiler_mac Mac OS X 10.4, 10.5, 10.6 (Tiger, Leopard, Snow Leopard)

On Mac OS X, you'll need the latest version of Xcode:
http://developer.apple.com/technology/xcode.html

\subsubsection compiler_linux Linux

You need the g++ compiler and a few support libraries. You can take care of
everything in one line:

\code 
sudo apt-get install g++ libx11-dev libasound2-dev libfreetype6-dev libxinerama-dev libglu1-mesa-dev
\endcode

\subsection svn_section SVN (Subversion)

The tip of the SVN repository is always going to have the latest improvements
and bug-fixes, so it may be worthwhile to tap into the trunk and update every
now and then. The branches of the repository will have stable releases, but will
not have all the latest improvements.

\subsubsection svn_windows Windows XP, Vista, 7
TortoiseSVN (http://tortoisesvn.tigris.org) is an excellent Explorer-plugin
interface to SVN. With this utility installed, right-click inside a folder you
want to store the repository and select SVN Checkout... The location of the
repository is:
https://bellebonnesage.svn.sourceforge.net/svnroot/bellebonnesage/trunk

To update to the latest revision, right-click on the bellebonnesage directory
and click SVN Update...

\subsubsection svn_mac Mac OS X
You will already have Subversion if you have installed the Xcode Developer
Tools. To check out the repository, launch Terminal, change to the directory you
would like to store the repository in and type:

\code
svn co https://bellebonnesage.svn.sourceforge.net/svnroot/bellebonnesage/trunk bellebonnesage
\endcode

To update to the latest revision, cd into the bellebonnesage directory and type:
svn update

\subsubsection svn_linux Linux
If you do not already have Subversion, you can get it using:
\code
sudo apt-get install subversion
\endcode

To check out use:
\code
svn co https://bellebonnesage.svn.sourceforge.net/svnroot/bellebonnesage/trunk bellebonnesage
\endcode

To update to the latest revision, cd into the bellebonnesage directory and type:
\code
svn update
\endcode

\subsection subsec_premake Premake4 (recommended)

Premake4 is a utility that generates makefiles, solutions, and projects for a
target toolset. It is useful since it allows build files to easily be generated
for the most common platforms. By installing Premake4, you will also be able to
quickly create your own projects from scratch (see the build notes).

You will need to get Premake 4.2.1 or higher:
http://sourceforge.net/projects/premake/files/

\subsubsection premake_windows Windows
Place premake4.exe in either C:\\Windows or C:\\Windows\\System32 so that it will
be visible on the system path.

You create build files for Visual Studio by running the command line. From Start
Menu, click Run, type cmd, and press enter. Then change into the bellebonnesage
directory.

To create a Visual Studio 2008 solution type:
\code
premake4 vs2008
\endcode

To create a Visual Studio 2005 solution type:
\code
premake4 vs2005
\endcode

At any point you can clear all the build files by typing:
\code
premake4 clean
\endcode

\subsubsection premake_mac Mac OS X
Install premake4 by unpacking it and typing in the terminal:
\code
sudo mv premake4 /usr/local/bin
\endcode

Then cd into the bellebonnesage directory.

To create Xcode3 projects (recommended since you can also create Universal
Binaries in Xcode):
\code
premake4 xcode3
\endcode

To create makefiles that compile directly with GCC (convenient, but only works
on the operating system platform that builds it):
\code
premake4 gmake
\endcode

\subsubsection premake_linux Linux
Install premake4 by unpacking it and typing in the terminal:
\code
sudo mv premake4 /usr/local/bin
\endcode

<b>64-bit Linux users note:</b> If a 64-bit Linux binary of Premake4 is not
available, you may need to build it from scratch. This can get tricky since
Premake is self-building.

On a platform for which one of the premake4 binaries do work, install premake4,
and download the source. Then cd into the premake4 directory and type:
\code
premake4 --os=linux --platform=x64 gmake
\endcode

Copy the build files it creates to your 64-bit machine and then type make. Then
you can install it by moving the binary to /usr/local/bin.

\section Building
Pregenerated build files are in the build directory: linux (via GCC), macosxgcc
(GCC with makefiles), xcode3, vs2005 (Visual Studio 2005), and vs2008 (Visual
Studio 2008). The compiled executable files will be placed in the
bellebonnesage/bin directory. If you generate your own makefiles using premake4
(see above), then the makefiles and/or project files will be placed in the root
level of <i>bellebonnesage</i>.

\subsection building_windows Windows using Visual Studio (build/vs2005 or build/vs2008)
Open the solution file (bellebonnesage.sln), change Debug to Release in the
toolbar, and select Build -> Build Solution from the menu. If you have installed
the 64-bit compiler, then you will also be able to select x64 (where Win32 is
displayed) in the toolbar. Note that 64-bit Windows executables only run on
64-bit Windows systems.

\subsection building_xcode Mac OS X using Xcode (build/xcode3)
Each of the targets is a separate project. To build a project, open it, and
select Build -> Build from the menu. To build a “universal” binary that can run
on Mac OS X 10.4+ and PPC and Intel architectures, select Universal32 Release
from the toolbar before building. To build a binary that can run on Mac OS X
10.5+ and take advantage of 64-bit processors when available, select Universal
Release from the toolbar.

\subsection building_macosxgcc Mac OS X using GNU Make (build/macosxgcc)
To build from the Terminal using GCC, cd to build/macosxgcc and type:
\code
make
\endcode

To build the debug binary (with symbols):
\code
make config=debug
\endcode

To clean release object files:
\code
make clean
\endcode

To clean debug object files:
\code
make clean config=debug
\endcode

\subsection building_linux Linux (build/linux)
First cd to build/linux and type:
\code
make
\endcode

To build the debug binary (with symbols):
\code
make config=debug
\endcode

To clean release object files:
\code
make clean
\endcode

To clean debug object files:
\code
make clean config=debug
\endcode
*/


//------------------------------//
//Tutorial 1 - Using the Sandbox//
//------------------------------//
/** \page tutorial1 Tutorial 1: Hello world!
\section tut1_entry_point Defining an Entry Point
<i>Belle, Bonne, Sage</i> can be used like a "sandbox" in which the application 
is treated essentially as a console application that performs some actions in a 
test sandbox area and then quits. Of course if you want you can make your own 
main() function; this is just here to speed things up for you! The code to 
create this sandbox is:

\n <b>Main.cpp</b>
\code
#include "prim.h"
using namespace prim;

void PlayInSandbox(List<String>& Arguments);

int main(count ArgumentCount, const ascii* ArgumentValues[])
{
  List<String> Arguments;
  for(count i=0;i<ArgumentCount;i++)
  {
    String Arg;
    Arg &= ArgumentValues[i];
    Arguments.Append(Arg);
  }

  PlayInSandbox(Arguments);

  Console c;
  c += "Press any key and hit return: ";
  c.WaitForKeyStroke();

  return 0;
}
\endcode
\section tut1_implementing Implementing the Sandbox
You can store this code in a seperate source file such as <b>Main.cpp</b>. 
Then you can create your sandbox file by implementing the <b>PlayInSandbox</b> 
method. Doing things this way allows you to cleanly start using <i>Belle,
Bonne, Sage</i> as a high-level language. The entire library is built on the 
premise that you should <i>never</i> need to include the C++ standard library
or any other libraries into your project, even in very complicated projects. 
With a few basic data types and access to the full gamut of mathematical 
functions you will be programming complex scores using <i>Belle, Bonne, Sage</i>
alone! The following 
tutorial prints some text to the console and then waits for the user to quit the
console application by typing a character and pressing return.

\n <b>Tutorial1.cpp</b> \code
//Include just the "prim" (primitive types) library.
#include "prim.h"

//Automatically scope into the prim namespace.
using namespace prim;

//Sandbox entry point
void PlayInSandbox(List<String>& Arguments)
{
  //Create a console object.
  Console c;

  //Prints some text to the console on the next available line.
  c += "I'm playing in the sandbox!";
  c += "And here's another line!";
  c++;
  c &= "Answer to Life, the Universe, and Everything:";
  c -= (number)42;
  c--;
  c &= "(according to Deep Thought)";

  //Print out all the arguments used to call this program.
  c += "Arguments passed to the program were:";
  for(count i = 0; i < Arguments.n(); i++)
    c += Arguments[i];
}
\endcode
\section tut1_namespaces Namespaces
<i>Belle, Bonne, Sage</i> uses namespaces and classes to organize access to 
different parts of the library. At the very heart of the library is a namespace
called <tt>prim</tt>, which stands both "primitives" and also embodies the word
"prim," which means "formally precise and proper." The goal of prim is to 
provide a high-level replacement for the C++ standard library, built from the
ground up.\n\n
An important feature of <i>Belle, Bonne, Sage</i> is that when you include its
header files, it will not pollute the global namespace with either its own methods
nor the C++ standard 
library (even though it does in fact use it in some parts at a lower level).
This means that you are free to do with the global namespace as you like, so that
if you need a method called abs(), it won't conflict with the C++ absolute value
function.\n\n
To use the namespace you can either using the scoping operator <tt>::</tt>
\code
prim::String s;
\endcode
or you can bring the namespace into scope with the <tt>using namespace ...;</tt>
statement at the beginning of a file, method, or scope block inside a method, 
such as a for loop.
\code
void foo(void)
{
  using namespace prim;
  String s;
}
\endcode
In general it is easiest to use the latter method so that you do not have to
call the scoping operator each time you need something from prim.


\section tut1_data_types Fundamental Data Types
In C++ there are a slew of data types, but nearly all of them 
are platform-architecture dependent. In other words, each data type
combines some general notion of its relative size and intended purpose, but in
most cases we either do not care about the precision so long as it is 
reasonably high, or we absolutely need to know its exact size. An \c int
is used for counting, or math, but it might also be used for storing binary data. With
no clear delineation in purpose, these data types can lead to serious 
programming errors especially across multiple platforms and architectures.\n\n
In <i>Belle, Bonne, Sage</i> we specify all numeric values as a prim type 
definition that tells us what its purpose is. The most basic data types are
prim::count, prim::integer, prim::number. A \c count is used for looping through
a number of things and is a signed data type. Its precision is largely 
irrelevant but assumed to be big enough to handle any practical number of 
looping iterations. An \c integer on the other hand
is used when doing mathematical computations with integers. We also assume that 
it is signed and can represent large numbers.\n\n
How large though? Well that is 
up to you. If you need large precision you can simply modify the primTypes.h
header file to specify its internal size. Changing this does not affect the
'counting' data type, and so you do not need to worry about how it might affect
the speed of loops for example. Likewise, you might realize you do not need more
than a few hundred of any object and so you can change the \c count data type
to a smaller size.
\n\n
The last significant data type is the \c number. A number essentially represents
a floating point number. It defaults to 32-bit precision, but you can easily
change this to 64-bit precision if you find that this provides an advantage.
\n\n C++ does provide one true abstract data type, and that is \c bool. Since \c bool
only represents the state of being true or false, there was no reason to further
abstract it as "prim::bool." In fact, bool is a good model for all other data 
types: we do not care about its size, only its ability to perform consistently
for a particular need.
\n\n
Abstracting these data types does cause one small inconvenience: you \b MUST
cast all constant decimal values. One solution is to suffix any numbers with
f (forces \c float type). If the precision of the constant is 
paramount (i.e. has several digits), then cast explicitly using \c (integer) or
\c (number). As an example:

\n <b>Casting Constant Values</b> \code
\\Don't do this:
String a = 4.67; //'number' is not necessarily double-precision, but 4.67 is.

\\Do this:
String b = 4.67f;
String c = (number)4.67; //Assures best floating-point representation.

\\Also a good idea:
String d = (integer)7948735;
\endcode

\n
There is also a data type for ASCII characters. These are represented interally
as 8-bit unsigned types. You can reference these as \c ascii for a single 
character, or if for some reason you need a constant string of them <tt>const 
ascii*</tt>. Likewise there is a data type for byte characters, which are 8-bit 
signed types. You reference these as \c byte. You sometimes use these if you 
need to read in or write to a file, an array of binary data.
\n\n
You may find you need specific data types such as a 16-bit unsigned integer if
for example you need to read or write binary data from a buffer. In this case,
prim defines fixed size data types, such as \c int16, \c uint16, \c float32, 
\c float64, etc.
\section tut1_strings Using Strings
A prim::String holds a certain number of characters encoded in the Unicode 
UTF-8 format, and is intended to hold text-only information. Strings are very
fast at appending and inserting because they are implemented as lists of 
substrings. So, when you append text to the end of a string, it will not have to
make a copy of itself, rather it simply tags on another substring. Likewise with
insertion: it will only reallocate the portions adjacent to the insertion point,
so that if the list is already fragmented, insertion will be faster! Here is
an example of many possible uses of String

\n <b>A Few Possible Uses of Strings</b> \code
String a = "Hi there!"; //Makes a string from a constant string.
String b = 10; //Makes a string from the integer 10.
String c = 34.7f //Makes a string from the decimal number 34.7.
String d = a; //Makes a string from string a.
a = "Other text"; //Sets string to different string.
b = 30; //Sets string to integer 30.
c = 9.3f; //Sets string to decimal number 9.3.
d = a; //Copys a to d.
\endcode
\n\n
\section tut1_appending Appending Strings
Appending strings to other strings and to the console output is a little
different in <i>Belle, Bonne, Sage</i>, but is very easy to do. One prominent 
distinction to be aware of is that tokens like spaces and newlines are 
typically appended before a string of characters rather than after. When
you see why, you won't want to go back to "latent" newlining!\n
\n
For all strings and consoles, we use the <tt>&=</tt>, <tt>+=</tt>, <tt>-=</tt>,
<tt>++</tt>, and <tt>--</tt> operators for appending operations. Their 
definitions are as follows:
\n\n
<b><tt>\&=</tt></b> is a literal appending operation; it simply appends the string.
\n\n
<b><tt>+=</tt></b> appends the string on the next available line. In other words 
adds a newline first, and then appends the string, BUT only adds the newline at 
the beginning IF the string/console isn't empty. This means that you don't have 
to worry about providing conditional logic for the first line.
\n\n
<b><tt>-=</tt></b> appends a space character and followed by the string.
\n\n
<b><tt>++</tt></b> adds a newline (regardless of whether it is the first line 
or not).
\n\n
<b><tt>--</tt></b> adds a space character.
\n\n
\section tut1_lists Lists
C++ is a strongly typed language, so when we make lists of a certain type of 
object, the list has to know what type it is. A prim::List is a <i>templated type
</i> which means you need to specify what kind of objects compose the list. To
do this you use the angle bracket notation <tt>prim::List\< type \> </tt>. The 
tutorial above made use of the program's argument list, which was just a 
prim::List of element type prim::String.
\n\n
Lists are internally represented as a doubly-linked list, with a built-in 
auto-iterator that keeps track of the pointer to the last element you referenced
so that if you happen to be iterating, it will not need to traverse the whole
list to find the next element. Thus, this is completely acceptable:
\code
List<String> MyList;
//... code adding elements to MyList omitted ...
for(int i = 0; i < MyList.n(); i++)
  MyList[i] = "";
\endcode
\n\n
To add elements to a list, you can use the prim::List::Add(), 
prim::List::Append(), prim::List::Prepend() methods. Use <tt>Add()</tt> if you
want the List to create a new element at the end of the list with a default 
constructor for you and pass back in its return value a reference to the 
element. <tt>Append()</tt> and <tt>Prepend()</tt> can be used when you want to
pass an existing element by reference to be <i>copied</i> in.
\n\n
Note that in C++ if you embed templated types within each other you need to put
spaces around each subtype so that it does not see <tt> \>\> </tt> and think 
you mean <tt>operator \>\> </tt>. For example a list of string lists:
\code
\\Don't do this:
List<List<String>> a;
\\Instead, do this:
List<List<String> > b;
\\Or, alternatively, do this:
List< List<String> > c;
\\But for just one templated type, this is OK:
List<String> d;
\endcode
\section tut1_console The Console
In <i>Belle, Bonne, Sage</i> the console is accessed by defining a prim::Console
object. \c Console is the functional equivalent of std::cout, but you can create
and destroy Console objects as you please. In otherwords, there isn't just one
static object that refers to the console screen. It can be whatever you call 
it! Note that creating multiple consoles doesn't create multiple console 
windows. Instead all calls to Console objects aggregate into the console window
in the order in which they are called. To suspend execution and wait for a
keystroke, you can use the prim::Console::WaitForKeyStroke() method.
*/

//------------------------------//
//Tutorial 2 - Using the Sandbox//
//------------------------------//
/** \page tutorial2 Tutorial 2: Unicode
\section tut2_entry_point Defining an Entry Point
*/

//------------------------------//
//Tutorial 3 - Using the Sandbox//
//------------------------------//
/** \page tutorial3 Tutorial 3: Creating a score
\section tut3_entry_point Defining an Entry Point
*/

//-----//
//Blume//
//-----//
/** \page blume Blume
*/

//----------------//
//Chorale Composer//
//----------------//
/** \page choralecomposer Chorale Composer
*/

//------------//
//Lua Overview//
//------------//
/** \page lua Lua Overview

A brief introduction to the Lua language intended for anyone with basic programming familiarity. The complete <a href="http://www.lua.org/manual/5.1/">Lua manual</a> can be accessed at the <a href="http://lua.org">Lua website</a>.

\section lua_comments Comments
Comments are parts of the code that are not evaluated. Comments begin with two hyphens.
\code
--This is a comment
a = 10 --This comment runs to the end of the line
\endcode

The multiline comment uses --[[ followed by a closing ]] as in this example:
\code
--[[This is a multiline comment.
It can run many
lines before it ends with the two right-brackets]]
\endcode

The combination of hyphens and brackets may seem confusing at first, but if you look down at the keyboard you will notice that the hyphen and brackets are adjacent in the order in which they appear for a multi-line comment.

\section lua_variables_values Variables and Values
A variable represents something that holds a single value. Variables themselves are not typed. Only the values they hold are typed. The most common types in Lua are:

\subsection lua_nil Nil
Stores a value that is different from all other values; it marks the absence of a meaningful value.
\code
a = nil
\endcode

\subsection lua_boolean Boolean
Stores the state of either true or false. Booleans are copied when assigned (passed by value).
\code
a = false
b = true
\endcode

\subsection lua_number Number
Stores a (floating-point) number. Numbers are copied when assigned (passed by value).
\code
a = 10
b = 42.8
\endcode

\subsection lua_string String
Stores a byte-sequence usually containing text characters. Strings are copied when assigned (passed by value).
\code
a = "hello"
\endcode

\subsection lua_table Table (i.e, list, array, vector, etc.)
Stores index-value or key-value pairs. Tables are objects containing references to other values, so only the references are copied when a table is assigned (passed by reference).

Values in tables are by default indexed using increasing whole numbers (starting at one).

\code
a = {"hello", "world"} is equivalent to:
a = {} --Create an empty table.
a[1] = "hello"
a[2] = "world"
\endcode

You can even assign names (keys) to values:
\code
b = {color="red"} is equivalent to:
b = {}
b["color"] = "red"
\endcode

When you do not specify a name, then the name defaults to a counting index:
\code
c = {"1", "2", something="else", "3"} is equivalent to:
c = {} --Create an empty table.
c[1] = "1" --Notice indices start with one in Lua (not zero)
c[2] = "2"
c["something"] = "else" --This does not affect the original counting.
c[3] = "3"
\endcode

You can also specify a different index:
\code
d = {[10]="10", "1", "2"} is equivalent to;
d = {} --Create an empty table.
d[10] = "10"
d[1] = "1"
d[2] = "2"
\endcode

You can of course nest tables in tables:
\code
xycoordinates = {{10, 4}, {3, 8}, {8, 7}}
\endcode

\subsection lua_function Function
Stores a reference to a function.
\code
function somefunction()
end
a = somefunction --Stores a reference to somefunction().
a() --Calls the function a points to, which is simply somefunction().
\endcode

\section lua_control_statements Control Statements

\subsection lua_boolean_logic Boolean Logic
Use the following keywords for boolean logic: <b>and or not</b>. Use parentheses <b>(</b> and <b>)</b> to group boolean statements together.

\subsection lua_conditional_logic Conditional Logic
Any <b>if</b> or <b>elseif</b> must be followed by a <b>then</b>. A group of one or more of these statements is concluded with <b>end</b> or <b>else ... end</b>.
\code
//Ignore the spurious highlighting...
if true then
  Does this.
end

if false and true then
  --Doesn't do this.
elseif true then
  --Does this.
end

if false and true then
  --Doesn't do this.
elseif false or not true then
  --Doesn't do this.
else
  --Does this.
end
\endcode

\section lua_defining_functions Defining and Calling Functions
Functions are defined to take a certain number of variables (delimited by commas), and can return multiple values (also delimited by commas).
\code
//Ignore the spurious highlighting...
function foo(x, y, z, w)  --Takes four arguments
  return x * y * z * w --Returns one argument (the product of x, y, z, w)
end

function bar(x, y, z) --Takes three arguments
  return x, y, z, y --Returns four arguments
end

foo(bar(1, 2, 3)) --Returns 1 * 2 * 3 * 2 = 12
\endcode

\section lua_loops_and_iteration Loops and Iteration

\subsection lua_counting_for_loop Counting loop
Count from 1 to 10 (inclusive)
\code
for i = 1, 10 do
end
\endcode

\subsection lua_counting_for_by_loop Counting loop by some amount
Count from 1 to 10 by 0.5
\code
for i = 1, 10, 0.5 do
end
\endcode

\subsection lua_iterate Iterate over a table (like list, array, or vector)
\code
x = {"a", "b", "c", something="else"}
for i, value in ipairs(x) do
  print(i, value)
end

Outputs:
1 a
2 b
3 c
something else
\endcode

\subsection lua_while While loop
Loop while no random number below 0.1 is generated
\code
while not math.random() < 0.1 do
end
\endcode

\subsection lua_repeat Repeat loop (i.e. do-until)
Loop (at least once) until a random number below 0.1 is generated
\code
repeat
until math.random() < 0.1
\endcode

\section lua_math_library Math Library
The following definitions were taken from the Lua manual.

\subsection lua_math_abs math.abs(x)

Returns the absolute value of x.

\subsection lua_math_acos math.acos(x)

Returns the arc cosine of x (in radians).

\subsection lua_math_asin math.asin(x)

Returns the arc sine of x (in radians).

\subsection lua_math_atan math.atan(x)

Returns the arc tangent of x (in radians).

\subsection lua_math_atan2 math.atan2(y, x)

Returns the arc tangent of y/x (in radians), but uses the signs of both parameters to find the quadrant of the result. (It also handles correctly the case of x being zero.)

\subsection lua_math_ceil math.ceil(x)

Returns the smallest integer larger than or equal to x.

\subsection lua_math_cos math.cos(x)

Returns the cosine of x (assumed to be in radians).

\subsection lua_math_cosh math.cosh(x)

Returns the hyperbolic cosine of x.

\subsection lua_math_deg math.deg(x)

Returns the angle x (given in radians) in degrees.

\subsection lua_math_exp math.exp(x)

Returns the value e^x.

\subsection lua_math_floor math.floor(x)

Returns the largest integer smaller than or equal to x.

\subsection lua_math_fmod math.fmod(x, y)

Returns the remainder of the division of x by y that rounds the quotient towards zero.

\subsection lua_math_frexp math.frexp(x)

Returns m and e such that x = m(2^e), e is an integer and the absolute value of m is in the range [0.5, 1) (or zero when x is zero).

\subsection lua_math_huge math.huge

The value HUGE_VAL, a value larger than or equal to any other numerical value.

\subsection lua_math_ldexp math.ldexp(m, e)

Returns m(2^e) (e should be an integer).

\subsection lua_math_log math.log(x)

Returns the natural logarithm of x.

\subsection lua_math_log10 math.log10(x)

Returns the base-10 logarithm of x.

\subsection lua_math_max math.max(x, ...)

Returns the maximum value among its arguments.

\subsection lua_math_min math.min(x, ...)

Returns the minimum value among its arguments.

\subsection lua_math_modf math.modf(x)

Returns two numbers, the integral part of x and the fractional part of x.

\subsection lua_math_pi math.pi

The value of pi.

\subsection lua_math_pow math.pow(x, y)

Returns x to the yth power. (You can also use the expression x^y to compute this value.)

\subsection lua_math_rad math.rad(x)

Returns the angle x (given in degrees) in radians.

\subsection lua_math_random math.random([m [, n]])

This function is an interface to the simple pseudo-random generator function rand provided by ANSI C. (No guarantees can be given for its statistical properties.)

When called without arguments, returns a uniform pseudo-random real number in the range [0,1). When called with an integer number m, math.random returns a uniform pseudo-random integer in the range [1, m]. When called with two integer numbers m and n, math.random returns a uniform pseudo-random integer in the range [m, n].

\subsection lua_math_randomseed math.randomseed(x)

Sets x as the "seed" for the pseudo-random generator: equal seeds produce equal sequences of numbers.

\subsection lua_math_sin math.sin(x)

Returns the sine of x (assumed to be in radians).

\subsection lua_math_sinh math.sinh(x)

Returns the hyperbolic sine of x.

\subsection lua_math_sqrt math.sqrt(x)

Returns the square root of x. (You can also use the expression x^0.5 to compute this value.)

\subsection lua_math_tan math.tan(x)

Returns the tangent of x (assumed to be in radians).

\subsection lua_math_tanh math.tanh(x)

Returns the hyperbolic tangent of x. 
*/

//----------------------------//
//Lune Manual: How to Use Lune//
//----------------------------//
/** \page lune Using Lune

*/

