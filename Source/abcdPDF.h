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

#ifndef abcdPDF
#define abcdPDF

#include "primString.h"

/**Contains supplied representations of the abstract classes. You can derive
these or create your own. The acronym stands for ABraCaDabra, or,
Another BBS Context Definition.*/
namespace abcd
{
  /**Preliminary support for outputting Portable Document Files. The PDF class
  supports multiple pages and layers, and currently conforms to the
  PDF/X-1a:2001 standard (a subset of PDF tailored for reliable printing).
  Also supported are embedded TrueType fonts and kerning; however, there is
  not yet support for Unicode encoding (though the prim::String class fully
  supports UTF-8).*/
  class PDF : public bbs::abstracts::Painter
  {
    friend class bbs::abstracts::Portfolio;
  public:
    /**\brief Properties structure to supply the PDF class with additional
    PDF-specific information.*/
    class Properties : public bbs::abstracts::Painter::Properties
    {
    public:
      /**A scaling value which is applied to all geometry. For Adobe's engine,
      this makes no difference because their curve generating algorithm depends
      on the resolution of the display it is drawing to. On other poorly 
      designed renderers, the number of interpolations is proportional to the
      literal values of the numbers involved. This results in the renderer 
      becoming to slow for very large numbers and too blocky for small numbers.
      The suggested value is 10000 as this will put it into the range of the
      PDFs they typically encounter. Update: it appears that software programs
      no longer are affected by the multiplier, and it is difficult to deal with
      the multiplier when images are involved due to the image space being
      constrained. It is recommended that this value remain at unit scale (keep
      at 1.0, the new default).*/
      prim::number CTMMultiplier;
      prim::String Filename;
      prim::String Output;
      prim::byte* ExtraData;
      prim::count ExtraDataLength;
      
      /**Controls the device color space to use. By default CMYK is used since
      it is better represents how a page will print in color. This will change
      some of the hues slightly, and blacks will not be completely black on the
      screen. RGB might be preferable though if only screen output is a
      concern.*/
      bool UseCMYKInsteadOfRGB;

      Properties() : CTMMultiplier(1.0f), ExtraData(0),
      ExtraDataLength(0), UseCMYKInsteadOfRGB(true) {}
    };

    ///Method to search an existing PDF file for BBS created metadata.
    static prim::count RetrievePDFMetadata(prim::String Filename, 
      prim::byte*& ByteData)
    {
      //Open up the file.
      using namespace prim;
      byte* WholeFile = 0;
      count WholeFileLength = File::Read(Filename.Merge(), WholeFile);
      
      //Search for the GUID code.
      String Code = "|DF62391C-36D3-4DFD-83EE-5B61177426FE|";
      byte* CodeBytes = (byte*)Code.Merge();
      count CodeLength = Code.ByteLength();
      count SearchLimit = WholeFileLength - CodeLength - 100;
      prim::count HexStart = 0;
      prim::count HexLength = 0;

      for(count i = 0; i < SearchLimit; i++)
      {
        bool FoundIndex = true;

        for(count j = 0; j < CodeLength; j++)
          if(WholeFile[i + j] != CodeBytes[j])
          {
            FoundIndex = false;
            break;
          }

        if(FoundIndex)
        {
          HexStart = i + CodeLength;
          for(count k = HexStart; k < SearchLimit; k++)
          {
            if(WholeFile[k] == (byte)'|')
            {
              HexLength = k - HexStart;
              break;
            }
          }
          break;
        }
      }

      if(!HexStart || !HexLength)
      {
        ByteData = 0;
        delete WholeFile;
        return 0;
      }

      count DataLength = HexLength / 2;
      ByteData = new byte[DataLength];

      count j = HexStart;
      for(count i = 0; i < DataLength; i++)
      {
        byte b1 = WholeFile[j];
        byte b2 = WholeFile[j + 1];

        if(b1 >= (byte)'0' && b1 <= (byte)'9')
          b1 -= (byte)'0';
        else if(b1 >= (byte)'A' && b1 <= (byte)'F')
          b1 -= (byte)'A' - 10;
        else
        {
          delete ByteData;
          delete WholeFile;
          return 0;
        }

        if(b2 >= (byte)'0' && b2 <= (byte)'9')
          b2 -= (byte)'0';
        else if(b2 >= (byte)'A' && b2 <= (byte)'F')
          b2 -= (byte)'A' - 10;
        else
        {
          delete ByteData;
          delete WholeFile;
          return 0;
        }

        ByteData[i] = (b1 << 4) + b2;

        j += 2;
      }

      delete WholeFile;
      return DataLength;
    }

    ///Helper function to quickly get the metadata out of a PDF as a string.
    static void RetrievePDFMetadataAsString(prim::String Filename, 
      prim::String& Metadata)
    {
      prim::byte* ByteData = 0;
      prim::count DataLength = RetrievePDFMetadata(Filename, ByteData);
      Metadata.Clear();

      if(ByteData && DataLength)
        Metadata.Append((prim::ascii*)ByteData, DataLength);

      delete ByteData;
    }

  protected:
    ///An internal representation of PDF objects
    struct Object
    {
      /**Stores the information for an embedded cross-reference. Whenever
      an object's stream cross-references another object, instead of
      immediately committing the reference, the insertion point and object
      pointer are saved so that after all the streams are committed, the
      insertions are made and the references are committed. This allows
      the objects to be instantiated and ordered arbitrarily.*/
      struct XRef
      {
        ///Pointer to the object that this XRef references
        Object*     ObjectToReference;

