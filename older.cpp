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

void template_older( FILE *fin1, FILE *fin2, FILE *fout );

/*
int main( int argc, char *argv[] )
{
    if( argc != 4 )
    {
        printf( "Generate html from custom template plus formatted input\nUsage: template input.txt template.txt output.html\n" );
        return -1;
    }
    FILE *fin1 = fopen( argv[1], "rt" );
    if( !fin1 )
    {
        printf( "File not found: %s\n", argv[1] );
        return -1;
    }
    FILE *fin2 = fopen( argv[2], "rt" );
    if( !fin2 )
    {
        fclose(fin1);
        printf( "File not found: %s\n", argv[2] );
        return -1;
    }
    FILE *fout = fopen( argv[3], "wt" );
    if( !fout )
    {
        fclose(fin2);
        fclose(fin1);
        printf( "Cannot open file: %s\n", argv[3] );
        return -1;
    }
    template_older(fin1,fin2,fout);
    fclose(fout);
    fclose(fin2);
    fclose(fin1);
    return 0;
}
  */
void rtrim( std::string &s );

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

    // Read the input file
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
 /* for( int i=0; i<pictures.size(); i++ )
    {
        printf( "Picture %d filename is [%s], alt is [%s]\n", i, pictures[i].filename.c_str(),  pictures[i].alt_text.c_str() );
        printf( "Caption is [%s]\n", pictures[i].caption.c_str() );
    }
    printf( "Header is %s", header.c_str() );
    printf( "Footer is %s", footer.c_str() );
    printf( "Last caption is [%s]", picture.caption.c_str() ); */

    // Write out the the html
    fprintf( fout, "%s", header.c_str() );

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
    fprintf( fout, "%s", footer.c_str() );
}

