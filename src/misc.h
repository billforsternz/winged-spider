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

#ifdef _DEBUG
#define GENERATED_PLAN_TXT  "example/generated-plan.txt"
#define PLAN_TXT            "example/plan.txt"
#define BASE_IN             "example/input"
#define BASE_OUT            "example/output"
#define BASE_TEMPLATE       "example/template"
#else
#define GENERATED_PLAN_TXT  "generated-plan.txt"
#define PLAN_TXT            "plan.txt"
#define BASE_IN             "input"
#define BASE_OUT            "output"
#define BASE_TEMPLATE       "template"
#endif

void recurse( const std::string &path, std::vector<Page> &results );
void recursive_file_copy( const std::string &src, const std::string &dst, bool init );
void parse( Page &p );
bool read_file( const char *plan_file, std::vector<Page> &results );
void write_file( const char *plan_file, const std::vector<Page> &results );
void treebuilder( bool force_rebuild, bool check_dependencies_only );
void recurse( const std::string &path, std::vector<Page> &results );
bool markdown_gen( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild );
bool pgn_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild );
bool html_gen( Page *p, bool force_rebuild );
void template_md( const std::string &md_file, const std::string &template_file, const std::string &html_out_file,
    std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );

std::string macro_substitution( const std::string &input,
    const std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
int cprintf( const char *fmt, ... );
int get_verbosity();
void test_builds();

#endif // MISC_H_INCLUDED
