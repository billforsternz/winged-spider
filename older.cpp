/*
  Simple templating program
  Inputs: a template file with header and footer, plus up to three
  other parts for expanding data that's gathered from a text file that is
  formatted as a list of photo plus text sections


*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "util.h"

void template_older( FILE *fin1, FILE *fin2, FILE *fout );

void rtrim( std::string &s );

static std::string macro_substitution_older( const std::string &input, 
    const std::map<char,std::string> &macros,
    const std::vector<std::string> &menu );

struct PICTURE
{
    std::string filename;
    std::string alt_text;
    std::string heading;
    std::string caption;
};

void template_older( FILE *fin1, FILE *fin2, FILE *fout )
{
    std::string header;
    std::string footer;
    std::string single;
    std::string pair;
    std::string triplet;
    std::vector<PICTURE> pictures;
    char buf[10000];

    // Read the template file
    const char *ret = fgets( buf, sizeof(buf)-2, fin2 );
    int state = 0;
    while( state<5 && ret != NULL )
    {
        std::string s(buf);
        rtrim(s);
        if( s.length() == 0 )
        {
            state++;
        }
        else if( s[0] != '#' )
        {
            switch( state )
            {
                case 0:
                    header += s;
                    header += '\n';
                    break;
                case 1:
                    single += s;
                    single += '\n';
                    break;
                case 2:
                    pair += s;
                    pair += '\n';
                    break;
                case 3:
                    triplet += s;
                    triplet += '\n';
                    break;
                case 4:
                    footer += s;
                    footer += '\n';
                    break;
            }
        }
        ret = fgets( buf, sizeof(buf)-2, fin2 );
    }

    if( state==2 || (state==3 && triplet=="") )
    {
        printf( "Info: Template file has three sections (header,single,footer)\n" );
        footer = pair;
        pair = "";
        triplet = "";
    }
    else if( state==3 || (state==4 && footer=="") )
    {
        printf( "Info: Template file has four sections (header,single,pair,footer)\n" );
        footer = triplet;
        triplet = "";
    }
    else if( state>=4 )
    {
        printf( "Info: Template file has five sections (header,single,pair,triplet,footer)\n" );
    }
    else
    {
        printf( "Error: Template file doesn't have three, four or five sections (header,single,[pair,triplet,]footer)\n" );
        return;
    }

    // Infer application from template file
    bool optional_single_photo_template =  triplet=="" && pair!=""  && (single.find("@F") == std::string::npos);

    // Read macros from the input file
    std::map<char,std::string> macros;
    std::vector<std::string> menu;
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
        if( s[1] == 'M' )
            menu.push_back(macro);
        else
            macros[s[1]] = macro;
        fgets( buf, sizeof(buf)-2, fin1 );
    }

    // Read pictures from the input file
    std::vector<std::string> whole_input_file;
    if( ret!=NULL && !have_line )
        ret = fgets( buf, sizeof(buf)-2, fin1 );
    state = 0;
    PICTURE picture;
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
                    picture.filename = s;
                    state++;
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
    for( int i=0; i<pictures.size(); i++ )
    {
        printf( "Picture %d filename is [%s], alt is [%s]\n", i, pictures[i].filename.c_str(),  pictures[i].alt_text.c_str() );
        printf( "Caption is [%s]\n", pictures[i].caption.c_str() );
    }
    printf( "Header is %s", header.c_str() );
    printf( "Footer is %s", footer.c_str() );
    printf( "Last caption is [%s]", picture.caption.c_str() );

    // Write out the the html
    std::string h = macro_substitution_older( header, macros, menu );
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
    int len = pictures.size();
    for( int i=0; i<len; i++ )
    {
        PICTURE *p = &pictures[i];
        PICTURE *q = NULL;
        PICTURE *r = NULL;
        std::string s = single;

        // New application, template is either single = text, or pair = photo plus text
        if( optional_single_photo_template )
        {
            if( p->filename != "NULL" )
                s = pair;
        }

        // Else template is a triplet, pair or single picture
        else
        {
            if( triplet!="" && i+2<len )
            {
                s = triplet;
                i++;
                q = &pictures[i];
                i++;
                r = &pictures[i];
            }
            else if( pair!="" && i+1 < len )
            {
                s = pair;
                i++;
                q = &pictures[i];
            }
        }

        // Replace @F, @A, @C with filename, alt_text and caption repeatedly
        //  Use the first picture (i.e. p) for the first instance of each
        int filename_idx = 0;
        int alt_text_idx = 0;
        int caption_idx  = 0;
        int heading_idx  = 0;
	    size_t next = 0;
	    for(;;)
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
                    if( !optional_single_photo_template )
                        replacement = "images/2019/" + replacement;
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
    std::string f = macro_substitution_older( footer, macros, menu );
    fprintf( fout, "%s", f.c_str() );
}

static std::string macro_substitution_older( const std::string &input, 
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
                            printf( "Menu item %s missing required .html termination of link part\n", s.c_str() );
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
