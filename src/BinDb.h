/****************************************************************************
 * BinDb - A non-sql, compact chess database subsystem
 *  Author:  Bill Forster
 *  License: MIT license. Full text of license is in associated file LICENSE
 *  Copyright 2010-2016, Bill Forster <billforsternz at gmail dot com>
 ****************************************************************************/
#ifndef BINDB_H
#define BINDB_H
#include <vector>
#include "BinaryConversions.h"
#include "ListableGame.h"

bool PgnStateMachine( FILE *pgn_file, int &typ, char *buf, int buflen );

void Pgn2Tdb( const char *infile, const char *outfile );
void Pgn2Tdb( FILE *fin, FILE *fout );
void Tdb2Pgn( const char *infile, const char *outfile );
void Tdb2Pgn( FILE *fin, FILE *fout );

int BitsRequired( int max );
void ReadStrings( FILE *fin, int nbr_strings, std::vector<std::string> &strings );

#endif  // BINDB_H
