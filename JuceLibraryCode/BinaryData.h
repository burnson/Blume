/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_31503847_INCLUDED
#define BINARYDATA_H_31503847_INCLUDED

namespace BinaryData
{
    extern const char*   Icon_icns;
    const int            Icon_icnsSize = 495531;

    extern const char*   Icon_jpg;
    const int            Icon_jpgSize = 233102;

    extern const char*   Info_plist;
    const int            Info_plistSize = 1431;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 3;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
