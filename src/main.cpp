/*
 
    Winged Spider is intended to be a innovative (I hope), spectactularly easy to use static
    website builder. The idea is that Winged Spider will take as input content organised
    nicely in a well named directory hierarchy, and generate as output web ready content
    with the same structure. The directory and file names from the input are reflected
    directly in the menu structure that is presented to the web consumer for navigation
    through the web content.

    Current status: The program is now more than usable - in fact I am using the program in
    production for the NZCF website, but it's a bit MicKey Mouse as I am actually running
    it from within the GUI. I need to tighten down some aspects, introduce quiet/verbose
    flags etc. etc.

    The notes below are largely historic although the idea of converting my @snippet etc
    keywords to patterns of otherwise legal markdown is on-point and I hope to actually
    implement it soon. The idea of using the directory structure to define the menus is the
    key concept within Winged Spider and it is captured below;
 
     Simple templating program
      Inputs: a template file with header and footer, plus up to three
      other parts for expanding data that's gathered from a text file that is
      formatted as a list of photo plus text sections

      An early version of this program (maybe git init version) is
      C:\BillsUtilities\photos.exe, used to generate snippets of HTML in NZCF
      images subdirectories.

      Another crystalised version of this program (after git comment
      "add new @w macro = ..") is C:\BillsUtilities\bills-cms-proto.exe,
      used to make first usable archives section of NZCF website

      Ideas:
      Use explicit directives in template file

      Grouped photos;
      @pair
      @triple
      @quad
      @single

      Stories;
      @snippet - no photos
      @solo    - one photo
      @panel   - highlighted snippet

      Allow these in content but migrate to detecting them as combinations of
      normal markdown syntax;

      eg the following lines

      >
      ## Headline

      para
      <

      Are interpreted as a @snippet

      and these

      >
      ## Headline

      ![ALT](image-file)caption
      para
      <

      are interpreted as a solo

      maybe @panel = @snippet with #Headline instead of ##Headline

      Also migrate the directory structure reflects site structure idea to provide the @M menu markup currently
      manually added to start of content

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

//
// Helpers etc.
//

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


/* Perhaps return to this concept at a later date,
   for now accept that we have some copy-pasted coded
   without a smooth mechanism for adding extra extensions */
