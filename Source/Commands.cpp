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

#include "Commands.h"

#include "Elements.h"
#include "Page.h"
#include "Score.h"
#include "Viewer.h"
#include "Window.h"

void Commands::getAllCommands(juce::Array<juce::CommandID>& commands)
{
  using namespace juce;

  //Make an array of all the command IDs.
  const CommandID ids[] =
  {
    FileNewWindow,
    FileOpen,
    FileCloseWindow,
    FileSave,
    FileSaveAs,
    FileSaveAsXML,
    FileQuit,
    
    ViewUseInches,
    ViewUseCentimeters,
    ViewShowCoarseGrid,
    ViewShowFineGrid,
    
    PageLandscape,
    PagePortrait,
    PageLetter,
    PageA4,
    PageB4,
    PageA3,
    PageTabloid,
    PageB3,
    PageCustomSize
  };

  commands.addArray(ids, sizeof(ids) / sizeof(CommandID));
}

juce::String Commands::getCommandNameFromID(const juce::CommandID id)
{
  int item = id & 0xff;
  int command = id - item;

  switch(command)
  {
  case FileNewWindow:
    return "New Window";
  case FileOpen:
    return "Open File...";
  case FileCloseWindow:
    return "Close Window";
  case FileSave:
    if(getDocument()->filename)
      return "Save"; 
    else
      return "Save...";
  case FileSaveAs:
    return "Save As...";
  case FileSaveAsXML:
    return "Save As XML...";
  case FileQuit:
    if(prim::OS::Windows())
      return "Exit";
    else
      return "Quit";
  case ViewUseInches:
    return "Inches";
  case ViewUseCentimeters:
    return "Centimeters";
  case ViewShowCoarseGrid:
    return "Show Grid";
  case ViewShowFineGrid:
    return "Show Fine Grid";
  case PageLandscape:
    return "Landscape";
  case PagePortrait:
    return "Portrait";
  case PageLetter:
    return "Letter (8.5x11 in)";
  case PageA4:
    return "A4 (210x297 mm)";
  case PageB4:
    return "B4 (250x353 mm)";
  case PageA3:
    return "A3 (297x420 mm)";
  case PageTabloid:
    return "Tabloid (11x17 in)";
  case PageB3:
    return "B3 (353x500 mm)";
  case PageCustomSize:
    return "Custom Size...";
  default:
    return "Command Item";
  }
}

void Commands::getShortcuts(const juce::CommandID id, 
  juce::ApplicationCommandInfo& info)
{
  using namespace juce;
  
  int item = id & 0xff;
  int command = id - item;

  if(command == FileNewWindow)
    info.addDefaultKeypress('N', ModifierKeys::commandModifier);
  else if(command == FileOpen)
    info.addDefaultKeypress('O', ModifierKeys::commandModifier);
  else if(command == FileCloseWindow)
    info.addDefaultKeypress('W', ModifierKeys::commandModifier);
  else if(command == FileSave)
    info.addDefaultKeypress('S', ModifierKeys::commandModifier);
  else if(command == FileSaveAs)
    info.addDefaultKeypress('S', ModifierKeys::commandModifier + 
    ModifierKeys::shiftModifier);
  else if(command == FileSaveAsXML)
    info.addDefaultKeypress('X', ModifierKeys::commandModifier);
  else if(command == FileQuit && !prim::OS::Windows())
    info.addDefaultKeypress('Q', ModifierKeys::commandModifier);
  else if(command == ViewUseInches)
    info.addDefaultKeypress('I', 0);
  else if(command == ViewUseCentimeters)
    info.addDefaultKeypress('C', 0);
  else if(command == ViewShowCoarseGrid)
    info.addDefaultKeypress('G', 0);
  else if(command == ViewShowFineGrid)
    info.addDefaultKeypress('F', 0);
  else if(command == PageLandscape)
    info.addDefaultKeypress('L', 0);
  else if(command == PagePortrait)
    info.addDefaultKeypress('P', 0);
}

