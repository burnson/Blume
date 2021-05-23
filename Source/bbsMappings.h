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

#ifndef bbsMappings
#define bbsMappings

#include "primTypes.h"

namespace bbs
{
  typedef prim::unicode::UCS4 Unicode;

  struct SymbolMapping
  {
    ///The virtual destructor
    virtual ~SymbolMapping() {}
    
    static const int NoMap = 0x0000;

    //Clefs
    virtual Unicode GClef(void){return NoMap;}
    virtual Unicode FClef(void){return NoMap;}
    virtual Unicode CClef(void){return NoMap;}
    virtual Unicode NeutralClef(void){return NoMap;}

    //Accidentals
    virtual Unicode DoubleFlat(void){return NoMap;}
    virtual Unicode Flat(void){return NoMap;}
    virtual Unicode Natural(void){return NoMap;}
    virtual Unicode Sharp(void){return NoMap;}
    virtual Unicode DoubleSharp(void){return NoMap;}

    //Transposition
    virtual Unicode EightVa(void){return NoMap;}
    virtual Unicode FifteenMa(void){return NoMap;}

    //Time signatures
    virtual Unicode CommonTime(void){return NoMap;}
    virtual Unicode CutTime(void){return NoMap;}

    //Rests
    virtual Unicode BreveRest(void){return NoMap;}
    virtual Unicode WholeRest(void){return NoMap;}
    virtual Unicode HalfRest(void){return NoMap;}
    virtual Unicode QuarterRest(void){return NoMap;}
    virtual Unicode EighthRest(void){return NoMap;}
    virtual Unicode SixteenthRest(void){return NoMap;}
    virtual Unicode ThirtySecondRest(void){return NoMap;}
    virtual Unicode SixtyFourthRest(void){return NoMap;}
    virtual Unicode OneTwentyEighthRest(void){return NoMap;}

    //Beaming
    virtual Unicode Flag(void){return NoMap;}

    //Dynamics
    virtual Unicode Niente(void){return NoMap;}
    virtual Unicode ppp(void){return NoMap;}
    virtual Unicode pp(void){return NoMap;}
    virtual Unicode p(void){return NoMap;}
    virtual Unicode mp(void){return NoMap;}
    virtual Unicode mf(void){return NoMap;}
    virtual Unicode f(void){return NoMap;}
    virtual Unicode ff(void){return NoMap;}
    virtual Unicode fff(void){return NoMap;}
    virtual Unicode sf(void){return NoMap;}
    virtual Unicode fz(void){return NoMap;}
    virtual Unicode sfz(void){return NoMap;}

    //Articulations
    virtual Unicode Marcato(void){return NoMap;}
    virtual Unicode MarcatoStaccato(void){return NoMap;}
    virtual Unicode Staccato(void){return NoMap;}
    virtual Unicode StaccatoTenuto(void){return NoMap;}
    virtual Unicode Tenuto(void){return NoMap;}
    virtual Unicode Trill(void){return NoMap;}
    virtual Unicode TurnUp(void){return NoMap;}
    virtual Unicode TurnDown(void){return NoMap;}
    virtual Unicode Breath(void){return NoMap;}
    virtual Unicode Fermata(void){return NoMap;}
    virtual Unicode Pedal(void){return NoMap;}
    virtual Unicode Asterisk(void){return NoMap;}

    //Layout
    virtual Unicode SystemBreak(void){return NoMap;}
  };

  struct JoieMapping : public SymbolMapping
  {
    ///The virtual destructor
    virtual ~JoieMapping() {}
    
    //Clefs
    virtual Unicode GClef(void){return 0x0041;}
    virtual Unicode FClef(void){return 0x0042;}
    virtual Unicode CClef(void){return 0x0043;}
    virtual Unicode NeutralClef(void){return 0x0044;}

    //Accidentals
    virtual Unicode DoubleFlat(void){return 0x0045;}
    virtual Unicode Flat(void){return 0x0046;}
    virtual Unicode Natural(void){return 0x0047;}
    virtual Unicode Sharp(void){return 0x0048;}
    virtual Unicode DoubleSharp(void){return 0x0049;}