/*
struct Extension
{
    bool ready = false;
    bool just_copy = false;
    std::string ext;
    std::string filename;
    std::vector<std::string> file_contents;
};

static std::vector<Extension> extensions; */

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
    std::vector<std::string> prerequisites{BASE_IN,BASE_OUT,"template"};
    for( std::string s: prerequisites )
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
        return ok;
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

    // Set up each extension, HTML is simple copy to output
    /*
    Extension html;
    html.ready = true;
    html.just_copy = true;
    extensions.push_back(html);

    // Markdown is primary and compulsory
    Extension md;
    md.ext = "md";
    md.filename = "template/template-md.txt";
    if( !fs::exists(md.filename) )
    {
        printf( "Error: The markdown template file \"%s\" is not present\n", md.filename.c_str() );
        return false;
    }
    md.ready = true;
    extensions.push_back(md);

    // PGN is optional
    Extension pgn;
    pgn.ext = "pgn";
    pgn.filename = "template/template-pgn.txt";
    if( fs::exists(pgn.filename) )
        pgn.ready = true;
    extensions.push_back(pgn);

    // .md2 is temporary dev only
    Extension md2;
    md2.ext = "md2";
    md2.filename = "template/template-md2.txt";
    if( fs::exists(md2.filename) )
        md2.ready = true;
    extensions.push_back(md2); */

    // Copy all template files, except .txt files, to output. The idea is that runtime
    //  files defined in the templates (eg javascript and css files) can be conveniently
    //  put in the template directory and will be copied (once) to the output directory
    std::string template_path("template");
    for( const auto & entry : fs::directory_iterator(template_path) )
    {
        std::string s( entry.path().string() );
        bool is_dir = is_directory(entry);
        bool is_file = !is_dir;
        if( is_file )
        {
            size_t offset = s.find_last_of('.');
            bool is_txt_file = (offset != std::string::npos  &&  util::tolower(s.substr(offset)) == ".txt");
            if( is_txt_file )
                continue;
            std::string out = BASE_OUT;
            out += "/";
            out += s.substr(strlen("template")+1);
            fs::path pout(out);
            bool copy=false;
            if( !fs::exists(pout) )
            {
                copy = true;
                printf( "Info: Copying %s to output directory\n", s.c_str() );
            }
            else
            {
                fs::file_time_type time_in  = last_write_time(entry);    
                fs::file_time_type time_out = last_write_time(pout);
                if( time_in > time_out )
                {
                    copy = true;
                    printf( "Info: Copying %s over %s because it post-dates it\n", s.c_str(), out.c_str() );
                }
            }
            if( copy )
            {
                bool ok2 = false;
                std::string err;
                try {
                    ok2 = fs::copy_file( entry, pout, fs::copy_options::overwrite_existing );
                } catch(fs::filesystem_error& e) {
                    ok = false;
                    err = " (";
                    err += e.what();
                    err += ")";
                }
                if( !ok2 )
                {
                    printf( "Error: Copying %s to %s failed%s\n", s.c_str(), out.c_str(), err.c_str() );
                    ok = false;
                }
            }
        }

/*
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
        fs::file_time_type time_template = last_write_time(ptemplate);
        if( time_template > time_out )
        {
            printf( "Info: %s post-dates %s\n", ftemplate.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
  */

    }
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
    " -f = force rebuild\n";
    bool force_rebuild = false;
    for( int i=1; i<argc; i++ )
    {
        std::string arg(argv[i]);
        if( arg == "-v" || arg=="-verbose" )
            verbose = true;
        else if( arg == "-f" || arg=="-force" )
            force_rebuild = true;
        else
        {
            printf( "%s", usage );
            return 0;
        }
    }

    // Check pre-requistites etc.
    bool ok = probe();

    // If all ok go for it
    if( ok )
    {
#if 0
        test_builds();
#else
        treebuilder( force_rebuild );
#endif
    }
    return ok ? 0 : -1;
}

//
// Original template system, now with real markdown support. Needs work
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

// Benched and replaced!
#if 0 

