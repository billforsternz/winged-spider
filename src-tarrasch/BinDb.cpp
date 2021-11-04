/****************************************************************************
 * BinDb - A non-sql, compact chess database subsystem
 *  Author:  Bill Forster
 *  License: MIT license. Full text of license is in associated file LICENSE
 *  Copyright 2010-2016, Bill Forster <billforsternz at gmail dot com>
 ****************************************************************************/
#include <vector>
#include <set>
#include <map>
#include "BinDb.h"

//
//  This was severely chopped to get just a few pieces needed to let WingedSpider link
//

static uint32_t game_id_bottom = 1;	// reserve 0 as a special value
static uint32_t game_id_top    = GAME_ID_SENTINEL-1;

// Database game ids are allocated from the top
uint32_t GameIdAllocateTop( uint32_t count )
{
    if( game_id_top-count <= game_id_bottom )
    {
        game_id_bottom = 1;
        game_id_top    = GAME_ID_SENTINEL-1;
    }
    game_id_top -= count;
    return game_id_top;
}

// Other game ids are allocated from the bottom
uint32_t GameIdAllocateBottom(uint32_t count)
{
	if (game_id_bottom + count >= game_id_top)
	{
		game_id_bottom = 1;
		game_id_top = GAME_ID_SENTINEL - 1;
	}
	uint32_t temp = game_id_bottom;
	game_id_bottom += count;
	return temp;
}

// Use 19 bits with format yyyyyyyyyymmmmddddd
// y year, 10 bits, values are 0=unknown, 1-1000 are years 1501-2500 (so fixed offset of 1500), 1001-1023 are reserved
// m month, 4 bits, values are 0=unknown, 1=January..12=December, 13-15 reserved
// d day,   5 bits, values are 0=unknown, 1-31 = conventional date days
uint32_t Date2Bin( const char *date )
{
    uint32_t dat=0;
    uint32_t yyyy=0, mm=0, dd=0;
    int state=0;
    while( *date && state<3 )
    {
        char c = *date++;
        if( c == '?' )
            c = '0';
        bool is_digit = isascii(c) && isdigit(c);
        if( is_digit )
        {
            dat = dat*10;
            dat += (c-'0');
        }
        else
        {
            switch( state )
            {
                case 0:  state=1;   yyyy=dat;   dat=0;    break;
                case 1:  state=2;   mm=dat;     dat=0;    break;
                case 2:  state=3;   dd=dat;               break;
                default: break;
            }
        }
    }
    switch( state )
    {
        case 0:  yyyy=dat;   break;
        case 1:  mm=dat;     break;
        case 2:  dd=dat;     break;
        default:             break;
    }
    if( yyyy<1501 || 2500<yyyy )
        yyyy = 1500;
    if( mm<1 || 12<mm )
        mm = 0;
    if( dd<1 || 31<dd )
        dd = 0;    // future: possibly add validation for values 29,30, or 31
    return ((yyyy-1500)<<9) + (mm<<5) + dd;
}

void Bin2Date( uint32_t bin, std::string &date )
{
    char buf[80];
    int yyyy = (bin>>9) & 0x3ff;
    if( yyyy > 0 )
        yyyy += 1500;
    int mm   = (bin>>5) & 0x0f;
    int dd   = (bin)    & 0x1f;
    sprintf( buf, "%04d.%02d.%02d", yyyy, mm, dd );
    if( yyyy == 0 )
    {
        buf[0] = '?';
        buf[1] = '?';
        buf[2] = '?';
        buf[3] = '?';
    }
    if( mm == 0 )
    {
        buf[5] = '?';
        buf[6] = '?';
    }
    if( dd == 0 )
    {
        buf[8] = '?';
        buf[9] = '?';
    }
    date = buf;
}


// for now 16 bits -> rrrrrrbbbbbbbbbb   rr=round (0-63), bb=board(0-1023)
uint16_t Round2Bin( const char *round )
{
    if( !round )
        return 0;
    uint16_t rnd=0,brd=0,bin=0;
    int state=0;
    while( *round && state<2 )
    {
        char c = *round++;
        if( c == '?' )
            c = '0';
        bool is_digit = isascii(c) && isdigit(c);
        if( is_digit )
        {
            bin = (bin*10);
            bin += (c-'0');
        }
        else
        {
            switch( state )
            {
                case 0:  state=1; rnd=bin; bin=0; break;
                case 1:  state=2; brd=bin; bin=0; break;
                default: break;
            }
        }
    }
    switch( state )
    {
        case 0:  rnd = bin;  break;
        case 1:  brd = bin;  break;
        default: break;
    }
    if( rnd > 63 )
        rnd = 63;
    if( brd > 1023 )
        brd = 1023;
    bin = (rnd<<10) + brd;
    return bin;
}

void Bin2Round( uint32_t bin, std::string &round )
{
    char buf[80];
    int r = (bin>>10) & 0x3f;
    int b = (bin) & 0x3ff;
    if( b == 0 )
        sprintf( buf, "%d", r );
    else
        sprintf( buf, "%d.%d", r, b );
    if( r == 0 )
        buf[0] = '?';
    round = buf;
}

// For now 500 codes (9 bits) (A..E)(00..99), 0 (or A00) if unknown, 500 if empty
uint16_t Eco2Bin( const char *eco )
{
    if( !eco || !*eco )
        return 500; // 500 is empty
    uint16_t bin=0;
    if( 'A'<=eco[0] && eco[0]<='E' &&
        '0'<=eco[1] && eco[1]<='9' &&
        '0'<=eco[2] && eco[2]<='9'
      )
    {
        bin = (eco[0]-'A')*100 + (eco[1]-'0')*10 + (eco[2]-'0');
    }
    return bin;
}

void Bin2Eco( uint32_t bin, std::string &eco )
{
    char buf[4];
    if( bin>499 )
        eco = "";   // 500 is empty
    else
    {
        int hundreds = bin/100;         // eg bin = 473 -> 4
        int tens     = (bin%100)/10;    // eg bin = 473 -> 73 -> 7
        int ones     = (bin%10);        // eg bin = 473 -> 3
        buf[0] = 'A'+hundreds;
        buf[1] = '0'+tens;
        buf[2] = '0'+ones;
        buf[3] = '\0';
        eco = buf;
    }
}

// 4 codes (2 bits)
uint8_t Result2Bin( const char *result )
{
    if( !result )
        return 0;
    uint8_t bin=0;
    if( 0 == strcmp(result,"*") )
        bin = 0;
    else if( 0 == strcmp(result,"1-0") )
        bin = 1;
    else if( 0 == strcmp(result,"0-1") )
        bin = 2;
    else if( 0 == strcmp(result,"1/2-1/2") )
        bin = 3;
    return bin;
}

void Bin2Result( uint32_t bin, std::string &result )
{
    switch( bin )
    {
        default:
        case 0:     result = "*";          break;
        case 1:     result = "1-0";        break;
        case 2:     result = "0-1";        break;
        case 3:     result = "1/2-1/2";    break;
    }
}


// 12 bits (range 0..4095)
uint16_t Elo2Bin( const char *elo )
{
    if( !elo )
        return 0;
    uint16_t bin=0;
    bin = atoi(elo);
    if( bin > 4095 )
        bin = 4095;
    else if( bin < 0 )
        bin = 0;
    return bin;
}

void Bin2Elo( uint32_t bin, std::string &elo )
{
    char buf[80];
    if( bin > 4095 )
        bin = 4095;
    else if( bin < 0 )
        bin = 0;
	if( bin )
		sprintf( buf, "%d", bin );
	else
		buf[0] = '\0';
    elo = buf;
}

