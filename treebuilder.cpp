// Directory traversal to build and maintain a 'plan' or sitemap - the core idea of Winged Spider
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <filesystem>
namespace fs = std::experimental::filesystem;
#include "util.h"
#include "page.h"
#include "misc.h"

static std::map<std::string,Page *> directory_to_target;

// Each directory comprises a group of pages
// If directory is selected from a parent menu, it needs to resolve
//  to a target. The target is the first page in the directory if
//  that page is a leaf (a html file). If the directory is has no
//  files, it is an empty intermediate directory. A simple .html
//  file is constructed to represent the directory in the menu
//  system, using the make_file_for_dir flag.
void construct_dir_target( std::vector<Page*> ptrs )
{
    // Look for a file the parent directory can point to 
    for( Page *p: ptrs )
    {
        if( p->is_file && !p->disabled )
        {
            directory_to_target[p->dir] = p;
            return;
        }
    }

    // If nothing better found create a file, empty but with a menu, to represent
    //  the parent directory in the menu hierarchy
    for( Page *p: ptrs )
    {
        directory_to_target[p->dir] = p;
        p->make_file_for_dir = true;
        return;
    }
}


// Construct all the pages in a directory. Build the whole menu for that directory
// first, because each page will have the same menu (but a different idx into the
// menu to be highlighted)
void construct_page_group( std::vector<Page*> ptrs )
{
    if( ptrs.size() == 0 )
        return;

    static int debug_count;
    //if( ++debug_count == 5 )
    //    printf( "Debug Curator's Notes\n" );

    // Calculate the menu to present for each page in the group,
    //  Start with each element of the split path
    std::vector< std::pair<std::string,std::string> > menu;
    std::string build_path;
    Page *p = ptrs[0];

    // If not at root, start with "Home" link to start of root directory
    if( p->dir.length() > 0 )
    {
        std::pair<std::string,std::string> menu_item( "index.html", "Home" );
        menu.push_back( menu_item );
    }

    // Each level of subdirectory gets a menu item, eg "Archives\Tournaments" gets "Archives", then "Tournaments"
    size_t offset1=0, offset2;
    while( offset1 < p->path.length() )
    {
        offset2 = p->path.find( PATH_SEPARATOR, offset1 );
        if( offset2 == std::string::npos )
        {
            if( p->is_dir )  // final element of directory is not terminated by PATH_SEPARATOR
            {
                std::string subdir = p->path;                    // eg "Archives\Tournaments"
                std::string name   = p->path.substr(offset1);    // eg "Tournaments"
                auto q = directory_to_target.find(subdir);
                if( q == directory_to_target.end() )
                    printf( "Warning: %s target not found\n", subdir.c_str() );
                else
                {
                    std::pair<std::string,std::string> menu_item( q->second->target, name );
                    menu.push_back( menu_item );
                }
            }
            break;
        }
        std::string subdir = p->path.substr( 0, offset2 );              // eg "Archives", then "Archives\Tournaments"
        std::string name   = p->path.substr(offset1,offset2-offset1);   // eg "Archives", then "Tournaments"
        auto q = directory_to_target.find(subdir);
        if( q == directory_to_target.end() )
            printf( "Warning: %s target not found\n", subdir.c_str() );
        else
        {
            std::pair<std::string,std::string> menu_item( q->second->target, name );
            menu.push_back( menu_item );
        }
        offset1 = offset2+1;
    }

    // After all elements of the split path, add the pages in the directory
    int menu_idx = menu.size();
    Page *make_file_for_dir = 0;
    bool first = true;
    for( Page *p: ptrs )
    {
        bool skip = false;
        std::pair<std::string,std::string> menu_item;
        if( p->is_dir )
        {
            auto q = directory_to_target.find(p->path);
            if( q == directory_to_target.end() )
            {
                skip = true;
                printf( "Warning: %s target not found\n", p->path.c_str() );
            }
            else
            {
                menu_item = std::pair<std::string,std::string> ( q->second->target, p->base );
                if( p->make_file_for_dir )
                    make_file_for_dir = p;
            }
        }
        else if( p->is_link )
            menu_item = std::pair<std::string,std::string> ( p->link, p->base );
        else if( p->is_file )
        {
            if( p->ext=="md" || p->ext=="pgn" || p->ext=="html" )
                menu_item = std::pair<std::string,std::string>( p->target, p->base );
            else
                skip = true;
        }
        if( !skip )
        {
            if( first && menu_idx>0 && menu[menu_idx-1] == menu_item )
            {
                menu.pop_back();
                menu_idx--;
            }
            menu.push_back(menu_item);
        }
        first = false;
    }
    printf("\nMenu>\n");
    for(std::pair<std::string,std::string> menu_item: menu )
    {
        printf("%s %s\n", menu_item.first.c_str(), menu_item.second.c_str() );
        //if( menu_item.second == "Tournaments" )
        //    printf("Debug break 1\n");
    }

    // If we are making an html file for an empty directory, do it now with
    //  index set to the last element of the split path
    if( make_file_for_dir )
        markdown_gen( make_file_for_dir, menu, menu_idx-1 );

    // Build each page in turn
    //  (force build for now)
    for( Page *p: ptrs )
    {
        if( !p->is_file || p->disabled )
            ;
        else if( "md" == p->ext )
        {
            markdown_gen( p, menu, menu_idx );
        }
        else if( "pgn" == p->ext )
        {
            pgn_to_html( p, menu, menu_idx );
        }
        else if( "html" == p->ext )
        {
            html_gen( p );
        }
        menu_idx++;
    }
}