    //Transposition
    virtual Unicode EightVa(void){return 0x004A;}
    virtual Unicode FifteenMa(void){return 0x004B;}

    //Time signatures
    virtual Unicode CommonTime(void){return 0x004C;}
    virtual Unicode CutTime(void){return 0x004D;}

    //Rests
    virtual Unicode BreveRest(void){return 0x004E;}
    virtual Unicode WholeRest(void){return 0x004F;}
    virtual Unicode HalfRest(void){return 0x0050;}
    virtual Unicode QuarterRest(void){return 0x0051;}
    virtual Unicode EighthRest(void){return 0x0052;}
    virtual Unicode SixteenthRest(void){return 0x0053;}
    virtual Unicode ThirtySecondRest(void){return 0x0054;}
    virtual Unicode SixtyFourthRest(void){return 0x0055;}
    virtual Unicode OneTwentyEighthRest(void){return 0x0056;}

    //Beaming
    virtual Unicode Flag(void){return 0x0057;}

    //Dynamics
    virtual Unicode Niente(void){return 0x0058;}
    virtual Unicode ppp(void){return 0x0059;}
    virtual Unicode pp(void){return 0x005A;}
    virtual Unicode p(void){return 0x0061;}
    virtual Unicode mp(void){return 0x0062;}
    virtual Unicode mf(void){return 0x0063;}
    virtual Unicode f(void){return 0x0064;}
    virtual Unicode ff(void){return 0x0065;}
    virtual Unicode fff(void){return 0x0066;}
    virtual Unicode sf(void){return 0x0067;}
    virtual Unicode fz(void){return 0x0068;}
    virtual Unicode sfz(void){return 0x0069;}

    //Articulations
    virtual Unicode Marcato(void){return 0x006A;}
    virtual Unicode MarcatoStaccato(void){return 0x006B;}
    virtual Unicode Staccato(void){return 0x006C;}
    virtual Unicode StaccatoTenuto(void){return 0x006D;}
    virtual Unicode Tenuto(void){return 0x006E;}
    virtual Unicode Trill(void){return 0x006F;}
    virtual Unicode TurnUp(void){return 0x0070;}
    virtual Unicode TurnDown(void){return 0x0071;}
    virtual Unicode Breath(void){return 0x0072;}
    virtual Unicode Fermata(void){return 0x0073;}
    virtual Unicode Pedal(void){return 0x0074;}
    virtual Unicode Asterisk(void){return 0x0075;}

    //Layout
    virtual Unicode SystemBreak(void){return 0x0076;}
  };

  struct TextMapping
  {
    ///The virtual destructor
    virtual ~TextMapping() {}
    
    static const int NoMap = 0x0000;

    //Accidentals
    virtual Unicode DoubleFlat(void){return NoMap;}
    virtual Unicode Flat(void){return NoMap;}
    virtual Unicode Natural(void){return NoMap;}
    virtual Unicode Sharp(void){return NoMap;}
    virtual Unicode DoubleSharp(void){return NoMap;}

    //Chord Qualities
    virtual Unicode Diminished(void){return NoMap;}
    virtual Unicode HalfDiminished(void){return NoMap;}
    virtual Unicode Augmented(void){return NoMap;}

    //Sonority Symbols
    virtual Unicode LittleMWithBar(void){return NoMap;}
  };

  struct TallysMapping : public TextMapping
  {
    ///The virtual destructor
    virtual ~TallysMapping() {}
    
    static const int NoMap = 0x0000;

    //Accidentals
    virtual Unicode DoubleFlat(void){return 0xF000;}
    virtual Unicode Flat(void){return 0xF001;}
    virtual Unicode Natural(void){return 0xF002;}
    virtual Unicode Sharp(void){return 0xF003;}
    virtual Unicode DoubleSharp(void){return 0xF004;}

    //Chord Qualities
    virtual Unicode Diminished(void){return 0xF005;}
    virtual Unicode HalfDiminished(void){return 0xF006;}
    virtual Unicode Augmented(void){return 0xF007;}

    //Sonority Symbols
    virtual Unicode LittleMWithBar(void){return 0xF008;}
  };
}
#endif