        ///The point of insertion in the string
        prim::count InsertionPoint;

        ///Default constructor zeroes the structure.
        XRef() : ObjectToReference(0), InsertionPoint(0) {}

        /**\brief This constructor allows the structure's data members
        to be passed as arguments.*/
        XRef(Object* ObjectToReference, prim::count InsertionPoint)
        {
          XRef::ObjectToReference = ObjectToReference;
          XRef::InsertionPoint = InsertionPoint;
        }
      };

      ///This object's cross-reference index
      prim::count XRefIndex;

      /**\brief This object's cross-reference offset from the beginning of
      the file*/
      prim::count XRefOffset;

      /**\brief Indicates whether or not the content stream's double
      brackets should be automatically included.*/
      bool NoAutoBrackets;

      ///The object's PDF dictionary
      prim::String Dictionary;

      ///The object's PDF content stream
      prim::String Content;

      /**\brief A list of pending cross-references to be inserted into
      dictionaries*/
      prim::List<XRef> DictionaryXRefs;

      /**\brief A list of pending cross-references to be inserted into
      content streams*/
      prim::List<XRef> ContentXRefs;

      /**\brief Default constructor turns on auto-brackets and zeroes
      everything else.*/
      Object() : XRefIndex(0), XRefOffset(0), NoAutoBrackets(false) {}

      /**\brief Inserts an object cross-reference to be committed to the
      current end of the dictionary string.*/
      void InsertDictionaryXRef(Object* ObjectToReference)
      {
        XRef ObjectXRef(ObjectToReference, Dictionary.n());
        DictionaryXRefs.Append(ObjectXRef);
      }

      /**\brief Inserts an object cross-reference to be committed to the
      current end of the content stream string.*/
      void InsertContentXRef(Object* ObjectToReference)
      {
        XRef ObjectXRef(ObjectToReference, Content.n());
        ContentXRefs.Append(ObjectXRef);
      }

      ///Commits a list of cross-references to an object string.
      void CommitXRefList(prim::List<XRef>& XRefList,
        prim::String& ObjectString)
      {
        using namespace prim;

        /**\brief Commit the cross-references by going through the list
        of XRefs and inserting the appropriate data into the string.
        \details Each insertion adds a few characters to the string, so
        this error must be corrected by keeping track of the insertion
        widths in InsertionBias.*/
        prim::count InsertionBias=0;
        for(count i=0;i<XRefList.n();i++)
        {
          XRef& CurrentXRef = XRefList[i];

          prim::count IndexOfReferent =
            CurrentXRef.ObjectToReference->XRefIndex;
          prim::count InsertionPointOfReferent =
            CurrentXRef.InsertionPoint;
          prim::count FinalInsertionPoint =
            InsertionBias + InsertionPointOfReferent;

          prim::String XRefString;
          XRefString &= (integer)IndexOfReferent;
          XRefString &= " 0 R";

          ObjectString.Insert(XRefString, FinalInsertionPoint);

          InsertionBias += XRefString.ByteLength();
        }
      }
    };

    ///A list of the objects which compose the PDF file.
    prim::List<Object*> Objects;

    /**A pointer to the currently active raster target. This object pointer
    is used by the drawing methods.*/
    Object* RasterObject;

    ///A list of pointers to valid Font objects.
    prim::List<bbs::Font*> FontList;
    
    ///The number of images used.
    prim::List<Object*> ImageList;

    ///Default constructor for the PDF painter
    PDF() : RasterObject(0) {}

    ///Properties of the PDF file
    PDF::Properties* PDFProperties;

    /**Internal method creates a new PDF object. PDF files are made of
    objects which are marked off by 1 0 obj and endobj. By storing these
    objects in a list, the actual indexing process can be deferred until the
    conclusion of data writing, so that the file can be optimally organized
    for debugging purposes.*/
    Object* CreatePDFObject(void)
    {
      Object* newPDFObject = new Object;
      Objects.Append(newPDFObject);
      return newPDFObject;
    }

