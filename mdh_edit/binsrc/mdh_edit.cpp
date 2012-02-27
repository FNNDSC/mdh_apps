/***************************************************************************
                          mdhEdit.cpp  -  description
                             -------------------
    begin                : Mon Dec 22 2003
    copyright            : (C) 2003 by Rudolph Pienaar
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
//      mdh_edit.cpp
//
// SYNOPSIS
//
//	(see synopsis_show function).
//
// DESC
//
//      `mdhEdit' edits the MDH field information on raw binary Siemens
//      meas.out files. An input ADC file, usually "meas.out" is parsed
//      and the flag bits in the MDH file are changed. The output is
//      written to a command-line specified file.
//
//
// NOTES / WARNINGS
//
//      Under active development! Behaviour subject to change!
//
// HISTORY
//
//      22 December 2003
//      o Development from Andre's insertMDHonline code
//

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

string          G_SELF          = "mdhEdit";
string          G_VERSION       = "$Id$";

string          Gstr_comargsInFile              = "scanning command line arguments for input file";
string          Gstr_comargsInFileError         = "I found no input file specifier.";
int             G_comargsInFileError            = 1;

string          Gstr_comargsOutFile             = "scanning command line arguments for output file";
string          Gstr_comargsOutFileError        = "I found no output file specifier.";
int             G_comargsOutFileError           = 2;

string          Gstr_inFileAccess               = "accessing input file";
string          Gstr_inFileAccessError          = "I was unable to open file";
int             G_inFileAccessError             = 3;

string          Gstr_outFileAccess              = "accessing output file";
string          Gstr_outFileAccessError         = "I was unable to open file";
int             G_outFileAccessError            = 4;

//
// Command line options understood by program
//
static struct option const longopts[] =
{
  {"in", 		required_argument, 	NULL, 'i'},
  {"out", 		required_argument, 	NULL, 'o'},
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

    cout << endl << "NAME";
    cout << endl << "";
    cout << endl << "\tmdhEdit";
    cout << endl << "\t\t" << G_VERSION;
    cout << endl << "";
    cout << endl << "SYNOPSIS";
    cout << endl << "";
    cout << endl << "\tmdhEdit [OPTIONS]";
    cout << endl << "";
    cout << endl << "DESC";
    cout << endl << "";
    cout << endl << "\t`mdhEdit' edits the MDH field of Siemens ADC raw data files.";
    cout << endl << "\tIts behaviour is governed largely by the following command-line options:";
    cout << endl << "";
    cout << endl << "OPTIONS:";
    cout << endl << "";
    cout << endl << "\t--in=<inputFile>, -i <inputFile>";
    cout << endl << "\tThis defines the input raw data file that is to be modified (usually meas.out)";
    cout << endl << "";
    cout << endl << "\t--out=<outputFile>, -o <outputFile>";
    cout << endl << "\tThe output file to which the modified raw data stream will be written.";
    cout << endl << "";
    cout << endl << "NOTES / WARNINGS";
    cout << endl << "";
    cout << endl << "Under active development! Behaviour subject to change!";
    cout << endl << "";
    cout << endl << "PRECONDITIONS";
    cout << endl << "";
    cout << endl << "o Input scanner ADC data file.";
    cout << endl << "";
    cout << endl << "POSTCONDITIONS";
    cout << endl << "";
    cout << endl << "o Siemens raw data file is processed in various ways.";
    cout << endl;

}

void
mdhEdit(
        FILE*   pFILE_measin,
        FILE*   pFILE_measout) {
    //
    // ARGS
    //  pFILE_measin            in              FILE pointer to read from
    //  pFILE_measout           in              FILE pointer to write to
    //
    // DESC
    //  Core function of program.
    //
    //  Edits the MDH flags for all non zeroes to OFFLINE
    //
    // HISTORY
    // 22 December 2003
    //  o Initial design and coding.
    //

    long int            meas_out_start_offset;
    sMDH                mdh;
    float               adc_data[4096];
    char*               pch_tmpbuffer;
    int                 navcounter                      = 0;

    fread(&meas_out_start_offset,sizeof(long int),1,pFILE_measin);

    pch_tmpbuffer = new char[meas_out_start_offset];
    fseek(pFILE_measin,0,SEEK_SET);
    fread( pch_tmpbuffer,sizeof(char),meas_out_start_offset,pFILE_measin);
    fwrite(pch_tmpbuffer,sizeof(char),meas_out_start_offset,pFILE_measout);
    delete pch_tmpbuffer;

    fread(&mdh,sizeof(sMDH),1,pFILE_measin);
    while (!feof(pFILE_measin)) {
        fread(adc_data,sizeof(float),mdh.ushSamplesInScan*2,pFILE_measin);

        // Make any changes here!
        // Set mdh_online flag to true if mdh_acqend flag is false:
        //    if (!(mdh.aulEvalInfoMask[0] & MDH_ACQEND_MASK))
        //      mdh.aulEvalInfoMask[0]=mdh.aulEvalInfoMask[0] | MDH_ONLINE_MASK;

        // Make any changes here!
        // For non-zero echoes, set MDH_ONLINE_MASK off
        if ((!(mdh.aulEvalInfoMask[0] & MDH_ACQEND_MASK))&&(mdh.ushSamplesInScan==512)) {
            if(mdh.sLC.ushEcho!=0 && mdh.sLC.ushEcho!=7) {
                mdh.aulEvalInfoMask[0]=mdh.aulEvalInfoMask[0] & (0xffffffff - MDH_ONLINE_MASK);
            }
            navcounter++;
        }

        fwrite(&mdh,sizeof(sMDH),1,pFILE_measout);
        fwrite(adc_data,sizeof(float),mdh.ushSamplesInScan*2,pFILE_measout);

        fread(&mdh,sizeof(sMDH),1,pFILE_measin);
    }
}

int
main(int argc, char* ppch_argv[])
{
    char*       pch_measin;
    char*       pch_measout;
    FILE*       pFILE_measin;
    FILE*       pFILE_measout;

    // Parse command line options
    int         option;
    int		verbosity;
    string	str_outFile	    = "";
    string	str_inFile	    = "";

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
            case 'o':
	        str_outFile.assign(optarg, strlen(optarg));
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

    if(str_outFile == "")
        error_exit(Gstr_comargsOutFile, Gstr_comargsOutFileError, G_comargsOutFileError);

    if (!(pFILE_measin=fopen(str_inFile.c_str(),"rb"))) {
        error_exit(Gstr_inFileAccess, Gstr_inFileAccessError, G_inFileAccessError);
    }

    if (!(pFILE_measout=fopen(str_outFile.c_str(),"wb"))) {
        error_exit(Gstr_outFileAccess, Gstr_outFileAccessError, G_outFileAccessError);
    }

    mdhEdit(pFILE_measin, pFILE_measout);

    // Close files and free memory
    fclose(pFILE_measin);
    fclose(pFILE_measout);

    return 0;
}

