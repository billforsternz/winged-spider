//
//  Directory traversal to build and maintain a 'plan' or sitemap - the core idea of Winged Spider
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

// Local Helpers and data
static void parse( Page &p );
static bool read_file( const char *plan_file, std::vector<Page> &results );
static void write_file( const char *plan_file, const std::vector<Page> &results );
static void recurse( const std::string &path, std::vector<Page> &results );
static void sync_debug( const char *msg, const std::vector<Page> &results );
static bool get_next_page_group( std::vector<Page> &results, std::vector<Page*> &ptrs, bool restart );
static unsigned count_md_gen;
static unsigned count_pgn_gen;
static unsigned count_html_gen;

// Build a map of the menus for each directory for the previous run, and use this
//  to help determine if a file needs to be rebuilt
static std::map< std::string, std::vector< std::pair<std::string,std::string> > >  menus;

//
// Identify groups of pages (pages with the same dir) in the plan. It's easy
//  because the pages can be sorted so they 'group' together. Construct menus
//  from these groups. Finally build the individual pages in each group.
//  These operations are gathered into a Builder class so we can do the whole
//  thing twice - the first time is a dummy run using the generated-plan.txt
//  file from the most recent invocation of Winged Spider on this work directory.
//  The second time is the actual run, if the menu generated for a group of
//  pages in the dummy run differs from the actual run - then those pages need
//  to be rebuilt even if their source files don't post-date their html files.
class Builder
{
    bool previous_run;

public:
    Builder( bool previous_run ) { this->previous_run = previous_run; }
    std::map<std::string,Page *> directory_to_target;
    void construct_dir_target( std::vector<Page*> ptrs );
    bool construct_page_group( std::vector<Page*> ptrs, bool force_rebuild );
    bool run( std::vector<Page> &results, bool force_rebuild=false );
};

// Each directory comprises a group of pages
// If the directory is selected from a parent menu, it needs to resolve
//  to a target. The target is the first page in the directory if
//  that page is a leaf (a html file). If the directory is has no
//  files, it is an empty intermediate directory. A simple .html
//  file is constructed to represent the directory in the menu
//  system, using the make_file_for_dir flag.
void Builder::construct_dir_target( std::vector<Page*> ptrs )
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
// Return true if changed menu detected
bool Builder::construct_page_group( std::vector<Page*> ptrs, bool force_rebuild )
{
    if( ptrs.size() == 0 )
        return false;

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
            if( p->ext=="md" || p->ext=="md2" || p->ext=="pgn" || p->ext=="html" )
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
    Page *p0 = ptrs[0];

    // If this is the previous run of Winged Spider on this input directory, save the menu of this
    //  page group and stop.
    if( previous_run )
    {
        menus[p0->dir] = menu;
        return false;
    }

    // This is not the previous run of Winged Spider on this input directory, - check if the menu
    //  for this page group has changed since that previous run
    bool same_menu_as_last_run = false;
    auto it = menus.find(p0->dir);
    same_menu_as_last_run = (it != menus.end() && it->second==menu);
    if( !same_menu_as_last_run )
    {
        printf( "Info: Menu changed for all pages in directory %s\n", p0->dir.c_str() );
    }
    if( get_verbosity() > 0 )
    {
        printf("\nMenu>\n");
        for(std::pair<std::string,std::string> menu_item: menu )
        {
            printf("%s %s\n", menu_item.first.c_str(), menu_item.second.c_str() );
            //if( menu_item.second == "Tournaments" )
            //    printf("Debug break 1\n");
        }
    }

    // If we are making an html file for an empty directory, do it now with
    //  index set to the last element of the split path
    if( make_file_for_dir )
        md_to_html( make_file_for_dir, menu, menu_idx-1, same_menu_as_last_run, force_rebuild );

    // Build each page in turn
    for( Page *p: ptrs )
    {
        if( !p->is_file || p->disabled )
            ;
        else if( "md" == p->ext )
        {
            if( md_to_html( p, menu, menu_idx, same_menu_as_last_run, force_rebuild ) )
            {
                count_md_gen++;
            }
        }
        else if( "pgn" == p->ext )
        {
            if( pgn_to_html( p, menu, menu_idx, same_menu_as_last_run, force_rebuild ) )
            {
                count_pgn_gen++;
            }
        }
        else if( "html" == p->ext )
        {
            if( html_to_html( p, force_rebuild ) )
            {
                count_html_gen++;
            }
        }
        menu_idx++;
    }
    return !same_menu_as_last_run;
}

