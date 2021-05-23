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

#include "Score.h"

#include "Elements.h"
#include "Interaction.h"
#include "Viewer.h"

using namespace prim;
using namespace prim::math;
using namespace prim::colors;
using namespace bbs;
using namespace bbs::abstracts;
using namespace abcd;

number ZoomConstant = 1.0;

namespace notation
{
  struct PositionInfo
  {
    count Level;
    number Position;
    
    bool operator < (const PositionInfo& Other) const
    {
      return Position < Other.Position;
    }
    bool operator <= (const PositionInfo& Other) const
    {
      return Position <= Other.Position;
    }
    bool operator == (const PositionInfo& Other) const
    {
      return Position == Other.Position;
    }
    bool operator > (const PositionInfo& Other) const
    {
      return Position > Other.Position;
    }
    bool operator >= (const PositionInfo& Other) const
    {
      return Position >= Other.Position;
    }
  };
  
  List<PositionInfo> State;
  
  Score::Score(Document* Document) : DocumentHandler(Document)
  {
  }

  Score::~Score()
  {
    Canvases.RemoveAndDeleteAll();
  }

  Score::Page::Page(Document* document, Score& score) :
    DocumentHandler(document)
  {
    Page::score = &score;
  }

  void Score::Page::PaintSection(Painter* Painter,
    Representation::Section* section,
    prim::number width, prim::number height, prim::number xOffset, bool up,
    prim::number beamSlant, prim::count recursion)
  {
    number TotalWidth = width;
    number Segments = (number)section->segments;
    number accel = section->scalarAccelerando;
    number percentage;
    if(accel >= 0)
      percentage = 1.0f / (accel + 1.0f);
    else
      percentage = 1.0f / (-accel + 1.0f);

    number W1 = TotalWidth / Segments * percentage;

    number deltaX = 2.0f * (TotalWidth - W1 * Segments) / 
      (Segments * Segments - Segments);
    
    if(accel >= 0)
    {
      //Have to reverse the algorithm so that the accels and decels are
      //symmetric.
      W1 = deltaX * (Segments - 1.0f) + W1; //Transform W1 into W2...
      deltaX *= -1.0f; //Reverse the delta...
    }
    
    number d;
    if(accel > 0)
      d = (number)1.0f + accel;
    else
      d = (number)1.0f / ((number)1.0f - accel);
    
    if(Abs(accel) < 0.001f)
      W1 = TotalWidth / Segments;
    else
    {
      number dn = 1.0f; for(count i = 0; i < Segments; i++) dn *= d;
      W1 = TotalWidth * (1.0f - d) / (1 - dn);
    }
    
    number y = height * section->scalarHeight * (up ? 1.0f : -1.0f);
    
    number factor = 0.6f, exponentialsize = 1.0f;
    for(count i = 1; i < recursion; i++)
      exponentialsize *= factor;
    
    number deltaY = section->scalarAccelerando * 
      exponentialsize *
      getContainer()->sizeMainSection.y *
      beamSlant *
      (up ? 1.0f : -1.0f);

    number y1 = y + deltaY;
    number y2 = y - deltaY;
    number x = xOffset;
    Vector off = getContainer()->offsetMainSection;
    
    Vector leftaccel = Vector(x + off.x, y1 + off.y);
    getInteractions().Add() = new Interaction(leftaccel, section,
      Interaction::SectionAccelerandoLeft, 0.05f * ZoomConstant, false, false, true);
    
    Vector sectionheight = Vector(x + off.x + TotalWidth * 0.5f, y + off.y);
    getInteractions().Add() = new Interaction(sectionheight, section,
      Interaction::SectionHeight, 0.12f * ZoomConstant, false, true, false);
      
    Vector deletesection = Vector(x + off.x + TotalWidth, y2 + off.y);
    if(recursion > 1)
    {
      getInteractions().Add() = new Interaction(deletesection, section,
        Interaction::DeleteSection, 0.05f * ZoomConstant, true, false, false);
    }
    else
    {
      getInteractions().Add() = new Interaction(deletesection, section,
        Interaction::ChangeMainSectionSegments, 0.03f * ZoomConstant, false, false, true);    
    }
      
    section->cachedHeight = height * (up ? 1.0f : -1.0f);
    section->cachedWidth = TotalWidth;
    section->cachedRecurseDepth = recursion;
    section->cachedBottomLeft = Vector(x + off.x, off.y);
    section->cachedExponentialScale = exponentialsize;
    
    number* xSubOffsets = new number[section->segments + 1];
    number currentW = W1;
    for(count i = 0; i <= section->segments; i++)
    {
      PositionInfo pi;
      pi.Level = recursion;
      pi.Position = x;
      State.Add() = pi;
      
      xSubOffsets[i] = x;
      number y = ((x - xOffset) / TotalWidth) * (y2 - y1) + y1;
      
      Vector Start(x, 0), End(x, y);
      Start += off; End += off;
      
      prim::Path p;
      bbs::Shapes::AddLine(p, Start, End, 0.01f * ZoomConstant);
      Painter->DrawPath(p, false, true);
      
      number oldX = x;
      x += currentW;
      currentW *= d;
      
      if(i == section->segments)
        continue;
      
      number createX = oldX;//(x + oldX) * 0.5f;
      number createY = height * -0.0f;
      Vector createSectionPos(createX + off.x, createY + off.y);
      bool alreadyHasSection = false;
      for(count j = section->CountChildrenOfType<Representation::Section>() - 1;
        j >= 0; j--)
      {
        if(section->GetChildOfType<Representation::Section>(j)->parentSegment
          == i)
        {
          alreadyHasSection = true;
          break;
        }
      }
      if(!alreadyHasSection)
        getInteractions().Add() = new Interaction(createSectionPos, section,
          Interaction::CreateSection, 0.03f * ZoomConstant, false, false, true, i);
    }
    xSubOffsets[section->segments] = xOffset + TotalWidth;
    
    prim::Path p;
    Vector Start(xOffset, y1), End(xOffset + TotalWidth, y2);
    Start += off; End += off;
    {
      Vector tl = Start, bl = Start, tr = End, br = End;
      number beamsize = 0.05f * ZoomConstant;
      tl.x -= 0.005f * ZoomConstant; bl.x -= 0.005f * ZoomConstant;
      tr.x += 0.005f * ZoomConstant; br.x += 0.005f * ZoomConstant;
      tl.y += exponentialsize * beamsize;
      bl.y -= exponentialsize * beamsize;
      tr.y += exponentialsize * beamsize;
      br.y -= exponentialsize * beamsize;
      p.AddComponent(tl);
      p.AddCurve(tr);
      p.AddCurve(br);
      p.AddCurve(bl);
      p.AddCurve(tl);
    }
    Painter->DrawPath(p, false, true);
    
    for(count i = section->CountChildrenOfType<Representation::Section>() - 1;
      i >= 0; i--)
    {
      Representation::Section* s =
        section->GetChildOfType<Representation::Section>(i);
      count ps = s->parentSegment;
      if(ps < section->segments)
        PaintSection(Painter, s, xSubOffsets[ps + 1] - xSubOffsets[ps],
          height * section->scalarHeight, xSubOffsets[ps], !up, beamSlant,
          recursion + 1);
    }
    
    delete [] xSubOffsets;
  }
  
