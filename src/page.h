/*

    Each page in the directory structure
    
*/

#ifndef PAGE_H_INCLUDED
#define PAGE_H_INCLUDED

#include <stdint.h>
#include <string>
#include <vector>

// The plan.txt file defines the structure of the content. Each line corresponds to
//  a page of content, or a ptr to a page of content, or a directory which starts
//  a folder of content (the pages in the directory).
//  An automatically generated plan from each run is saved as  generated-plan.txt, it
//  can be used as plan.txt, normally some edits (eg ordering items) are desirable.
//  If there is no plan.txt file, generated-plan.txt is created from the input directory
//  structure alone.
struct Page
{
    int level=0;    // depth in hierarchy

    // One and only one of the following three
    bool is_file = false; 
    bool is_dir  = false;
    bool is_link = false;

    // Main characteristics
    std::string path;       // the full path name, eg "path/to/file.txt"
    std::string target;     // the full unix path name to the final html file, eg "path-to-file.html"
    std::string filename;   // if it's a file, this is the name of the file eg "file.txt"
    std::string base;       // if it's a file, this is the name without the extension eg "file"
    std::string ext;        // if it's a file, this is the name of the ext eg "txt" (lowercased)
    std::string dir;        // if it's a file, this is the name of the dir eg "path/to"
    std::string link;       // if is_link is true

    // Misc
    bool from_plan_file=false;      // (from plan file, or from directory hierarchy)
    unsigned long plan_line_nbr = ULONG_MAX;
    unsigned long added_to_plan_line_nbr=0;
    bool disabled = false;
    bool make_file_for_dir = false;

    // Auto generated summary text
    std::string subheading;    // Eg "Archives Tournaments" (by convention @S)
    std::string subsubheading; // Eg "2021" (by convention @Z)

    // Features to be added later to support more user annotation for files
    bool empty_line_prefix = false;
    bool empty_line_suffix = false;
    std::vector<std::string> prefix;
    std::vector<std::string> suffix;
};


#endif // PAGE_H_INCLUDED
