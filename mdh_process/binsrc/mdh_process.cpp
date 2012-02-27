/***************************************************************************
                          mdh_process.cpp  -  description
                             -------------------
    begin                : Wed Aug 13 2003
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
//      mdh_process.cpp
//
// SYNOPSIS
//
//	(see synopsis_show function).
//
// DESC
//
//      `mdh_process' is a multi-purpose post-processor for scanner collected
//	data files.
//
//
// NOTES / WARNINGS
//
//      Under active development! Behaviour subject to change!
//
// PRECONDITIONS
//
//      o Valid optionsFiles and implicitly valid meta-data.
//
// POSTCONDITIONS
//
//      o kSpace data is saved to an external file.
//
//
// HISTORY
//
//      June/July 2003
//      o Initial design and development
//
//      August 2003
//      o Deployment and dedicated testing.
//
//	October 2003
//	o Multiple format saving
//	o Mulitple operating modes defined.
//
//	November 2003
//	o Multiple channel data
//
//	June 2004
//	o Release 2
//
//	24 November 2004
//	o AMD64 build.
//


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>

#include <sys/times.h>
#include <unistd.h>
#include <C_scanopt.h>
#include <stdlib.h>

#include <getopt.h>

#include <ltl/marray.h>
#include <ltl/marray_io.h>

#include <complex>
#include <cmath>

#include "cmatrix.h"
#include "c_SMessage.h"

#include "asch.h"
#include "c_adcpack.h"
#include "c_adc.h"
#include "c_io.h"

//BEGIN: Added by Mohana R to accomodate Rec File Creation
#include "RecFile.h"
#include "StringUtils.h"
//END:: Addition by Mohana R

using namespace ltl;
using namespace std;
using namespace mdh;

typedef enum _savetype {
    e_native            = 0,
    e_mgh_realImag      = 10,
    e_mgh_magPhase      = 11,
    e_analyze75_snorm   = 20
} e_SAVETYPE;

//
// within an OO type framework, one might consider the "top level" program
//	that knits together all the objects pertaining to the program function
//	as a class in and of itself. Within this context, one might consider
//	top-level functions to be the analogue of class methods, and similarly
//      global variables can be considered to be "member
//	variables", visible to all functions at this level. Promoting several
//	variables to "global members" simplifies access to them from functions
//	within this global scope.
//
// Basically, this little rant is to justify using globals :-P - which makes
//	sense when used properly.
//
// Global variables are prefixed with a "G".
//

// Some global "member" variables
string                  G_SELF          = "";
string                  G_VERSION       = "$Id: mdh_process.cpp 3 2012-02-10 14:30:22Z rudolph $";
C_adcPack*              Gpc_measOut;			// pointer to measOut parsing
                                                        //	object. This object embodies
                                                        //      the machinery that does the
                                                        //      actual raw data unpacking.
C_SMessage*	        Gpcsm;				// output message handling object
                                                        //       used to dump syslog-type
                                                        //       messages to user spec'd
                                                        //       log files.
bool		        Gb_pause	    = false;	// pause flag - if true toggles
                                                        //     pausing between each major 
                                                        //     sub system.
bool		        Gb_syslogPrepend    = false;	// prepend with syslog stamp
bool                	Gb_is3D  	    = true;	// the acquisition dimensionality

CVol<GSL_complex_float>*
                        GpVl;		                // Debugging variable - pointer
                                                        //	access to volume

string	                Gstr_runID	    = "RDP";	// runID is global since it also
                                                        //	serves as a file save
                                                        //	prefix.
string			Gstr_inDir	    = "/tmp";	// Input dir containing raw data
                                                        //	sets and/or preprocessed
							//	saves
string			Gstr_outDir	    = "/tmp";	// Output dir containing recon'd
                                                        //	data

e_SAVETYPE		Ge_saveType	    = e_mgh_realImag;
                                                        // Type of output data to save

//BEGIN: Added by Mohana R to create Rec File
string		   	Gstr_recParamFile   = "";	// Rec Param file name 
							//	from which param 
							//	values are to be 
							//	included
string		   	Gstr_flipAngle      = "";
string		   	Gstr_recFileData    = "Parameter \t = \t Value \n";
//END:: Addition by Mohana R
							
							
// The following is an ugly but effective hack, and assuming the above
//	global variables, simplifies some operations
#define	COUT( msg )	Gpcsm->dump(Gb_syslogPrepend, ( msg ) );
#define	COUTnl( msg )	Gpcsm->dump(false, ( msg ) );
#define IFPAUSE( msg )	if(Gb_pause) {cout << ( msg ) << endl; cin >> ch;}

//
// Command line options understood by program
//
static struct option const longopts[] =
{
  {"outDir", 		required_argument, 	NULL, 'o'},
  {"inDir", 		required_argument, 	NULL, 'i'},
  {"optionsFile",	required_argument, 	NULL, 'f'},
  {"verbosity", 	required_argument, 	NULL, 't'},
  {"log",               required_argument, 	NULL, 'l'},
  {"meas",		required_argument, 	NULL, 'm'},
  {"channelTarget",     required_argument, 	NULL, 'c'},
  {"echoTarget",        required_argument, 	NULL, 'e'},
  {"repetitionTarget",  required_argument, 	NULL, 'r'},
  {"runID",             required_argument, 	NULL, 'd'},
  //BEGIN:: Addition by Mohana R to accomodate Rec File
  {"recParamFile",    	required_argument,	NULL, 'R'},
  //END:: Addition by Mohana R
  {"syslogPrepend",     no_argument, 	        NULL, 'p'},
  {"preprocessSave",    no_argument, 	        NULL, 'S'},
  {"preprocessLoad",    no_argument, 	        NULL, 'L'},
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
    cout << endl << "\tmdh_process.cpp";
    cout << endl << "\t\t" << G_VERSION;
    cout << endl << "";
    cout << endl << "SYNOPSIS";
    cout << endl << "";
    cout << endl << "\tmdh_process [OPTIONS]";
    cout << endl << "";
    cout << endl << "DESC";
    cout << endl << "";
    cout << endl << "\t`mdh_process' unpacks and processes Siemens' scanner ADC raw data files.";
    cout << endl << "\tIn the default case scenario, it performs the inverse FFT on the raw";
    cout << endl << "\tfrequency data, saving the reconstruction a variety of formats.";
    cout << endl << "";
    cout << endl << "\tIts behaviour is governed largely by the following command-line options:";
    cout << endl << "";
    cout << endl << "OPTIONS:";
    cout << endl << "";
    cout << endl << "\t--inDir=<inDir>, -i <inDir>";
    cout << endl << "\tThis defines the input directory that contains the original Siemens";
    cout << endl << "\traw data file <meas.out> and its text header <meas.asc>.";
    cout << endl << "";
    cout << endl << "\t--optionsFile=<optionsFile>, -f <optionsFile>";
    cout << endl << "\tThe optionsFile specifies a filename containing meta-data information describing";
    cout << endl << "\tvolumetric data and other miscellaneous run-time parameters.";
    cout << endl << "";
    cout << endl << "\t--meas=<measFile>, -m <measFile>";
    cout << endl << "\tThis specifies the Siemens meas.asc filename containing scan specific";
    cout << endl << "\theader data.";
    cout << endl << "";
    cout << endl << "\t--outDir=<outputDir>, -o <outputDir>";
    cout << endl << "\tThis defines the output directory to which the output image volumes will be written.";
    cout << endl << "\tThe type of volume image is specified in the optionsFile.";
    cout << endl << "";
    cout << endl << "\t--runID=<string>, -d <string>";
    cout << endl << "\tAn optional string can be defined that describes a current process run. This";
    cout << endl << "\tstring will be used as a prefix for any output files that are created. It is";
    cout << endl << "\tprobably a good idea to *not* include spaces in this string.";
    cout << endl << "";
    cout << endl << "\t--verbosity=<level>, -v <level>";
    cout << endl << "\tThe verbosity option controls console output of debugging-type information. Anything";
    cout << endl << "\tAnything above zero will activate log-file type output as per the C_SMessage class"; 
    cout << endl << "\tdefinition.";
    cout << endl << "";
    cout << endl << "\t--log=<file>, -l <file>";
    cout << endl << "\tLogs any output to the specified file. Can use stdout and stderr as file names to";
    cout << endl << "\tstream to standard output or standard error.";
    cout << endl << "";
    cout << endl << "\t--channelTarget=<id>, -c <id>";
    cout << endl << "\tParses raw data file *only* for the specified channel ID. All other channel data";
    cout << endl << "\tis ignored";
    cout << endl << "";
    cout << endl << "\t--echoTarget=<num>, -e <num>";
    cout << endl << "\tParses raw data file *only* for the specified echo.";
    cout << endl << "\tThis parameter *must* be used in conjunction with --repetitionTarget";
    cout << endl << "";
    cout << endl << "\t--repetitionTarget=<num>, -r <num>";
    cout << endl << "\tParses raw data file *only* for the specified repetition.";
    cout << endl << "\tThis parameter *must* be used in conjunction with --echoTarget";
    cout << endl << "";
    cout << endl << "\t--preprocessSave, -S";
    cout << endl << "\tForces unpack *only* (i.e. no reconstruction). Volumes are parsed from the raw data";
    cout << endl << "\tand packed into channel/echo/repetition images. Volumes are saved in --inDir";
    cout << endl << "\tin a binary format using the --runID string as a file prefix and ending in <mdh>.";
    cout << endl << "\tSubsequent runs of this program can use --preprocessLoad to load these unpacked";
    cout << endl << "\tvolumes for reconstruction. Note that loading preprocessed volumes is considerably";
    cout << endl << "\tfaster than parsing the original raw data file.";
    cout << endl << "";
    cout << endl << "\t--preprocessLoad, -L";
    cout << endl << "\tLoads volumes generated by an earlier --preprocessSave and reconstructs. These";
    cout << endl << "\tinput volumes are loaded from --inDir and saved to --outDir. The --runID specifies";
    cout << endl << "\tthe file prefix name to load and is also used to construct the output filenames.";
    cout << endl << "";
    cout << endl << "\t--syslogPrepend, -p";
    cout << endl << "\tPrepends output with syslog-style data/host stamps.";
    cout << endl << "";
    //BEGIN:: Addition by MOhana R to accomodate Rec File creation
    cout << endl << "\t--recParamFile, -R";
    cout << endl << "\t Param file using which information will be included into the Rec file.";
    cout << endl << "";
    //END:: Addition by Mohana R   
    cout << endl << "NOTES / WARNINGS";
    cout << endl << "";
    cout << endl << "Under active development! Behaviour subject to change!";
    cout << endl << "";
    cout << endl << "PRECONDITIONS";
    cout << endl << "";
    cout << endl << "o Valid optionsFiles and implicitly valid meta-data.";
    cout << endl << "";
    cout << endl << "POSTCONDITIONS";
    cout << endl << "";
    cout << endl << "o Siemens raw data file is processed in various ways.";
    cout << endl;
}

//BEGIN: Addition by Mohana R to accomodate Rec File creation
string 
RecFile_fileName_get(
	int		a_channelId,
    	int		a_echoIndex,
    	int		a_repetitionIndex,
    	int 		a_totalEchoes ) 
{
    //
    // ARGS
    //	
    //	a_channelId		in	channel being processed
    //	a_echoIndex		in	echo index in main loop
    //	a_repetitionIndex	in	repetition index in main loop
    //	a_totalEchoes		in	total echoes in scan
    //
    // DESC
    //	Creates a file name for RecFile purposes.
    //
    // HISTORY
    // 28 June 2004
    //	o Import and "cleanup".
    //
    
    stringstream sout("");
    sout << Gstr_runID;
    sout << Gstr_flipAngle ;
    sout << "_" << a_echoIndex  << "x" << a_totalEchoes;
    return sout.str();
}

void 
RecFile_flipAngle_set(
	C_scanopt&	acso_meas) 
{
    //
    // ARGS
    //	acso_meas	in		scanopt object to process
    //
    // DESC
    //	Parses the flip angle from the meas.asc scanopt object
    //	and sets a global string variable accordingly.
    //
    // HISTORY
    // 28 June 2004
    //	o Import and "cleanup".
    //
    
    acso_meas.scanFor("dFlipAngleDegree", &Gstr_flipAngle);
    if (Gstr_flipAngle=="")
	error_exit(	"reading value for Flip Angle",
			"I couldn't find a value for dFlipAngleDegree.",
			-1);
}

void  
RecFile_paramFileData_set(
	C_scanopt&	acso_meas) 
{
    //
    // ARGS
    //	acso_meas	in		scanopt object to process
    //
    // DESC
    //	Sets some additional global string data pertinent to RecFile
    //	
    // HISTORY
    // 28 June 2004
    //	o Import and "cleanup".
    //
    
    string 	param, str_value;
    string	recParamFileName = Gstr_recParamFile;


    ifstream recParamFile(recParamFileName.c_str());
    if (!recParamFile)
	error_exit(	"reading the Rec Param File",
			"I had access problems", 
			-1);

    // run through file to find the Rec Param
    while(recParamFile  >> param) {
	str_value = "";
	acso_meas.scanFor(param, &str_value);
	if (str_value=="")
	    error_exit(	"reading values for Rec Param File",
	    		"I did not find a match in options file", -1);
	Gstr_recFileData += param + "\t = \t" + str_value + "\n";
	if (param=="m_tFrameOfReference") {
	    stringstream dateTime("");
	    dateTime << StringUtils::getDateTime(str_value);
	    string date, time;
	    dateTime >> date >> time;
	    Gstr_recFileData += "Scan Date \t = \t" + date + "\n";
	    Gstr_recFileData += "Scan Time \t = \t" + time + "\n";
	  }
	}
}

void
RecFile_volumeSave(
	int	a_channelIO,
	int	a_echoIO,
	int	a_repetitionIO,
	int	a_totalEchoes,
	int	a_argc,
	char**	appch_argv
) {
    //
    // ARGS
    //	a_channelIO		in		current channel processed
    //	a_echoIO		in		current echo processed
    //	a_repetitionIO		in		current repetition processed
    //	a_totalEchoes		in		total number of scan echoes
    //	a_argc			in		number of args passed to prog
    //	appch_argv		in		arguments
    //
    // DESC
    //	This function creates the actual recfile itself.
    //
    // HISTORY
    // 30 June 2004
    //	o Incorporated from Mohana's code.
    //

    string 	str_rcsId = "TO DO RCS VERSIONING \n";	// RecFile information
    
    if (RecFile::getOptedFor() && !RecFile::isNull()) {
	string fName 	= RecFile_fileName_get(	a_channelIO, 	a_echoIO, 
						a_repetitionIO,	a_totalEchoes) 
				+ ".img";
	RecFile::getInstance()->setRecFileName(fName);
	RecFile::getInstance()->startrec(a_argc, appch_argv, str_rcsId);
	RecFile::getInstance()->append(Gstr_recFileData);
    }

    if (RecFile::getOptedFor() && !RecFile::isNull())
	RecFile::getInstance()->endrec ();  
}

//END: Addition by Mohana R to accomodate Rec File creation

void
volume_selectedValuesShow(
    string	astr_prefix )
{
    //
    // ARGS
    //	astr_prefix		in		prefix text to dump
    //
    // DESC
    //	A simple function that merely dumps some selected values from the volume.
    //	Useful to compare values generated by this code to similar values dumped
    //	by a MatLAB script.
    //
    // HISTORY
    // 22 October 2003
    //	o Initial design and coding.
    //
    
    cout 	<< astr_prefix                  << endl;
    cout 	<< GpVl->val(0, 0, 0)		<< endl;
    cout 	<< GpVl->val(1, 1, 1)		<< endl;
    cout 	<< GpVl->val(2, 2, 2)		<< endl;
    
}

void
volume_extract(
    int		a_echoTarget,
    int		a_repetitionTarget
    ) {
    //
    // ARGS
    //	a_echoTarget		in		target echo to extract
    //	a_repetitionTarget	in		target repetition to extract
    //
    // DESC
    //	Extracts a given echo/repetition from the raw data
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    sout << "\tExtracting vol values from memory:....\t";
    COUT(sout.str());	sout.str("");
    Gpc_measOut->dataMemory_volumeExtract(	a_repetitionTarget, 
    						a_echoTarget,
					     	e_normalKSpace);
    COUTnl("\t[OK]\n"); 
        
}

void
volume_zeroPad() {
    //
    // DESC
    //	Zero pads an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    sout << "\tzeroPad'ding vol values:...\t";
    COUT(sout.str());	sout.str("");
    Gpc_measOut->dataMemory_volumeZeroPad(     e_normalKSpace);
    COUTnl("\t\t[OK]\n");
}
    
void
volume_ifftshift() {
    //
    // DESC
    //	ifftshifts an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    //

    stringstream        sout("");
    char		ch;

    IFPAUSE( "Enter a char to continue" );
    sout << "\tifftshift'ing vol values:...\t";
    COUT(sout.str());	sout.str("");
    Gpc_measOut->dataMemory_volumeIfftShift(   e_normalKSpace);
    COUTnl("\t\t[OK]\n");
    
}

void
volume_fftshift() {
    //
    // DESC
    //	fftshifts an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    sout << "\tfftshift'ing vol values:...\t";
    COUT(sout.str());	sout.str("");
    Gpc_measOut->dataMemory_volumefftShift(    e_normalKSpace);
    COUTnl("\t\t[OK]\n");
}

void
volume_ifft() {
    //
    // DESC
    //	ifft's an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    //
    
    stringstream        sout("");
    char		ch;
    string		str_dim		= Gb_is3D ? "3D" : "2D";
    
    IFPAUSE( "Enter a char to continue" );
    sout << "\t" << str_dim << " ifft'ing vol values:...\t";
    COUT(sout.str());	sout.str("");
    Gpc_measOut->dataMemory_volumeifft(        e_normalKSpace);
    COUTnl("\t\t[OK]\n");
}

void
volume_extractSave(
    int		a_channelId,
    int         a_echoIndex,
    int         a_repetitionIndex 
) {
    //
    // ARGS
    //	a_channelId		in		current channel being processed
    //	a_echoIndex		in		current echo being processed
    //	a_repetitionIndex	in		current rep being processed
    //
    // DESC
    //	Saves an extracted volume. An extracted volume is merely a "raw" volume
    //	that has been filtered out of the raw data file. Saving it in a separate
    //	file provides the benefit of subsequent process runs to read these extracted
    //	files directly, without needing to re-parse the (possibly very large)
    //	raw data file.
    //
    // HISTORY
    // 22 October 2003
    //	o Initial design and coding.
    //
    // 06 November 2003
    //	o Multichannel.
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    COUT("\tsaving extracted volume:...\t\t");
    sout << Gstr_inDir << "/" << Gstr_runID;
    sout << "_extracted_channel" << a_channelId;
    sout << "_echo" << a_echoIndex  << "_rep" << a_repetitionIndex;
    sout << ".mdh"; 
    Gpc_measOut->dataMemory_volumeSave(sout.str());
    COUTnl("\t[OK]\n"); sout.str("");
}

void
volume_extractLoad(
    int		a_channelId,
    int         a_echoIndex, 
    int         a_repetitionIndex
) {
    //
    // ARGS
    //	a_channelId		in		current channel being processed
    //	a_echoIndex		in		current echo being processed
    //	a_repetitionIndex	in		current rep being processed
    //
    // DESC
    //	Loads an extracted volume. An extracted volume is merely a "raw" volume
    //	that has been filtered out of the raw data file. 
    //
    // HISTORY
    // 22 October 2003
    //	o Initial design and coding.
    //
    // 06 November 2003
    //	o Multichannel.
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    COUT("\tloading extracted volume:...\t\t");
    sout << Gstr_inDir << "/" << Gstr_runID;
    sout << "_extracted_channel" << a_channelId;
    sout << "_echo" << a_echoIndex  << "_rep" << a_repetitionIndex;
    sout << ".mdh"; 
    Gpc_measOut->dataMemory_volumeConstruct();
    Gpc_measOut->dataMemory_volumeLoad(sout.str());
    COUTnl("\t[OK]\n"); sout.str("");
}

void
volume_saveAnalyze75(
    int		a_channelId,
    int		a_echoIndex,
    int		a_repetitionIndex
) {
    //
    // ARGS
    //	a_channelId		in		current channel being processed
    //	a_echoIndex		in		current echo being processed
    //	a_repetitionIndex	in		current rep being processed
    //
    // DESC
    //	Saves an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    // 22 October 2003
    //	o Added repetition parameter
    //
    // 06 November 2003
    //	o Multichannel.
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    COUT("\tSaving (short norm) of extracted volume... ");
    sout << Gstr_outDir << "/" << Gstr_runID;
    sout << "_channel" << a_channelId;
    sout << "_echo" << a_echoIndex  << "_rep" << a_repetitionIndex;
    sout << "-snorm";
    Gpc_measOut->dataMemory_volumeSaveNorm(sout.str());
    COUTnl("\t[OK]\n"); sout.str("");
}

void
volume_saveMGH(
    int		a_channelId,
    int		a_echoIndex,
    int		a_repetitionIndex
) {
    //
    // ARGS
    //	a_channelId		in		current channel being processed
    //	a_echoIndex		in		current echo being processed
    //	a_repetitionIndex	in		current rep being processed
    //
    // DESC
    //	Saves an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    // 22 October 2003
    //	o Added repetition parameter
    //
    // 06 November 2003
    //	o Multichannel.
    //

    stringstream        sout("");
    char		ch;
    string              str_target;
    e_IOTYPE            e_iotype;

    IFPAUSE( "Enter a char to continue" );
    for(int i=0; i<2; i++) {
        switch(Ge_saveType) {
            case e_mgh_realImag:
                str_target      = !i ? "real" : "imag";
                e_iotype        = !i ? e_real : e_imaginary;
            break;
            case e_mgh_magPhase:
                str_target      = !i ? "mag" : "phase";
                e_iotype        = !i ? e_magnitude : e_phase;
            break;
        }

        COUT("\tSaving " + str_target + " component of extracted volume...");
        sout << Gstr_outDir << "/" << Gstr_runID;
        sout << "_channel" << a_channelId;
        sout << "_echo" << a_echoIndex  << "_rep" << a_repetitionIndex;
        sout << "-" << str_target << ".mgh";
        Gpc_measOut->dataMemory_volumeSave( sout.str(), e_iotype);
        COUTnl("\t[OK]\n"); sout.str("");
    }
}

void
volume_save(
    int		a_channelId,
    int		a_echoIndex,
    int		a_repetitionIndex
) {
    //
    // ARGS
    //	a_channelId		in		current channel being processed
    //	a_echoIndex		in		current echo being processed
    //	a_repetitionIndex	in		current rep being processed
    //
    // DESC
    //	Dispatching layer to saving an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    // 23 October 2003
    //	o Initial design and coding.
    //
    // 06 November 2003
    //	o Multichannel.
    //
    
    switch(Ge_saveType) {
        case e_mgh_magPhase:
	case e_mgh_realImag:
	    volume_saveMGH( a_channelId, a_echoIndex, a_repetitionIndex);
	    break;
	case e_analyze75_snorm:
	    volume_saveAnalyze75( a_channelId, a_echoIndex, a_repetitionIndex);
	    break;
    }
}

void
volume_destruct() {
    //
    // DESC
    //	Destructs an extracted volume.
    //
    // HISTORY
    // 16 October 2003
    //	o Initial design and coding.
    //
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    sout << "\tDestructing extraced volume:...\t";
    COUT(sout.str());    sout.str("");
    Gpc_measOut->dataMemory_volumeDestruct(            e_normalKSpace);
    COUTnl("\t\t[OK]\n"); sout.str("");
}

void
volume_preprocess(	
	int		echoIndex, 		
	int		echoTarget,
	int		repetitionIndex, 	
	int		repetitionTarget) {
    //
    // ARGS
    //	echoIndex		in		current echo loop
    //	echoTarget		in		target echo passed from system
    //	repetitionIndex		in		current repetition in loop	
    //	repetitionTarget	in		target repetition passed from
    //							system
    //
    // DESC
    //	After some earlier design attempts, it was decided to add this
    //	function as a mechanism to implement some IO-class specific
    //	ability *before* starting the main reconstruction functions.
    //
    // HISTORY
    // 24 June 2004
    //	o Initial design and coding.
    //
    
    stringstream        sout("");
    char		ch;
    
    IFPAUSE( "Enter a char to continue" );
    sout << "\tPreprocessing extracted volume:...\t";
    COUT(sout.str());    sout.str("");
    Gpc_measOut->dataMemory_volumePreprocess(	echoIndex,
    						echoTarget,
						repetitionIndex,
						repetitionTarget,
    						e_normalKSpace);
    COUTnl("\t[OK]\n"); sout.str("");
}


int
main(
    int     argc,
    char    *ppch_argv[]) {

    // ARGS
    //  argc        in          number of command line arguments
    //  ppch_argv   in          char array of command line arguments
    //
    // DESC
    //  Somewhat straightforward implementation of example usage of the
    //  c_adcpack object class.
    //
    // HISTORY
    //  June/July 2003
    //  o Initial design and development
    //
    //  August 2003
    //  o Deployment and dedicated testing.
    //
    // 24 June 2004
    //	o added volume "preprocess" step
    //

    G_SELF              = ppch_argv[0];
    stringstream        sout("");
    
    // Channel data
    //	The main program processes channel information
    int				allScanChannels = 0;	// Number of channels spec'd in
                                                        //	meta file
    const int			allPackChannels	= 1;	// adcPack will always contain
                                                        //	1 channel (never more)
    int				channelIndex	= 0;	// channel loop counter
    int				totalChannels	= 1;	// number of channels to loop
                                                        //	over. Set initially to
                                                        //	1 but is variable.
    int				echo, repetition; 	// dummy variable to hold current
    							//	echo and repetition
    // Some timing-related variables
    float                       f_totalTimeCPU  = 0.0;
    float                       f_totalTimeReal = 0.0;
    float			f_echoTimeCPU	= 0.0;
    float			f_echoTimeReal	= 0.0;
    struct tms                  st_start, st_stop;		// CPU time for total
    struct tms                  st_echoStart, st_echoStop;	// CPU time for echo
    time_t			tt_start, tt_stop;	        // Real time for total
    time_t			tt_echoStart, tt_echoStop;	// Real time for echo

    // Parse command line options
    int         option;
    int		verbosity;
    string      str_cfgFile         = "";
    string	str_measFile	    = "meas.asc";
    string	str_outDir	    = "/tmp";
    string	str_logFile	    = "stdout";
    int		echoTarget	    = -1;		// By default, parse all channels
    int		repetitionTarget    = -1;		//	echoes, and repetitions 
    int         channelTarget       = -1;               //      in data
    bool	b_preprocessSave    = false;
    bool	b_preprocessLoad    = false;
    int         ret		    = 0;		// program return value    
        
    while(1) {
        int opt;
        int optionIndex = 0;
        opt = getopt_long(argc, ppch_argv, "", longopts, &optionIndex);
        if( opt == -1)
            break;

        switch(opt) {
            case 'i':
	        Gstr_inDir.assign(optarg, strlen(optarg));
            break;
            case 'o':
	        Gstr_outDir.assign(optarg, strlen(optarg));
            break;
            case 'f':
	        str_cfgFile.assign(optarg, strlen(optarg));
            break;
            case 't':
	        verbosity = atoi(optarg);
            break;
            case 'l':
	        str_logFile.assign(optarg, strlen(optarg));
            break;
            case 'm':
	        str_measFile.assign(optarg, strlen(optarg));
            break;
            case 'c':
	        channelTarget = atoi(optarg);
            break;
            case 'e':
	        echoTarget = atoi(optarg);
            break;
            case 'r':
	        repetitionTarget = atoi(optarg);
            break;
            case 'd':
	        Gstr_runID.assign(optarg, strlen(optarg));
            break;
            case 'p':
	        Gb_syslogPrepend = true;
            break;
            case 'L':
	        b_preprocessLoad = true;
            break;
            case 'S':
	        b_preprocessSave = true;
            break;
	    //BEGIN: Added by Mohana R to accomodate Rec File creation
	    case 'R':
	        Gstr_recParamFile.assign(optarg, strlen(optarg));
            break;
	    //END: Addition by Mohana R
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

    if((echoTarget!=-1 && repetitionTarget==-1) || (echoTarget==-1 && repetitionTarget!=-1))
	error_exit("While checking command line options",
		   "You must specify *both* an echo and repetition target",
		   -1);
   
    //Gstr_inDir="/home/rudolph/proj/recon/data/test/";
    //Gstr_inDir="/home/rudolph/proj/recon/data/gleek/203610";
    //Gstr_inDir="/home/rudolph/proj/recon/data/multiChannelTest/2rep";
    //Gstr_inDir="/home/rudolph/proj/recon/data/Ashok/111150";
    //Gstr_inDir="/home/rudolph/proj/recon/data/gleek/Marianna/m4";
    //Gstr_inDir="/home/rudolph/proj/recon/data/multiChannelTest/2rep_float_AF";
    //Gstr_inDir="/home/rudolph/proj/recon/data/multiChannelTest/2rep_base";
    //Gstr_inDir="/space/kaos/1/users/rudolph/data/recon/gleek/pfizer/may15_04/140708";
    //str_cfgFile="measMetaData.asc";
    //Gb_syslogPrepend = true;
    //Gstr_runID = "multiTest2";
    //Gstr_outDir = Gstr_inDir+"/recon";
    //b_preprocessLoad = true;
    //b_preprocessSave = true;
    //echoTarget=0;
    //repetitionTarget=0;
    
    if(str_cfgFile == "")
	error_exit("parsing command line arguments", "I could not find metaData file spec!", 1);
    
    // Create the object that dumps status information
    Gpcsm	= new C_SMessage("",
			eSM_raw,	
			str_logFile,
			eSM_cpp);
    Gpcsm->str_syslogID_set(Gstr_runID);

    // and an options file parser
    C_scanopt                   cso_optionsFile(    Gstr_inDir+"/"+str_cfgFile,
						    e_EquLink);
						    
    // as well as a meas.asc parser for RecFile purposes
    C_scanopt*			pcso_measFile;
    if(Gstr_recParamFile.length())
    	pcso_measFile	= new C_scanopt(	Gstr_inDir+"/"+str_measFile,
						e_EquLink);

    // Now setup the size of the volumetric space by parsing the options metaData file,
    //	which will describe the dimension structure of the raw data. Note that the
    //	unpack object will use this information, and will also create its *own* 
    //	C_dimensionLists defining the actual data that it unpacked (which may be
    //	a subset of the larger data set).
    //
    // The size of memory allocated to the C_adcPack->C_adc objects depends on the 
    //	dimension structure of the problem space. If a "preprocessLoad" is true, there
    //	is no need to allocate memory to hold unpacked information since the original
    //	raw data file is never accessed. In such a case, unity dimension values are
    //	passed to the C_adcPack object, and the C_adc objects have minimum size. Volumes
    //	in such a case are read directly into the pVl_extracted variables from file.
    //
    //  The pCdim_disk describes the dimension structure as parsed from the metaData file.
    //	The pCdim_unity describes a "unity" dimension structure, used when not accessing
    //	the original raw data file.
    C_dimensionLists*	pCdim_disk	= new C_dimensionLists(Gstr_inDir+"/"+str_cfgFile);
    C_dimensionLists*	pCdim_unity	= new C_dimensionLists();
    C_dimensionLists*	pCdim;

    // By creating the dimensionLists structure with no arguments we force unity
    //	dimensions. We still need to set the optionsFileName, though.
    pCdim_unity->str_optionsFileName_set(Gstr_inDir+"/"+str_cfgFile);

    // Which dimension structure do we use?
    if(b_preprocessLoad)
	pCdim	= pCdim_unity;
    else
	pCdim	= pCdim_disk;

    // Parse optionsFile for any additional meta data...
    pCdim->metaData_parse();

    Gpcsm->timer(eSM_start);

    string		str_value   	= "";
    string		str_dim		= "3D";
    if(cso_optionsFile.scanFor("3DflagFile", &str_value)) {
    	CMatrix<int>	bM_3D((char*)str_value.c_str());
	Gb_is3D		= bM_3D.val(0, 0);
	if(!Gb_is3D)	str_dim		= "2D";
    }
    COUT("Acquisition dimensionality\t\t\t\t");
    COUTnl("[" + str_dim + "]\n");

    times(&st_start); time(&tt_start);
    COUT("allocating memory for unpacked data... ");

    // The core object that unpacks and stores raw data - remember that by spec'ing a
    //  non -1 value for the echo/repetitionTargets, the object will "home in" on
    //  only the spec'd echo/repetition pair.

    e_SAVETYPE		e_saveType;
    if(cso_optionsFile.scanFor("outputFormat", &str_value))
	Ge_saveType	= (e_SAVETYPE) atoi(str_value.c_str());
    else
	error_exit("parsing options file",
	           "could not find \"outputFormat\" spec in\n" + Gstr_inDir+"/"+str_cfgFile,
		   1);
    if(cso_optionsFile.scanFor("channels", &str_value))
	allScanChannels	= atoi(str_value.c_str());
    else
	error_exit("parsing options file",
	           "could not find \"channels\" spec in\n" + Gstr_inDir+"/"+str_cfgFile,
		   1);


    switch(Ge_saveType) {
	case e_mgh_realImag:
        case e_mgh_magPhase:
	    Gpc_measOut		= new C_adcPack_mgh(pCdim->str_ADCfileBaseName_get(),
                                    &(*pCdim),
                                    Gb_is3D,
				    echoTarget,
				    repetitionTarget);
	break;
	case e_analyze75_snorm:
	    Gpc_measOut		= new C_adcPack_analyze75(pCdim->str_ADCfileBaseName_get(),
                                    &(*pCdim),
                                    Gb_is3D,
				    echoTarget,
				    repetitionTarget);
	break;
        default:
	    error_exit(	"checking options file " + str_cfgFile, 
	    		"No valid output save types found", 1);
        break;
    }

    COUTnl("\t\t\t[OK]\n");

    //BEGIN: Added by Mohana R to accomodate Rec File cretaion
    if(Gstr_recParamFile != "") {
	RecFile::setOptedFor(true);
	RecFile::getInstance()->setOutDir(Gstr_outDir);
	RecFile_paramFileData_set(*pcso_measFile);
	RecFile_flipAngle_set(*pcso_measFile);
    }
    //END: Addition by Mohana R
   
    
    totalChannels	= allScanChannels;
    if(channelTarget!=-1)
	totalChannels	= 1;


    // Now we process the data. The outer loop is the channel id.
    for(channelIndex=0; channelIndex<totalChannels; channelIndex++) {
	Gpc_measOut->channelTarget_set(channelIndex);
	if(channelTarget!=-1)
	    Gpc_measOut->channelTarget_set(channelTarget);
        
	// Variables spec'ing the particular volume data to process
	e_KSPACEDATATYPE        e_kspace	= e_normalKSpace;
	int                     repetitionIndex = 0;
	int                     echoIndex       = 0;
    
	//	allScanEchoes are the echoes in the raw data
	//	allPackEchoes are the echoes in the unpacked C_adcPack object
	//	(likewise for the repetition counters).
	//	NOTE that echoes and repetitions are indexed starting from zero (0)
	int	                allScanEchoes   = pCdim_disk->M_echoList_get().cols_get();
	int			allPackEchoes	= Gpc_measOut->pc_dimension_get()->
						  M_echoList_get().cols_get();
	int	                allScanReps     = pCdim_disk->M_repetitionList_get().cols_get();
	int			allPackReps	= Gpc_measOut->pc_dimension_get()->
						  M_repetitionList_get().cols_get();
	int			sampleCount	= 0;
    
	times(&st_start); time(&tt_start);
	// Reading from disk / unpacking into memory
    
	if(!b_preprocessLoad) {
	    //
	    // If we are not to load preprocessed volumes, crunch through the
	    //	original raw data file. This is the default beahviour.
	    //
	    sout << "Channel " << (channelTarget==-1?channelIndex:channelTarget) << endl;
	    COUT(sout.str()); sout.str("");
	    COUT("Header processing... ");
	    Gpc_measOut->headerFile_process();         COUTnl("\t\t\t\t\t[OK]\n");
	    COUT("Data file processing... ");
	    sampleCount = Gpc_measOut->dataFile_process();
	    sout << "(" << sampleCount << " samples processed)"; COUTnl(sout.str()); sout.str("");
	    COUTnl("\t[OK]\n"); sout.str("");
	    times(&st_stop); time(&tt_stop);
	    f_totalTimeCPU  = difftime(st_stop.tms_utime, st_start.tms_utime) / 100;
	    f_totalTimeReal = difftime(tt_stop, tt_start);
	    sout << "\tTotal CPU time for raw data preprocessing: ";
	    COUT(sout.str());   sout.str("");
	    sout <<  f_totalTimeCPU << " seconds." << endl;
	    COUTnl(sout.str()); sout.str("");
	    sout << "\tTotal core time for raw data preprocessing: ";
	    COUT(sout.str());   sout.str("");
	    sout <<  f_totalTimeReal << " seconds." << endl << endl;
	    COUTnl(sout.str()); sout.str("");
	    
	    if(!sampleCount) {
		sout << "No samples were found in raw data file corresponding to:" << endl;
		COUT(sout.str()); sout.str("");
		sout << "\tChannel\t\t     ";
		sout << Gpc_measOut->channelTarget_get()                           << endl;
		COUT(sout.str()); sout.str("");
		sout << "\tEcho\t\t     ";
		sout << Gpc_measOut->echoTarget_get()                              << endl;
		COUT(sout.str()); sout.str("");
		sout << "\tRepetition\t     ";
		sout << Gpc_measOut->repetitionTarget_get()                        << endl;
		COUT(sout.str()); sout.str("");
		ret = 1;
		continue;
	    }
	}
	

	//
	// Quick analysis table
	//
	sout << "\tData analysis / run summary:-"	<< endl;
	COUT(sout.str()); sout.str("");
	sout << "\t\tRaw Data\t\tadcPack"	        << endl;
	COUT(sout.str()); sout.str("");
	sout << "Channels\t    "    << allScanChannels  << "\t\t\t    " << allPackChannels  << endl;
	COUT(sout.str()); sout.str("");
	sout << "Echoes\t\t    "    << allScanEchoes    << "\t\t\t    " << allPackEchoes    << endl;
	COUT(sout.str()); sout.str("");
	sout << "Repetitions\t    " << allScanReps	<< "\t\t\t    " << allPackReps	    << endl;
	COUT(sout.str()); sout.str("");
	sout << endl;
	COUT(sout.str()); sout.str("");
    
	//
	// The main processing loop indices depend on several factors:
	//	1. Size of internally unpacked objects (if read from raw data)
	//	2. Target echoes/repetitions if specified
	//	3. Loading of preprocessed data
	//
	// NB! Note that the order of the following 'if' processing is important!
    
	int totalReps	    = allScanReps;
	int totalEchoes     = allScanEchoes;
    	
	if(echoTarget!=-1 && repetitionTarget !=-1)	{
	    totalReps       = 1;
	    totalEchoes	    = 1;
	}	
	
	// 
	// Set of variables that identifies channels/echoes/repetitions for
	//	file I/O
	//
	int channelIO;
	int echoIO;
	int repetitionIO;
    
	// main processing loops: repetitions and echoes
	for(repetitionIndex=0; repetitionIndex<totalReps; repetitionIndex++) {
	    for(echoIndex=0; echoIndex<totalEchoes; echoIndex++) {
	    	if(echoTarget==-1)
		    echo = echoIndex;
		else 
		    echo = echoTarget;
	    	if(repetitionTarget==-1)
		    repetition = repetitionIndex;
		else 
		    repetition = repetitionTarget;
		times(&st_echoStart); time(&tt_echoStart);
		sout << "\tCurrent Processing Loop:"            << endl;
		COUT(sout.str()); sout.str("");
		sout << "\t\tRaw Data\t\tadcPack"	        << endl;
		COUT(sout.str()); sout.str("");
		sout << "Channel\t\t     ";
		sout << (channelTarget!=-1?channelTarget:channelIndex);
		sout << "\t\t\t    "<< allPackChannels     << endl;
		COUT(sout.str()); sout.str("");
		sout << "Echo\t\t     ";
		sout << (echoTarget!=-1?echoTarget:echoIndex);
		sout << "\t\t\t    " << allPackEchoes      << endl;
		COUT(sout.str()); sout.str("");
		sout << "Repetition\t     ";
		sout << (repetitionTarget!=-1?repetitionTarget:repetitionIndex);
		sout << "\t\t\t    " << allPackReps	   << endl;
		COUT(sout.str()); sout.str("");
		
		// Determine channel/echo/repetition IO
		if(channelTarget==-1 && echoTarget==-1 && repetitionTarget==-1) {
		    channelIO		= channelIndex; 
		    echoIO 		= echoIndex;
		    repetitionIO	= repetitionIndex;
		}
		if(channelTarget==-1 && echoTarget!=-1 && repetitionTarget!=-1) {
		    channelIO		= channelIndex;
		    echoIO		= echoTarget;
		    repetitionIO	= repetitionTarget;
		}
		if(channelTarget!=-1 && echoTarget==-1 && repetitionTarget==-1) {
		    channelIO		= channelTarget;
		    echoIO		= echoIndex;
		    repetitionIO	= repetitionIndex;
		}
		if(channelTarget!=-1 && echoTarget!=-1 && repetitionTarget!=-1) {
		    channelIO		= channelTarget;
		    echoIO		= echoTarget;
		    repetitionIO	= repetitionTarget;
		}
	    
		if(!b_preprocessLoad) {
		    // For the extraction from the C_adcPack object, remember
		    //	that this object has already parsed the target
		    //	echo and repetition (if spec'd) from the raw data
		    //	file. Also, the target channel has already been filtered.
		    volume_extract(echoIndex, repetitionIndex);
		}	
		else {
		    // In the case when we load a native volume from file, we
		    //	need to pass the correct target arguments if
		    //	spec'd (in order to properly create the file name).
		    volume_extractLoad(channelIO, echoIO, repetitionIO);
		}
		if(b_preprocessSave) {
		    // In the case when we save a native volume to file, we
		    //	need to pass the correct target arguments if
		    //	spec'd
		    volume_extractSave(channelIO, echoIO, repetitionIO);
		    // Processing thread continues with next loop if
		    //	we are saving extracted volumes
		} else {
		    //
		    // otherwise process the extracted volume and reconstruct
		    //
		
		    // The GpVl pointer is captured as a return from most functions
		    //	and is used in the volume_selectedValuesShow() function.
		
		    GpVl    = Gpc_measOut->dataMemory_volumeGet(	e_normalKSpace);
		    //volume_selectedValuesShow("Selected extract coords:");
	    
		    if(!Gpc_measOut->b_unpackWpadShift_get()) {
			// If this flag is false, the meas.out in memory has not
			//	already been zeroPadded and phase shifted.
			
			volume_zeroPad();
			GpVl 	= Gpc_measOut->dataMemory_volumeGet(	e_normalKSpace);
			//volume_selectedValuesShow("Selected zeroPadded coords:");
	    
			volume_ifftshift();	
			GpVl 	= Gpc_measOut->dataMemory_volumeGet(	e_normalKSpace);
			//volume_selectedValuesShow("Selected ifftshift coords:");
		    }
		    
		    volume_preprocess(	echoIndex, 		
		    			echoTarget,
		    			repetitionIndex, 	
					repetitionTarget);
		    
		    volume_ifft();
		    GpVl 	= Gpc_measOut->dataMemory_volumeGet(	e_normalKSpace);
		    //volume_selectedValuesShow("Selected ifft coords:");
	    
		    volume_fftshift();
		    GpVl 	= Gpc_measOut->dataMemory_volumeGet(	e_normalKSpace);
		    //volume_selectedValuesShow("Selected fftshift coords:");
	    
		    times(&st_echoStop); time(&tt_echoStop);
		    f_echoTimeCPU  = difftime(st_echoStop.tms_utime, st_echoStart.tms_utime) / 100;
		    f_echoTimeReal = difftime(tt_echoStop, tt_echoStart);
		    sout << "\t\tRecon CPU time for echo " << echo << " processing: ";
		    COUT(sout.str());	sout.str("");	
		    sout <<  f_echoTimeCPU << " seconds." << endl;
		    COUTnl(sout.str()); sout.str("");
		    sout << "\t\tRecon process time for echo " << echo << " processing: ";
		    COUT(sout.str());	sout.str("");
		    sout <<  f_echoTimeReal << " seconds." << endl;
		    COUTnl(sout.str()); sout.str("");
	 
		    if(Gstr_recParamFile.length()) 
		    	RecFile_volumeSave(	channelIO, 	echoIO, 
						repetitionIO, 	totalEchoes,
						argc,		ppch_argv);
		    volume_save(channelIO, echoIO, repetitionIO);

		}
		
		// Common "tail-end" operations
		
		volume_destruct();
	    
		times(&st_echoStop); time(&tt_echoStop);
		f_echoTimeCPU  = difftime(st_echoStop.tms_utime, st_echoStart.tms_utime) / 100;
		f_echoTimeReal = difftime(tt_echoStop, tt_echoStart);
		sout << "Total CPU time for echo " << echo << " processing: ";
		COUT(sout.str());   sout.str("");
		sout <<  f_echoTimeCPU << " seconds." << endl;
		COUTnl(sout.str()); sout.str("");
		sout << "Total process time for echo " << echo << " processing: ";
		COUT(sout.str());   sout.str("");
		sout <<  f_echoTimeReal << " seconds." << endl << endl;
		COUTnl(sout.str()); sout.str("");
	    }	
	}
    }
    times(&st_stop); time(&tt_stop);
    f_totalTimeCPU  = difftime(st_stop.tms_utime, st_start.tms_utime) / 100;
    f_totalTimeReal = difftime(tt_stop, tt_start);
    sout << "Total CPU time for data processing (including file I/O): ";
    COUT(sout.str()); sout.str("");
    sout <<  f_totalTimeCPU << " seconds." << endl;
    COUTnl(sout.str()); sout.str("");
    sout << "Total process time for all echo processing: ";
    COUT(sout.str()); sout.str("");
    sout <<  f_totalTimeReal << " seconds." << endl << endl;
    COUTnl(sout.str()); sout.str("");

/*    
    // Compare with same indices in MatLab script
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(0, 0, 0, 0, 0)   << endl;
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(1, 1, 1, 0, 0)   << endl;
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(2, 2, 2, 0, 0)   << endl;
*/    
    /*
    // Test for multi-repetition sequences...
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(0, 0, 0, 1, 0)   << endl;
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(1, 1, 1, 1, 0)   << endl;
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(2, 2, 2, 1, 0)   << endl;
    */
            
/*
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(50, 50, 50, 0, 0)   << endl;
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(100, 100, 100, 0, 0)   << endl;
    cout << c_measOut.pCadc_kSpace_get()->Az_data()(127, 191, 511, 0, 0)   << endl;
*/
    
    Gpcsm->timer(eSM_stop);

    
    char*   pch_audio   = "progcomp.sh 2>/dev/null >/dev/null";
    system(pch_audio);

    if(ret==0)
	COUT("Normal termination.\n");
    if(ret!=0)
	COUT("Some error has occured.\n");

    delete pCdim_disk;
    delete pCdim_unity;
    
    //delete Gpcsm;
    //delete Gpc_measOut;
    //delete pcso_measFile;

    //BEGIN: Addition by Mohana R
    if (RecFile::getOptedFor() && !RecFile::isNull())
	RecFile::Destroy();
    //END: Addition by Mohana R
    
    return ret;
}