// Predicate for sorting when we are comparing the plan file and the actual directory
//  structure
bool less_than_sync_plan_to_directory_structure( const Page &lhs,  const Page &rhs )
{
    if( lhs.level != rhs.level )
        return lhs.level < rhs.level;
    else if( lhs.dir != rhs.dir )
        return lhs.dir < rhs.dir;
    else if( lhs.filename != rhs.filename )
        return lhs.filename < rhs.filename;
    else if( lhs.from_plan_file != rhs.from_plan_file )
        return lhs.from_plan_file;
    else if( lhs.plan_line_nbr != rhs.plan_line_nbr )
        return lhs.plan_line_nbr < rhs.plan_line_nbr;
    return false;
}

// Predicate for sorting when we are restoring original plan file order
bool less_than_restore_order( const Page &lhs,  const Page &rhs )
{
    if( lhs.plan_line_nbr != rhs.plan_line_nbr )
        return lhs.plan_line_nbr < rhs.plan_line_nbr;
    return lhs.added_to_plan_line_nbr < rhs.added_to_plan_line_nbr;
}

void parse( Page &p )
{
    size_t offset = p.path.find_last_of( PATH_SEPARATOR );
    if( offset == std::string::npos )
    {
        p.dir  = "";
        p.filename = p.path;
    }
    else
    {
        p.dir  = p.path.substr(0,offset);
        p.filename = p.path.substr(offset+1);
    }
    offset = p.filename.find_last_of( '.' );
    if( offset == std::string::npos )
        p.base = p.filename;
    else
    {
        p.base = p.filename.substr(0,offset);
        p.ext  = util::tolower(p.filename.substr(offset+1));
    }
    if( p.is_dir )
        p.target = p.dir + ".html";
    else
    {
        if( p.dir.length() == 0 )
            p.target = p.base + ".html";
        else
        {
            p.target = p.dir + '-' + p.base + ".html";

            // A refinement, if filename is same as folder name, remove that duplication from the
            //  target; eg archives/tournaments/tournaments.md -> archives-tournaments.html not archives-tournaments-tournaments.html
            std::string final_folder = p.dir;
            size_t offset = p.dir.find_last_of( PATH_SEPARATOR );
            if( offset != std::string::npos )
                final_folder = p.dir.substr(offset+1);
            if( util::tolower(final_folder) == util::tolower(p.base) )
                p.target = p.dir + ".html"; // eg archives/archives.md -> archives.html not archives-archives.html   
        }
    }
    for( char &c: p.target )
    {
        if( isascii(c) && isalnum(c) )
            c = tolower(c);
        else if( c != '.' )
            c = '-';
    }
    if( p.target == "home.html" )
        p.target = "index.html";

    // Split up the path
    size_t offset1=0, offset2;
    std::vector<std::string> folders;
    while( offset1 < p.path.length() )
    {
        offset2 = p.path.find( PATH_SEPARATOR, offset1 );
        if( offset2 == std::string::npos )
            break;
        std::string name = p.path.substr(offset1,offset2-offset1);   // eg "Archives", then "Tournaments"
        folders.push_back( name );
        offset1 = offset2+1;
    }

    // Auto generate tile, category, summary
    p.title = p.base;
    p.category = p.base;
    p.summary = "";
    size_t len = folders.size();
    if( len > 1 )
    {
        p.category = folders[len-1];
        if( p.base == folders[len-1] )
            p.summary  = folders[len-2] + " - " + folders[len-1];
        else
            p.summary  = folders[len-2] + " - " + folders[len-1] + " - " + p.base;
    }
    else if( len > 0 )
    {
        p.category = folders[len-1];
        if( p.base != folders[len-1] )
            p.summary  = folders[len-1] + " - " + p.base;
    }
}

