/*

    Each page in the directory structure
    
*/

#ifndef PAGE_H_INCLUDED
#define PAGE_H_INCLUDED

#include <stdint.h>
#include <string>
#include <vector>

// The plan.txt file defines the structure of the content. Each line corresponds to
//  a page of content, or a ptr to a page of content. The plan is created automatically
//  from the input directory structure. The plan from each run is saved as plan.txt
//  and it can be edited, for example to reorder pages
struct Page
{
    int level=0;

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
    bool from_plan_file=false;
    unsigned long plan_line_nbr = ULONG_MAX;
    unsigned long added_to_plan_line_nbr=0;
    bool disabled = false;
    bool make_file_for_dir = false;

    // Auto generated summary text
    std::string heading;    // Eg "Archives Tournaments" (@S for historical reasons)
    std::string subheading; // Eg "2021" (@Z for historical reasons)

    // Features to be added later to support more user annotation for files
    bool empty_line_prefix = false;
    bool empty_line_suffix = false;
    std::vector<std::string> prefix;
    std::vector<std::string> suffix;
};


#endif // PAGE_H_INCLUDED