  void Score::Page::DrawGridlines(Painter* Painter)
  {
    Vector pageSize = getContainer()->sizePage;
    Vector gridSize = getContainer()->sizeGrid;
    Vector subgridSize = getContainer()->sizeSubgrid;
    
    count gridcountX = (count)(pageSize.x / gridSize.x * 0.5f) + 2;
    count gridcountY = (count)(pageSize.y / gridSize.y * 0.5f) + 2;
    count subgridcountX = (count)(pageSize.x / subgridSize.x * 0.5f) + 2;
    count subgridcountY = (count)(pageSize.y / subgridSize.y * 0.5f) + 2;
    
    prim::Path p1, p2;
    
    for(count i = -subgridcountX; i <= subgridcountX; i++)
      bbs::Shapes::AddLine(p1,
      Vector((number)i * subgridSize.x, pageSize.y * 0.5f),
      Vector((number)i * subgridSize.x, pageSize.y * -0.5f), 0.005f); 
    for(count i = -subgridcountY; i <= subgridcountY; i++)
      bbs::Shapes::AddLine(p1,
      Vector(pageSize.x * 0.5f, (number)i * subgridSize.y),
      Vector(pageSize.x * -0.5f, (number)i * subgridSize.y), 0.005f);
          
    for(count i = -gridcountX; i <= gridcountX; i++)
      bbs::Shapes::AddLine(p2,
      Vector((number)i * gridSize.x, pageSize.y * 0.5f),
      Vector((number)i * gridSize.x, pageSize.y * -0.5f), 0.01f);
    for(count i = -gridcountY; i <= gridcountY; i++)
      bbs::Shapes::AddLine(p2,
      Vector(pageSize.x * 0.5f, (number)i * gridSize.y),
      Vector(pageSize.x * -0.5f, (number)i * gridSize.y), 0.01f);
      
    Painter->FillColor(LightGray);
    if(getDocument()->showFineGrid)
      Painter->DrawPath(p1, false, true);
    Painter->FillColor(Gray);
    if(getDocument()->showGrid)
      Painter->DrawPath(p2, false, true);    
    Painter->FillColor(Black);
  }
  
