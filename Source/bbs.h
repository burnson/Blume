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

#ifndef BBS_H_BELLEBONNESAGE
#define BBS_H_BELLEBONNESAGE

//Dependencies -- alphabetical order
#include "prim.h"

//The core Belle, Bonne, Sage headers -- alphabetical order
#include "bbsAbstracts.h"
#include "bbsFont.h"
#include "bbsGlyph.h"
#include "bbsMappings.h"
#include "bbsOptics.h"
#include "bbsShapes.h"
#include "bbsText.h"

//---------------------------------//
//Another Belle Context Definitions//
//---------------------------------//
/*ABCD's are painter implementations. This header includes the ABCD files so
that by including bbs.h, the entire core Belle, Bonne, Sage library is
included.*/
#include "abcd.h"

#endif