void template_bill_md( const std::string &md_file, const std::string &md2_file, const std::string &template_file, const std::string &html_out_file,
    std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx )
{
    //if( html_out_file == "output\\archives-tournaments-tournaments.html" )
    //    printf( "Debug\n" );
    bool have_input_file = false;
    std::ifstream fin1;
    if( md_file != "" )
    {
        have_input_file = true;
        fin1.open( md_file );
        if( !fin1.is_open() )
        {
            printf( "Error: Could not open input file %s\n", md_file.c_str() );
            return;
        }
    }

    std::ifstream fin2( template_file );
    if( !fin2 )
    {
        printf( "Error: Could not open template file %s\n", template_file.c_str() );
        return;
    }

    std::ofstream fout( html_out_file );
    if( !fout )
    {
        printf( "Error: Could not create output file %s\n", html_out_file.c_str() );
        return;
    }

    std::ofstream fout2( md2_file );
    if( !fout2 )
    {
        printf( "Error: Could not create output file %s\n", md2_file.c_str() );
        return;
    }

    std::string header;
    std::string footer;
    std::string single;
    std::string pair;
    std::string triple;
    std::string grid_2of2;
    std::string grid_1of2;
    std::string grid_1of1;
    std::string solo;
    std::string snippet;
    std::string panel;
    std::vector<PICTURE> pictures;

    // Read the template file
    bool in_section = false;
    enum { s_header, s_footer, s_single, s_pair, s_triple, s_2of2, s_1of2, s_1of1, s_snippet, s_solo, s_panel } section;
    for(;;)
    {
        std::string s;
        if( !std::getline(fin2,s) )
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
            else if( s == "@single" )
                section = s_single;
            else if( s == "@pair" )
                section = s_pair;
            else if( s == "@triple" )
                section = s_triple;
            else if( s == "@2of2" )
                section = s_2of2;
            else if( s == "@1of2" )
                section = s_1of2;
            else if( s == "@1of1" )
                section = s_1of1;
            else if( s == "@snippet" )
                section = s_snippet;
            else if( s == "@solo" )
                section = s_solo;
            else if( s == "@panel" )
                section = s_panel;
            else
            {
                if( s[0] == '@' )
                    printf( "Error: Template file has unknown section [%s] (not @header,\n@footer, @single, @pair, @triple, @2of2, @1of2, @1of1, @snippet, @solo or @panel)\n", s.c_str() );
                else
                    printf( "Error: Template file has section starting without a section identifier (eg @header,\n@footer, @single, @pair, @triple, @2of2, @1of2, @1of1, @snippet, @solo or @panel)\n" );
                return;
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
                    case s_single:
                        single += s;
                        single += '\n';
                        break;
                    case s_pair:
                        pair += s;
                        pair += '\n';
                        break;
                    case s_triple:
                        triple += s;
                        triple += '\n';
                        break;
                    case s_2of2:
                        grid_2of2 += s;
                        grid_2of2 += '\n';
                        break;
                    case s_1of2:
                        grid_1of2 += s;
                        grid_1of2 += '\n';
                        break;
                    case s_1of1:
                        grid_1of1 += s;
                        grid_1of1 += '\n';
                        break;
                    case s_footer:
                        footer += s;
                        footer += '\n';
                        break;
                    case s_solo:
                        solo += s;
                        solo += '\n';
                        break;
                    case s_snippet:
                        snippet += s;
                        snippet += '\n';
                        break;
                    case s_panel:
                        panel += s;
                        panel += '\n';
                        break;
                }
            }
        }
    }

    // Read macros from the input file
    std::string first_non_macro_line;
    bool first_non_macro_line_flag=false;
    bool md_only_no_extensions = true;
    while( have_input_file )
    {
        std::string s;
        if( !std::getline(fin1,s) )
            break;
        util::rtrim(s);
        if( s.length()<3 || s[0]!='@' || s[2]!=' ' )
        {
            first_non_macro_line = s;
            first_non_macro_line_flag = true;
            break;
        }
        if( s[1]=='S' || s[1]=='Z' )
            util::putline(fout2,s);
        //md_only_no_extensions = false;
        std::string macro = s.substr(3);
        macros[s[1]] = macro;
    }
    if( macros.find('S')!=macros.end() || macros.find('Z')!=macros.end() )
        util::putline(fout2,"");

    // If no custom heading, subheading - use autogenerated ones
    if( macros.find('S')==macros.end() && macros.find('Z')==macros.end() )
    {
        macros['S'] = macros['s'];
        macros['Z'] = macros['z'];
    }

    // Read paragraphs and pictures from the input file
    std::vector<std::string> whole_input_file;
    int state = 0;
    PICTURE picture;
    picture.typ.clear();
    picture.filename.clear();
    picture.alt_text.clear();
    picture.caption.clear();
    for(;;)
    {
        std::string s;
        if( first_non_macro_line_flag )
        {
            s = first_non_macro_line;
            first_non_macro_line_flag = false;
        }
        else if( !std::getline(fin1,s) )
            break;
        util::rtrim(s);
        whole_input_file.push_back(s);
        if( s.length() == 0 )
        {
            util::rtrim( picture.caption );
            if( picture.typ != "" )
                pictures.push_back(picture);
            state = 0;
            picture.typ.clear();
            picture.filename.clear();
            picture.alt_text.clear();
            picture.caption.clear();
            picture.heading.clear();
        }
        else
        {
            switch( state )
            {
                case 0:
                    if( s == "@grid" )
                    {
                        md_only_no_extensions = false;
                        picture.typ = s;    // stay in this state
                        state++;            // next is expect .jpg
                    }
                    else if( s == "@panel" || s=="@snippet" || s=="@naked" )
                    {
                        md_only_no_extensions = false;
                        picture.typ = s;
                        state+=3;           // next is heading
                    }
                    else if( s == "NULL" )
                    {
                        md_only_no_extensions = false;
                        picture.typ = "@snippet";
                        state+=2;            // next is ALT
                    }
                    else if( util::suffix(s,".jpg" ) || util::suffix(s,".png" ) || util::suffix(s,".gif" ) )
                    {
                        md_only_no_extensions = false;
                        picture.typ = "@solo";  // solo is default, unspoken
                        picture.filename = s;
                        state+=2;            // next is ALT
                    }
                    else
                    {
                        picture.typ = "@para";  // Not a picture at all, a paragraph of MD text
                        picture.caption += s;
                        picture.caption += '\n';
                        state = 4;
                    }
                    break;
                case 1:
                    picture.filename = s;
                    state++;            // next is ALT
                    break;
                case 2:
                    picture.alt_text = s;
                    state++;
                    break;
                case 3:
                    if( s.substr(0,2) == "@H" )
                        picture.heading = s.substr(2);
                    else
                    {
                        picture.caption += s;
                        picture.caption += '\n';
                    }
                    break;
                case 4:
                    picture.caption += s;
                    picture.caption += '\n';
                    break;
            }
        }
    }
    if( state >= 3 )
    {
        util::rtrim( picture.caption );
        pictures.push_back(picture);
    }
