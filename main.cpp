/*
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

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
namespace fs = std::experimental::filesystem;
#include "util.h"
#include "page.h"
#include "misc.h"

// While we are getting this up to speed with some basic reconstruction of some existing functionality
//  in this new context define one of these or the other
//#define BUILD_HOME
//#define BUILD_RESULTS

void templat( FILE *fin1, FILE *fin2, FILE *fout );
std::string macro_substitution( const std::string &input,
    const std::map<char,std::string> &macros,
    const std::vector<std::string> &menu );
void treebuilder();

int main( int argc, char *argv[] )
{
#ifdef BUILD_HOME
    #define DEBUG_JUST_ONE_FILE
    FILE *fin1 = fopen( "/Users/Bill/Documents/Github/winged-spider/base/Home.md", "rt" );
    FILE *fin2 = fopen( "/Users/Bill/Documents/Github/winged-spider/template-main.txt", "rt" );
    FILE *fout = fopen( "/Users/Bill/Documents/Github/winged-spider/output/index.html", "wt" );
#endif
#ifdef BUILD_RESULTS
#define DEBUG_JUST_ONE_FILE
    FILE *fin1 = fopen( "/Users/Bill/Documents/Github/winged-spider/base/Archives/Results/Results.md", "rt" );
    FILE *fin2 = fopen( "/Users/Bill/Documents/Github/winged-spider/template-older.txt", "rt" );
    FILE *fout = fopen( "/Users/Bill/Documents/Github/winged-spider/output/Archives/Results/Results.html", "wt" );
#endif
#ifdef DEBUG_JUST_ONE_FILE
    if( !fin1 || !fin2 || !fout )
    {
        printf( "File problem during debugging\n" );
        return -1;
    }
    templat(fin1,fin2,fout);
    fclose(fout);
    fclose(fin2);
    fclose(fin1);
#else
    treebuilder();
#endif
    return 0;
}

void rtrim( std::string &s )
{
    size_t final_char_offset = s.find_last_not_of(" \n\r\t");
    if( final_char_offset == std::string::npos )
        s.clear();
    else
        s.erase(final_char_offset+1);
}

struct PICTURE
{
    std::string typ;
    std::string filename;
    std::string alt_text;
    std::string heading;
    std::string caption;
};

void templat( FILE *fin1, FILE *fin2, FILE *fout, const std::vector<std::string> &menu )
{
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
    char buf[10000];

    // Read the template file
    const char *ret = fgets( buf, sizeof(buf)-2, fin2 );
    bool in_section = false;
    enum { s_header, s_footer, s_single, s_pair, s_triple, s_2of2, s_1of2, s_1of1, s_snippet, s_solo, s_panel } section;
    while( ret != NULL )
    {
        std::string s(buf);
        rtrim(s);
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
        ret = fgets( buf, sizeof(buf)-2, fin2 );
    }

    // Read macros from the input file
    std::map<char,std::string> macros;
    ret = fgets( buf, sizeof(buf)-2, fin1 );
    bool have_line=false;
    while( ret != NULL )
    {
        std::string s(buf);
        rtrim(s);
        have_line = s.length()>0;
        if( s.length()<3 || s[0]!='@' || s[2]!=' ' )
            break;
        std::string macro = s.substr(3);
        macros[s[1]] = macro;
        fgets( buf, sizeof(buf)-2, fin1 );
    }

    // Read pictures from the input file
    std::vector<std::string> whole_input_file;
    if( ret!=NULL && !have_line )
        ret = fgets( buf, sizeof(buf)-2, fin1 );
    int state = 0;
    PICTURE picture;
    picture.typ.clear();
    picture.filename.clear();
    picture.alt_text.clear();
    picture.caption.clear();
    while( ret != NULL )
    {
        std::string s(buf);
        rtrim(s);
        whole_input_file.push_back(s);
        if( s.length() == 0 )
        {
            rtrim( picture.caption );
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
                        picture.typ = s;    // stay in this state
                    }
                    else if( s == "@panel" || s=="@snippet" || s=="@naked" )
                    {
                        picture.typ = s;
                        state+=2;           // next is heading/caption
                    }
                    else if( s == "NULL" )
                    {
                        picture.typ = "@snippet";
                        state++;            // next is ALT
                    }
                    else
                    {
                        if( picture.typ == "" )     // solo is default, unspoken
                            picture.typ = "@solo";
                        picture.filename = s;
                        state++;            // next is ALT
                    }
                    break;
                case 1:
                    picture.alt_text = s;
                    state++;
                    break;
                case 2:
                    if( s.substr(0,2) == "@H" )
                        picture.heading = s.substr(2);
                    else
                    {
                        picture.caption += s;
                        picture.caption += '\n';
                    }
                    break;
            }
        }
        ret = fgets( buf, sizeof(buf)-2, fin1 );
    }
    if( state == 2 )
    {
        rtrim( picture.caption );
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
    std::string h = macro_substitution( header, macros, menu );
    fprintf( fout, "%s", h.c_str() );

    // New feature - Macro @W in the header just indicates write out the whole input file between header and footer
    auto it = macros.find('W');
    if( it != macros.end() )
    {
        for( std::string line : whole_input_file )
            fprintf( fout, "%s\n", line.c_str() );
        pictures.clear();
    }

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
        if( p->typ == "@naked" )
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
                    printf( "Unexpected @ in template text: \n[\n%s\n]\n", q==NULL? single.c_str() : pair.c_str() );
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
        fprintf( fout, "%s", s.c_str() );
    }
    std::string f = macro_substitution( footer, macros, menu );
    fprintf( fout, "%s", f.c_str() );
}

std::string macro_substitution( const std::string &input,
    const std::map<char,std::string> &macros,
    const std::vector<std::string> &menu )
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
                        std::string s = menu[j];
                        bool highlight = s.length()>0 && s[0]=='*';
                        std::string t = normal;
                        if( highlight )
                        {
                            s = s.substr(1);
                            t = highlighted;
                        }
                        size_t offset = s.find( ".html " );
                        if( offset == std::string::npos )
                        {
                            offset = s.find( "/ " );
                            if( offset != std::string::npos )
                                offset -= 4;    // align with .html offset
                            else
                            {
                                offset = s.find( ".pdf " );
                                if( offset != std::string::npos )
                                    offset -= 1;    // align with .html offset
                            }
                        }
                        if( offset == std::string::npos )
                            printf( "Menu item %s missing required .html (or /) termination of link part\n", s.c_str() );
                        else
                        {
                            std::string link  = s.substr( 0, offset+5 );
                            std::string label = s.substr( offset+6 );
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
    }
    return out;
}


static std::set<std::string> directories;

bool markdown_gen( Page *p, const std::vector<std::string> &menu )
{
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out_dir = (p->dir.length()==0 ? std::string(BASE_OUT) : std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->dir);
    std::string out = out_dir + PATH_SEPARATOR + p->base + ".html";
    auto it = directories.find(out_dir);
    bool found = (it!=directories.end());
    if( !found )
    {
        directories.insert(out_dir);
        if (!fs::exists(out_dir))
        {
            fs::create_directories(out_dir);
        }
    }

    FILE *fin1 = fopen( in.c_str(), "rt" );
    FILE *fin2 = fopen( "/Users/Bill/Documents/Github/winged-spider/template-main.txt", "rt" );
    FILE *fout = fopen( out.c_str(), "wt" );
    if( !fin1 || !fin2 || !fout )
    {
        printf( "File problem during debugging\n" );
        return false;
    }
    templat(fin1,fin2,fout,menu);
    fclose(fout);
    fclose(fin2);
    fclose(fin1);
    return true;
}

// Just copy the file - later check to see whether we need to copy it
bool html_gen( Page *p, const std::vector<std::string> &menu )
{
    std::string in  = std::string(BASE_IN) + std::string(PATH_SEPARATOR_STR) + p->path;
    std::string out_dir = (p->dir.length()==0 ? std::string(BASE_OUT) : std::string(BASE_OUT) + std::string(PATH_SEPARATOR_STR) + p->dir);
    std::string out = out_dir + PATH_SEPARATOR + p->base + ".html";
    auto it = directories.find(out_dir);
    bool found = it!=directories.end();
    if( !found )
    {
        directories.insert(out_dir);
        if (!fs::exists(out_dir))
        {
            fs::create_directories(out_dir);
        }
    }
    std::ifstream fin( in.c_str() );
    if( !fin )
    {
        printf( "Could not open file %s for reading\n", in.c_str() );
        return false;
    }
    std::ofstream fout( out.c_str() );
    if( !fout )
    {
        printf( "Could not open file %s for writing\n", out.c_str() );
        return false;
    }
    for(;;)
    {
        std::string line;
        if( !std::getline(fin,line) )
            break;
        util::putline(fout,line);
    }
    return true;
}
