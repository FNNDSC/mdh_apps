/***************************************************************************
                          mdhSliceData.cpp  -  description
                             -------------------
    begin                : Wed May 31 2006
    copyright            : (C) 2006 by Rudolph Pienaar
    email                : rudolph@nmr.mgh.harvard.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//
// NAME
//
//      mdhSliceData.cpp
//
// SYNOPSIS
//
//	(see synopsis_show function).
//
// DESC
//
//      `mdhSliceData' searches for a given echo on a given channel
//	and reports the index data:
//
//		- slice index
//		- position vector
//
//	for a 2D multi-echo 'meas.out' file.
//
// NOTES / WARNINGS
//
//      Under active development! Behaviour subject to change!
//
// HISTORY
//
//      31 May 2006
//      o Development from mdhEdit.cpp
//

#define	CE		cout << endl

#include <string>
#include <iostream>
#include <cstring>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "mdh64.h"

#define MDH_ONLINE_MASK         (1<<3)
#define MDH_RTFEEDBACK_MASK     (1<<1)
#define MDH_ACQEND_MASK         (1)

string          G_SELF          = "mdhSliceData";
string          G_VERSION       = "$Id$";

string          Gstr_comargsInFile              = 
		"scanning command line arguments for input file";
string          Gstr_comargsInFileError         = 
		"I found no input file specifier.";
int             G_comargsInFileError            = 1;

string          Gstr_comargsOutFile             = 
		"scanning command line arguments for output file";
string          Gstr_comargsOutFileError        = 
		"I found no output file specifier.";
int             G_comargsOutFileError           = 2;

string          Gstr_inFileAccess               = 
		"accessing input file";
string          Gstr_inFileAccessError          = 
		"I was unable to open file";
int             G_inFileAccessError             = 3;

string          Gstr_outFileAccess              = 
		"accessing output file";
string          Gstr_outFileAccessError         = 
		"I was unable to open file";
int             G_outFileAccessError            = 4;

//
// Command line options understood by program
//
static struct option const longopts[] =
{
  {"in", 		required_argument, 	NULL, 'i'},
  {"echo", 		required_argument, 	NULL, 'e'},
  {"version",           no_argument,            NULL, 'v'},
  {NULL, 0, NULL, 0}
};

void
error_exit(
    string              str_action,
    string              str_errorMsg,
    int                 errorCode) {

    //
    // ARGS
    //  str_action      in              action that failed
    //  str_errorMsg    in              error message to dump to stderr
    //  errorCode       in              code to return to OS
    //
    // DESC
    //  Dumps a simple error message and then exits to syste with
    //  errorCode.
    //
    // HISTORY
    // 13 August 2003
    //  o Initial design and coding.
    //

    cerr << endl << G_SELF;
    cerr << endl << "\tDanger, Will Robinson!";
    cerr << endl << "\tWhile I was "    << str_action;
    cerr << endl << "\t"                << str_errorMsg;
    cerr << endl;
    cerr << endl << "\tExiting to system with code " << errorCode;
    cerr << endl;
    exit(errorCode);
}

void
version_show(void) {
    //
    // DESC
    //  Show version number and exits.
    //
    // HISTORY
    // 15 December 2003
    //  o Initial design and coding.
    //

    cout << endl << "\t\t" << G_VERSION << endl;
    exit(0);
}

void
synopsis_show(void) {
    //
    // DESC
    //  Show a simple synopsis of program usage.
    //
    // HISTORY
    // 13 August 2003
    //  o Initial design and coding.
    //

    CE << "NAME";
    CE << "";
    CE << "\tmdhSliceData";
    CE << "\t\t" << G_VERSION;
    CE << "";
    CE << "SYNOPSIS";
    CE << "";
    CE << "\tmdhSliceData\t[--in=<\"meas.out\">]\t\\";
    CE << "\t\t\t[--echo=0]";
    CE << "";
    CE << "DESC";
    CE << "";
    CE << "\t`mdhSliceData' searches for given echo in a multi-echo 2D Siemens";
    CE << "\tADC raw data file. For the target echo, the slice index and actual";
    CE << "\tposition vector data is output.";
    CE;
    CE << "\tThe main purpose of this program is to compare the logical slice";
    CE << "\tordering with the actual anatomical position.";
    CE;
    CE << "\tIts behaviour is governed largely by the following command-line options:";
    CE << "";
    CE << "OPTIONS:";
    CE << "";
    CE << "\t--in=<inputFile>, -i <inputFile>";
    CE << "\tThis defines the input raw data file that is to be modified";
    CE << "\t(usually \"meas.out\")";
    CE << "";
    CE << "\t--echo=<echo>, -e <echo>";
    CE << "\tThe target echo index (default to zero).";
    CE << "";
    CE << "NOTES / WARNINGS";
    CE << "";
    CE << "\tUnder active development! Behaviour subject to change!";
    CE << "";
    CE << "PRECONDITIONS";
    CE << "";
    CE << "\to Input scanner ADC data file for a multi-slice multi-echo.";
    CE << "\t  sequence.";
    CE << "\to No sanity checking is done on the sequence!";
    CE << "";
    CE << "POSTCONDITIONS";
    CE << "";
    CE << "\to For the target echo, the slice index and position vector";
    CE << "\t  is output to stdout.";
    CE;

}

void
mdhSlice_parse(
        FILE*   pFILE_measin,
        int	a_echo) {
    //
    // ARGS
    //  pFILE_measin            in              FILE pointer to input data file
    //  a_echo		        in              FILE pointer to write to
    //
    // DESC
    //  Core function of program.
    //
    //  Finds a target echo and outputs its slice index and position vector
    //
    // HISTORY
    // 31 May 2006
    //  o Initial design and coding.
    //

    long int            meas_out_start_offset;
    sMDH                s_MDH;
    float               adc_data[4096];
    char*               pch_tmpbuffer;
    int                 navcounter                      = 0;

    int			indexEcho			= -1;
    int			indexLine			= -1;
    int			indexSlice			= 0;
    int			indexRepetition			= 0;
    int			indexChannel			= 0;

    fread(&meas_out_start_offset,sizeof(long int),1,pFILE_measin);

    pch_tmpbuffer = new char[meas_out_start_offset];
    fseek(pFILE_measin,0,SEEK_SET);
    fread( pch_tmpbuffer,sizeof(char),meas_out_start_offset,pFILE_measin);
    delete pch_tmpbuffer;

    // Read first MDH header
    fread(&s_MDH, sizeof(sMDH), 1, pFILE_measin);

    while (!feof(pFILE_measin)) {
	indexSlice	 	= 	s_MDH.sLC.ushSlice;
	indexLine		= 	s_MDH.sLC.ushLine;
        indexRepetition         =       s_MDH.sLC.ushRepetition;
        indexEcho               =       s_MDH.sLC.ushEcho;
	indexChannel		= 	s_MDH.ulChannelId;

	// Is this what we're looking for?
	if(indexEcho == a_echo) {
	    float	f_sag	= s_MDH.sSD.sSlicePosVec.flSag;
	    float	f_cor	= s_MDH.sSD.sSlicePosVec.flCor;
	    float	f_tra	= s_MDH.sSD.sSlicePosVec.flTra;

	    printf("%5i%5i%20.6f%20.6f%20.6f\n", indexSlice, indexLine,
						f_sag, f_cor, f_tra);

	}

	// Read in the line of actual ADC data
        fread(	adc_data, sizeof(float), 
		s_MDH.ushSamplesInScan*2, pFILE_measin);
	

	// And now read in MDH header for the next line of data
        fread(&s_MDH, sizeof(sMDH), 1, pFILE_measin);
    }
}

int
main(int argc, char* ppch_argv[])
{
    char*       pch_measin;
    FILE*       pFILE_measin;

    // Parse command line options
    int         option;
    int		verbosity;
    string	str_inFile	    	= "";
    int		echo			= 0;

    while(1) {
        int opt;
        int optionIndex = 0;
        opt = getopt_long(argc, ppch_argv, "", longopts, &optionIndex);
        if( opt == -1)
            break;

        switch(opt) {
            case 'i':
	        str_inFile.assign(optarg, strlen(optarg));
            break;
            case 'e':
		echo	= atoi(optarg);
            break;
            case 'v':
                version_show();
            break;
            case '?':
                synopsis_show();
                exit(1);
            break;
            default:
                cout << "?? getopt returned character code " << opt << endl;
        }
    }

    if(str_inFile == "")
        error_exit(Gstr_comargsInFile, Gstr_comargsInFileError, G_comargsInFileError);


    if (!(pFILE_measin=fopen(str_inFile.c_str(),"rb"))) {
        error_exit(Gstr_inFileAccess, Gstr_inFileAccessError, G_inFileAccessError);
    }


    mdhSlice_parse(pFILE_measin, echo);

    // Close files and free memory
    fclose(pFILE_measin);

    return 0;
}

