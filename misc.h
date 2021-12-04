/*

    Miscellaneous stuff until the structure clarifies somewhat
    
*/

#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <string>
#include <map>
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
#define BASE_OUT "C:\\Users\\Bill\\Documents\\newzealandchess.co.nz"

void construct_page_group( std::vector<Page*> ptrs );
void recurse( const std::string &path, std::vector<Page> &results );
void parse( Page &p );
void read_file( const char *plan_file, std::vector<Page> &results );
void write_file( const char *plan_file, const std::vector<Page> &results );
void treebuilder();
void recurse( const std::string &path, std::vector<Page> &results );
bool markdown_gen( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
bool pgn_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
bool html_gen( Page *p );
void templat( const std::string &md_file, const std::string &template_file, const std::string &html_out_file,
              std::map<char,std::string> &macros,
              const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
std::string macro_substitution( const std::string &input,
    const std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );


#endif // MISC_H_INCLUDED