    /**\brief Writes all of the objects to a single flat stream as a string
    referenced in the parameter list.*/
    void CommitObjects(prim::String& ByteStream)
    {
      //Include the namespaces.
      using namespace prim;
      using namespace prim::unicode::latin;
      using namespace bbs::abstracts;

      //Put the objects in the same order they were created.
      for(prim::count i=0;i<Objects.n();i++)
        Objects[i]->XRefIndex=i+1;

      //The first object in this case will be the root.
      Object* RootObject=Objects.first();

      //The info (metadata) object will be the second object.
      Object* InfoObject=Objects[1];

      //Commit the indexes to each of the objects.
      for(prim::count i=0;i<Objects.n();i++)
      {
        //Commit the dictionary cross-references.
        Objects[i]->CommitXRefList(
          Objects[i]->DictionaryXRefs,Objects[i]->Dictionary);

        //Commit the content cross-references.
        Objects[i]->CommitXRefList(
          Objects[i]->ContentXRefs,Objects[i]->Content);
      }

      //Write the header.
      ByteStream = "%PDF-1.3"; //Can be adjusted as necessary.
      ByteStream += "%";
      ByteStream.Append(
        diacritics::a_Circumflex,
        diacritics::a_Tilde,
        diacritics::I_Umlaut,
        diacritics::O_Acute);
      ByteStream++;

      //Write the objects.
      for(prim::count XRefObject=1;XRefObject<Objects.n()+1;XRefObject++)
      {
        //Find the Object by looking it up by XRef object number.
        Object* CurrentObject=0;
        for(prim::count LookupIndex=0;LookupIndex<Objects.n();LookupIndex++)
        {
          CurrentObject=Objects[LookupIndex];
          if(CurrentObject->XRefIndex==XRefObject)
            break;
        }

        /*Save the XRef offset of this object to help with the object
        table of contents at the end of the file.*/
        CurrentObject->XRefOffset=ByteStream.ByteLength();

        //Begin the object.
        ByteStream &= (integer)XRefObject;
        ByteStream &= " 0 obj";
        ByteStream++;

        //Decide whether or not to make brackets appear.
        if(!CurrentObject->NoAutoBrackets)
        {
          ByteStream &= "<<";
          ByteStream++;
        }

        //Write the dictionary.
        ByteStream &= CurrentObject->Dictionary;
        ByteStream++;

        //Close brackets if they were done before.
        if(!CurrentObject->NoAutoBrackets)
        {
          ByteStream &= ">>";
          ByteStream++;
        }

        //If there is a content stream then write it.
        if(CurrentObject->Content.n()>0)
        {
          ByteStream &= "stream";
          ByteStream++;
          ByteStream &= CurrentObject->Content;
          ByteStream++;
          ByteStream &= "endstream";
          ByteStream++;
        }

        //End the object.
        ByteStream &= "endobj";
        ByteStream++;
        ByteStream++;
      }

      //Write the XRef table of contents found at the end of the PDF file.
      prim::count XRefLocation = ByteStream.ByteLength();
      ByteStream &= "xref";
      ByteStream += "0 ";
      ByteStream &= (integer)(Objects.n() + 1);
      ByteStream += "0000000000 65535 f";
      ByteStream.Append(13,10);

      //Write each XRef entry.
      for(count ObjectIndex=1;ObjectIndex<Objects.n()+1;ObjectIndex++)
      {
        //Again find the current object by looking it up by XRef index.
        Object* CurrentObject=0;
        for(count LookupIndex=0;LookupIndex<Objects.n();LookupIndex++)
        {
          CurrentObject=Objects[LookupIndex];
          if(CurrentObject->XRefIndex==ObjectIndex)
            break;
        }

        ByteStream.AppendInteger(CurrentObject->XRefOffset,10);
        ByteStream &= " 00000 n";
        /*Note: PDF Reference states: "each line is 20 bytes long." This
        implies that we MUST use a CR + LF line encoding.*/
        ByteStream.Append(13,10);
      }

      //Write the PDF trailer.
      ByteStream &= "trailer";
      ByteStream += "<<";
      ByteStream += "/Size ";
      ByteStream &= (integer)(Objects.n() + 1L);
      ByteStream += "/Root ";
      ByteStream &= (integer)RootObject->XRefIndex;
      ByteStream &= " 0 R";
      ByteStream += "/Info ";
      ByteStream &= (integer)InfoObject->XRefIndex;
      ByteStream &= " 0 R";
      ByteStream += "/ID[<8F64B905EA13AD4AAE6094175973E02D>";
      ByteStream &= "<8B911DB58AB86C44BFD52F30772A298C>]";
      ByteStream += ">>";
      ByteStream += "startxref";
      ByteStream += (integer)XRefLocation;
      ByteStream += "%%EOF";

      //Delete each object manually now that they are no longer necessary.
      for(int i=0;i<Objects.n();i++)
        delete Objects[i];

      //Remove everything in the list.
      Objects.RemoveAll();
    }

