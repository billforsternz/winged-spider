//
// Markdown and PGN templates for .md and .pgn -> .html conversion
//

#ifndef TEMPLATES_H_INCLUDED
#define TEMPLATES_H_INCLUDED
#include <string>
#include <vector>
#include <map>
#include "defs.h"

// Template for markdown -> html conversion
struct MD_TEMPLATE
{
    std::string         filename;
    fs::file_time_type  file_time;
    std::string         header;
    std::string         footer;
    std::string         grid_1of3;
    std::string         grid_2of3;
    std::string         triple;
    std::string         pair;
    std::string         single;
    std::string         solo;
    std::string         panel;

    // Read in the markdown->html template from file, once
    bool read_template( const std::string &template_file );

    // Use the template to generate each html file from markdown
    void gen_html(  const std::string &in_file,
                    const std::string &html_out_file,
                    std::map<char,std::string> &macros,
                    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
};

// Template for PGN -> html conversion
struct PGN_TEMPLATE
{
    std::string         filename;
    fs::file_time_type  file_time;
    std::string         header;
    std::string         footer;

    // Read in the pgn->html template from file, once
    bool read_template( const std::string &template_file );

    // Use the template to generate each html file from PGN
    void gen_html(  const std::string &in_file,
                    const std::string &html_out_file,
                    const std::string &pgn_asset_full,
                    std::map<char,std::string> &macros,
                    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
};

#endif // TEMPLATES_H_INCLUDED
