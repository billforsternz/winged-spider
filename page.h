/*

    Each page in the directory structure
    
*/

#ifndef PAGE_H_INCLUDED
#define PAGE_H_INCLUDED

#include <string>
#include <vector>

// The plan.txt file defines the structure of the content. Each line corresponds to
//  a page of content, or a ptr to a page of content. The plan is created automatically
//  from the input directory structure. The plan from each run is saved as plan.txt
//  and it can be edited, for example to reorder pages
struct Page
{
    int level=0;
    bool is_dir=false;
    bool from_plan_file=false;
    int  plan_line_nbr=0;
    std::string path;       // the full path name, eg "path/to/file.txt"
    std::string target;     // the full unix path name to the final html file, eg "/path/to/file.html"
    std::string name;       // if it's a file, this is the name of the file eg "file.txt"
    std::string base;       // if it's a file, this is the name without the extension eg "file"
    std::string ext;        // if it's a file, this is the name of the ext eg "txt" (lowercased)
    std::string dir;        // if it's a file, this is the name of the dir eg "path/to"

    // Features to be added later to support more user annotation for files
    bool empty_line_prefix = false;
    bool empty_line_suffix = false;
    std::vector<std::string> prefix;
    std::vector<std::string> suffix;
    std::vector<std::string> link;    // if this is actually a ptr to another page
};


#endif // PAGE_H_INCLUDED