// Return true if any menu changes detected
bool Builder::run( std::vector<Page> &results, bool force_rebuild )
{
    bool any=false;

    // Initial scan of each page, identifying targets for each directory
    bool first = true;
    for( bool more=true; more;)
    {
        std::vector<Page*> ptrs;
        more = get_next_page_group( results, ptrs, first );
        first = false;
        //-------------
        cprintf( "Debug: ------ Page group begin\n" );
        for( const Page *p: ptrs )
        {
            std::string status = p->is_file ? "is_file" : (p->is_link ? "is_link" : (p->is_dir?"is_dir":"is_??") );
            cprintf( "status:%s dir:\"%s\" path:\"%s\"\n", status.c_str(), p->dir.c_str(), p->path.c_str() );
        }
        cprintf( "Debug: ------ Page group end\n" );
        //-------------
        cprintf( "Debug: construct_dir_target()\n" );
        construct_dir_target(ptrs);
    }

    // Second scan of each page, building menus
    first = true;
    for( bool more=true; more;)
    {
        std::vector<Page*> ptrs;
        more = get_next_page_group( results, ptrs, first );
        first = false;
        cprintf( "Debug: construct_page_group()\n" );
        if( construct_page_group(ptrs,force_rebuild) )
            any = true;
    }
    return any;
}

// Predicate for sorting when we are comparing the plan file and the actual directory
//  structure
static bool less_than_sync_plan_to_directory_structure( const Page &lhs,  const Page &rhs )
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
static bool less_than_restore_order( const Page &lhs,  const Page &rhs )
{
    if( lhs.plan_line_nbr != rhs.plan_line_nbr )
        return lhs.plan_line_nbr < rhs.plan_line_nbr;
    return lhs.added_to_plan_line_nbr < rhs.added_to_plan_line_nbr;
}

//
// main() does some startup work, then calls this to sync the directory structure to the
//  plan file and build the project
//