  void Score::Page::DrawHandles(Painter* Painter)
  {
    number thickness = 0.01f * ZoomConstant;
       
    for(count i = getInteractions().n() - 1; i >= 0; i--)
    {
      Painter->FillColor(Red);
      Painter->StrokeColor(Blue);
      
      Interaction* interaction = getInteractions()[i];
      Vector c = interaction->position;
      number r = interaction->radius;
      Vector tl, tr, bl, br;
      tl.x = bl.x = c.x - r;
      tr.x = br.x = c.x + r;
      bl.y = br.y = c.y - r;
      tl.y = tr.y = c.y + r;
      
      prim::Path pf, ps;
      if(interaction->drawCross)
      {
        bbs::Shapes::AddLine(pf, tl, br, thickness);
        bbs::Shapes::AddLine(pf, bl, tr, thickness);
        Painter->FillColor(Red);
        Painter->StrokeColor(Red);
      }
      if(interaction->drawRectangle)
      {
        bbs::Shapes::AddLine(pf, tl, tr, thickness);
        bbs::Shapes::AddLine(pf, tr, br, thickness);
        bbs::Shapes::AddLine(pf, br, bl, thickness);
        bbs::Shapes::AddLine(pf, bl, tl, thickness);
        Painter->FillColor(Green);
        Painter->StrokeColor(Green);         
      }
      if(interaction->drawCircle)
      {
        bbs::Shapes::AddCircle(ps, c, r);
        Painter->FillColor(Blue);
        Painter->StrokeColor(Blue);
      }
      
      Painter->DrawPath(pf, false, true);
      Painter->DrawPath(ps, true, false, true, thickness);
    }
    Painter->FillColor(Black);
    Painter->StrokeColor(Black);    
  }
  