    virtual void CommitASCIITrueTypeFont(bbs::Font* ASCIITrueTypeFont,
      Object* Parent, Object* Dictionary, Object* Program)
    {
      //----------------------------------------------------------//
      //                                                          //
      //BELOW HERE IS A NON-UNICODE IMPLEMENTATION WITH ONLY 1-127//
      //                                                          //
      //----------------------------------------------------------//
      using namespace prim;
      using namespace prim::math;
      using namespace prim::typeface;
      using namespace bbs;

      TrueType* TTFont = ASCIITrueTypeFont->Program->Interface<TrueType>();
      if(!TTFont)
        return;

      String RandomFontName = "EmbeddedASCIITrueTypeFont-";
      RandomFontName.AppendInteger(Rand(1,(integer)99999),5);

      Parent->Dictionary += "/Type /Font";
      Parent->Dictionary += "/Subtype /TrueType";
      Parent->Dictionary += "/BaseFont /";
      Parent->Dictionary &= RandomFontName;
      Parent->Dictionary += "/Encoding /WinAnsiEncoding";
      Parent->Dictionary += "/FontDescriptor ";
      Parent->InsertDictionaryXRef(Dictionary);

      //Advance widths
      Parent->Dictionary += "/FirstChar 0";
      Parent->Dictionary += "/LastChar 255";
      Parent->Dictionary += "/Widths [";
      number NullWidth=0;
      number SmallLWidth=0;
      for(count i=0;i<=255;i++)
      {
        prim::uint16 Code = (prim::uint16)i;

        prim::uint16 Glyph = TTFont->CMAP.GlyphIndexFromCharacterCode(Code);

        //Not sure why sometimes we get out of bound glyphs.
        if(Glyph>=TTFont->HMTX.NumberOfHMetrics)
          Glyph=0;

        prim::uint16 AdvanceWidth = TTFont->HMTX.AdvanceWidths[Glyph];

        number KiloUnitsWidth = (number)AdvanceWidth /
          (number)TTFont->HEAD.UnitsPerEm * (number)1000.0;
        Parent->Dictionary -= (KiloUnitsWidth);
        if(i==0)
          NullWidth=KiloUnitsWidth;
        else if(i==108)
          SmallLWidth=KiloUnitsWidth;
      };
      Parent->Dictionary -= "]";

      //FONT DESCRIPTOR
      //==================================================================
      Dictionary->Dictionary += "/Type /FontDescriptor";

      //Font name
      Dictionary->Dictionary += "/FontName /";
      Dictionary->Dictionary &= RandomFontName;

      //Flags (these are just cues for the interpreter)
      Dictionary->Dictionary += "/Flags 262178";

      //Bounding box
      Dictionary->Dictionary += "/FontBBox [";
      Dictionary->Dictionary -= (integer)TTFont->HEAD.xMin;
      Dictionary->Dictionary -= (integer)TTFont->HEAD.yMin;
      Dictionary->Dictionary -= (integer)TTFont->HEAD.xMax;
      Dictionary->Dictionary -= (integer)TTFont->HEAD.yMax;
      Dictionary->Dictionary -= "]";

      //Vertical stem width (need to calculate from width of 'l')
      Dictionary->Dictionary += "/StemV";
      Dictionary->Dictionary -= SmallLWidth;

      //Missing width
      Dictionary->Dictionary += "/MissingWidth";
      Dictionary->Dictionary -= NullWidth;

      //Capital height
      Dictionary->Dictionary += "/CapHeight";
      Dictionary->Dictionary -= (integer)TTFont->OS2.CapHeight;

      //Ascender height
      Dictionary->Dictionary += "/Ascent";
      Dictionary->Dictionary -= (integer)TTFont->HHEA.Ascender;

      //Descender height
      Dictionary->Dictionary += "/Descent";
      Dictionary->Dictionary -= (integer)TTFont->HHEA.Descender;

      //Italic angle
      Dictionary->Dictionary += "/ItalicAngle";
      number ItalicAngle =
        (number)TTFont->POST.ItalicAngleInteger +
        (number)TTFont->POST.ItalicAngleDecimal / (number)65536.0;
      Dictionary->Dictionary -= ItalicAngle;

      //Font program
      Dictionary->Dictionary += "/FontFile2 ";
      Dictionary->InsertDictionaryXRef(Program);

      //EMBEDDING
      //==================================================================
      String HexEncodedFontProgram;
      prim::Text::EncodeDataAsHexString(
        ASCIITrueTypeFont->ProgramData,
        ASCIITrueTypeFont->ProgramDataByteLength,
        HexEncodedFontProgram);

      Program->Content &= HexEncodedFontProgram;
      Program->Dictionary &= "/Filter /ASCIIHexDecode";
      Program->Dictionary += "/Length ";
      Program->Dictionary &= (integer)HexEncodedFontProgram.n();
      Program->Dictionary += "/Length1 ";
      Program->Dictionary &= (integer)ASCIITrueTypeFont->ProgramDataByteLength;
    }