void read_file( const char *plan_file, std::vector<Page> &results )
{
    std::ifstream fin( plan_file );
    unsigned long line_nbr=0;
    if( !fin )
    {
        printf( "Warning: Could not read plan file, creating new plan from directory structure\n" );
        return;
    }
    for(;;)
    {
        std::string line;
        if( !std::getline(fin,line) )
            break;
        line_nbr++;
        util::rtrim(line);
        size_t len = line.length();
        if( len > 0 && line[0]>=' ' )
        {
            Page p;
            size_t offset = line.find("->");
            if( offset != std::string::npos )
            {
                p.link = line.substr(offset+2);
                line = line.substr(0,offset);
                util::rtrim(line);
                util::ltrim(p.link);
                util::rtrim(p.link);
                p.is_link = (p.link.length()>1);
                len = line.length();
            }
            int level = 1;
            for( char c: line )
            {
                if( c == PATH_SEPARATOR )
                    level++;
            }
            if( line[len-1] == PATH_SEPARATOR && !p.is_link)
            {
                p.is_dir = true;
                line = line.substr(0,len-1);
                level--;
            }
            if( !p.is_dir && !p.is_link)
                p.is_file = true;
            p.path = line;
            p.plan_line_nbr = line_nbr;
            p.from_plan_file = true;
            p.level = level;
            parse(p);
            results.push_back( p );
        }
    }
}

void write_file( const char *plan_file, const std::vector<Page> &results )
{
    std::ofstream fout( plan_file );
    if( !fout )
    {
        printf( "Error: Could not update plan file %s\n", plan_file );
        return;
    }
    for( Page p: results )
    {
        std::string s = p.path;
        if( p.is_dir )
            s += PATH_SEPARATOR_STR;
        else if( p.is_link )
        {
            s += " -> ";
            s += p.link;
        }
        util::putline( fout, s );
    }
}

// Returns bool more
static bool get_next_page_group( std::vector<Page> &results, std::vector<Page*> &ptrs, bool restart )
{
    static size_t idx;
    if( restart )
        idx = 0;
    ptrs.clear();
    bool first = true;
    std::string group;
    while( idx < results.size() )
    {
        Page &p = results[idx++];
        if( !p.disabled )
        {
            if( first )
            {
                ptrs.push_back(&p);
                group = p.dir;
                first = false;
            }
            else
            {
                if( p.dir == group )
                    ptrs.push_back(&p);
                else
                {
                    idx--;
                    return true;
                }
            }
        }
    }
    return false;
}

// New pages not currently smoothly added, until this bug fixed do't do it
#define BUG_NEEDS_TO_BE_FIXED   // hmm acutally we need it defined or worse happens

