// treebuilder.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

static std::map<std::string,std::string> directory_to_target;

// Each directory comprises a group of pages
// If directory is selected from a parent menu, it needs to resolve
//  to a target. The target is the first page in the directory if
//  that page is a leaf (a html file). If instead it's a ptr to
//  another directory, we need to go to that directory to find the
//  target. This can repeat of course, so we iterate until all the
//  targets are locked down
//
// Return true if more iterations needed to find this target
bool construct_target( std::vector<Page*> ptrs )
{
    if( ptrs.size() == 0 )
        return false;

    // First page in each directory is menu target for that directory
    Page *p = ptrs[0];

    // Stop if the target for this directory is already established
    auto it = directory_to_target.find(p->dir);
    bool found = (it != directory_to_target.end());
    if( found )
        return false;

    // Typically target is a leaf html page 
    std::string target = p->target;
    found = true;
    if( p->is_dir )  // but if it's a directory, look to forward to the
                     //  target for that directory
    {
        auto it = directory_to_target.find(p->path);
        found = (it != directory_to_target.end());
        if( found )
            target = it->second;
    }
    if( found )
        directory_to_target[p->dir] = target;
    return !found;   // return true if more work to do
}

void construct_page_group( std::vector<Page*> ptrs )
{
    if( ptrs.size() == 0 )
        return;

    // Calculate the menu to present for each page in the group
    std::vector<std::string> menu;
    std::string build_path;
    Page *p = ptrs[0];

    // If not at root, start with "Home" link to start of root directory
    if( p->dir.length() > 0 )
    {
        std::string menu_txt = directory_to_target[""] + " Home" ;
        menu.push_back( menu_txt );
    }

    // Each level of subdirectory gets a menu item, eg "Archives\Tournaments
    size_t offset1=0, offset2;
    while( offset1 < p->dir.length() )
    {
        offset2 = p->dir.find( PATH_SEPARATOR, offset1 );
        if( offset2 == std::string::npos )
            break;
        std::string subdir = p->dir.substr( 0, offset2 );              // eg "Archives", then "Archives\Tournaments"
        std::string name   = p->dir.substr(offset1,offset2-offset1);   // eg "Archives", then "Tournaments"
        std::string menu_txt =  directory_to_target[subdir] + " " + name;
        menu.push_back( menu_txt );
        offset1 = offset2+1;
    }
    for( Page *p: ptrs )
    {
        std::string menu_txt;
        if( p->is_dir )
            menu_txt = directory_to_target[p->path] + " " + p->base;
        else
            menu_txt = p->target + " " + p->base;
        menu.push_back( menu_txt );
    }
    printf("\nMenu>\n");
    for(std::string s:menu)
    {
        printf(" %s\n",s.c_str());
    }

    // Build each page in turn
    //  (force build for now)
    for( Page *p: ptrs )
    {
        if( "md" == p->ext )
        {
            markdown_gen( p, menu );
        }
        else if( "html" == p->ext )
        {
            html_gen( p, menu );
        }
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
    else if( lhs.name != rhs.name )
        return lhs.name < rhs.name;
    else if( lhs.from_plan_file != rhs.from_plan_file )
        return lhs.from_plan_file;
    else if( lhs.plan_line_nbr != rhs.plan_line_nbr )
        return lhs.plan_line_nbr < rhs.plan_line_nbr;
    return false;
}

void parse( Page &p )
{
    size_t offset = p.path.find_last_of( PATH_SEPARATOR );
    if( offset == std::string::npos )
    {
        p.dir  = "";
        p.name = p.path;
    }
    else
    {
        p.dir  = p.path.substr(0,offset);
        p.name = p.path.substr(offset+1);
    }
    offset = p.name.find_last_of( '.' );
    if( offset == std::string::npos )
        p.base = p.name;
    else
    {
        p.base = p.name.substr(0,offset);
        p.ext  = util::tolower(p.name.substr(offset+1));
    }
    if( !p.is_dir )
    {
        if( p.dir.length() == 0 )
            p.target = PATH_SEPARATOR_STR + p.base + ".html";
        else
            p.target = PATH_SEPARATOR_STR + p.dir + PATH_SEPARATOR_STR + p.base + ".html";
        #if PATH_SEPARATOR == '\\'
        for( char &c: p.target )
        {
            if( c == PATH_SEPARATOR )
                c = '/';
        }
        #endif
    }
}

void read_file( const char *plan_file, std::vector<Page> &results )
{
    std::ifstream fin( plan_file );
    int line_nbr=0;
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
            int level = 1;
            for( char c: line )
            {
                if( c == PATH_SEPARATOR )
                    level++;
            }
            if( line[len-1] == PATH_SEPARATOR )
            {
                p.is_dir = true;
                line = line.substr(0,len-1);
                level--;
            }
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
        printf( "Error: Could not update plan file\n" );
        return;
    }
    for( Page p: results )
    {
        util::putline( fout, p.path );
    }
}