    virtual void Paint(bbs::abstracts::Portfolio* PortfolioToPaint,
      bbs::abstracts::Painter::Properties* PortfolioProperties)
    {

      //Include the prim namespace.
      using namespace prim;
      using namespace bbs::abstracts;

      //Ensure that we have a valid PDF::Properties object pointer.
      Properties* p = PortfolioProperties->Interface<Properties>();
      if(!p)
        return;

      //Save for later reference by other methods.
      PDFProperties = p;

      //Create the main object entries in the PDF.
      Object* Catalog = CreatePDFObject(); //must be 1 0 R
      Object* Info = CreatePDFObject(); //must be 2 0 R
      Object* Metadata = CreatePDFObject();
      Object* Pages = CreatePDFObject();
      Object* FontCatalog = CreatePDFObject();
      Object* ImageCatalog = CreatePDFObject();
      Object* OutputIntent = CreatePDFObject();

      //Create a catalog of the pages.
      Catalog->Dictionary = "/Type /Catalog";
      Catalog->Dictionary += "/Pages ";
      Catalog->InsertDictionaryXRef(Pages);
      Catalog->Dictionary += "/Metadata ";
      Catalog->InsertDictionaryXRef(Metadata);
      Catalog->Dictionary += "/OutputIntents [ ";
      Catalog->InsertDictionaryXRef(OutputIntent);
      Catalog->Dictionary &= " ]";

      //Grab the canvas list from the portfolio.
      List<Portfolio::Canvas*>& cl  = PortfolioToPaint->Canvases;

      //An internal list of page content objects.
      List<Object*> PageObjects;

      //Loop through each canvas and commit it to a PDF page.
      for(count i=0;i<cl.n();i++)
      {
        //Create objects for page header and content information.
        Object* PageHeader = CreatePDFObject();
        Object* PageContent = RasterObject = CreatePDFObject();
        PageObjects.Append(PageHeader);

        //Write the page's dictionary.
        PageHeader->Dictionary &= "/Type /Page";
        PageHeader->Dictionary += "/Parent ";
        PageHeader->InsertDictionaryXRef(Pages);
        PageHeader->Dictionary += "/Contents ";
        PageHeader->InsertDictionaryXRef(PageContent);
        PageHeader->Dictionary += "/MediaBox [ 0 0";
        PageHeader->Dictionary -= cl[i]->Dimensions.x;
        PageHeader->Dictionary -= cl[i]->Dimensions.y;
        PageHeader->Dictionary -= "]";

        PageHeader->Dictionary += "/CropBox [ 0 0";
        PageHeader->Dictionary -= cl[i]->Dimensions.x;
        PageHeader->Dictionary -= cl[i]->Dimensions.y;
        PageHeader->Dictionary -= "]";

        PageHeader->Dictionary += "/TrimBox [ 0 0";
        PageHeader->Dictionary -= cl[i]->Dimensions.x;
        PageHeader->Dictionary -= cl[i]->Dimensions.y;
        PageHeader->Dictionary -= "]";

        //Write out a reference to the catalog of fonts.
        PageHeader->Dictionary += "/Resources";
        {
          PageHeader->Dictionary += "  <<";
          PageHeader->Dictionary += "    /Font ";
          PageHeader->InsertDictionaryXRef(FontCatalog);
          PageHeader->Dictionary += "    /XObject ";
          PageHeader->InsertDictionaryXRef(ImageCatalog);          
          PageHeader->Dictionary += "  >>";
        }

        //Get a reference to the list of layers.
        List<Portfolio::Canvas::Layer*>& ll = cl[i]->Layers;

        /*Convert device space into inches and divide by the
        CTMMultiplier, which allows applications which have static
        curve segmenting algorithms to produce smoother curves. For
        example FoxIt apparently uses the unit value as its step for
        segmentation meaning that if you are operating in inches then
        you have no chance of getting a smooth curve. Working in a
        "multiplied" CTM (in which the vectors themselves are multiplied
        by a number, allows the smoothing methods to work well on the
        unit assumption (which is not part of the PDF standard, and a
        poor algorithm, but it is a popular alternative viewer...)*/
        number CTMInches = (number)72 / p->CTMMultiplier;
        PageContent->Content += CTMInches;
        PageContent->Content -= "0 0";
        PageContent->Content -= CTMInches;
        PageContent->Content -= "0 0 cm";
        

        if(PDFProperties->UseCMYKInsteadOfRGB)
        {
          //Use CMYK color (to do rough conversions later from RGB).
          PageContent->Content += "/DeviceCMYK cs";
          PageContent->Content += "/DeviceCMYK CS";
        }
        else
        {
          //Use RGB color which does not require conversion.
          PageContent->Content += "/DeviceRGB cs";
          PageContent->Content += "/DeviceRGB CS";
        }


        //Save transformation matrix.
        PageContent->Content += "q";

        //Draw the main canvas layer.
        cl[i]->Paint(this);

        //Loop through each layer in the canvas and draw it.
        for(count j=0;j<ll.n();j++)
        {
          //Save transformation matrix.
          PageContent->Content += "q";

          //Draw individual layer.
          ll[j]->Paint(this);

          //Revert the transformation matrix.
          PageContent->Content += "Q";
        }

        //Revert the transformation matrix.
        PageContent->Content += "Q";

        //Write the page content object's dictionary.
        PageContent->Dictionary &= "/Length";
        PageContent->Dictionary -= (integer)PageContent->Content.ByteLength();

        //Set the current drawing target to null to be safe.
        RasterObject = 0;
      }

      //Write the table of contents for the pages.
      Pages->Dictionary="/Type /Pages";
      Pages->Dictionary++;
      Pages->Dictionary&="/Kids [";
      for(count i=0;i<PageObjects.n();i++)
      {
        if(i!=0)
          Pages->Dictionary&=" ";

        Pages->InsertDictionaryXRef(PageObjects[i]);
      }
      Pages->Dictionary&="]";
      Pages->Dictionary++;
      Pages->Dictionary&="/Count ";
      Pages->Dictionary&=(integer)cl.n();

      /*Now actually create the embedded fonts. Notice that the pages
      actually referenced the fonts before they were instantiated due to
      the deferred object writing mechanism.*/
      for(prim::count i = 0; i < FontList.n(); i++)
      {
        Object* FontParent=CreatePDFObject();
        Object* FontDictionary=CreatePDFObject();
        Object* FontProgram=CreatePDFObject();
        //UNICODE ONLY: Object* fontUnicodeMap=CreatePDFObject();

        CommitASCIITrueTypeFont(FontList[i],
          FontParent, FontDictionary, FontProgram);

        FontCatalog->Dictionary += "/F";
        FontCatalog->Dictionary &= (integer)i;
        FontCatalog->Dictionary &= " ";
        FontCatalog->InsertDictionaryXRef(FontParent);
      }
      
      //Create the catalog of images.
      for(prim::count i = 0; i < ImageList.n(); i++)
      {
        ImageCatalog->Dictionary += "/Im";
        ImageCatalog->Dictionary &= (integer)i;
        ImageCatalog->Dictionary &= " ";
        ImageCatalog->InsertDictionaryXRef(ImageList[i]);
      }

      //Create the info object.
      Info->Dictionary += "/Title ()";
      Info->Dictionary += "/Author ()";
      Info->Dictionary += "/Creator ()";
      Info->Dictionary += "/Producer ()";
      Info->Dictionary += "/CreationDate (D:20080719142857-05'00')";
      Info->Dictionary += "/ModDate (D:20080719142857-05'00')";
      Info->Dictionary += "/Trapped /False";
      Info->Dictionary += "/GTS_PDFXVersion (PDF/X-1:2001)";
      Info->Dictionary += "/GTS_PDFXConformance (PDF/X-1a:2001)";

      Metadata->Content += "<?xpacket begin=\"";
      Metadata->Content.Append(
        unicode::latin::diacritics::i_Umlaut,
        unicode::latin::punctuation::RightDoubleAngleQuote,
        unicode::latin::punctuation::InvertedQuestionMark);
      Metadata->Content &= "\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>";

      //Extra metadata...
      if(p->ExtraData)
      {
        /*Write the tag that contains a GUID identifying to a data importer
        that this section unambiguously contains info encoded in hex code.*/
        Metadata->Content += 
          "<extra>|DF62391C-36D3-4DFD-83EE-5B61177426FE|";
        
        //Write the data as hex in case some of it is binary.
        String HexString;
        Text::EncodeDataAsHexString(p->ExtraData, p->ExtraDataLength,
          HexString);
        Metadata->Content &= HexString;

        //Write a pipe to indicate the termination of the hex stream.
        Metadata->Content &= "|</extra>";
      }

      Metadata->Content += "<?xpacket end=\"w\"?>";

      Metadata->Dictionary += "/Type /Metadata";
      Metadata->Dictionary += "/Subtype /XML";
      Metadata->Dictionary += "/Length";
      Metadata->Dictionary -= (integer)Metadata->Content.ByteLength();

      //Create the output intent for PDF-X compliance.
      OutputIntent->Dictionary += "/Type /OutputIntent";
      OutputIntent->Dictionary += "/OutputConditionIdentifier (sRGB)";
      OutputIntent->Dictionary += "/S /GTS_PDFX";
      OutputIntent->Dictionary += "/RegistryName (http://www.color.org)";

      //Commit all of the objects to the output string.
      CommitObjects(p->Output);

      //If applicable send the output to file.
      if(p->Filename != "")
        File::Replace(p->Filename.Merge(), p->Output.Merge());
    }