void Commands::getCommandInfo(const juce::CommandID id,
  juce::ApplicationCommandInfo& info)
{
  using namespace juce;

  //Get the menu item name.
  info.shortName = getCommandNameFromID(id);

  //Get the menu item shortcut if there is one.
  getShortcuts(id, info);

  //Set default values for the menu flags.
  info.setActive(true);
  info.setTicked(false);

  //Cache some other document states.
  /*bool hasFilename = 
    getScore()->settings.filename != "";
  bool hasEdits = 
    getScore()->settings.hasBeenEdited;*/

  int item = id & 0xff;
  int command = id - item;

  switch(command)
  {
  case FileSaveAs:
    info.setActive(getDocument()->filename);
    break;
    
  case ViewUseCentimeters:
    info.setTicked(!getDocument()->useInches);
    break;
    
  case ViewUseInches:
    info.setTicked(getDocument()->useInches);
    break; 
    
  case ViewShowCoarseGrid:
    info.setTicked(getDocument()->showGrid);
    break;
    
  case ViewShowFineGrid:
    info.setTicked(getDocument()->showFineGrid);
    break;
    
  case PagePortrait:
    info.setTicked(getContainer()->sizePage.x < getContainer()->sizePage.y);
    break;
    
  case PageLandscape:
    info.setTicked(getContainer()->sizePage.x >= getContainer()->sizePage.y);
    break;
    
  case PageLetter:
    if(prim::math::Abs(getContainer()->sizePage.x * getContainer()->sizePage.y -
      8.5f * 11.0f) < 0.25f)
      info.setTicked(true);
    break;
  case PageA4:
    if(prim::math::Abs(getContainer()->sizePage.x * getContainer()->sizePage.y -
      11.693f * 8.268f) < 0.25f)
      info.setTicked(true);
    break;
    
  case PageB4:
    if(prim::math::Abs(getContainer()->sizePage.x * getContainer()->sizePage.y -
      9.8425f * 13.897f) < 0.25f)
      info.setTicked(true);
    break;
    
  case PageA3:
    if(prim::math::Abs(getContainer()->sizePage.x * getContainer()->sizePage.y -
      16.535f * 11.693f) < 0.25f)
      info.setTicked(true);
    break;
    
  case PageTabloid:
    if(prim::math::Abs(getContainer()->sizePage.x * getContainer()->sizePage.y -
      11.0f * 17.0f) < 0.25f)
      info.setTicked(true);
    break;
    
  case PageB3:
    if(prim::math::Abs(getContainer()->sizePage.x * getContainer()->sizePage.y -
      13.897f * 19.685f) < 0.25f)
      info.setTicked(true);
    break;    
  }
}

