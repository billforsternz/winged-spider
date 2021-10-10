// treebuilder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
namespace fs = std::experimental::filesystem;
#include "..\util.h"


#ifdef _WIN64
#define PATH_SEPARATOR '\\'
#endif
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#endif
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR '/'
#endif

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
    std::string path;   // the full path name, eg "path/to/file.txt"
    std::string name;   // if it's a file, this is the name of the file eg "file.txt"
    std::string dir;    // if it's a file, this is the name of the dir eg "path/to"

    // Features to be added later to support more user annotation for files
    bool empty_line_prefix = false;
    bool empty_line_suffix = false;
    std::vector<std::string> prefix;
    std::vector<std::string> suffix;
    std::vector<std::string> link;    // if this is actually a ptr to another page
};

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

void recurse( const std::string &path, std::vector<Page> &results );

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
        if( line.length() > 0 && line[0]>=' ' ) 
        {
            Page p;
            p.path = line;
            p.plan_line_nbr = line_nbr;
            p.from_plan_file = true;
            int level = 1;
            for( char c: line )
            {
                if( c == PATH_SEPARATOR )
                    level++;
            }
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

int main()
{
    std::vector<Page> results;
    const char *plan_file = "plan.txt";
    read_file( plan_file, results );
    recurse("../base",results);
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
//  write_file( plan_file, results );
    return 0;
}

void recurse( const std::string &path, std::vector<Page> &results )
{
    static int level;
    level++;
    for( const auto & entry : fs::directory_iterator(path) )
    {
        Page p;
        p.level = level;
        std::string s( entry.path().string() );
        std::string t;
        if( s.length() >= 8 )
            t = s.substr(8);
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