    //---------------//
    //Drawing Methods//
    //---------------//

  protected:
    void Rasterize(prim::String t)
    {
      if(RasterObject)
        RasterObject->Content += t;
    }

    //--------------//
    //Transformation//
    //--------------//

    virtual void Transform(const prim::math::StateMatrix<RasterState>& m)
    {
      using namespace prim;
      using namespace prim::math;

      State.Push(m);
      String t;
      number CTMMultiplier = PDFProperties->CTMMultiplier;
      t += "q";
      t += m.a;
      t -= m.b;
      t -= m.c;
      t -= m.d;
      t -= m.e * CTMMultiplier;
      t -= m.f * CTMMultiplier;
      t -= "cm";
      Rasterize(t);
    }

    virtual void UndoTransformation(prim::count NumberToUndo = 1)
    {
      using namespace prim;
      using namespace prim::math;

      String t;
      for(count i = 0; i < NumberToUndo; i++)
      {
        t += "Q";
        State.Pop();
      }
      Rasterize(t);
    }

    //-------------//
    //Raster States//
    //-------------//

    virtual void StrokeColor(prim::colors::RGB Color)
    {
      using namespace prim::colors;
      
      bbs::abstracts::Painter::StrokeColor(Color);

      prim::String t;
      if(PDFProperties->UseCMYKInsteadOfRGB)
      {
        t += Component::c(Color);
        t -= Component::m(Color);
        t -= Component::y(Color);
        t -= Component::k(Color);
      }
      else
      {
        t += Component::r(Color);
        t -= Component::g(Color);
        t -= Component::b(Color);
      }

      t -= "SC";
      Rasterize(t);
    }

    virtual void FillColor(prim::colors::RGB Color)
    {
      using namespace prim::colors;

      bbs::abstracts::Painter::FillColor(Color);

      prim::String t;
      if(PDFProperties->UseCMYKInsteadOfRGB)
      {
        t += Component::c(Color);
        t -= Component::m(Color);
        t -= Component::y(Color);
        t -= Component::k(Color);
      }
      else
      {
        t += Component::r(Color);
        t -= Component::g(Color);
        t -= Component::b(Color);
      }
      t -= "sc";
      Rasterize(t);
    }
    
    //------//
    //Images//
    //------//
    public:
    //Helpers
    static void GetJPEGImageSize(const prim::byte* ByteArray, prim::count Bytes,
      prim::count& Width, prim::count& Height)
    {
      Width = 0;
      Height = 0;
      prim::count i = 0;
      while(i < Bytes - 9)
      {
        prim::byte Code = ByteArray[i]; i++;
        
        if(Code != 0xFF)
          return;
        Code = ByteArray[i]; i++;
        switch(Code)
        {
          //Filler byte
          case 0xFF:
            i--;
            break;
          //Packets without data
          case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: 
          case 0xD5: case 0xD6: case 0xD7: case 0xD8: case 0xD9:
            break;
          //Packets with size information
          case 0xC0: case 0xC1: case 0xC2: case 0xC3:
          case 0xC4: case 0xC5: case 0xC6: case 0xC7:
          case 0xC8: case 0xC9: case 0xCA: case 0xCB:
          case 0xCC: case 0xCD: case 0xCE: case 0xCF:
            i += 3;
            {
            prim::uint16 h = ((prim::uint16)ByteArray[i] << 8) |
              (prim::uint16)ByteArray[i + 1]; i += 2;
            prim::uint16 w = ((prim::uint16)ByteArray[i] << 8) |
              (prim::uint16)ByteArray[i + 1]; i += 2;
            Width = w;
            Height = h;
            }
            return;
          //Irrelevant variable-length packets
          default:
            prim::uint16 Length = ((prim::uint16)ByteArray[i] << 8) |
              (prim::uint16)ByteArray[i + 1]; i += 2;
            i += (prim::count)Length - 2;
            break;
        }
      }
    }
    