/* for( unsigned int i=0; i<pictures.size(); i++ )
    {
        printf( "Picture %d filename is [%s], alt is [%s]\n", i, pictures[i].filename.c_str(),  pictures[i].alt_text.c_str() );
        printf( "Caption is [%s]\n", pictures[i].caption.c_str() );
    }
    printf( "Header is %s", header.c_str() );
    printf( "Footer is %s", footer.c_str() );
    printf( "Last caption is [%s]", picture.caption.c_str() ); */

    // Write out the the html
    std::string h = macro_substitution( header, macros, menu, menu_idx );
    util::putline(fout,h);

    // Call external markdown processor if appropriate
    if( md_only_no_extensions )
    {
        std::string in;
        std::string out;
        for( std::string line : whole_input_file )
        {
            util::putline(fout2,line);
            in += line;
            in += "\n";
        }
        md_html( in.c_str(), in.length(),
            md_callback,
            &out,                           // userdata
            MD_FLAG_PERMISSIVEATXHEADERS,   // parser_flags, (allow "#Heading" as well as "# Heading")
            0                               // unsigned renderer_flags
        );
        util::puts(fout,out);
        whole_input_file.clear();
        pictures.clear();
    }

    // New feature - Macro @W in the header just indicates write out the whole input file between header and footer
    auto it = macros.find('W');
    if( it != macros.end() )
    {
        for( std::string line : whole_input_file )
            util::putline(fout,line);
        pictures.clear();
    }

    // Loop through the pictures (1)
    size_t len = pictures.size();
    PICTURE *p_previous = NULL;
    for( size_t i=0; i<len; i++ )
    {
        PICTURE *p = &pictures[i];
        if( p_previous && p_previous->typ=="@grid" && p->typ!="@grid" )
        {
            util::putline(fout2,"");
        }
        std::string txt = p->caption;
        util::replace_all( txt, "</p><p>\n", "\n");
        if( p->typ == "@solo" )
        {
            util::puts(fout2,"##");
            util::putline(fout2,p->heading);
            std::string filename = p->filename;
            util::replace_all( filename, " ", "%20");
            std::string t = util::sprintf("![%s](%s)\n%s\n", p->alt_text.c_str(), filename.c_str(), txt.c_str() );
            util::putline(fout2,t.c_str());
        }
        else if( p->typ == "@para" )
        {
            util::putline(fout2,txt);
            util::putline(fout2,"");
        }
        else if( p->typ == "@naked" )
        {
            util::putline(fout2,txt);
            util::putline(fout2,"");
        }
        else if( p->typ == "@panel" )
        {
            util::puts(fout2,"##(panel)");
            util::putline(fout2,p->heading);
            util::putline(fout2,txt);
            util::putline(fout2,"");

        }
        else if( p->typ == "@snippet" )
        {
            util::puts(fout2,"##");
            util::putline(fout2,p->heading);
            util::putline(fout2,txt);
            util::putline(fout2,"");
        }
        else if( p->typ == "@grid" )
        {
            std::string filename = p->filename;
            util::replace_all( filename, " ", "%20");
            std::string t = util::sprintf("![%s](%s)%s", p->alt_text.c_str(), filename.c_str(), p->caption.c_str() );
            util::putline(fout2,t.c_str());
        }
        p_previous = p;
    }

    // Loop through the pictures (2)
    bool in_grid=false;
    for( size_t i=0; i<len; i++ )
    {
        PICTURE *p = &pictures[i];
        PICTURE *q = NULL;
        PICTURE *r = NULL;
        std::string s = solo;
        bool macro_substitution_required = true;
        if( p->typ == "@para" )
        {
            in_grid = false;
            macro_substitution_required = false;
            size_t idx=0;
            while( idx<p->caption.length() && p->caption[idx]=='#' )
                idx++;
            if( idx>0 && idx<10 && idx<p->caption.length() )
            {
                s = util::sprintf( "<h%c>\n", '0'+idx );
                s += p->caption.substr(idx);
                s += util::sprintf( "</h%c>\n", '0'+idx );
            }
            else
            {
                s = "<p>\n";
                s += p->caption;
                s += "</p>\n";
            }
        }
        else if( p->typ == "@naked" )
        {
            in_grid = false;
            macro_substitution_required = false;
            s = p->caption;
        }
        else if( p->typ == "@panel" )
        {
            in_grid = false;
            s = panel;
        }
        else if( p->typ == "@snippet" )
        {
            in_grid = false;
            s = snippet;
        }
        else if( p->typ == "@grid" )
        {
            int grid_count = 1;

            // Look ahead to see if we have two or three @grid pictures in a row
            for( size_t j=i+1; j<len; j++  )
            {
                if( pictures[j].typ == "@grid" )
                    grid_count++;
                else
                    break;
            }
            switch( grid_count )
            {
                case 1:
                {
                    s = in_grid? single : grid_1of1;
                    break;
                }
                case 2:
                {
                    s = in_grid? pair : grid_1of2;
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
            in_grid = true;
        }
        else
        {
            in_grid = false;
        }

        // Replace @F, @A, @C with filename, alt_text and caption repeatedly
        //  Use the first picture (i.e. p) for the first instance of each
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
                    printf( "Warning: Unexpected @ in template text: \n[\n%s\n]\n", q==NULL? single.c_str() : pair.c_str() );
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
                case 'T':   // 'T'ext, an alternative to heading now that photos can be optional
                case 'C':
                {
                    replacement = caption_idx==0 ? p->caption : (caption_idx==1?q->caption:r->caption);
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
    }
    std::string f = macro_substitution( footer, macros, menu, menu_idx );
    util::putline(fout,f);
}

bool bill_markdown_gen( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild )
{
    bool needs_rebuild = !same_menu_as_last_run;
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->target;
    std::string ftemplate = "template/template-md.txt";
    std::string fmd2 = std::string("input3") + std::string(PATH_SEPARATOR_STR) + p->path;

    fs::path pin(in); 
    fs::path pout(out);
    fs::path ptemplate(ftemplate);
    if( !fs::exists(ptemplate) )
    {
        printf( "Error: Cannot open template file %s\n", ftemplate.c_str() );
        return false;
    }
    if( force_rebuild  || !fs::exists(pout) )
        needs_rebuild = true;
    else
    {
        fs::file_time_type time_out = last_write_time(pout);
        if( !p->make_file_for_dir )
        {
            bool exists = fs::exists(pin);
            fs::file_time_type time_in;
            if( exists )
                time_in = last_write_time(pin);    
            else
                needs_rebuild = true;
            if( exists && time_in > time_out )
            {
                printf( "Info: %s post-dates %s\n", in.c_str(), out.c_str() );
                needs_rebuild = true;
            }
        }
        fs::file_time_type time_template = last_write_time(ptemplate);
        if( time_template > time_out )
        {
            printf( "Info: %s post-dates %s\n", ftemplate.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
    if( needs_rebuild )
    {
        printf( "Info: Rebuilding %s\n", out.c_str() );
        std::map<char,std::string> macros;
        macros['s'] = p->heading;       // lower case 's' and 'z' mean auto-generated
        macros['z'] = p->subheading;
        if( p->make_file_for_dir )
            in = "";
        template_bill_md(in,fmd2,ftemplate,out,macros,menu,menu_idx);
    }
    return needs_rebuild;
}

#endif

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

static std::set<std::string> directories;

bool markdown_gen( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild )
{
    bool needs_rebuild = !same_menu_as_last_run;
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->target;
    std::string ftemplate = "template/template-md.txt";
    fs::path pin(in); 
    fs::path pout(out);
    fs::path ptemplate(ftemplate);
    if( !fs::exists(ptemplate) )
    {
        printf( "Error: Cannot open template file %s\n", ftemplate.c_str() );
        return false;
    }
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
        fs::file_time_type time_template = last_write_time(ptemplate);
        if( time_template > time_out )
        {
            printf( "Info: %s post-dates %s\n", ftemplate.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
    if( needs_rebuild )
    {
        printf( "Info: Rebuilding %s\n", out.c_str() );
        std::map<char,std::string> macros;
        macros['s'] = p->heading;       // lower case 's' and 'z' mean auto-generated
        macros['z'] = p->subheading;
        if( p->make_file_for_dir )
            in = "";
        template_md(in,ftemplate,out,macros,menu,menu_idx);
    }
    return needs_rebuild;
}

bool pgn_to_html( Page *p, const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx, bool same_menu_as_last_run, bool force_rebuild )
{
    bool needs_rebuild = !same_menu_as_last_run;
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out = std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->target;
    std::string pgn_asset =  std::string("assets") + std::string(PATH_SEPARATOR_STR) + p->target;
    std::string ftemplate = "template/template-pgn.txt";
    fs::path pin(in); 
    fs::path pout(out);
    fs::path ptemplate(ftemplate);
    if( !fs::exists(ptemplate) )
    {
        printf( "Error: Cannot open template file %s\n", ftemplate.c_str() );
        return false;
    }
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
        fs::file_time_type time_template = last_write_time(ptemplate);
        if( time_template > time_out )
        {
            printf( "Info: %s post-dates %s\n", ftemplate.c_str(), out.c_str() );
            needs_rebuild = true;
        }
    }
    if( needs_rebuild )
    {
        printf( "Info: Rebuilding %s\n", out.c_str() );
        GamesCache gc;
        std::map<char,std::string> macros;
        macros['T'] = p->heading;
        macros['S'] = p->heading;
        macros['Z'] = p->subheading;
        macros['G'] = pgn_asset;
        gc.Load(in,pgn_asset_full);
        gc.Publish(ftemplate,out,macros,menu,menu_idx);
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
    return needs_rebuild;
}

//
// Test builds
//

// While we are getting this up to speed with some basic reconstruction of some existing functionality
//  in this new context define one of these or the other
//#define BUILD_HOME
//#define BUILD_RESULTS
#define BUILD_MIGRATE
//#define BUILD_TOURNAMENT
//#define BUILD_PGN
//#define BUILD_TRY_MD4C

void test_builds()
{
#ifdef BUILD_PGN
    #define DEBUG_JUST_ONE_FILE
    std::string fin1 = "/Users/Bill/Documents/Github/winged-spider/base/Bulletins/October 2020/Magic in the Basement.pgn";
    std::string fin2 = "/Users/Bill/Documents/Github/winged-spider/template-pgn.txt";
    std::string fout = "/Users/Bill/Documents/Github/winged-spider/output/bulletins-october-2020-magic-in-the-basement.html";
#endif
#ifdef BUILD_HOME
    #define DEBUG_JUST_ONE_FILE
    std::string fin1 = "/Users/Bill/Documents/Github/winged-spider/base/Home.md";
    std::string fin2 = "/Users/Bill/Documents/Github/winged-spider/template-md.txt";
    std::string fout = "/Users/Bill/Documents/Github/winged-spider/output/index.html";
#endif
#ifdef BUILD_MIGRATE
    #define DEBUG_JUST_ONE_FILE
    std::string fin1 = "input2/Results.md3";
    std::string fin2 = "template/template-md.txt";
    std::string fout = "output2/Results2.html";
#endif
#ifdef BUILD_RESULTS
#define DEBUG_JUST_ONE_FILE
    std::string fin1 = "/Users/Bill/Documents/Github/winged-spider/base/Archives/Results/Results.md";
    std::string fin2 = "/Users/Bill/Documents/Github/winged-spider/template-older.txt";
    std::string fout = "/Users/Bill/Documents/Github/winged-spider/output/archives-results-results.html";
#endif
#ifdef BUILD_TOURNAMENT
#define DEBUG_JUST_ONE_FILE
    std::string fin1 = "/Users/Bill/Documents/Github/winged-spider/base/Archives/Tournaments/2020.md";
    std::string fin2 = "/Users/Bill/Documents/Github/winged-spider/template-older.txt";
    std::string fout = "/Users/Bill/Documents/Github/winged-spider/output/archives-tournaments-2020.html";
#endif
#ifdef BUILD_TRY_MD4C
    #define DEBUG_JUST_ONE_FILE
    std::string fin1 = "/Users/Bill/Documents/Github/winged-spider/base/Bulletins/February 2019/February 2019.md";
    std::string fin2 = "/Users/Bill/Documents/Github/winged-spider/template-md.txt";
    std::string fout = "/Users/Bill/Documents/Github/winged-spider/output/bulletins-february-2019-february-2019.html";
#endif
#ifdef DEBUG_JUST_ONE_FILE
    std::vector<std::pair<std::string,std::string>> menu;
    std::pair<std::string,std::string> menu_item1("index.html","Home");
    std::pair<std::string,std::string> menu_item2("archives-archives.html","Archives");
    std::pair<std::string,std::string> menu_item3("archives-tournaments.html","Tournaments");
    std::pair<std::string,std::string> menu_item4("archives-tournaments-2021.html","2021");
    std::pair<std::string,std::string> menu_item5("archives-tournaments-2021.html","2022");
    menu.push_back(menu_item1);
    menu.push_back(menu_item2);
    menu.push_back(menu_item3);
    menu.push_back(menu_item4);
    menu.push_back(menu_item5);
    std::map<char,std::string> macros;
    macros['S'] = "Heading";
    macros['Z'] = "Subheading";
    macros['T'] = "Testing 1, 2, 3";
    #ifdef BUILD_PGN
    std::pair<std::string,std::string> menu_item6("history-trusts-best-games.html","Trusts Best Games");
    menu.push_back(menu_item6);
    GamesCache gc;
    gc.Load(fin1);
    gc.Publish(fin2,fout,macros,menu,menu.size()-1);
    #else
void template_md( const std::string &md_file, const std::string &template_file, const std::string &html_out_file,
    std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx );
    template_md(fin1,fin2,fout,macros,menu,menu.size()-1);
    #endif
#endif
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

void template_md( const std::string &md_file, const std::string &template_file, const std::string &html_out_file,
    std::map<char,std::string> &macros,
    const std::vector<std::pair<std::string,std::string>> &menu, int menu_idx )
{
    //if( html_out_file == "output\\archives-tournaments-tournaments.html" )
    //    printf( "Debug\n" );
    bool have_input_file = false;
    std::ifstream fin1;
    if( md_file != "" )
    {
        have_input_file = true;
        fin1.open( md_file );
        if( !fin1.is_open() )
        {
            printf( "Error: Could not open input file %s\n", md_file.c_str() );
            return;
        }
    }

    std::ifstream fin2( template_file );
    if( !fin2 )
    {
        printf( "Error: Could not open template file %s\n", template_file.c_str() );
        return;
    }

    std::ofstream fout( html_out_file );
    if( !fout )
    {
        printf( "Error: Could not create output file %s\n", html_out_file.c_str() );
        return;
    }

    std::string header;
    std::string footer;
    std::string single;
    std::string pair;
    std::string triple;
    std::string grid_2of2;
    std::string grid_1of2;
    std::string grid_1of1;
    std::string solo;
    std::string snippet;
    std::string panel;

    // Read the template file
    bool in_section = false;
    enum { s_header, s_footer, s_single, s_pair, s_triple, s_2of2, s_1of2, s_1of1, s_snippet, s_solo, s_panel } section;
    for(;;)
    {
        std::string s;
        if( !std::getline(fin2,s) )
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
            else if( s == "@single" )
                section = s_single;
            else if( s == "@pair" )
                section = s_pair;
            else if( s == "@triple" )
                section = s_triple;
            else if( s == "@2of2" )
                section = s_2of2;
            else if( s == "@1of2" )
                section = s_1of2;
            else if( s == "@1of1" )
                section = s_1of1;
            else if( s == "@snippet" )
                section = s_snippet;
            else if( s == "@solo" )
                section = s_solo;
            else if( s == "@panel" )
                section = s_panel;
            else
            {
                if( s[0] == '@' )
                    printf( "Error: Template file has unknown section [%s] (not @header,\n@footer, @single, @pair, @triple, @2of2, @1of2, @1of1, @snippet, @solo or @panel)\n", s.c_str() );
                else
                    printf( "Error: Template file has section starting without a section identifier (eg @header,\n@footer, @single, @pair, @triple, @2of2, @1of2, @1of1, @snippet, @solo or @panel)\n" );
                return;
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
                    case s_single:
                        single += s;
                        single += '\n';
                        break;
                    case s_pair:
                        pair += s;
                        pair += '\n';
                        break;
                    case s_triple:
                        triple += s;
                        triple += '\n';
                        break;
                    case s_2of2:
                        grid_2of2 += s;
                        grid_2of2 += '\n';
                        break;
                    case s_1of2:
                        grid_1of2 += s;
                        grid_1of2 += '\n';
                        break;
                    case s_1of1:
                        grid_1of1 += s;
                        grid_1of1 += '\n';
                        break;
                    case s_footer:
                        footer += s;
                        footer += '\n';
                        break;
                    case s_solo:
                        solo += s;
                        solo += '\n';
                        break;
                    case s_snippet:
                        snippet += s;
                        snippet += '\n';
                        break;
                    case s_panel:
                        panel += s;
                        panel += '\n';
                        break;
                }
            }
        }
    }

    // Read macros from the input file
    std::string first_non_macro_line;
    bool first_non_macro_line_flag=false;
    bool md_only_no_extensions = true;
    while( have_input_file )
    {
        std::string s;
        if( !std::getline(fin1,s) )
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
            else if( !std::getline(fin1,s) )
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
        //if( change )
        //    printf( "%s: %s -> %s\n", events[ev], states[old_state], states[state] );
        if( change && ev!=ev_txt && (old_state==st_idle || old_state==st_echo) )
        {
            //printf( "%d flushes\n", ++flush_count );
            std::string html = md(accum);
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
                        s = in_grid? single : grid_1of1;
                        break;
                    }
                    case 2:
                    {
                        s = in_grid? pair : grid_1of2;
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

            // Replace @F, @A, @C with filename, alt_text and caption repeatedly
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
                    case 'T':   // 'T'ext, an alternative to heading now that photos can be optional
                    case 'C':
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

