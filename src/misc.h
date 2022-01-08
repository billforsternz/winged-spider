/*

    Miscellaneous stuff until the structure clarifies somewhat
    
*/

#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <string>
#include <map>
#include <vector>
#include "Page.h"

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"

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

void treebuilder( bool force_rebuild, bool check_dependencies_only );
bool md_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild );
bool pgn_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild );
bool html_to_html( Page *p, bool force_rebuild );
std::string macro_substitution( const std::string &input,
    const std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
int cprintf( const char *fmt, ... );
int get_verbosity();
std::string md( const std::string &in );

#endif // MISC_H_INCLUDED