    static void GetJPEGImageSize(prim::String Filename, prim::count& Width,
      prim::count& Height)
    {
      prim::byte* ByteArray = 0;
      prim::count Bytes = prim::File::Read(Filename, ByteArray);
      GetJPEGImageSize(ByteArray, Bytes, Width, Height);
      delete ByteArray;      
    }
    
    protected:
    virtual void DrawJPEG(prim::String Filename, prim::number Width = 1.0f,
      prim::number Height = 0.0f)
    {
      //Attempt to load in the data from file.
      prim::byte* ByteArray = 0;
      prim::count Bytes = prim::File::Read(Filename, ByteArray);
      prim::count PixelsWidth = 0, PixelsHeight = 0;
      GetJPEGImageSize(ByteArray, Bytes, PixelsWidth, PixelsHeight);
      prim::number AspectRatio = (prim::number)PixelsHeight /
        (prim::number)PixelsWidth;
      if(Width == 0.00f && Height != 0.0f)
        Width = Height / AspectRatio;
      else if(Height == 0.00f && Width != 0.0f)
        Height = Width * AspectRatio;
      else if(Width == 0.0f && Height == 0.0f)
      {
        delete ByteArray;
        return;
      }
      
      prim::String HexString;
      prim::Text::EncodeDataAsHexString(ByteArray, Bytes, HexString);
      HexString &= ">";
      delete ByteArray;
      
      //If the file does not exist or is empty, do not continue.
      if(!HexString)
        return;
      
      //Create an image object (an XObject in the PDF file).
      ImageList.Add() = CreatePDFObject();
      
      //Enter in the appropriate dictionary information.
      prim::String Dictionary;
      Dictionary += "   /Type /XObject";
      Dictionary += "   /Subtype /Image";
      Dictionary += "   /Width ";
      Dictionary &= (prim::integer)PixelsWidth;
      Dictionary += "   /Height ";
      Dictionary &= (prim::integer)PixelsHeight;
      Dictionary += "   /ColorSpace /DeviceRGB"; //FIXED FOR NOW.
      Dictionary += "   /BitsPerComponent 8"; //Always 8 for JPEGs.
      Dictionary += "   /Length ";
      Dictionary &= (prim::integer)HexString.n();
      Dictionary += "   /Filter [/ASCIIHexDecode /DCTDecode]";
      ImageList.last()->Dictionary = Dictionary;
      
      /*Load the data into the stream, filtered by hex to make things easier.
      Note that because of the hex filter, PDF has placed a restriction that
      the JPEG can not be of the progressive format. For now we will just 
      assume that the JPEG is not in this format.*/
      ImageList.last()->Content = HexString;
      
      /*Add the image painting operator. Note that image space is defined by the
      PDF specification to be from [0, 0] to [1, 1]. Thus the proper common
      transformation matrix must be used for the image to scale correctly.*/
      prim::String t;
      Scale(prim::math::Vector(Width, Height));
      t = "/Im";
      t &= (prim::integer)(ImageList.n() - 1);
      t -= "Do";
      Rasterize(t);
      UndoTransformation();
    }
    
    //-----//
    //Paths//
    //-----//
    virtual void Draw(prim::Path& p, prim::number StrokeWidth = 0.0f,
      prim::count ContextIndex = -1)
    {
      using namespace prim;
      using namespace prim::math;
      String t;
      number CTMMultiplier = PDFProperties->CTMMultiplier;

      if(StrokeWidth != 0.f)
      {
        t += Abs(StrokeWidth) * CTMMultiplier;
        t -= "w";
      }

      for(count i = 0; i < p.Components.n(); i++)
      {
        Path::Component& comp = p.Components[i];
        Path::Curve& start = comp.Curves[0];
        t += start.End.x * CTMMultiplier;
        t -= start.End.y * CTMMultiplier;
        t -= "m";
        for(count j = 1; j < comp.Curves.n(); j++)
        {
          Path::Curve& curv = comp.Curves[j];
          if(!curv.IsCurve)
          {
            t += curv.End.x * CTMMultiplier;
            t -= curv.End.y * CTMMultiplier;
            t -= "l";
          }
          else
          {
            t += curv.StartControl.x * CTMMultiplier;
            t -= curv.StartControl.y * CTMMultiplier;
            t -= curv.EndControl.x * CTMMultiplier;
            t -= curv.EndControl.y * CTMMultiplier;
            t -= curv.End.x * CTMMultiplier;
            t -= curv.End.y * CTMMultiplier;
            t -= "c";
          }
        }
      }

      if(StrokeWidth > 0)
        t += "S"; //Stroke only.
      else if(StrokeWidth == 0)
        t += "f"; //Fill only.
      else if(StrokeWidth < 0)
        t += "B"; //Fill and stroke.
      else
        t += "n"; //"No-op"
      
      //Collapse the temporary operator string.
      t.Merge();
      
      count numContexts = p.Contexts.n();
      if(numContexts)
      {
        for(count i = 0; i < numContexts; i++)
        {
          if(ContextIndex == -1 || i == ContextIndex)
          {
            const prim::math::StateMatrix<RasterState>& m = p.Contexts[i];
            Transform(m);
            Rasterize(t);
            UndoTransformation();
          }
        }
      }
      else
        Rasterize(t);
    }