  void Score::Page::Paint(Painter* Painter)
  {
    ZoomConstant = 1.0 / getViewer()->percentageZoom;
    
    State.RemoveAll();
    
    Vector pageSize = getContainer()->sizePage;
    Painter->FillColor(Black);  
    
    Painter->Translate(pageSize * 0.5f);
    {
      //Caches some objects.
      Representation::Section* s =
        getContainer()->GetChildOfType<Representation::Section>();
      Vector ssize = getContainer()->sizeMainSection;
      Vector off = getContainer()->offsetMainSection;
          
      //Remove existing interactive handles.
      getInteractions().RemoveAndDeleteAll();
      
      //Draw the grid lines.
      DrawGridlines(Painter);
      
      //Add an offset changer.
      Vector crosspos = off;
      crosspos.y += ssize.y * 0.5f;
      getInteractions().Add() = new Interaction(crosspos, s,
        Interaction::MainSectionPosition, 0.08f * ZoomConstant, true, false, false);
        
      //Add an main width changers.
      Vector lwidthchanger = off, rwidthchanger = off;
      lwidthchanger.y = rwidthchanger.y = off.y + ssize.y * 0.5f;
      lwidthchanger.x -= ssize.x * 0.5f;
      rwidthchanger.x += ssize.x * 0.5f;
      getInteractions().Add() = new Interaction(lwidthchanger, s,
        Interaction::MainSectionWidth, 0.12f * ZoomConstant, false, true, false);        
      getInteractions().Add() = new Interaction(rwidthchanger, s,
        Interaction::MainSectionWidth, 0.12f * ZoomConstant, false, true, false);

      //Draw the beam sections.
      PaintSection(Painter, s, ssize.x, ssize.y, ssize.x * -0.5f, true,
        getContainer()->scalarBeamSlant, 1);
      
      //Draw the ground line.
      Vector GroundLeft(getContainer()->sizeMainSection.x * -0.5f - 0.005f, 0),
        GroundRight(getContainer()->sizeMainSection.x * 0.5f + 0.005f, 0);   
      GroundLeft += off; GroundRight += off;
      prim::Path p;
      Shapes::AddLine(p, GroundLeft, GroundRight, 0.04f * ZoomConstant, false, false, false);
      Painter->DrawPath(p, false, true);

      //Draw all the interactive handles.
      if(!getDocument()->temporarilyHideHandles)
        DrawHandles(Painter);
    }
    Painter->UndoTransformation();
    
    
    //Find maximum
    if(!State.n())
      return;
      
    prim::number min = State[0].Position, max = min;
    for(count i = 0; i < State.n(); i++)
    {
      min = prim::math::Min(min, State[i].Position);
      max = prim::math::Max(max, State[i].Position);
    }
    for(count i = 0; i < State.n(); i++)
    {
      State[i].Position += -min;
      State[i].Position = State[i].Position / (max - min);
    }
    prim::number mindist = 1.0;
    
    for(count i = 0; i < State.n(); i++)
    {
      for(count j = 0; j < State.n(); j++)
      {
        if(i == j) continue;
        if(math::Abs(State[i].Position - State[j].Position) < 0.00001 &&
          State[i].Level < State[j].Level)
        {
          State.Remove(j);
          i = 0;
          j = -1;
        }
      }
    }
    State.Sort();
    for(count i = 1; i < State.n(); i++)
    {
      mindist = math::Min(mindist, State[i].Position - State[i - 1].Position);
    }
    prim::String s;
    s += "INFO BEGIN";
    s += "Minimum Distance: ";
    s &= mindist;
    s++;
    for(count i = 0; i < State.n(); i++)
    {
      PositionInfo p = State[i];
      for(count j = 1; j < p.Level; j++)
        s &= "  ";
      if(p.Position == 0.0)
        s &= "0.0000000";
      else if(p.Position == 1.0)
        s &= "1.0000000";
      else
      {
        prim::String x = math::NumberToString(p.Position, 7);
        for(count k = 9 - x.n(); k > 0; k--)
          x &= "0";
        s &= x;
      }
      
      s++;
    }
    s &= "INFO END";
    prim::String f =
      juce::File::getSpecialLocation(
      juce::File::userDesktopDirectory).getFullPathName().toRawUTF8();
    f &= "/Info.txt";
    prim::File::Write(f, (const byte*)s.Merge(), s.n());
    /*
    prim::Console c;
    c += s;
    c++;
    */
  }
}