void treebuilder()
{
    std::vector<Page> results;
    const char *plan_file = "plan.txt";
    read_file( plan_file, results );

    // Syncing the directory structure to the plan needs some work. My simple sorting approach won't cut it;
    // It requires a map of the lines in the plan I think. Return to this later
    #if 0
    recurse(BASE_IN,results);
    std::sort( results.begin(), results.end(), less_than_sync_plan_to_directory_structure );
    bool expecting_page_from_plan = true;
    bool rewrite = false;
    Page *plan_page=NULL;
    for( Page &p: results )
    {
        printf( "%d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );
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
                // Multiple pages in a row from directory structure = new files we don't know about
                printf( "Info: new file present: %s\n", p.path.c_str() );
                rewrite = true;
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
                    printf( " (as expected)\n" );
                }
                else
                {
                    // Unknown pages from directory structure = new files we don't know about
                    printf( "Info: new file present %s\n", p.path.c_str() );
                    rewrite = true;
                }
            }
            else
            {
                // Multiple pages in a row from plan file = old files have been deleted
                printf( "Info: previous file absent: %s\n", plan_page->path.c_str() );
                rewrite = true;
                plan_page = &p;
            }
        }
    }
    #endif

    // Initial scan of each page, identifying targets for each directory
    bool keep_going = true;
    for( int i=0; keep_going && i<1000; i++ )
    {
        bool final = (i+1 >= 1000);
        keep_going = false;   // stop once all targets constructed
        std::vector<Page*> ptrs;
        std::string group;
        for( Page &p: results )
        {
            if( p.from_plan_file )
            {
                if( p.dir == group )
                    ptrs.push_back(&p);
                else
                {
                    if( construct_target(ptrs) )
                    {
                        keep_going = true;
                        if( final )
                        {
                            printf( "Fatal: At least one unresolved directory with no target html file\n"
                                    "Directory is %s\n", ptrs[0]->dir.c_str() );
                            return;
                        }
                    }
                    group = p.dir;
                    ptrs.clear();
                    ptrs.push_back(&p);
                }
            }
        }
        if( construct_target(ptrs) )
        {
            keep_going = true;
            if( final )
            {
                printf( "Fatal: At least one unresolved directory with no target html file\n"
                    "Directory is %s\n", ptrs[0]->dir.c_str() );
                return;
            }
        }
    }

    // Second scan of each page, building pages
    std::vector<Page*> ptrs;
    std::string group;
    for( Page &p: results )
    {
        if( p.from_plan_file )
        {
            if( p.dir == group )
                ptrs.push_back(&p);
            else
            {
                construct_page_group(ptrs);
                group = p.dir;
                ptrs.clear();
                ptrs.push_back(&p);
            }
        }
    }
    construct_page_group(ptrs);
    //  write_file( plan_file, results );
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
        parse(p);
        p.is_dir = is_directory(entry);
        if( p.is_dir )
            recurse(s,results);
        else
            results.push_back( p );
    }
    level--;
}