void treebuilder()
{
    std::vector<Page> results;
    const char *plan_file = "plan.txt";
    read_file( plan_file, results );

    // Sync the directory structure to the plan
    recurse(BASE_IN,results);
    std::sort( results.begin(), results.end(), less_than_sync_plan_to_directory_structure );
    bool expecting_page_from_plan = true;
    bool rewrite = false;
    Page *plan_page=NULL;
    unsigned long sec_sort = 1;      // Used to insert new plan lines at an appropriate point

    printf( "### Sync stage 1\n" );
    for( Page &p: results )
        printf( "%d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );
    for( Page &p: results )
    {
        //printf( "%d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );
        if( expecting_page_from_plan )
        {
            if( p.from_plan_file )
            {
                // As expected path
                expecting_page_from_plan = false;
                plan_page = &p;
            }
            else
            {
                if( p.is_dir || p.ext=="md" || p.ext=="pgn" || p.ext=="html" )
                {
                    // Multiple pages in a row from directory structure = new files we don't know about
                    printf( "Info: new file or directory present: %s\n", p.path.c_str() );

                    // Add this line to plan, immediately after this point in plan
                    p.plan_line_nbr = plan_page ? plan_page->plan_line_nbr : 0;
                    p.added_to_plan_line_nbr = sec_sort++;

                    #ifdef BUG_NEEDS_TO_BE_FIXED
                    p.from_plan_file = true;
                    rewrite = true;
                    #endif
                }
            }
        }
        else
        {
            if( !p.from_plan_file )
            {
                expecting_page_from_plan = true;
                if( p.path == plan_page->path)
                {
                    // As expected path
                    //printf( " (as expected)\n" );
                }
                else
                {
                    // Unknown pages from directory structure = new files we don't know about
                    printf( "Info: A new file not present in plan file added: %s\n", p.path.c_str() );

                    // Add this line to plan, immediately after this point in plan
                    p.from_plan_file = true;
                    p.plan_line_nbr = plan_page ? plan_page->plan_line_nbr : 0;
                    p.added_to_plan_line_nbr = sec_sort++;

                    #ifdef BUG_NEEDS_TO_BE_FIXED
                    p.from_plan_file = true;
                    rewrite = true;
                    #endif
                }
            }
            else
            {
                // Multiple pages in a row from plan file = old files have been deleted
                if( plan_page->is_file )
                {
                    printf( "Info: A page in plan file unexpectedly absent, disabled: %s\n", plan_page->path.c_str() );
                    plan_page->disabled = true;
                }
                plan_page = &p;
            }
        }
    }

    // Reorder with the pages from the plan file first, in their original order (possibly supplemented by
    //  new files discovered in the sync process)
    std::sort( results.begin(), results.end(), less_than_restore_order );

    // After the sort the pages from the (possibly updated plan file) are first, from checking the directory
    //  structure are second, remove the directory section it has served its purpose
    auto it = results.begin();
    for( it = results.begin(); it!=results.end(); it++ )
    {
        if( !it->from_plan_file )
            break;
    }
    results.erase( it, results.end() );

    printf( "### Sync after removing Dir entries\n" );
    for( Page &p: results )
        printf( "%d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );

    // Rewrite the plan file. For the moment we use a temporary file name and don't overwrite the actual plan file
    if( rewrite )
        write_file( "temp_plan_file.txt", results );

    // Temporarily revert to sync order, to identify duplicate leaf nodes, eg Results.md *and* Results.html
    //  both generate same target
    std::sort( results.begin(), results.end(), less_than_sync_plan_to_directory_structure );
    Page *previous = NULL;
    for( Page &p: results )
    {
        if( p.is_file )
        {
            if( p.ext != "md" && p.ext != "html" && p.ext != "pgn")
            {
                p.disabled = true;
                printf( "Info: Page %s has unsupported extension (not .md or .pgn or .html), disabled\n", p.path.c_str() );
                
            }
            else if( previous && previous->is_file && p.target==previous->target )
            {
                if( previous->ext == p.ext || previous->ext == "md")
                {
                    p.disabled = true;
                    printf( "Info: Page %s resolves to a duplicate, disabled\n", p.path.c_str() );
                }
                else if( p.ext == "md" )
                {
                    previous->disabled = true;
                    printf( "Info: Page %s resolves to a duplicate, disabled\n", previous->path.c_str() );
                }
            }
        }
        previous = &p;
    }
    std::sort( results.begin(), results.end(), less_than_restore_order ); // restore to plan file order

    // Initial scan of each page, identifying targets for each directory
    bool first = true;
    for( bool more=true; more;)
    {
        std::vector<Page*> ptrs;
        more = get_next_page_group( results, ptrs, first );
        first = false;
        //-------------
        printf( "Debug: ------ Page group begin\n" );
        for( const Page *p: ptrs )
        {
            std::string status = p->is_file ? "is_file" : (p->is_link ? "is_link" : (p->is_dir?"is_dir":"is_??") );
            printf( "status:%s dir:\"%s\" path:\"%s\"\n", status.c_str(), p->dir.c_str(), p->path.c_str() );
        }
        printf( "Debug: ------ Page group end\n" );
        //-------------
        printf( "Debug: construct_dir_target()\n" );
        construct_dir_target(ptrs);
    }

    // Second scan of each page, building menus
    first = true;
    for( bool more=true; more;)
    {
        std::vector<Page*> ptrs;
        more = get_next_page_group( results, ptrs, first );
        first = false;
        printf( "Debug: construct_page_group()\n" );
        construct_page_group(ptrs);
    }
}

void recurse( const std::string &path, std::vector<Page> &results )
{
    static int level;
    level++;
    size_t base_in_len = strlen(BASE_IN);
    for( const auto & entry : fs::directory_iterator(path) )
    {
        Page p;
        p.level = level;
        std::string s( entry.path().string() );
        std::string t;
        if( s.length() >= (base_in_len+1) )
            t = s.substr((base_in_len+1));
        p.path = t;
        p.is_dir = is_directory(entry);
        p.is_file = !p.is_dir;
        parse(p);
        if( p.is_dir )
        {
            results.push_back( p );
            recurse( s, results );
        }
        else
            results.push_back( p );
    }
    level--;
}