    virtual void DrawPath(prim::Path& p,
      bool Stroke = true, bool Fill = false, bool ClosePath = false,
      prim::number StrokeWidth = 0.0f)
    {
      using namespace prim;
      using namespace prim::math;
      String t;
      number CTMMultiplier = PDFProperties->CTMMultiplier;

      if(StrokeWidth > 0)
      {
        t += StrokeWidth * CTMMultiplier;
        t -= "w";
      }

      for(count i = 0; i < p.Components.n(); i++)
      {
        Path::Component& comp = p.Components[i];
        Path::Curve& start = comp.Curves[0];
        t += start.End.x * CTMMultiplier;
        t -= start.End.y * CTMMultiplier;
        t -= "m";
        for(count j = 1; j < comp.Curves.n(); j++)
        {
          Path::Curve& curv = comp.Curves[j];
          if(!curv.IsCurve)
          {
            t += curv.End.x * CTMMultiplier;
            t -= curv.End.y * CTMMultiplier;
            t -= "l";
          }
          else
          {
            t += curv.StartControl.x * CTMMultiplier;
            t -= curv.StartControl.y * CTMMultiplier;
            t -= curv.EndControl.x * CTMMultiplier;
            t -= curv.EndControl.y * CTMMultiplier;
            t -= curv.End.x * CTMMultiplier;
            t -= curv.End.y * CTMMultiplier;
            t -= "c";
          }
        }
        if(ClosePath)
          t += "h"; //Close the path.
      }

      if(Stroke && !Fill)
        t += "S"; //Stroke only.
      else if(Fill && !Stroke)
        t += "f"; //Fill only.
      else if(Fill && Stroke)
        t += "B"; //Fill and stroke.
      else
        t += "n"; //"No-op"

      Rasterize(t);
    }

    //TEXT//
    virtual prim::number DrawText(
      bbs::Font* FontToUse,
      prim::String Text,
      prim::math::Vector Position,
      prim::number Scale=1.0,
      prim::Justification JustificationType=prim::Justifications::Left,
      prim::number WrapWidth=0.0, bool ReturnWidthOnly=false)
    {
      using namespace prim;
      using namespace prim::typeface;
      using namespace bbs;

      //Stop unreferenced formal parameter warning.
      WrapWidth = (prim::number)0.0;

      //Strip unsupported characters (use ASCII for now).
      prim::Text::MakeASCIIPrintableString(Text);

      //Determine if the font needs to be added.
      Font* ptrFont = FontToUse;
      bool FoundFont = false;
      prim::count FontIndex = 0;
      for(count i=0;i<FontList.n();i++)
      {
        if(FontList[i]==ptrFont)
        {
          FoundFont=true;
          FontIndex = i;
        }
      }

      //Add the font if it was not already in the list.
      if(!FoundFont)
      {
        FontList.Append(ptrFont);
        FontIndex = FontList.n() - 1;
      }

      //Get a pointer to the OpenType font (otherwise exit)
      OpenType* OpenTypeFont = ptrFont->Program->Interface<OpenType>();
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
        /*Here we assume that, in a PDF file, the advance widths are
        supplemented by kerning values for each character, so not only
        do we need advance widths but kerning values as well in order to
        calculate the true widths of each character.*/
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

      //Initial LSB correction disabled temporarily...
      number FirstLSB = 0;//(number)LSBList[0];

      //Get the units per em to determine the scale.
      number UnitsPerEm = (number)OpenTypeFont->HEAD.UnitsPerEm;

      if(!ReturnWidthOnly)
      {
        String t;
        number CTMMultiplier = PDFProperties->CTMMultiplier;

        t += "BT";

        t += Position.x * CTMMultiplier;
        t -= Position.y * CTMMultiplier;
        t -= "Td";

        t += "/F";
        t &= (integer)FontIndex;
        t -= Scale * CTMMultiplier;
        t -= "Tf";

        t += "[";
        const ascii* TextChars = Text.Merge();

        switch(JustificationType)
        {
          case Justifications::Full:
          case Justifications::Left:
            t &= FirstLSB / UnitsPerEm * (number)1000.0;
            break;
          case Justifications::Right:
            t &= (TotalWidth)
            / UnitsPerEm * (number)1000.0;
            break;
          case Justifications::Center:
            t &= (TotalWidth + FirstLSB) / (number)2.0
            / UnitsPerEm * (number)1000.0;
            break;
        }

        for(count i=0;i<Text.n();i++)
        {
          t &= "<";
          t &= prim::Text::EncodeDataAsHexString((byte)TextChars[i]);
          t &= ">";

          number KernValue = (number)KernList[i] / UnitsPerEm
            * (number)-1000.0;

          if(KernValue!=(number)0.0)
            t &= KernValue;
        }
        t &= "] TJ";

        t += "ET";

        Rasterize(t);
      }

      delete [] WidthsList;
      delete [] LSBList;
      delete [] KernList;

      return (TotalWidth - FirstLSB) * (Scale / UnitsPerEm);
    }
  };
}

#endif
