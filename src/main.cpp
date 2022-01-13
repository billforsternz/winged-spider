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
#include "defs.h"
#include "util.h"
#include "page.h"
#include "templates.h"
#include "md4c-html.h"

// Local Helpers and data
static bool startup_and_sanity_checks();
static bool recursive_file_copy( const std::string &src, const std::string &dst, bool root );
static void md_callback( const MD_CHAR* txt, MD_SIZE len, void *addr_std_string );
static bool check_dependencies_only;
static bool verbose;

// Template instances
static MD_TEMPLATE md_template;
static PGN_TEMPLATE pgn_template;

//
// main()
//

int main( int argc, char *argv[] )
{

    // Process command line arguments
    const char *usage =
    "Winged Spider V1.00\n"
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
    bool ok = startup_and_sanity_checks();

    // If all ok go for it
    if( ok )
        traversal( force_rebuild, check_dependencies_only );
    return ok ? 0 : -1;
}

//
//  Startup
//

static bool startup_and_sanity_checks()
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
//  Interface to templating system, convert [md or pgn] -> html (if needed)
//

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

// Also html -> html which is just copy (if needed)
bool html_to_html( Page *p, bool force_rebuild )
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
            printf( "Info: Copying %s to %s\n", in.c_str(), out.c_str() );
            bool ok = fs::copy_file( pin, pout, fs::copy_options::overwrite_existing );
            if( !ok )
                printf( "Error: Copying %s to %s failed\n", in.c_str(), out.c_str() );
        }
    }
    return needs_rebuild;
}

//
// Helpers
//

// Markdown in -> html out. A convenient C++ interface function for MD4C 3rd party C library 
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

static void md_callback( const MD_CHAR* txt, MD_SIZE len, void *addr_std_string )
{
    std::string *ps = (std::string *)addr_std_string;
    std::string s(txt,len);
    *ps += s;
}

int get_verbosity() // later, might have more than two values, so int not bool
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

static bool recursive_file_copy( const std::string &src, const std::string &dst, bool root )
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
                    if( check_dependencies_only )
                        printf( "Info: Check dependencies only, %s will be copied to new file %s on normal run\n", in.c_str(), out.c_str() );
                    else
                    {
                        copy = true;
                        printf( "Info: Copying %s to new file %s\n", in.c_str(), out.c_str() );
                    }
                }
                else
                {
                    fs::file_time_type time_in  = last_write_time(entry);    
                    fs::file_time_type time_out = last_write_time(pout);
                    if( time_in > time_out )
                    {
                        if( check_dependencies_only )
                            printf( "Info: Check dependencies only, %s will be copied over %s because it post-dates it on normal run\n", in.c_str(), out.c_str() );
                        else
                        {
                            copy = true;
                            printf( "Info: Copying %s over %s because it post-dates it\n", in.c_str(), out.c_str() );
                        }
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
        if( check_dependencies_only )
            printf( "Error: Checking need to copy template files from %s to %s failed %s\n", src.c_str(), dst.c_str(), err.c_str() );
        else
            printf( "Error: Copying template files from %s to %s failed %s\n", src.c_str(), dst.c_str(), err.c_str() );
        ok = false;
    }
    return ok;
}

//
// Simple Macro substitution implementation, all macros are @ plus one ascii character
//   Special support for menu macros, which are @M plus a list of (@1,@2) pairs
//

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

