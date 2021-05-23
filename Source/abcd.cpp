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

#include "abcd.h"

/*The following gets rid of a linker warning about there not being any public 
symbols in the abcd object file by declaring a symbol. This should eventually be
removed. Even if this file does not contain anything, it is still useful to
compile, since it checks for the correctness of the header file abcd.h.*/
bool abcd_LNK4221_Resolve;

//------------------------//
//Externs for abcdMusicXML//
//------------------------//

namespace abcd
{
  /*Need a workaround here because String uses static heaps of links, and these
  may or may not be instantiated before this static variable is constructed.*/
  /*
  prim::String MusicXML::Identification::Order[7] = {"creator", "rights",
    "encoding", "source", "relation", "miscellaneous", ""};//*/
}