bool Commands::perform(
  const juce::ApplicationCommandTarget::InvocationInfo& info)
{
  using namespace juce;
  
  //Cancel any events that currently going on.
  //getPageList()[0]->cancelAllOtherPageEvents(0);

  int item = info.commandID & 0xff;
  int command = info.commandID - item;

  juce::String filename;
  
  switch(command)
  {
  case FileNewWindow:
    new Window(new Document::Initialization);
    break;

  case FileOpen:
    break;

  case FileCloseWindow:
    getDocument()->window->close();
    break;

  case FileSave:
    filename = getDocument()->filename.Merge();

  case FileSaveAs:
    {
      if(filename == juce::String::empty)
      {
      
        juce::String defaultFilename = juce::File::getSpecialLocation(
          juce::File::userDesktopDirectory).getFullPathName();
        defaultFilename << juce::File::separatorString;
        defaultFilename << "Untitled.pdf";
        
        juce::FileChooser SaveDialog("Please choose a filename",
          defaultFilename, "*.pdf", true);
        
        if(!SaveDialog.browseForFileToSave(true))
          break;
        
        filename = SaveDialog.getResult().getFullPathName().toUTF8();
        
        /*
         juce::String path =
          SaveDialog.getResult().getParentDirectory().getFullPathName();
        */
        
        //Add the extension if necessary.
        if(!filename.containsIgnoreCase(juce::String(".pdf")))
          filename << ".pdf";
        
        //Force extension to lowercase.
        filename = filename.replace(juce::String(".pdf"), juce::String(".pdf"),
          true);
          
        //Update the title of the window.
        setDocumentTitle(
          SaveDialog.getResult().getFileNameWithoutExtension().toRawUTF8());
      }

      //Save the metadata to the PDF properties.
      prim::String allXMLMetadata;
      getRepresentation()->toString(allXMLMetadata);

      //Set the PDF output properties.
      abcd::PDF::Properties prop;
      prop.ExtraData = (prim::byte*)allXMLMetadata.Merge();
      prop.ExtraDataLength = allXMLMetadata.ByteLength();  
      
      //Create a PDF.
      getDocument()->temporarilyHideHandles = true;
      getScore()->Create<abcd::PDF>(&prop);

      //Write the PDF to file.
      prim::File::Replace(filename.toUTF8(), prop.Output.Merge());
      
      //Update the current save file.
      getDocument()->filename = filename.toUTF8();
    }
    break;
    
  case FileSaveAsXML:
    {
      juce::String filename;
      juce::String defaultFilename = juce::File::getSpecialLocation(
        juce::File::userDesktopDirectory).getFullPathName();
      defaultFilename << juce::File::separatorString;
      defaultFilename << "Untitled.xml";

      juce::FileChooser SaveDialog("Please choose a filename",
        defaultFilename, "*.xml", true);
      
      if(!SaveDialog.browseForFileToSave(true))
        break;
      
      filename = SaveDialog.getResult().getFullPathName().toUTF8();
      
      //Add the extension if necessary.
      if(!filename.containsIgnoreCase(juce::String(".xml")))
        filename << ".xml";
      
      //Force extension to lowercase.
      filename = filename.replace(juce::String(".xml"), juce::String(".xml"),
        true);

      //Save the metadata to the PDF properties.
      prim::String allXMLMetadata;
      getRepresentation()->toString(allXMLMetadata);
      prim::File::Replace(filename.toUTF8(), allXMLMetadata);
    }
    break;
   
  case FileQuit:
    Window::closeAllWindows();
    break;
    
  case ViewUseCentimeters:
    getDocument()->useInches = false;
    getContainer()->sizeGrid = prim::math::Millimeters(50.0f, 50.0f);
    getContainer()->sizeSubgrid = prim::math::Millimeters(10.0f, 10.0f);
    getPage(0)->repaint();
    break;
    
  case ViewUseInches:
    getDocument()->useInches = true;
    getContainer()->sizeGrid.x = getContainer()->sizeGrid.y = 1.0f;
    getContainer()->sizeSubgrid.x = getContainer()->sizeSubgrid.y = 0.5f;
    getPage(0)->repaint();
    break;
    
  case ViewShowCoarseGrid:
    getDocument()->showGrid = !getDocument()->showGrid;
    getPage(0)->repaint();
    break;
    
  case ViewShowFineGrid:
    getDocument()->showFineGrid = !getDocument()->showFineGrid;
    getPage(0)->repaint();
    break;
    
  case PagePortrait:
    if(getContainer()->sizePage.x > getContainer()->sizePage.y)
    {
      prim::math::Swap(getContainer()->sizePage.x, getContainer()->sizePage.y);
      getCanvas(0)->Dimensions = getContainer()->sizePage;
      getViewer()->positionPages(true);
      getPage(0)->repaint();
    }
    break;
    
  case PageLandscape:
    if(getContainer()->sizePage.x < getContainer()->sizePage.y)
    {
      prim::math::Swap(getContainer()->sizePage.x, getContainer()->sizePage.y);
      getCanvas(0)->Dimensions = getContainer()->sizePage;
      getViewer()->positionPages(true);
      getPage(0)->repaint();
    }
    break;
    
  case PageLetter:
    getContainer()->sizePage = prim::math::Inches(11.0f, 8.5f);
    getCanvas(0)->Dimensions = prim::math::Inches(11.0f, 8.5f);
    getViewer()->positionPages(true);
    getPage(0)->repaint();
    break;
    
  case PageA4:
    getContainer()->sizePage = prim::math::Millimeters(297.0f, 210.0f);
    getCanvas(0)->Dimensions = prim::math::Millimeters(297.0f, 210.0f);
    getViewer()->positionPages(true);
    getPage(0)->repaint();
    break;

  case PageB4:
    getContainer()->sizePage = prim::math::Millimeters(353.0f, 250.0f);
    getCanvas(0)->Dimensions = prim::math::Millimeters(353.0f, 250.0f);
    getViewer()->positionPages(true);
    getPage(0)->repaint();
    break;
        
  case PageA3:
    getContainer()->sizePage = prim::math::Millimeters(420.0f, 297.0f);
    getCanvas(0)->Dimensions = prim::math::Millimeters(420.0f, 297.0f);
    getViewer()->positionPages(true);
    getPage(0)->repaint();
    break;
    
  case PageTabloid:
    getContainer()->sizePage = prim::math::Inches(17.0f, 11.0f);
    getCanvas(0)->Dimensions = prim::math::Inches(17.0f, 11.0f);
    getViewer()->positionPages(true);
    getPage(0)->repaint();
    break;
    
  case PageB3:
    getContainer()->sizePage = prim::math::Millimeters(500.0f, 353.0f);
    getCanvas(0)->Dimensions = prim::math::Millimeters(500.0f, 353.0f);
    getViewer()->positionPages(true);
    getPage(0)->repaint();
    break;    
    
  case PageCustomSize:
    break;
  }

  return true;
}
