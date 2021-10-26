/*

    Miscellaneous stuff until the structure clarifies somewhat
    
*/

#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <string>
#include <vector>
#include "Page.h"

#ifdef _WIN64
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#endif
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#endif
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#endif

#define BASE_IN  "base"
#define BASE_OUT "output"

void construct_page_group( std::vector<Page*> ptrs );
void recurse( const std::string &path, std::vector<Page> &results );
void parse( Page &p );
void read_file( const char *plan_file, std::vector<Page> &results );
void write_file( const char *plan_file, const std::vector<Page> &results );
void treebuilder();
void recurse( const std::string &path, std::vector<Page> &results );
bool markdown_gen( Page *p, const std::vector<std::string> &menu );
bool html_gen( Page *p, const std::vector<std::string> &menu );

#endif // MISC_H_INCLUDED
