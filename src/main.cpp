/*

    Winged Spider is a static website builder. For full documentation start at the project's
    home page

    https://github.com/billforsternz/winged-spider.html

    Winged Spider is currently approaching V1.0 release status

*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdarg.h>     // va_arg() etc
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
namespace fs = std::experimental::filesystem;
#include "util.h"
#include "page.h"
#include "misc.h"
#include "../src-tarrasch/GamesCache.h"
#include "md4c-html.h"

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
    bool read_template( const std::string &template_file )
    {
        filename = template_file;
        fs::path ptemplate(template_file);
        if( !fs::exists(ptemplate) )
        {
            printf( "Error: Template file %s doesn't exist\n", template_file.c_str() );
            return false;
        }
        file_time = last_write_time(ptemplate);

        std::ifstream fin( template_file );
        if( !fin )
        {
            printf( "Error: Could not open template file %s\n", template_file.c_str() );
            return false;
        }
        bool in_section = false;
        enum { s_header, s_footer, s_1of3, s_2of3, s_triple, s_pair, s_single, s_solo, s_panel } section;
        for(;;)
        {
            std::string s;
            if( !std::getline(fin,s) )
                break;
            util::rtrim(s);
            if( !in_section )
            {
                in_section = true;
                if( s == "" )
                    in_section = false;
                else if( s == "@header" )
                    section = s_header;
                else if( s == "@footer" )
                    section = s_footer;
                else if( s == "@1of3" )
                    section = s_1of3;
                else if( s == "@2of3" )
                    section = s_2of3;
                else if( s == "@single" )
                    section = s_single;
                else if( s == "@pair" )
                    section = s_pair;
                else if( s == "@triple" )
                    section = s_triple;
                else if( s == "@solo" )
                    section = s_solo;
                else if( s == "@panel" )
                    section = s_panel;
                else
                {
                    if( s[0] == '@' )
                        printf( "Error: Template file has unknown section [%s] (not @header,\n@footer, @solo, @panel, @single, @pair, @1of3, @2of3 or @triple)\n", s.c_str() );
                    else
                        printf( "Error: Template file has section starting without a section identifier (eg @header,\n@footer, @solo, @panel, @single, @pair, @1of3, @2of3 or @triple)\n" );
                    return false;
                }
            }
            else
            {
                if( s.length() == 0 )
                {
                    in_section = false;
                }
                else
                {
                    switch( section )
                    {
                        case s_header:
                            header += s;
                            header += '\n';
                            break;
                        case s_1of3:
                            grid_1of3 += s;
                            grid_1of3 += '\n';
                            break;
                        case s_2of3:
                            grid_2of3 += s;
                            grid_2of3 += '\n';
                            break;
                        case s_triple:
                            triple += s;
                            triple += '\n';
                            break;
                        case s_pair:
                            pair += s;
                            pair += '\n';
                            break;
                        case s_single:
                            single += s;
                            single += '\n';
                            break;
                        case s_footer:
                            footer += s;
                            footer += '\n';
                            break;
                        case s_solo:
                            solo += s;
                            solo += '\n';
                            break;
                        case s_panel:
                            panel += s;
                            panel += '\n';
                            break;
                    }
                }
            }
        }
        return true;
    }

    // Use the template to generate each html file from markdown
    void gen_html(  const std::string &in_file,
                    const std::string &html_out_file,
                    std::map<char,std::string> &macros,
                    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
};

struct PGN_TEMPLATE
{
    std::string         filename;
    fs::file_time_type  file_time;
    std::string         header;
    std::string         footer;

    // Read in the pgn->html template from file, once
    bool read_template( const std::string &template_file )
    {
        filename = template_file;
        fs::path ptemplate(template_file);
        if( !fs::exists(ptemplate) )
        {
            printf( "Error: Template file %s doesn't exist\n", template_file.c_str() );
            return false;
        }
        file_time = last_write_time(ptemplate);
        std::ifstream fin( template_file );
        if( !fin )
        {
            printf( "Error: Could not open template file %s\n", template_file.c_str() );
            return false;
        }

        // Read the template file
        bool in_section = false;
        enum { s_header, s_footer } section;
        for(;;)
        {
            std::string s;
            if( !std::getline(fin,s) )
                break;
            util::rtrim(s);
            if( !in_section )
            {
                in_section = true;
                if( s == "" )
                    in_section = false;
                else if( s == "@header" )
                    section = s_header;
                else if( s == "@footer" )
                    section = s_footer;
                else
                {
                    if( s[0] == '@' )
                        printf( "Error: PGN Template file has unknown section [%s] (not @header or @footer)\n", s.c_str() );
                    else
                        printf( "Error: PGN Template file has section starting without a section identifier (eg @header or @footer)\n" );
                    return false;
                }
            }
            else
            {
                if( s.length() == 0 )
                {
                    in_section = false;
                }
                else
                {
                    switch( section )
                    {
                        case s_header:
                            header += s;
                            header += '\n';
                            break;
                        case s_footer:
                            footer += s;
                            footer += '\n';
                            break;
                    }
                }
            }
        }
        return true;
    }

    // Use the template to generate each html file from PGN
    void gen_html(  const std::string &in_file,
                    const std::string &html_out_file,
                    const std::string &pgn_asset_full,
                    std::map<char,std::string> &macros,
                    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx )
    {
        GamesCache gc;
        gc.Load(in_file,pgn_asset_full);
        gc.Publish(html_out_file,header,footer,macros,menu,menu_idx);
    }
};

//
// Helpers etc.
//

static MD_TEMPLATE md_template;
static PGN_TEMPLATE pgn_template;
static bool check_dependencies_only;
static bool verbose;
int get_verbosity()
{
    return verbose ? 1 : 0;
}

int cprintf( const char *fmt, ... )
{
    if( !verbose )
        return 0;
    int ret=0;
	va_list args;
	va_start( args, fmt );
    char buf[1000];
    vsnprintf( buf, sizeof(buf)-2, fmt, args ); 
    buf[sizeof(buf)-1] = '\0';
    printf("%s",buf);
    va_end(args);
    return ret;
}

static bool probe()
{
    bool ok = true;
    const char* help =
    "Winged Spider is a tool for building websites. It expects to see \"input\"\n"
    "\"output\" and \"template\" directories. The \"input\" directory holds\n"
    "the content of the website, hopefully organised in a sensible folder\n"
    "hierarchy. Winged Spider converts this input into html files in the\n"
    "\"output\" directory, using templates from the \"template\" directory.\n"
    "Normally the input files are in markdown format, and the template-md.txt\n"
    "file in the \"template\" directory guides the conversion process. Winged\n"
    "Spider is distributed with a simple example to get you started.\n";
    
    //std::string path("template");
    std::vector<std::string> prerequisite_directories{BASE_IN,BASE_OUT,BASE_TEMPLATE};
    for( std::string s: prerequisite_directories )
    {
        fs::path dir(s); 
        if( !fs::exists(dir) )
        {
            printf( "Error: Required directory \"%s\" doesn't exist\n", s.c_str() );
            ok = false;
        }
        else if( !fs::is_directory(dir) )
        {
            printf( "Error: Required directory \"%s\" is a file not a directory\n", s.c_str() );
            ok = false;
        }
    }
    if( !ok )
    {
        printf( "%s", help );
        return false;
    }
    std::string assets = BASE_OUT;
    assets += PATH_SEPARATOR_STR;
    assets += "assets";
    fs::path dir(assets); 
    if( !fs::exists(dir) || !fs::is_directory(dir) )
    {
        bool ok2 = fs::create_directory(dir);
        if( ok2 )
            printf( "Warning: Directory \"%s\" not found, successfully created\n", assets.c_str() );
        else
        {
            printf( "Error: Directory \"%s\" not found and could not be created\n", assets.c_str() );
            return false;
        }
    }

    // Copy all template files, except template .txt files, to output. The idea is that runtime
    //  files defined in the templates (eg javascript and css files) can be conveniently
    //  put in the template directory and will be copied (once) to the output directory
    ok = recursive_file_copy( std::string(BASE_TEMPLATE), std::string(BASE_OUT), true );
    if( !ok )
        return false;

    // Read the markdown template, once
    ok = md_template.read_template( std::string(BASE_TEMPLATE) + "/template-md.txt" );
    if( !ok )
        return false;

    // Read the pgn template, once
    ok = pgn_template.read_template( std::string(BASE_TEMPLATE) + "/template-pgn.txt" );
    return ok;
}

//
// main()
//

int main( int argc, char *argv[] )
{

    // Process command line arguments
    const char *usage =
    "Winged Spider V0.9\n"
    "Winged Spider is a simple static website builder\n"
    "Winged Spider takes as input a directory hierarchy of markdown and PGN content\n"
    "and generates from it a website users can navigate through using menus defined\n"
    "by the folder hierarchy\n"
    "Currently command line arguments are;\n"
    " -v = verbose\n"
    " -f = force rebuild\n"
    " -s = status check, don't rebuild targets\n";
    bool force_rebuild = false;
    for( int i=1; i<argc; i++ )
    {
        std::string arg(argv[i]);
        if( arg == "-v" || arg=="-verbose" )
            verbose = true;
        else if( arg == "-f" || arg=="-force" )
            force_rebuild = true;
        else if( arg == "-s" || arg=="-status" )
            check_dependencies_only = true;
        else
        {
            printf( "%s", usage );
            return 0;
        }
    }
    if( check_dependencies_only && force_rebuild )
    {
        force_rebuild = false;
        printf( "-s option overrules -f option, -f option turned off\n" );
    }

    // Check pre-requisites etc.
    bool ok = probe();

    // If all ok go for it
    if( ok )
        treebuilder( force_rebuild, check_dependencies_only );
    return ok ? 0 : -1;
}


bool md_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild )
{
    bool needs_rebuild = !same_menu_as_last_run;
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->target;
    fs::path pin(in); 
    fs::path pout(out);
    if( force_rebuild  || !fs::exists(pout) )
        needs_rebuild = true;
    else
    {
        fs::file_time_type time_out = last_write_time(pout);
        if( !p->make_file_for_dir )
        {
            fs::file_time_type time_in = last_write_time(pin);    
            if( time_in > time_out )
            {
                printf( "Info: %s post-dates %s\n", in.c_str(), out.c_str() );
                needs_rebuild = true;
            }
        }
        if( md_template.file_time > time_out )
        {
            printf( "Info: %s post-dates %s\n", md_template.filename.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
    if( needs_rebuild )
    {
        if( check_dependencies_only )
            printf( "Info: Check dependencies only, %s will be rebuilt on normal run\n", out.c_str() );
        else
        {
            printf( "Info: Rebuilding %s\n", out.c_str() );
            std::map<char,std::string> macros;
            macros['s'] = p->heading;       // lower case 's' and 'z' mean auto-generated
            macros['z'] = p->subheading;
            if( p->make_file_for_dir )
                in = "";
            md_template.gen_html(in,out,macros,menu,menu_idx);
        }
    }
    return needs_rebuild;
}

bool pgn_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild )
{
    bool needs_rebuild = !same_menu_as_last_run;
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->target;
    std::string pgn_asset =  std::string("assets") + std::string(PATH_SEPARATOR_STR) + p->target;
    fs::path pin(in); 
    fs::path pout(out);
    size_t offset = pgn_asset.find_last_of('.');
    if( offset != std::string::npos )
        pgn_asset = pgn_asset.substr(0,offset) + ".pgn";
    std::string pgn_asset_full = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + pgn_asset;
    if( force_rebuild || !fs::exists(pout) )
        needs_rebuild = true;
    else
    {
        fs::file_time_type time_out = last_write_time(pout);
        fs::file_time_type time_in = last_write_time(pin);    
        if( time_in > time_out )
        {
            printf( "Info: %s post-dates %s\n", in.c_str(), out.c_str() );
            needs_rebuild = true;
        }
        if( pgn_template.file_time > time_out )
        {
            printf( "Info: %s post-dates %s\n", pgn_template.filename.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
    if( needs_rebuild )
    {
        if( check_dependencies_only )
            printf( "Info: Check dependencies only, %s will be rebuilt on normal run\n", out.c_str() );
        else
        {
            printf( "Info: Rebuilding %s\n", out.c_str() );
            std::map<char,std::string> macros;
            macros['T'] = p->heading;
            macros['S'] = p->heading;
            macros['Z'] = p->subheading;
            macros['G'] = pgn_asset;
            pgn_template.gen_html(in,out,pgn_asset_full,macros,menu,menu_idx);
        }
    }
    return needs_rebuild;
}

// Just copy the file - later check to see whether we need to copy it
bool html_gen( Page *p, bool force_rebuild )
{
    bool needs_rebuild = false;
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->target;
    fs::path pin(in); 
    fs::path pout(out);
    if( force_rebuild || !fs::exists(pout) )
        needs_rebuild = true;
    else
    {
        fs::file_time_type time_out = last_write_time(pout);
        fs::file_time_type time_in = last_write_time(pin);    
        if( time_in > time_out )
        {
            printf( "Info: %s succeeds %s\n", in.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
    if( needs_rebuild )
    {
        if( check_dependencies_only )
            printf( "Info: Check dependencies only, %s will be copied on normal run\n", out.c_str() );
        else
        {
            printf( "Info: Copying %s\n", out.c_str() );
            std::ifstream fin( in.c_str() );
            if( !fin )
            {
                printf( "Error: Could not open file %s for reading\n", in.c_str() );
                return false;
            }
            std::ofstream fout( out.c_str() );
            if( !fout )
            {
                printf( "Error: Could not open file %s for writing\n", out.c_str() );
                return false;
            }
            for(;;)
            {
                std::string line;
                if( !std::getline(fin,line) )
                    break;
                util::putline(fout,line);
            }
        }
    }
    return needs_rebuild;
}


//
// A much improved templating system, now using a real markdown processor, and recognising patterns in the
//  Markdown to implement our custom @solo, @grid and @panel features.
//

struct PICTURE
{
    std::string typ;
    std::string filename;
    std::string alt_text;
    std::string heading;
    std::string caption;
};

static void md_callback( const MD_CHAR* txt, MD_SIZE len, void *addr_std_string )
{
    std::string *ps = (std::string *)addr_std_string;
    std::string s(txt,len);
    *ps += s;
}

std::string macro_substitution( const std::string &input,
    const std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx )
{
    std::string out;
    size_t len = input.length();
    bool menu_mode = false;
    int line_count=0;
    std::string normal;
    std::string highlighted;
    for( size_t i=0; i<len; i++ )
    {
        char c =  input[i];
        if( !menu_mode )
        {
            if( c != '@' || i+1>=len)
                out += c;
            else
            {
                char key = input[++i];
                if( key == 'M' )
                {
                    menu_mode = true;
                    line_count = 0;
                    normal.clear();
                    highlighted.clear();
                }
                else
                {
                    auto it = macros.find(key);
                    if( it != macros.end() )
                    {
                        out += it->second;
                    }
                }
            }
        }
        else
        {
            if( c != '\n' )
            {
                if( line_count == 1 )
                    normal += c;
                else if( line_count == 2 )
                    highlighted += c;
            }
            else
            {
                line_count++;
                if( line_count == 3 )
                {
                    menu_mode=false;
                    for( unsigned int j=0; j<menu.size(); j++ )
                    {
                        std::string link  = menu[j].first;
                        std::string label = menu[j].second;
                        std::string t = normal;
                        if( j == menu_idx )
                            t = highlighted;
                        std::string text = t;
                        util::replace_all( text, "@1", link );
                        util::replace_all( text, "@2", label );
                        out += text;
                        out += "\n";
                    }
                }
            }
        }
    }
    return out;
}

std::string md( const std::string &in )
{
    std::string out;
    md_html( in.c_str(), in.length(),
        md_callback,
        &out,                           // userdata
        MD_FLAG_PERMISSIVEATXHEADERS,   // parser_flags, (allow "#Heading" as well as "# Heading")
        0                               // unsigned renderer_flags
    );
    return out;
}

bool recursive_file_copy( const std::string &src, const std::string &dst, bool root )
{
    bool ok = true;
    try {
        fs::create_directory(dst);
        for( const auto & entry : fs::directory_iterator(src) )
        {
            std::string in( entry.path().string() );
            if( is_directory(entry) )
            {
                std::string src_subdir = entry.path().string();
                std::string _subdir = src_subdir.substr(src.length());
                std::string dst_subdir = dst + _subdir;
                recursive_file_copy( src_subdir, dst_subdir, false );
            }
            else
            {
                std::string filename = entry.path().filename().string();
                std::string out = dst + std::string("/") + filename;
                fs::path pout(out);
                if( root && (filename=="template-md.txt" || filename=="template-pgn.txt") )
                    continue;
                bool copy=false;
                if( !fs::exists(pout) )
                {
                    copy = true;
                    printf( "Info: Copying %s to new file %s\n", in.c_str(), out.c_str() );
                }
                else
                {
                    fs::file_time_type time_in  = last_write_time(entry);    
                    fs::file_time_type time_out = last_write_time(pout);
                    if( time_in > time_out )
                    {
                        copy = true;
                        printf( "Info: Copying %s over %s because it post-dates it\n", in.c_str(), out.c_str() );
                    }
                }
                if( copy )
                {
                    ok = fs::copy_file( entry, pout, fs::copy_options::overwrite_existing );
                    if( !ok )
                    {
                        printf( "Error: Copying template file %s to %s failed\n", in.c_str(), out.c_str() );
                        break;
                    }
                }
            }
        }
    } catch( fs::filesystem_error& e ) {
        std::string err = " (";
        err += e.what();
        err += ")";
        printf( "Error: Copying template files %s to %s failed %s\n", src.c_str(), dst.c_str(), err.c_str() );
        ok = false;
    }
    return ok;
}

void MD_TEMPLATE::gen_html(  const std::string &in_file,
                             const std::string &html_out_file,
                             std::map<char,std::string> &macros,
                             const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx )
{
    bool have_input_file = false;
    std::ifstream fin;
    if( in_file != "" )
    {
        have_input_file = true;
        fin.open( in_file );
        if( !fin.is_open() )
        {
            printf( "Error: Could not open input file %s\n", in_file.c_str() );
            return;
        }
    }

    std::ofstream fout( html_out_file );
    if( !fout )
    {
        printf( "Error: Could not create output file %s\n", html_out_file.c_str() );
        return;
    }

    // Read macros from the input file
    std::string first_non_macro_line;
    bool first_non_macro_line_flag=false;
    bool md_only_no_extensions = true;
    while( have_input_file )
    {
        std::string s;
        if( !std::getline(fin,s) )
            break;
        util::rtrim(s);
        if( s.length()<3 || s[0]!='@' || s[2]!=' ' )
        {
            first_non_macro_line = s;
            first_non_macro_line_flag = true;
            break;
        }
        //md_only_no_extensions = false;
        std::string macro = s.substr(3);
        macros[s[1]] = macro;
    }

    // If no custom heading, subheading - use autogenerated ones
    if( macros.find('S')==macros.end() && macros.find('Z')==macros.end() )
    {
        macros['S'] = macros['s'];
        macros['Z'] = macros['z'];
    }

    // Write out the the html header
    std::string h = macro_substitution( header, macros, menu, menu_idx );
    util::putline(fout,h);
 
/*

@snippet = normal markdown, nothing special needed

##Heading

Text

(or)

##Heading
Text

@solo = heading immediately followed by image and multi-para text

##Heading
![ALT](image.lpg)Caption
Text

Text

Text

@grid = group of adjacent images

![ALT](image.lpg)Caption
![ALT](image.lpg)Caption
![ALT](image.lpg)Caption


@panel = heading starts with text (panel) and then Text

##(panel)Heading
Text


Line by line state machine

evaluate each line as
blank if empty
img if starts with ![
heading if starts with #
txt if starts with alphanum
something else otherwise

idle
    -> heading         (on heading)              idle->heading->heading_img->solo->solo_idle->solo->solo_idle->[idle/heading/panel/img/echo]
    or panel           (on (panel)heading)       idle->panel->panel_solo->panel_solo_idle->panel_solo->panel_solo_idle->[idle/heading/panel/img/echo]
    or grid            (on img)                  idle->grid[->grid]->idle
    or echo            (on not blank)
heading
    -> idle            (on blank)
    or heading_img     (on img)
    or echo            (on something else)
heading_img
    -> idle            (on blank)
    or solo            (on text)
    or echo            otherwise
solo
    -> solo            (on text)
    or solo_idle       (on blank)
    or echo            (on anything else)
panel
    -> idle            (on blank)
    or panel_solo      (on text)
    or echo            (on something else)
solo_idle
    -> solo            (on text)
    or idle            (on blank)
    or heading         (on heading)
    or panel           (on (panel)heading)
    or img             (on img)
    or echo            (on anything else)
panel_solo
    -> panel_solo      (on text)
    or panel_solo_idle (on blank)
    or echo            (on anything else)
panel_solo_idle
    -> panel_solo      (on text)
    or idle            (on blank)
    or heading         (on heading)
    or panel           (on (panel)heading)
    or img             (on img)
    or echo            (on anything else)
grid
    -> idle            (on blank)
    or grid            (on img)
    or echo            (on something else)
echo
    -> idle            (on blank)

    Generate grid/solo/panel
    idle->heading->heading_img->solo->solo_idle->solo->solo_idle->[idle/heading/panel/img/echo]
    idle->panel->panel_solo->panel_solo_idle->panel_solo->panel_solo_idle->[idle/heading/panel/img/echo]
    idle->grid[->grid]->idle

    Bail out
    idle->heading->idle
    idle->heading->heading_img->idle
    idle->panel->idle

    Accumulate until leave idle, immediately begin accumulating again
    Flush accumulation without writing it only on generation of grid/solo/panel

*/

    enum State {
        st_idle, st_echo,
        st_heading, st_heading_img,
        st_solo, st_solo_idle,
        st_panel, st_panel_solo, st_panel_solo_idle,
        st_grid
    };
    const char *states[] = {
        "st_idle", "st_echo",
        "st_heading", "st_heading_img",
        "st_solo", "st_solo_idle",
        "st_panel", "st_panel_solo", "st_panel_solo_idle",
        "st_grid"
    };
    State state=st_idle;
    enum Event {
        ev_blank, ev_txt, ev_heading, ev_panel_heading, ev_img, ev_other
    };
    const char *events[] = {
        "ev_blank", "ev_txt", "ev_heading", "ev_panel_heading", "ev_img", "ev_other"
    };
    std::string accum;
    std::string solo_txt;
    std::string alt;
    std::string imgfile;
    std::string caption;
    std::string header_txt;
    std::string solo_alt;
    std::string solo_imgfile;
    std::string solo_caption;
    std::string solo_header_txt;
    std::vector<PICTURE> pictures;
    int running=2;
    int flush_count=0;
    while( running > 0 )
    {
        // Get next line
        std::string s;
        if( running == 1 )
            running--;
        else
        {
            if( first_non_macro_line_flag )
            {
                s = first_non_macro_line;
                first_non_macro_line_flag = false;
            }
            else if( !std::getline(fin,s) )
            {
                s = "";
                running = 1;    // process the last line like a ev_blank followed by ev_other
            }
        }
        util::rtrim(s);

        // What kind of event?
        Event ev = ev_other;
        if( s.length() == 0 )
            ev = ev_blank;
        else if( s[0] == '#' )
        {
            ev = ev_heading;
            size_t idx =0;
            for( char c:s )
            {
                if( c != '#' )
                {
                    std::string t = s.substr(idx);
                    header_txt = t;
                    if( util::prefix(t,"(panel)") )
                    {
                        ev = ev_panel_heading;
                        header_txt = t.substr(strlen("(panel)"));
                    }
                    break;
                }
                idx++;
            }
        }
        else if( s.length()>=2 && s[0]=='!' && s[1]=='[' )
        {
            size_t offset = s.find("](",2);
            if( offset != std::string::npos && s.length() > offset+2 )
            {
                alt = s.substr(2,offset-2);
                offset += 2;
                size_t offset2 = s.find(')',offset);
                if( offset2 != std::string::npos )
                {
                    imgfile = s.substr(offset,offset2-offset);
                    caption = s.substr(offset2+1);
                    offset = imgfile.find_last_of('.');
                    if( offset != std::string::npos )
                    {
                        std::string ext = imgfile.substr(offset+1);
                        ext = util::tolower(ext);
                        if( ext=="jpg" || ext=="jpeg" || ext == "png" || ext=="gif" )
                            ev = ev_img;
                    }
                }
            }
        }
        else if( s[0] != ' ' )
            ev = ev_txt;
        if( running == 0 )
            ev = ev_other;

        // State machine
        State old_state = state;
        switch( state )
        {
            case st_idle:
            {
                switch( ev )
                {
                    case ev_heading:        state = st_heading;        break;
                    case ev_panel_heading:  state = st_panel;          break;
                    case ev_img:            state = st_grid;           break;  
                    case ev_blank:                                     break; 
                    default:                state = st_echo;           break;         
                }
                break;
            }
            case st_heading:
            {
                switch( ev )
                {
                    case ev_blank:          state = st_idle;           break;         
                    case ev_img:            state = st_heading_img;    break;  
                    default:                state = st_echo;           break;         
                }
                break;
            }
            case st_heading_img:
            {
                switch( ev )
                {
                    case ev_blank:          state = st_idle;           break; 
                    case ev_txt:            state = st_solo;           break; 
                    default:                state = st_echo;           break; 
                }
                break;
            }
            case st_panel:
            {
                switch( ev )
                {
                    case ev_blank:          state = st_idle;           break;         
                    case ev_txt:            state = st_panel_solo;     break;   
                    default:                state = st_echo;           break;         
                }
                break;
            }
            case st_solo:
            {
                switch( ev )
                {
                    case ev_txt:            state = st_solo;           break;        
                    case ev_blank:          state = st_solo_idle;      break;   
                    default:                state = st_echo;           break;        
                }
                break;
            }
            case st_solo_idle:
            {
                switch( ev )
                {
                    case ev_txt:            state = st_solo;           break;   
                    case ev_blank:          state = st_idle;           break;   
                    case ev_heading:        state = st_heading;        break;
                    case ev_panel_heading:  state = st_panel;          break;  
                    case ev_img:            state = st_grid;           break;    
                    default:                state = st_echo;           break;   
                }
                break;
            }
            case st_panel_solo:
            {
                switch( ev )
                {
                    case ev_txt:            state = st_panel_solo;     break;     
                    case ev_blank:          state = st_panel_solo_idle;break;
                    default:                state = st_echo;           break;           
                }
                break;
            }
            case st_panel_solo_idle:
            {
                switch( ev )
                {
                    case ev_txt:            state = st_panel_solo;     break;   
                    case ev_blank:          state = st_idle;           break;         
                    case ev_heading:        state = st_heading;        break;      
                    case ev_panel_heading:  state = st_panel;          break;        
                    case ev_img:            state = st_grid;           break;          
                    default:                state = st_echo;           break;         
                }
                break;
            }
            case st_grid:
            {
                switch( ev )
                {
                    case ev_blank:          state = st_idle;           break;
                    case ev_img:            state = st_grid;           break;
                    default:                state = st_echo;           break;
                }
                break;
            }
            case st_echo:
            {
                switch( ev )
                {
                    case ev_blank:          state = st_idle;           break;
                }
                break;
            }
        }

        // Accumulate until leave idle, immediately begin accumulating again
        //  Flush accumulation without writing it only on generation of grid/solo/panel
        bool change = (old_state!=state);
        if( change )
            cprintf( "%s: %s -> %s\n", events[ev], states[old_state], states[state] );
        if( change && ev!=ev_txt && (old_state==st_idle || old_state==st_echo) )
        {
            cprintf( "%d flushes\n", ++flush_count );
            std::string html = md(accum);
            cprintf( "***ACCUM in  ***\n%s\n", accum.c_str() );
            cprintf( "***ACCUM out ***\n%s\n", html.c_str() );
            accum.clear();
            util::puts(fout,html);
        }

        // Enter/continue Grid
        if( state == st_grid )
        {
            PICTURE picture;
            if( old_state == st_idle )
                pictures.clear();
            picture.alt_text = alt;
            picture.caption  = caption;
            picture.filename = imgfile;
            pictures.push_back(picture);
        }

        // Complete grid
        else if( state==st_idle && old_state==st_grid )
        {
            accum.clear(); //  Flush accumulation without writing it only on generation of grid/solo/panel

            // Loop through the pictures
            size_t len = pictures.size();
            bool in_grid=false;
            for( size_t i=0; i<len; i++ )
            {
                PICTURE *p = &pictures[i];
                PICTURE *q = NULL;
                PICTURE *r = NULL;
                std::string s = solo;
                bool macro_substitution_required = true;
                int grid_count = len-i;
                switch( grid_count )
                {
                    case 1:
                    {
                        s = in_grid? grid_1of3 : single;
                        break;
                    }
                    case 2:
                    {
                        s = in_grid? grid_2of3 : pair;
                        i++;
                        q = &pictures[i];
                        break;
                    }
                    default:
                    case 3:
                    {
                        s = triple;
                        i++;
                        q = &pictures[i];
                        i++;
                        r = &pictures[i];
                        break;
                    }
                }

                // Replace @F, @A, @C with filename, alt_text and caption repeatedly
                //  Use the first picture (i.e. p) for the first instance of each
                int filename_idx = 0;
                int alt_text_idx = 0;
                int caption_idx  = 0;
                int heading_idx  = 0;
                size_t next = 0;
                std::string template_txt = s;
                while( macro_substitution_required )
                {
                    size_t offset = s.find('@',next);
                    if( offset == std::string::npos )
                        break;
                    if( offset+1 >= s.length() )
                        break;
                    bool do_replace = true;
                    std::string replacement;
                    switch( s[offset+1] )
                    {
                        default:
                        {
                            printf( "Warning: Unexpected @ in template text: \n[\n%s\n]\n", template_txt.c_str() );
                            do_replace = false;
                            next = offset+2;
                            break;
                        }
                        case 'F':
                        {
                            replacement = filename_idx==0 ? p->filename : (filename_idx==1?q->filename:r->filename);
                            filename_idx++;
                            if( filename_idx==1 && q==NULL )
                                filename_idx--;
                            else if( filename_idx==2 && r==NULL )
                                filename_idx--;
                            if( filename_idx > 2 )
                                filename_idx = 2;
                            break;
                        }
                        case 'A':
                        {
                            replacement = alt_text_idx==0 ? p->alt_text : (alt_text_idx==1?q->alt_text:r->alt_text);
                            alt_text_idx++;
                            if( alt_text_idx==1 && q==NULL )
                                alt_text_idx--;
                            else if( alt_text_idx==2 && r==NULL )
                                alt_text_idx--;
                            if( alt_text_idx > 2 )
                                alt_text_idx = 2;
                            break;
                        }
                        case 'H':
                        {
                            replacement = heading_idx==0 ? p->heading : (heading_idx==1?q->heading:r->heading);
                            heading_idx++;
                            if( heading_idx==1 && q==NULL )
                                heading_idx--;
                            else if( heading_idx==2 && r==NULL )
                                heading_idx--;
                            if( heading_idx > 2 )
                                heading_idx = 2;
                            break;
                        }
                        case 'T':   // 'T'ext (or caption)
                        {
                            replacement = caption_idx==0 ? p->caption : (caption_idx==1?q->caption:r->caption);
                            replacement = md(replacement);
                            caption_idx++;
                            if( caption_idx==1 && q==NULL )
                                caption_idx--;
                            else if( caption_idx==2 && r==NULL )
                                caption_idx--;
                            if( caption_idx > 2 )
                                caption_idx = 2;
                            break;
                        }
                    }
                    if( do_replace )
                    {
                        next = offset + replacement.length();
                        std::string temp = s.substr(0,offset) +
                            replacement +
                            s.substr(offset+2);
                        s = temp;
                    }
                }
                util::putline(fout,s);
                in_grid = true;
            }
        }

        // Enter Solo
        if( change && state==st_solo && old_state==st_heading_img )
        {
            solo_alt        = alt;
            solo_imgfile    = imgfile;
            solo_caption    = caption;
            solo_header_txt = header_txt;
            solo_txt.clear();
        }

        // Complete Solo
        else if( change && state!=st_solo && old_state==st_solo_idle )
        {
            accum.clear();  // Flush accumulation without writing it only on generation of grid/solo/panel
            //printf( "solo: %s\n", solo_txt.c_str() );
            std::string s = solo;
            bool macro_substitution_required = true;

            // Replace @F, @A, @C, @T with filename, alt_text, caption and paragraph text
            int filename_idx = 0;
            int alt_text_idx = 0;
            int caption_idx  = 0;
            int heading_idx  = 0;
            size_t next = 0;
            while( macro_substitution_required )
            {
                size_t offset = s.find('@',next);
                if( offset == std::string::npos )
                    break;
                if( offset+1 >= s.length() )
                    break;
                bool do_replace = true;
                std::string replacement;
                switch( s[offset+1] )
                {
                    default:
                    {
                        printf( "Warning: Unexpected @ in template text: \n[\n%s\n]\n", solo.c_str() );
                        do_replace = false;
                        next = offset+2;
                        break;
                    }
                    case 'F':
                    {
                        replacement = solo_imgfile;
                        break;
                    }
                    case 'A':
                    {
                        replacement = solo_alt;
                        break;
                    }
                    case 'H':
                    {
                        replacement = solo_header_txt;
                        break;
                    }
                    case 'C':
                    {
                        replacement = md(solo_caption);
                        break;
                    }
                    case 'T':
                    {
                        replacement = md(solo_txt);
                        break;
                    }
                }
                if( do_replace )
                {
                    next = offset + replacement.length();
                    std::string temp = s.substr(0,offset) +
                        replacement +
                        s.substr(offset+2);
                    s = temp;
                }
            }
            util::putline(fout,s);
            solo_txt.clear();
        }

        // Enter Panel
        if( change && state==st_panel_solo && old_state==st_panel )
        {
            solo_header_txt = header_txt;
            solo_txt.clear();
        }

        // Leave Panel
        else if( change && state!=st_panel_solo && old_state==st_panel_solo_idle )
        {
            accum.clear();  // Flush accumulation without writing it only on generation of grid/solo/panel
            //printf( "panel: %s\n", accum.c_str() );
            std::string s = panel;
            bool macro_substitution_required = true;

            // Replace @T with text
            size_t next = 0;
            while( macro_substitution_required )
            {
                size_t offset = s.find('@',next);
                if( offset == std::string::npos )
                    break;
                if( offset+1 >= s.length() )
                    break;
                bool do_replace = true;
                std::string replacement;
                switch( s[offset+1] )
                {
                    default:
                    {
                        printf( "Warning: Unexpected @ in template text: \n[\n%s\n]\n", panel.c_str() );
                        do_replace = false;
                        next = offset+2;
                        break;
                    }
                    case 'T':
                    {
                        replacement = md(solo_txt);
                        break;
                    }
                    case 'H':
                    {
                        replacement = solo_header_txt;
                        break;
                    }
                }
                if( do_replace )
                {
                    next = offset + replacement.length();
                    std::string temp = s.substr(0,offset) +
                        replacement +
                        s.substr(offset+2);
                    s = temp;
                }
            }
            util::putline(fout,s);
            solo_txt.clear();
        }
        accum += s;
        accum += "\n";
        solo_txt += s;
        solo_txt += "\n";
    }
    std::string f = macro_substitution( footer, macros, menu, menu_idx );
    util::putline(fout,f);
}