void treebuilder( bool force_rebuild, bool check_dependencies_only )
{
    Builder previous_time(true);
    Builder this_time(false);

    // Build the menus for the previous run, to help us determine which pages need to be rebuilt
    std::vector<Page> pages;
    const char *previous_plan_file = GENERATED_PLAN_TXT;
    bool previous_run_plan_found = read_file( previous_plan_file, pages );
    sync_debug( "After reading previous run", pages );
    previous_time.run( pages );
    sync_debug( "After previous_time.run()", pages );

    // Now do it for real    
    std::vector<Page> results;
    const char *plan_file = PLAN_TXT;
    bool plan_found = read_file( plan_file, results );
    sync_debug( "Start with plan", results );
    if( !plan_found )
    {
        const char *msg = 
        "Info: File plan.txt not found. Winged Spider will use its auto generated plan\n"
        "file generated-plan.txt instead and will copy that file to plan.txt for\n"
        "next time. Please review and edit plan.txt if necessary\n";
        printf( "%s", msg );
    }
    else if( !previous_run_plan_found )
    {
        const char *msg = 
        "Info: Winged Spider could not find file generated-plan.txt from its most\n"
        "recent run. This means it cannot check for directory changes, so it will\n"
        "have to assume the directory structure has changed and generate new files\n";
        printf( "%s", msg );
    }
    else
    {
        const char *msg = 
        "Info: Creating menu structure and checking for changes from the last run\n";
        printf( "%s", msg );
    }

    // Sync the directory structure to the plan
    printf( "Info: Synching to directory structure, in case there are changes\n" );
    recurse(BASE_IN,results);
    sync_debug( "Add directory structure", results );

    std::sort( results.begin(), results.end(), less_than_sync_plan_to_directory_structure );
    sync_debug( "Sort to start sync", results );
    bool expecting_page_from_plan = true;
    bool rewrite = false;
    Page *plan_page=NULL;
    unsigned long sec_sort = 1;      // Used to insert new plan lines at an appropriate point

    if( get_verbosity() > 1 )
    {
        printf( "Before syncing\n" );
        for( Page &p: results )
            printf( "%d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );
    }
    int idx=0;
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

                    // Sorry this is a bit complicated (tag RefineNewFilePlanLocation)
                    // We scan backwards through the group of pages preceding this one, looking for the
                    // one with the greatest line number in the plan file. This new page is going to go
                    // after that one. The group is defined as pages with the same dir.
                    int idx2 = idx-1;
                    unsigned long max_so_far = plan_page ? plan_page->plan_line_nbr : 0;
                    bool triggered = false;
                    while( plan_page && idx2 >= 0 )
                    {
                        const Page *q = &results[idx2--];
                        if( q == plan_page )
                            triggered = true;
                        cprintf( "q->path=%s, q->from_plan_file=%s, q->dir=%s, plan_page->dir=%s, q->plan_line_nbr=%d, max_so_far=%d\n", 
                            q->path.c_str(),
                            q->from_plan_file?"true":"false", q->dir.c_str(), plan_page->dir.c_str(), q->plan_line_nbr, max_so_far );
                        if( q->from_plan_file && q->dir==plan_page->dir && q->plan_line_nbr>max_so_far )
                            max_so_far = q->plan_line_nbr;
                        if( q->dir != plan_page->dir && triggered )
                            break;                            
                    }
                    cprintf( "> max_so_far=%lu\n", max_so_far );
                    p.plan_line_nbr = max_so_far;
                    p.added_to_plan_line_nbr = sec_sort++;
                    p.from_plan_file = true;
                    rewrite = true;
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

                    p.from_plan_file = true;
                    rewrite = true;
                }
            }
            else
            {

                // Multiple pages in a row from plan file = old files have been deleted
                if( plan_page->is_file )   //i.e not a link
                {
                    rewrite = true;
                    printf( "Info: A page in plan file unexpectedly absent, disabled: %s\n", plan_page->path.c_str() );
                    plan_page->disabled = true;
                }
                plan_page = &p;
            }
        }
        idx++;
    }

    // Reorder with the pages from the plan file first, in their original order (possibly supplemented by
    //  new files discovered in the sync process)
    std::sort( results.begin(), results.end(), less_than_restore_order );
    sync_debug( "After sync", results );

    // After the sort the pages from the (possibly updated plan file) are first, from checking the directory
    //  structure are second, remove the directory section it has served its purpose
    auto it = results.begin();
    for( it = results.begin(); it!=results.end(); it++ )
    {
        if( !it->from_plan_file )
            break;
    }
    results.erase( it, results.end() );
    sync_debug( "After removing directory entries", results );

    if( get_verbosity() > 1 )
    {
        printf( "Syncing complete\n" );
        for( Page &p: results )
            cprintf( "%d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );
    }

    // Temporarily revert to sync order, to identify duplicate leaf nodes, eg Results.md *and* Results.html
    //  both generate same target
    std::sort( results.begin(), results.end(), less_than_sync_plan_to_directory_structure );
    sync_debug( "After temporarily reverting to sync order", results );
    Page *previous = NULL;
    for( Page &p: results )
    {
        if( p.is_file )
        {
            if( p.ext != "md" && p.ext != "md2" && p.ext != "html" && p.ext != "pgn")
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
    sync_debug( "After restoring plan file order", results );

    bool menu_changes = this_time.run( results, force_rebuild );
    sync_debug( "After this_time.run()", results );

    // Report results
    if( check_dependencies_only )
    {
        if( count_md_gen==0 && count_pgn_gen==0 && count_html_gen==0 )
            printf( "Info: No html files need to be (re)generated\n" );
        else
        {
            printf( "Info: %u output html file%s need to be (re)generated from input markdown files\n", count_md_gen, count_md_gen==1?"":"s" );
            printf( "Info: %u output html file%s need to be (re)generated from input pgn files\n", count_pgn_gen, count_pgn_gen==1?"":"s" );
            printf( "Info: %u output html file%s need to be copied from input html files\n", count_html_gen, count_html_gen==1?"":"s" );
        }
    }
    else
    {
        if( count_md_gen==0 && count_pgn_gen==0 && count_html_gen==0 )
            printf( "Info: No html files (re)generated\n" );
        else
        {
            printf( "Info: %u output html file%s (re)generated from input markdown files\n", count_md_gen, count_md_gen==1?" was":"s were" );
            printf( "Info: %u output html file%s (re)generated from input pgn files\n", count_pgn_gen, count_pgn_gen==1?" was":"s were" );
            printf( "Info: %u output html file%s copied from input html files\n", count_html_gen, count_html_gen==1?" was":"s were" );
        }
    }

    // Rewrite the plan file. For the moment at least we use a temporary file name and don't overwrite the actual plan file
    if( check_dependencies_only )
        return;
    if( rewrite || menu_changes || !plan_found || !previous_run_plan_found )
    {
        printf( "Info: %s generated-plan.txt\n", previous_run_plan_found?"Writing":"Rewriting" );
        write_file( GENERATED_PLAN_TXT, results );
        if( !plan_found )
            write_file( PLAN_TXT, results );
    }
}

//
// Helpers
//

// Recursively read the contents of the input directory, identifying every file in every subdirectory
//  and storing them as a list of pages
static void recurse( const std::string &path, std::vector<Page> &results )
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

// Parse the path (i.e. fully qualified filename) from a page to find all the
//  other attributes of that page
static void parse( Page &p )
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

    // Split path into component names, // eg "Archives/Tournaments/2020.md" -> "Archives", "Tournaments", "2020"
    size_t offset1=0, offset2;
    std::vector<std::string> components;
    while( offset1 < p.path.length() )
    {
        offset2 = p.path.find( PATH_SEPARATOR, offset1 );
        if( offset2 == std::string::npos )
        {
            offset2 = p.path.find( '.', offset1 );
            if( offset2 != std::string::npos )
            {
                std::string name = p.path.substr(offset1,offset2-offset1);   // eg 
                components.push_back( name );
            }
            break;
        }
        std::string name = p.path.substr(offset1,offset2-offset1);   // eg "Archives", then "Tournaments"
        components.push_back( name );
        offset1 = offset2+1;
    }

    // Auto generate Heading, subheading
    // std::string heading;    // Eg "Archives Tournaments" (@S for historical reasons)
    // std::string subheading; // Eg "2021" (@Z for historical reasons)
    size_t len = components.size();
    if( len >=2 && components[len-1] == components[len-2] )
    {
        components.pop_back();
        len--;
    }
    if( len == 1 )
    {
        p.subheading = "";
        p.heading = components[0];
    }
    else if( len > 1 )
    {
        p.subheading = components[len-1];
        p.heading = "";
        for( size_t i=0; i<len-1; i++ )
        {
            p.heading += components[i];
            if( i+1 < len-1 )
                p.heading += " ";
        }
    }
}

// Read a plan file into a vector of Pages
static bool read_file( const char *plan_file, std::vector<Page> &results )
{
    std::ifstream fin( plan_file );
    unsigned long line_nbr=0;
    if( !fin )
        return false;
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
    return true;
}

// Write a vector of Pages into a plan file
static void write_file( const char *plan_file, const std::vector<Page> &results )
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

// Identify a groups of pages (pages with the same dir) from a plan. It's easy
//  because the pages are sorted so they 'group' together.
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

// For troubleshooting sync, it helps to look at the list of pages as they are sorted and
//  filtered
static void sync_debug( const char *msg, const std::vector<Page> &results )
{
    if( get_verbosity() == 0 )
        return;
    cprintf( "** Sync debug: %s\n", msg );
    for( const Page &p: results )
    {
        //cprintf( " %d: %s%s\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str() );
        cprintf( " %d: %s%s (%d,%d)\n", p.level, p.from_plan_file? "F> " : "D> ", p.path.c_str(), p.plan_line_nbr, p.added_to_plan_line_nbr );
    }
}
