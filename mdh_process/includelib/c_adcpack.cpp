/***************************************************************************
 *   Copyright (C) 2003 by Rudolph Pienaar                                 *
 *   rudolph@nmr.mgh.harvard.edu                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <fstream>
#include <iostream>
#include <string>

#include <c_adc.h>
#include <c_adcpack.h>
#include <scanopt.h>

#include "cmatrix.h"
#include "math_misc.h"

using namespace std;
using namespace mdh;

#define MDH_ACQEND_MASK (1)

//
//\\\***
// C_dimensionLists definitions ****>>>>
/////***
//

void
C_dimensionLists::debug_push(
        string                          astr_currentProc) {
    //
    // ARGS
    //  astr_currentProc        in      method name to
    //                                          "push" on the "stack"
    //
    // DESC
    //  This attempts to keep a simple record of methods that
    //  are called. Note that this "stack" is severely crippled in
    //  that it has no "memory" - names pushed on overwrite those
    //  currently there.
    //

    if(stackDepth_get() >= C_dimensionLists_STACKDEPTH-1)
        error(  "Out of str_proc stack depth");
    stackDepth_set(stackDepth_get()+1);
    str_proc_set(stackDepth_get(), astr_currentProc);
}

void
C_dimensionLists::debug_pop() {
    //
    // DESC
    //  "pop" the stack. Since the previous name has been
    //  overwritten, there is no restoration, per se. The
    //  only important parameter really is the stackDepth.
    //

    stackDepth_set(stackDepth_get()-1);
}

void
C_dimensionLists::error(
        string          astr_msg        /*= "Some error has occured"    */,
        int             code            /*= -1                          */)
{
    //
    // ARGS
    //  astr_class              in              name of the current class
    //  atr_msg                 in              message to dump to stderr
    //  code                    in              error code
    //
    // DESC
    //  Print error related information. This routine throws an exception
    //  to the class itself, allowing for coarse grained, but simple
    //  error flagging.
    //

    cerr << "\nFatal error encountered.\n";
    cerr << "\tC_dimensionLists object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc_get() << "\n";
    cerr << "\t" << astr_msg << "\n";
    cerr << "Throwing an exception to (this) with code " << code << "\n\n";
    throw(this);
}

void
C_dimensionLists::warn(
        string          astr_msg,
	int             code            /*= -1                  */
) {
    //
    // ARGS
    //  astr_class       in              name of the current class
    //  atr_msg          in              message to dump to stderr
    //  code             in              error code
    //
    // DESC
    //  Print error related information. Conceptually identical to
    //  the `error' method, but no expection is thrown.
    //

    cerr << "\nWarning.\n";
    cerr << "\tC_dimensionLists object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc_get() << "\n";
    cerr << "\t" << astr_msg << "(code: " << code << ")\n";
}

void
C_dimensionLists::function_trace(
        string          astr_msg,
        string          astr_separator) {
    //
    // ARGS
    //  astr_msg         in      message to print
    //  astr_separator   in      separator char between successive calls
    //
    // DESC
    //  This method allows for run-time debugging-related information
    //  in a particular class and method to be displayed to stderr.
    //
    
    string str_tab                      = "";
    static string str_objectName        = "";
    static string str_funcName          = "";

    if(verbosity_get() >= stackDepth_get()) {
        cerr << astr_separator;
        for(int i = 0; i < stackDepth_get(); i++)
            str_tab += "\t";
        if(str_objectName != str_name )
            cerr << "\nC_dimensionLists`" << str_name;
            cerr << "' (id: " << id << ")\n";
        if(str_funcName != str_proc_get()) {
            cerr << "\n" << str_tab << "Current function: " << str_obj;
            cerr << "::" << str_proc_get() << endl;
            cerr << "\tverbosity = "    << verbosity;
            cerr << ", stackDepth = "   << stackDepth << endl;
        }
        cerr << "\n" << str_tab << astr_msg;
    }
    str_objectName      = str_name_get();
    str_funcName        = str_proc_get();
}

void
C_dimensionLists::core_construct(
        string          astr_name       /*= "unnamed"           */,
        int             a_id            /*= -1                  */,
        int             a_iter          /*= 0                   */,
        int             a_verbosity     /*= 0                   */,
        int             a_warnings      /*= 0                   */,
        int             a_stackDepth    /*= 0                   */,
        string          astr_proc       /*= "noproc"            */
) {
    //
    // ARGS
    //  astr_name        in              name of object
    //  a_id             in              id of object
    //  a_iter           in              current iteration in arbitrary scheme
    //  a_verbosity      in              verbosity of object
    //  a_stackDepth     in              stackDepth
    //  astr_proc        in              current that has been "debug_push"ed
    //
    // DESC
    //  Simply fill in the core values of the object with some defaults
    //
    // HISTORY
    // 07 September 2000
    //  o Initial design and coding
    //
    // 05 March 2003
    //	o pV_dimensionStructure is initialised to "1" so that unity volume
    //	  stuctures can be created.
    //

    str_name                    = astr_name;
    id                          = a_id;
    iter                        = a_iter;
    verbosity                   = a_verbosity;
    warnings                    = a_warnings;
    stackDepth                  = a_stackDepth;
    str_proc[stackDepth]        = astr_proc;
    
    e_byteOrder			= e_littleEndian;

    str_obj                     = "C_dimensionLists";
    pV_dimensionStructure       = new CMatrix<int>(1, 5, 1);
}

C_dimensionLists::C_dimensionLists() {
    //
    // DESC
    //	Parameter-less constructor. Creates internal structures with
    //	"unity" values, i.e. all matrices/volumes/multi-dimensional
    //	structures have size [1 x 1 x ... x 1]
    //
    // POSTCONDITIONS
    //	o Fully constructed dimensionList with unity structures.
    //
    // HISTORY
    // 24 October 2003
    //	o Initial design and coding.
    //
    // 05 March 2004
    //	o Explicitly set the RO and PE dimensions in ROpePC to 1
    //	  so that unity volumes can be created.
    //
    
    core_construct();

    pM_sliceSelectList      = new CMatrix<int>(1, 1);
    pM_repetitionList       = new CMatrix<int>(1, 1);
    pM_echoList             = new CMatrix<int>(1, 1);
    pM_ROpePC               = new CMatrix<int>(1, 3);
    pM_ROpePC->val(0)	    = 1;
    pM_ROpePC->val(1)	    = 1;
}

C_dimensionLists::C_dimensionLists(
        const CMatrix<int>&     aM_sliceSelectList,
        const CMatrix<int>&     aM_repetitionList,
        const CMatrix<int>&     aM_echoList,
        int                     a_linesReadOut,
        int                     a_linesPhaseEncode,
        int                     a_linesPhaseCorrect) {
    //
    // ARGS
    //  aM_sliceSelectList      in              Vector: slice select list
    //  aM_repetitionList       in              Vector: repetition list
    //  aM_echoList             in              Vector: echo list
    //  a_linesReadOut          in              number of readout lines
    //  a_linesPhaseEncode      in              number of phase encode lines
    //  a_linesPhaseCorrect     in              number of phase correct lines
    //
    // DESC
    //  Direct constructor.
    //
    //  This constructor receives pre-built vectors/scalars as arguments
    //  and populates its corresponding internal structures.
    //
    // HISTORY
    //  14 August 2003
    //  o Expansion of original class definition.
    //

    core_construct();

    pM_sliceSelectList      = new CMatrix<int>(1, 1);
    pM_sliceSelectList->copy(aM_sliceSelectList);
    pM_repetitionList       = new CMatrix<int>(1, 1);
    pM_repetitionList->copy(aM_repetitionList);
    pM_echoList             = new CMatrix<int>(1, 1);
    pM_echoList->copy(aM_echoList);
    pM_ROpePC               = new CMatrix<int>(1, 3);
    pM_ROpePC->val(0, 0)    = a_linesReadOut;
    pM_ROpePC->val(0, 1)    = a_linesPhaseEncode;
    pM_ROpePC->val(0, 2)    = a_linesPhaseCorrect;
};

C_dimensionLists::C_dimensionLists(
        string                  astr_optionsFileName) {
    //
    // ARGS
    //  astr_optionsFileName    in              filename containing meta data
    //                                                  options
    //
    // DESC
    //  File-based meta constructor
    //
    //  This constructor accepts as argument a filename that contains
    //  meta data about the various dimensions.
    //
    // HISTORY
    //  14 August 2003
    //  o Initial design and coding.
    //


    core_construct();
    debug_push("C_dimensionLists(string astr_optionsFileName)");

    int i, j;
    
    str_optionsFileName			= astr_optionsFileName;

    string  str_ADCbaseDirectory        = "";
    string  str_kListDimensionFile      = "";
    string  str_ROpePCDimensionFile     = "";
    string  str_repListDimensionFile    = "";
    string  str_echoListDimensionFile   = "";

    C_scanopt                   cso_optionsFile(    astr_optionsFileName, e_EquLink);
    string                      str_value;
    string                      str_parsing     = "While parsing " + astr_optionsFileName;

    // Now parse the optionsFile itself
    if(cso_optionsFile.scanFor("ADCbaseDirectory",      &str_value))
        str_ADCbaseDirectory        = str_value;
    else error(str_parsing + ", no ADCbaseDirectory variable found", 2);
    str_ADCfileBaseName = str_ADCbaseDirectory + "/meas";
    
    if(cso_optionsFile.scanFor("ROpePCDimensionFile",     &str_value))
        str_ROpePCDimensionFile         = str_value;
    else error(str_parsing + ",\n\tno ROpePCDimensionFile variable found", 2);
    str_ROpePCDimensionFile = str_ADCbaseDirectory + "/" + str_ROpePCDimensionFile;
    pM_ROpePC       = new CMatrix<int>((char*)str_ROpePCDimensionFile.c_str());

    if(cso_optionsFile.scanFor("kListDimensionFile",    &str_value))
        str_kListDimensionFile      = str_value;
    else error(str_parsing + ",\n\tno kListDimensionFile variable found", 2);
    str_kListDimensionFile = str_ADCbaseDirectory + "/" + str_kListDimensionFile;

    if(cso_optionsFile.scanFor("repListDimensionFile",  &str_value))
        str_repListDimensionFile    = str_value;
    else error(str_parsing + ",\n\tno repListDimensionFile variable found", 2);
    str_repListDimensionFile = str_ADCbaseDirectory + "/" + str_repListDimensionFile;

    if(cso_optionsFile.scanFor("echoListDimensionFile",  &str_value))
        str_echoListDimensionFile   = str_value;
    else error(str_parsing + ",\n\tno echoListDimensionFile variable found", 2);
    str_echoListDimensionFile = str_ADCbaseDirectory + "/" + str_echoListDimensionFile;
    
    // Define the base directory
    string          str_fileBaseName    = str_ADCbaseDirectory + "/meas";

    CMatrix<int>*   pM_listData;        // 1x2 matrix of list endpoints
    int start, end;

    pM_listData                 = new CMatrix<int>((char*)str_kListDimensionFile.c_str());
    start   = pM_listData->val(0, 0);   end = pM_listData->val(0, 1); j = start;
    if(end==0)
        pM_sliceSelectList      = new CMatrix<int>(1, 1, 0);
    else {
        pM_sliceSelectList      = new CMatrix<int>(1, end-start+1);
        for(i=0; i<=end-start; i++) {
            pM_sliceSelectList->val(0, i)    = j++;
        }
    }

    delete pM_listData;
    pM_listData                 = new CMatrix<int>((char*)str_repListDimensionFile.c_str());
    start   = pM_listData->val(0, 0);   end = pM_listData->val(0, 1); j = start;
    if(end==0)
        pM_repetitionList       = new CMatrix<int>(1, 1, 0);
    else {
        pM_repetitionList       = new CMatrix<int>(1, end-start+1);
        for(i=0; i<=end-start; i++) {
            pM_repetitionList->val(0, i)    = j++;
        }
    }
    
    delete pM_listData;
    pM_listData                 = new CMatrix<int>((char*)str_echoListDimensionFile.c_str());
    start   = pM_listData->val(0, 0);   end = pM_listData->val(0, 1); j = start;
    if(end==0)
        pM_echoList             = new CMatrix<int>(1, 1, 0);
    else {
        pM_echoList             = new CMatrix<int>(1, end-start+1);
        for(i=0; i<=end-start; i++) {
            pM_echoList->val(0, i)          = j++;
        }
    }

    delete pM_listData;
    debug_pop();
}

C_dimensionLists::~C_dimensionLists() {
    delete  pM_sliceSelectList;
    delete  pM_repetitionList;
    delete  pM_echoList;	
    delete  pM_ROpePC;			
    delete  pV_dimensionStructure;
};	

void
C_dimensionLists::metaData_parse() {
    //
    // DESC
    //	The dimensionLists object is "evolving" beyond its original
    //	spec and becoming a repository for handling more information than
    //	just dimension stuctures of the problems. In effect, it is become
    //	an "environment" or really "meta" data holder as well.
    //
    //	This method parses the meta data file and process additional 
    //	non-dimensional information, such as byteOrder, saveFormats, etc.
    //
    // PRECONDITIONS
    //	o meta data file must exist and be defined in str_optionsFileName.
    //
    // HISTORY
    // 04 November 2003
    //	o Initial design and coding.
    //
    // 25 February 2004
    //	o Added several boolean flags.
    //
    
    C_scanopt                   cso_optionsFile(    str_optionsFileName, e_EquLink);
    string                      str_value;
    string                      str_parsing     = "While parsing " + str_optionsFileName;
    
    e_byteOrder			= e_littleEndian;
    b_unpackWpadShift		= true;
    b_packAdditionalData	= false;
    b_readOutCrop		= true;
    b_phaseCorrect		= false;
    b_shiftInPlace		= false;
    
    if(cso_optionsFile.scanFor("byteOrder",  &str_value))
	e_byteOrder		= (e_BYTEORDER) atoi(str_value.c_str());
    if(cso_optionsFile.scanFor("unpackWpadShift",	&str_value))
	b_unpackWpadShift	= (bool) atoi(str_value.c_str());
    if(cso_optionsFile.scanFor("packAdditionalData",    &str_value))
	b_packAdditionalData    = (bool) atoi(str_value.c_str());
    if(cso_optionsFile.scanFor("readOutCrop",  &str_value))
	b_readOutCrop		= (bool) atoi(str_value.c_str());
    if(cso_optionsFile.scanFor("phaseCorrect",  &str_value))
	b_phaseCorrect		= (bool) atoi(str_value.c_str());
    if(cso_optionsFile.scanFor("shiftInPlace",  &str_value))
	b_shiftInPlace		= (bool) atoi(str_value.c_str());
}

int
C_dimensionLists::repetitionIndex_find(
        int                     a_index) {
    //
    // ARGS
    //  a_index                 in              index number whose ordinal
    //                                                  position is sought.
    //
    // DESC
    //  This straightforward "find" method searches for the ordinal position
    //  of "a_index" within the list vector. Often times this ordinal index
    //  will be the same as "a_index" -- however if this list has interleaved
    //  indices the ordinal number will obviously differ.
    //
    // HISTORY
    //  14 August 2003
    //  o Spilt into header/definition components.
    //

    CMatrix<int>    M_index(1, 2);
    int             count = 0;

    M_index.copy(pM_repetitionList->findAll(a_index, count));
    if(!count)
        return -1;
    else
    return(M_index(0, 1));
};

int
C_dimensionLists::sliceSelectIndex_find(
        int                     a_index) {
    //
    // ARGS
    //  a_index                 in              index number whose ordinal
    //                                                  position is sought.
    //
    // DESC
    //  This straightforward "find" method searches for the ordinal position
    //  of "a_index" within the list vector. Often times this ordinal index
    //  will be the same as "a_index" -- however if this list has interleaved
    //  indices the ordinal number will obviously differ.
    //
    // HISTORY
    //  14 August 2003
    //  o Spilt into header/definition components.
    //

    CMatrix<int>    M_index(1, 2);
    int             count = 0;

    M_index.copy(pM_sliceSelectList->findAll(a_index, count));
    if(!count)
        return -1;
    else
        return(M_index(0, 1));
};

int
C_dimensionLists::echoIndex_find(
        int                     a_index) {
    //
    // ARGS
    //  a_index                 in              index number whose ordinal
    //                                                  position is sought.
    //
    // DESC
    //  This straightforward "find" method searches for the ordinal position
    //  of "a_index" within the list vector. Often times this ordinal index
    //  will be the same as "a_index" -- however if this list has interleaved
    //  indices the ordinal number will obviously differ.
    //
    // HISTORY
    //  14 August 2003
    //  o Spilt into header/definition components.
    //

    CMatrix<int>    M_index(1, 2);
    int             count = 0;

    M_index.copy(pM_echoList->findAll(a_index, count));
    if(!count)
        return -1;
    else
        return(M_index(0, 1));
};

void
C_dimensionLists::print() {
    //
    // DESC
    //  A simple "print" method that dumps the internal contents of the
    //  object.
    //
    //  Warning: may not be pretty :-P
    //

    cout << "Print dump for"                            << endl;
    cerr << "C_dimensionLists object `" << str_name << "' (id: " << id << ")\n";

    pM_ROpePC->print("ROpePC");                         cout << endl;
    pM_sliceSelectList->print("slice select (k)");      cout << endl;
    pM_repetitionList->print("repetition list");        cout << endl;
    pM_echoList->print("echo list");                    cout << endl;

}

//
//\\\***
// C_adcPack definitions ****>>>>
/////***
//

void
C_adcPack::debug_push(
        string                          astr_currentProc) {
    //
    // ARGS
    //  astr_currentProc        in      method name to
    //                                          "push" on the "stack"
    //
    // DESC
    //  This attempts to keep a simple record of methods that
    //  are called. Note that this "stack" is severely crippled in
    //  that it has no "memory" - names pushed on overwrite those
    //  currently there.
    //

    if(stackDepth_get() >= C_adcPack_STACKDEPTH-1)
        error(  "Out of str_proc stack depth");
    stackDepth_set(stackDepth_get()+1);
    str_proc_set(stackDepth_get(), astr_currentProc);
}

void
C_adcPack::debug_pop() {
    //
    // DESC
    //  "pop" the stack. Since the previous name has been
    //  overwritten, there is no restoration, per se. The
    //  only important parameter really is the stackDepth.
    //

    stackDepth_set(stackDepth_get()-1);
}

void
C_adcPack::error(
        string          astr_msg        /*= "Some error has occured"    */,
        int             code            /*= -1                          */)
{
    //
    // ARGS
    //  astr_class              in              name of the current class
    //  atr_msg                 in              message to dump to stderr
    //  code                    in              error code
    //
    // DESC
    //  Print error related information. This routine throws an exception
    //  to the class itself, allowing for coarse grained, but simple
    //  error flagging.
    //

    cerr << "\nFatal error encountered.\n";
    cerr << "\tC_adcPack object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc_get() << "\n";
    cerr << "\t" << astr_msg << "\n";
    cerr << "Throwing an exception to (this) with code " << code << "\n\n";
    throw(this);
}

void
C_adcPack::warn(
        string          astr_msg,
        int             code            /*= -1                  */
) {
    //
    // ARGS
    //  astr_class       in              name of the current class
    //  atr_msg          in              message to dump to stderr
    //  code             in              error code
    //
    // DESC
    //  Print error related information. Conceptually identical to
    //  the `error' method, but no expection is thrown.
    //

    cerr << "\nWarning.\n";
    cerr << "\tC_adcPack object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc_get() << "\n";
    cerr << "\t" << astr_msg << "(code: " << code << ")\n";
}

void
C_adcPack::function_trace(
        string          astr_msg,
        string          astr_separator) {
    //
    // ARGS
    //  astr_msg         in      message to print
    //  astr_separator   in      separator char between successive calls
    //
    // DESC
    //  This method allows for run-time debugging-related information
    //  in a particular class and method to be displayed to stderr.
    //

    string str_tab                      = "";
    static string str_objectName        = "";
    static string str_funcName          = "";

    if(verbosity_get() >= stackDepth_get()) {
        cerr << astr_separator;
       for(int i = 0; i < stackDepth_get(); i++)
            str_tab += "\t";
        if(str_objectName != str_name )
            cerr << "\nC_adcPack`" << str_name;
            cerr << "' (id: " << id << ")\n";
        if(str_funcName != str_proc_get()) {
            cerr << "\n" << str_tab << "Current function: " << str_obj;
            cerr << "::" << str_proc_get() << endl;
            cerr << "\tverbosity = "    << verbosity;
            cerr << ", stackDepth = "   << stackDepth << endl;
        }
        cerr << "\n" << str_tab << astr_msg;
    }
    str_objectName      = str_name_get();
    str_funcName        = str_proc_get();
}

void
C_adcPack::core_construct(
        string          astr_name       /*= "unnamed"           */,
        int             a_id            /*= -1                  */,
        int             a_iter          /*= 0                   */,
        int             a_verbosity     /*= 0                   */,
        int             a_warnings      /*= 0                   */,
        int             a_stackDepth    /*= 0                   */,
        string          astr_proc       /*= "noproc"            */
) {
    //
    // ARGS
    //  astr_name        in              name of object
    //  a_id             in              id of object
    //  a_iter           in              current iteration in arbitrary scheme
    //  a_verbosity      in              verbosity of object
    //  a_stackDepth     in              stackDepth
    //  astr_proc        in              current that has been "debug_push"ed
    //
    // DESC
    //  Simply fill in the core values of the object with some defaults
    //
    // HISTORY
    // 07 September 2000
    //  o Initial design and coding
    //
    // 23 February 2004
    //	o b_unpackWpadShift
    //
    // 25 February 2004
    //	o b_unpackWpadShift moved to pC_dimension class
    //

    str_name                    = astr_name;
    id                          = a_id;
    iter                        = a_iter;
    verbosity                   = a_verbosity;
    warnings                    = a_warnings;
    stackDepth                  = a_stackDepth;
    str_proc[stackDepth]        = astr_proc;

    echoTarget			= -1;		// default is *all* echoes and
    repetitionTarget		= -1;		//	repetitions.
    
    zeroPad_row			= 0;
    zeroPad_column		= 0;
    zeroPad_slice		= 0;
    
    str_obj                     = "C_adcPack";

}

C_adcPack::~C_adcPack() {
   //
   // DESC
   //   Destructor
   //
   // HISTORY
   // 05 May 2003
   //   o Initial design and coding.
   //
   // 01 September 2004
   //	o Added pV_echoesUnpacked
   //

   delete pCadc_kSpace;
   delete pCadc_phaseCorrected;
      
   delete pC_dimension;
   
   delete pV_echoesUnpacked;

}

C_adcPack::C_adcPack(
        const string            astr_baseFileName,
        C_dimensionLists*       apC_dimension,
        const bool              a_isData3D,
	int                     a_echoTarget,
	int                     a_repetitionTarget,
	int                     a_channelTarget) {
    //
    // ARGS
    //  astr_baseFilename               in              path and base name of
    //                                                          raw data
    //  apC_dimension                   in              an amalgamated class
    //                                                          describing the
    //                                                          dimensions of
    //                                                          the scan
    //                                                          space
    //  a_isData3D                      in              is this a 3D scan?
    //	a_echoTarget			in/optional	force the unpacking of a
    //								subset of echoes
    //	a_repetitionTarget		in/optional	force the unpacking of a
    //								subset of repetitions.
    //	a_channelTarget		        in/optional	force the unpacking of a
    //								subset of channels.
    //
    // DESC
    //  C_adcPack constructor.
    //
    //	In some cases it is more desirable to only unpack a specific echo/repetition/channel
    //	from a data set - particularly in the case when a given raw data set is
    //	physically too large to fit into active memory. Under such a case the 
    //	a_echoTarget/a_repetitionTarget/a_channelTarget arguments specify a 
    //  desired target coordinate tuple.
    //
    //	The default values for these variables are both -1.
    //
    // PRECONDITIONS
    //  o astr_aschFileName *must* be a valid and complete filename
    //  o all vectors must be *row* vectors
    //	o If a target echo/repetition is specified, make sure it is a 
    //	  valid set!
    //
    // POSTCONDITIONS
    //  o Internal class fields are filled with values read from asc file.
    //  o Values that are not found are initialized to zero.
    //
    // HISTORY
    // 03 May 2003
    //  o Initial design and coding.
    //
    // 08 October 2003
    //	o Added pV_dimensionStructure
    //
    // 16 October 2003
    //	o Added "targeted" echo / repetition processing.
    //
    // 06 November 2003
    //	o Added channelTarget processing.
    //
    // 23 February 2004
    //	o Added b_unpackWpadShift processing.
    //
    // 02 March 2004
    //	o Index reorganizing.
    //
    // 01 September 2004
    //	o Added pV_echoesUnpacked
    //

    stackDepth = 0;
    debug_push("C_adcPack");

    core_construct();

    str_baseFileName            = astr_baseFileName;
    flag3D                      = a_isData3D;

    // "Unpack" the data from the C_dimensionLists structure
    //	The C_dimensionLists structure is essentially read from
    //	disk. The echoList and repetitionList values, however,
    //	can be overwritten by passing non-default values to the
    //	a_echoTarget and a_repetitionTarget variables.

    CMatrix<int>*               pM_sliceSelectList      = new CMatrix<int>(1,1);
    pM_sliceSelectList->copy(apC_dimension->M_sliceSelectionList_get());
    
    CMatrix<int>*               pM_repetitionList       = new CMatrix<int>(1,1);
    if(a_repetitionTarget == -1)
	pM_repetitionList->copy(apC_dimension->M_repetitionList_get());
    CMatrix<int>*               pM_echoList             = new CMatrix<int>(1,1);
    if(a_echoTarget == -1)
	pM_echoList->copy(apC_dimension->M_echoList_get());
    
    echoTarget			= a_echoTarget;
    repetitionTarget		= a_repetitionTarget;
    channelTarget		= a_channelTarget;

    int numRepetitions          = pM_repetitionList->cols_get();
    int numEchoes               = pM_echoList->cols_get();
    
    int linesReadOut            = apC_dimension->linesReadOut_get();
    int linesPhaseEncode        = apC_dimension->linesPhaseEncode_get();
    int linesSliceSelect        = apC_dimension->M_sliceSelectionList_get().cols_get();
    int linesPhaseCorrect       = apC_dimension->linesPhaseCorrect_get();

    CMatrix<int>                M_dimensions(1, 5);
    M_dimensions(0, e_readOut)          = linesReadOut;
    M_dimensions(0, e_phaseEncode)      = linesPhaseEncode;
    M_dimensions(0, e_sliceSelect)      = linesSliceSelect;
    M_dimensions(0, e_numRepetitions)   = numRepetitions;
    M_dimensions(0, e_numEchoes)        = numEchoes;
    pV_echoesUnpacked			= new CMatrix<int>(1, numEchoes, 0);
    
    // The following code is used if "intelligent" unpack is selected.
    //	If true, we pre-calculate the final size (with possible zeroPadding)
    //	of the final volume, and pass this information onto the internal
    //	dimensionList variable.
    //
    // Things are a bit clumsy with the slice select since we're still 
    //	carrying around the concept of a slice *list* as inherited from
    //	Anders' original MatLAB code, but for now we'll keep slogging
    //	this about.
    if(apC_dimension->b_unpackWpadShift_get()) {
	CMatrix<int>		M_origVol(1, 3);
	CMatrix<int>		M_padWshiftVol(1, 3);
	M_origVol(0)		= linesReadOut;
	M_origVol(1)		= linesPhaseEncode;
	M_origVol(2)		= linesSliceSelect;
	M_padWshiftVol          = dimension_zeroPad(M_origVol);
	if(linesSliceSelect != M_padWshiftVol(2)) {
	    delete pM_sliceSelectList;
	    pM_sliceSelectList	= new CMatrix<int>(1, M_padWshiftVol(2));
	    for(int i=0; i<M_padWshiftVol(2); i++)
		pM_sliceSelectList->val(i) = i;
	}
	linesReadOut	                = M_padWshiftVol(0);
	linesPhaseEncode	        = M_padWshiftVol(1);
	linesSliceSelect	        = M_padWshiftVol(2);
	M_dimensions(0, e_readOut)	= linesReadOut;
	M_dimensions(0, e_phaseEncode)	= linesPhaseEncode;
	M_dimensions(0, e_sliceSelect)	= linesSliceSelect;
	
	//M_origVol.print("Orig vol");
	//M_padWshiftVol.print("\nNew vol");
	//exit(1);
    }

    //M_dimensions.print("M_dimensions");

    pC_dimension                = new C_dimensionLists( *pM_sliceSelectList,
                                                        *pM_repetitionList,
                                                        *pM_echoList,
                                                        linesReadOut,
                                                        linesPhaseEncode,
                                                        linesPhaseCorrect);
    
    // The internal dimension structure contains data relevant to the particular
    //	objects contained in the process. Copy some misc final values from the
    //	external dimension structure to the internal.
    pC_dimension->str_optionsFileName_set(apC_dimension->str_optionsFileName_get());
    pC_dimension->metaData_parse();
    pC_dimension->pV_dimensionStructure_set(&M_dimensions);
    
    debug_pop();
}

void
C_adcPack::headerFile_process(
        bool            b_headerDump    /* = false */) {
    //
    // ARGS
    //  b_headerDump    in              flag controlling dump of header
    //                                          to stdout
    //
    // DESC
    //  Process the ASCII header filename
    //
    // PRECONDITIONS
    //
    // POSTCONDITIONS
    //
    //
    // HISTORY
    // 07 May 2003
    //  o Initial design and coding.
    //

    debug_push("headerFile_process");

    str_ascFileName = str_baseFileName + ".asc";

    pcasch_measASCfile  = new C_asch(str_ascFileName);
    if(b_headerDump)
        pcasch_measASCfile->print();

    debug_pop();
}

bool
C_adcPack::disk2memory_voxelMap(
    int			indexChannel,
    int			indexSlicePartition,
    int			indexRepetition,
    int			indexEcho,
    int		        loopCounter,
    int&		slicePartitionIndex,
    int&		repetitionIndex,
    int&		echoIndex
) {
    //
    // ARGS
    //	index*		        in		voxel indices as read from disk:
    //							indexSlicePartition
    //							indexChannel
    //							indexRepetition
    //							indexEcho
    //	loopCounter		in		current overall process loop
    //	M_dataIndex		in/out		Vector containing indices corresponding
    //							to memory locations
    //	*Index			int/out		corresponding memory voxel indices
    //							slicePartitionIndex
    //							repetitionIndex
    //							echoIndex
    //
    // DESC
    //	This method is called by dataFile_process() and is part of an attempt
    //	to streamline (and increase readability) of that method. It associates a 
    //	given voxel index that is read from disk with the same voxel as stored 
    //	in memory.
    //
    //	The parameter list is a bit longer than I'd usually like, but I decided
    //	to err on the side of increased speed (I could have improved readability
    //	by passing a CMatrix in and out).
    //
    // PRECONDITIONS
    //	o Called by dataFile_process()
    //
    // POSTCONDITIONS
    //	o returns a bool value indicating whether or not to store the 
    //	  current data read from meas.out
    //
    // HISTORY
    // 04 March 2004
    //	o Initial design and coding
    //
    
    bool	b_canUnpack	        = true;
    
    // For multi-channel data, check channel indices. Targetted channel
    //	data can be thought of as a simple binary decision on whether
    //	or not to unpack echo/repetition data. The channelId is the 
    //	most significant differentiator.
    if(channelTarget>=0)
	if(indexChannel!=channelTarget)
	    return false;
	
    // Check repetition target
    if(repetitionTarget>=0) {
	if(indexRepetition!=repetitionTarget) {
	    // Since we have spec'd a particular repetition, and since
	    //  the current repetition is *not* our target, break
	    //  out of the surrounding while loop
	    return false;
	} else {
	    repetitionIndex = 0;
	}
    } else {
	repetitionIndex     =   pC_dimension->repetitionIndex_find(
                                                indexRepetition);
	if(repetitionIndex  == -1) {
	    char pch_msg[1024];
	    warn("Invalid repetitionIndex. Make sure that the repetitionlist variable is correct.");
	    cout << "\t\tindexRepetition\t= " 	<<	indexRepetition	<< endl;
	    cout << "\t\tindexEcho\t= "		<< 	indexEcho	<< endl;
	    cout << "\t\tindexChannel\t= " 	<<	indexChannel	<< endl;
	    cout << "\t\tloopCounter\t= " 	<<	loopCounter	<< endl;
	    return false;
	}
    }

    // Check echo target
    if(echoTarget>=0) {
	if(indexEcho!=echoTarget) {
	    // Since we have spec'd a particular echo, and since
	    //  the current echo is *not* our target, break
	    //  out of the surrounding while loop
	    return false;
	} else {
	    echoIndex   = 0;
	}
    } else {
	echoIndex       =   pC_dimension->echoIndex_find(indexEcho);
	if(echoIndex    == -1) {
	    char pch_msg[1024];
	    warn("Invalid echoIndex. Make sure that the echolist variable is correct.");
	    cout << "\t\tindexRepetition\t= " 	<<	indexRepetition	<< endl;
	    cout << "\t\tindexEcho\t= "		<< 	indexEcho	<< endl;
	    cout << "\t\tindexChannel\t= " 	<<	indexChannel	<< endl;
	    cout << "\t\tloopCounter\t= " 	<<	loopCounter	<< endl;
	    return  false;
	}	
    }
    
    // Check slice
    slicePartitionIndex =       pC_dimension->sliceSelectIndex_find(
                                                            indexSlicePartition);
    if(slicePartitionIndex  == -1) {
	char pch_msg[1024];
	warn("Invalid indexSlicePartition.");
	cout << "\t\tindexRepetition\t= " 	<<	indexRepetition	<< endl;
	cout << "\t\tindexEcho\t= "		<< 	indexEcho	<< endl;
	cout << "\t\tslicePartition\t= " 	<<	indexSlicePartition << endl;
	cout << "\t\tloopCounter\t= " 		<<	loopCounter	<< endl;
	return false;
    }
    return true;
}

int
C_adcPack::phaseCorrect_unpack(
    CMatrix<GSL_complex_float>&		Mz_adc,
    bool				b_reflect,
    unsigned long			ul_timeStamp,
    int			                linesPhaseEncode,
    int			                linesSliceSelect,	
    int			                readOutIndex,
    int			                phaseEncodeIndex,
    int			                slicePartitionIndex,
    int			                repetitionIndex,
    int			                echoIndex
) {
    //
    // ARGS
    //	Mz_adc              in		current PE k-space line
    //						read from disk
    //  b_bitReflect	    in		bit to track
    //	ul_timeStamp	    in		timeStamp to track
    //	lines*		    in		voxel dimension info
    //	*Index		    in		memory voxel indices
    //
    // DESC
    //	Unpacks a given phase encoded line of k-space data (read
    //	from disk) to memory.
    //
    //	This method is used to phase corrected data processing.
    //
    // HISTORY
    // 04 March 2004
    //	o Initial design and coding.
    //  o p_packAdditionalData processing.
    //
    
    cout << "In phase correction..." << endl; exit(1);
		
    int         indexCorrected;
    echoIndex	= 1;	// All phase corrected volumes belong to echo 1
		
    // Calculate zeroPadded / shifted indices first, if necessary
    //  as defined by b_unpackWpadShift_get(). These
    //	are then used by subsequent addressing.
    //  First, a ifftshift to get the slicePartitionIndex
    //	for the indexCorrected		
    if(b_unpackWpadShift_get())
	slicePartitionIndex	= ifftshiftIndex(linesSliceSelect,
						 slicePartitionIndex);
		
    indexCorrected          = pCadc_phaseCorrected->A_ssIndex()(
	                                            slicePartitionIndex,	
						    repetitionIndex,	
						    echoIndex) + 1;
    readOutIndex		= indexCorrected;
		
    // Now loop along the linesPhaseEncode, shifting each
    //	voxel.
    for(int i=0; i<Mz_adc.cols_get(); i++) {
	if(b_unpackWpadShift_get()) {
	    phaseEncodeIndex	= ifftshiftIndex(linesPhaseEncode,
						 i+zeroPad_column);
	} else {
	    phaseEncodeIndex	= i;
	}
	
	pCadc_phaseCorrected->pMz_data_get()->val(
					    readOutIndex,
                                            phaseEncodeIndex,
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                            )   = Mz_adc(i);
    }
    pCadc_phaseCorrected->A_ssIndex()(
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                            )   = indexCorrected;
		
    if(b_packAdditionalData_get()) {
	// Only unpack the Ab_reverse and Aul_timeStamp if explicitly 
	//	requested
	pCadc_phaseCorrected->Ab_reverse()(
                                            indexCorrected,
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                            )   = b_reflect;

	pCadc_phaseCorrected->Aul_timeStamp()(
                                            indexCorrected,
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                   		)   = ul_timeStamp;
    }
    return(0);
}

int
C_adcPack::kSpace_unpack(
    CMatrix<GSL_complex_float>&		Mz_adc,
    bool				b_reflect,
    unsigned long			ul_timeStamp,
    int					indexLine,
    int					linesReadOut,
    int			                linesPhaseEncode,
    int			                linesSliceSelect,	
    int			                readOutIndex,
    int			                phaseEncodeIndex,
    int			                slicePartitionIndex,
    int			                repetitionIndex,
    int			                echoIndex
) {
    //
    // ARGS
    //	Mz_adc              in		current PE k-space line
    //						read from disk
    //  b_bitReflect	    in		bit to track
    //	ul_timeStamp	    in		timeStamp to track
    //	indexLine	    in		readOut line to track
    //	lines*		    in		voxel dimension info
    //	*Index		    in		memory voxel indices
    //
    // DESC
    //	Unpacks a given phase encoded line of k-space data (read
    //	from disk) to memory.
    //
    // HISTORY
    // 04 March 2004
    //	o Initial design and coding.
    //
    // Calculate zeroPadded / shifted indices first, if necessary
    //  as defined by b_unpackWpadShift_get(). These
    //	are then used by subsequent addressing.
    //  First, a ifftshift to get the slicePartitionIndex
    //	for the indexCorrected		

    //cout << "indexLine\t\t"             << indexLine            << endl;
    //cout << "linesReadOut\t\t"          << linesReadOut         << endl;
    //cout << "linesPhaseEncode\t"        << linesPhaseEncode     << endl;
    //cout << "linesSliceSelect\t"        << linesSliceSelect     << endl;
        		    
    if(b_unpackWpadShift_get()) {
//        cout << endl;
//	cout << "indexLine                    \t" << indexLine                  << " / ";
//	cout << "\t"                              << linesPhaseEncode           << endl;
//	cout << "slicePartitionIndex          \t" << slicePartitionIndex        << " /";
//	cout << "\t"                              << linesSliceSelect           << endl;
	phaseEncodeIndex	= ifftshiftIndex(linesPhaseEncode,
					 indexLine);
	slicePartitionIndex	= ifftshiftIndex(linesSliceSelect,
						 slicePartitionIndex);
//	cout << "indexLine (shifted)          \t" << phaseEncodeIndex           << endl;
//	cout << "slicePartitionIndex (shifted)\t" << slicePartitionIndex        << endl;
    } else
	phaseEncodeIndex	= indexLine;
    
    if(b_packAdditionalData_get()) {
	// Only unpack the Ab_reverse and Aul_timeStamp if explicitly 
	//	requested
	pCadc_kSpace->Ab_reverse()(
                                            readOutIndex,
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                            )   = b_reflect;

	pCadc_kSpace->Aul_timeStamp()(
                                            readOutIndex,
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                            )  = ul_timeStamp;
    }
		    
    // Now loop along the ReadOut dimension, shifting each phase encode indexLine 
    //  voxel if necessary
    
    for(int i=0; i<Mz_adc.cols_get(); i++) {
/*        cout << endl;
        cout << "Packing readOutIndex " << i << "... "      << endl;
	cout << "readOutIndex\t\t"      << readOutIndex         << endl;
	cout << "phaseEncodeIndex\t"    << phaseEncodeIndex     << endl;
	cout << "slicePartitionIndex\t" << slicePartitionIndex  << endl;
	cout << "repetitionIndex\t\t"   << repetitionIndex      << endl;
	cout << "echoIndex\t\t"         << echoIndex            << endl;*/
        if(b_unpackWpadShift_get()) {
            readOutIndex 	= ifftshiftIndex(linesReadOut,
                                                 i+zeroPad_column);
        } else {
            readOutIndex	= i;
        }

	pCadc_kSpace->pMz_data_get()->val(
					    readOutIndex,
                                            phaseEncodeIndex,
                                            slicePartitionIndex,
                                            repetitionIndex,
                                            echoIndex
                                            )   = Mz_adc(i);
        //cout << "packed." << endl;
    }
    return(0);
}

int
C_adcPack::dataFile_process() 
{
    //
    // DEPENDENCIES
    //  repetitionTarget    in/opt          specific repetition to filter out
    //  echoTarget          in/opt          specific echo number to filter out
    //
    // DESC
    //  Process the actual raw data.
    //
    //  Note that specific echoes and/or repetitions can be selected by setting
    //  the corresponding *Target arguments to values >= 0. This is useful for
    //  instances when the raw data file contains the results of several echoes/
    //  repetitions and might be too large to process in memory in its
    //  entirety.
    //
    // PRECONDITIONS
    //  o Corresponding header file must have been processed.
    //	o Member variables, echoTarget and repetitionTarget, define either
    //    *all* samples (in which case both are -1), or a *specific*
    //	  repetition/echo combination.
    //  o If *Targets are specified, make sure that they are valid for the
    //    data set being unpacked! Also, only *single* (as opposed to vector)
    //    target sets can be specified.
    //
    // POSTCONDITIONS
    //	o Returns the number of samples processed.
    //  o The Siemens raw data file is processed and unpacked to memory
    //    memory structures.
    //
    // HISTORY
    // 07 May 2003
    //  o Initial design and coding.
    //
    // 16 September 2003
    //  o Added optional echo / repetition target arguments
    //
    // 06 November 2003
    //	o channelId processing
    //
    // 02 March 2004
    //	o Index reorganizing.
    //
    // 04 March 2004
    //	o Switch over to libCMatrix type stuctures
    //
    // 05 March 2004
    //	o Long overdue cleanup started.
    //	o Testing with libCMatrix for core kspace data complete -
    //	  references to ltl MArray for core processing removed.
    //
    // 01 September 2004
    //	o pV_echoesUnpacked processing
    //
    // May 2006
    // 	o 2D volume processing. Note that 2D slice ordering can be
    //	  interleaved!
    //
    // 31 May 2006
    //	o Assume that all 2D volumes are interleaved. This should probably
    //	  be abstracted to a higher level and user specified.
    //

    debug_push("dataFile_process()");

    int            	l_adcStartOffset        = -1;

    int                 i                       = -1;

    // The following variables are mostly read from the Siemens mdh structure

    // Current indices into the volumetric space as read from meas.out 
    // (from s_MDH.sLC)...
    int                 indexSlicePartition     = 0;
    int			indexSlice2D		= 0;
    int                 indexLine               = 0;
    int                 indexEcho               = 0;
    int                 indexRepetition         = 0;
    int                 samplesInScan           = 0;
    unsigned long	indexChannel		= 0;

    // Corresponding indices into adc class space (in some cases these will
    //  be identical to the volumetric indices) that we maintain in memory
    int                 slicePartitionIndex     = 0;
    int			phaseEncodeIndex        = 0;
    int			readOutIndex		= 0;
    int                 echoIndex               = 0;
    int                 repetitionIndex         = 0;
    
    int			linesReadOut		= pC_dimension->linesReadOut_get();
    int			linesPhaseEncode	= pC_dimension->linesPhaseEncode_get();
    int			linesSliceSelect	= pC_dimension->linesSliceSelect_get();

    //cout << linesReadOut << "\t" << linesPhaseEncode << "\t" << linesSliceSelect << endl;
    
    // Some bits masked out of evalInfoMask
    bool                bit_online              = false;
    bool                bit_phaseCorrection     = false;
    bool                bit_reflect             = false;
    const bool          bit_true                = true;
    bool                b_canUnpack             = true;     // flag for echo- and

    unsigned long       pul_evalInfoMask[2];

    str_adcFileName         = str_baseFileName + ".out";
    ifstream    istream_adcFile(str_adcFileName.c_str(), ios::in | ios::binary);
    //ifstream    istream_adcFile(str_adcFileName.c_str(), ios::in);
    if(!istream_adcFile) {
	cout << "\n\tgoodbit:\t" << (istream_adcFile.rdstate() == ios::goodbit) << endl;
	cout << "\teofbit:\t\t"  << (istream_adcFile.rdstate() == ios::eofbit)  << endl;
	cout << "\tfailbit:\t"   << (istream_adcFile.rdstate() == ios::failbit) << endl;
	cout << "\tbadbit:\t\t"  << (istream_adcFile.rdstate() == ios::badbit)  << endl;
        error("Some error occurred while accessing input file: " + str_adcFileName);
    }

    // Read in the offset to the actual data
    istream_adcFile.read((char*) &l_adcStartOffset, sizeof(int));
    //  ... and jump to this location
    istream_adcFile.seekg(l_adcStartOffset, ios::beg);

    // Read in first record
    if(!istream_adcFile.read((char*) &s_MDH, sizeof(sMDH)))
        error("Could not access first record");

    long int    loopCounter     = 0;
    long int    echoCount       = 0;
    while(      !(s_MDH.aulEvalInfoMask[0] & MDH_ACQEND_MASK)) {
        loopCounter++;
        samplesInScan   = s_MDH.ushSamplesInScan;
        indexLine       = s_MDH.sLC.ushLine;
        if(samplesInScan <= 0)
            error("Error in processing samplesInScan: readSamples were <= 0");
        //cout << samplesInScan << endl;

        // Read the ADC data corresponding to the current MDH structure
	CMatrix<GSL_complex_float>	Mz_adc(1, samplesInScan);
        float*                          pf_adc  = new float[samplesInScan*2];

        istream_adcFile.read((char*) pf_adc, sizeof(float)*samplesInScan*2);

        for(i=0; i<samplesInScan; i++) {
	    GSL_complex_float	z(pf_adc[i*2], pf_adc[i*2+1]);
	    Mz_adc(i)		= z;
        }

        if(flag3D)
            indexSlicePartition = s_MDH.sLC.ushPartition;
        else {
            indexSlice2D 	= s_MDH.sLC.ushSlice;
	    // Correct for interleaving
	    if(indexSlice2D < (linesSliceSelect-2*zeroPad_slice) / 2)
		indexSlicePartition	= indexSlice2D*2 + 1;
	    else {
            	indexSlicePartition 	= (indexSlice2D-(linesSliceSelect-2*zeroPad_slice)/2)*2;
		}
	}

        indexRepetition         =       s_MDH.sLC.ushRepetition;
        indexEcho               =       s_MDH.sLC.ushEcho;
	indexChannel		= 	s_MDH.ulChannelId;

	//cout << "k space index: " << indexSlicePartition << endl;

        pul_evalInfoMask[0]     =       s_MDH.aulEvalInfoMask[0];
        pul_evalInfoMask[1]     =       s_MDH.aulEvalInfoMask[1];
        bit_online              =       pul_evalInfoMask[0] & (bit_true << 3);
        bit_phaseCorrection     =       pul_evalInfoMask[0] & (bit_true << 21);
        bit_reflect             =       pul_evalInfoMask[0] & (bit_true << 24);

        // Now, based on the indices read from the MDH structure, find
        //      corresponding indices in the dimension lists of the
        //      sequences used to construct the core adc components.
	//
	// Since we might only be interested in a single volume (and not
	//	everything in the meas.out), we need to map the disk
	//	voxel indices to corresponding indices for our memory
	//	structure.
		
	b_canUnpack = disk2memory_voxelMap(
	    indexChannel,
	    indexSlicePartition,
	    indexRepetition,
	    indexEcho,
	    loopCounter,
	    slicePartitionIndex,
	    repetitionIndex,
	    echoIndex
	    );
	            
	if(b_canUnpack) {
	
	    // Record this echo-index in the pV_echoesUnpacked vector
	    pV_echoesUnpacked->val(0, echoIndex)	= 1;
	    
	    // Before we do anything, we need to offset volume indices for
	    //	possible zeroPadding. This is controlled by the b_unpackWpadShift
	    //	boolean flag. If false, the offset vector is zero; if true, the
	    //	offset vector has been set by a prior call to dimension_zeroPad()
	    slicePartitionIndex += zeroPad_slice;
	    indexLine		+= zeroPad_row;
	    // Since a phase encoded line (i.e. column dimension) is the pf_adc
	    //	array, each element needs to be offset with zeroPad_column

            if(bit_reflect) {
                // Reverse the order of elements in pz_adc
		//cout << "Entering bit reflect..." << endl;
		CMatrix<GSL_complex_float>	Mz_adcCopy(1, samplesInScan);
		Mz_adcCopy.copy(Mz_adc);
                // Remember that Az_adcCopy has its "base" counting from 1
                for(i=0; i<samplesInScan; i++)
		    Mz_adc(i)		= Mz_adcCopy(samplesInScan-i-1);
		//cout << "Leaving bit reflect..." << endl;
            }
	        
            echoCount++;
            
	    //cout << "About to enter kSpace_unpack()" << endl;   
	        
            if(bit_phaseCorrection) {
		phaseCorrect_unpack(
		    Mz_adc,
		    bit_reflect,
		    s_MDH.ulTimeStamp,
		    linesSliceSelect,	
		    linesPhaseEncode,
		    readOutIndex,
		    phaseEncodeIndex,
		    slicePartitionIndex,
		    repetitionIndex,
		    echoIndex
		    );		
            } else {
		kSpace_unpack(
		    Mz_adc,
		    bit_reflect,
		    s_MDH.ulTimeStamp,
		    indexLine,
		    linesReadOut,
		    linesPhaseEncode,
		    linesSliceSelect,	
		    readOutIndex,
		    phaseEncodeIndex,
		    slicePartitionIndex,
		    repetitionIndex,
		    echoIndex
		    ); 
            }
        }

        delete  []      pf_adc;
        // Read in next record
        if(!istream_adcFile.read((char*) &s_MDH, sizeof(sMDH)))
            error("Problems accessing record. Unexpected end of file reached.");
    }
    int allEchoesUnpacked	= pV_echoesUnpacked->innerProd();
    if(!allEchoesUnpacked && echoTarget==-1) {
    	string str_echoesUnpacked;
    	warn("The raw data contains less echoes than the configuration file suggests.\n\tRecon will continue, but please verify. ");
	str_echoesUnpacked = pV_echoesUnpacked->sprint("pV_echoesUnpacked");
	cerr << str_echoesUnpacked << endl;
    }
    debug_pop();
    return echoCount;
}

void
C_adcPack::dataMemory_volumeExtract(
    int                     a_repetition,
    int                     a_echo,
    e_KSPACEDATATYPE        ae_kspace       /*= e_normalKSpace*/)
{
    //
    // ARGS
    //  a_repetition        in          the repitition number
    //  a_echo              in          the echo number
    //  ae_kspace           in/opt      the kspace data set to process
    //
    // DESC
    //  This method "extracts" the volume bounded by the given
    //  repetition and echo. Note, that the actual data is "contained"
    //  (or rather, pointed to) by the C_adc class.
    //
    // PRECONDITIONS
    //  o None.
    //
    // POSTCONDITIONS
    //  o The spec'd (ae_kspace) C_adc class has its internal volume
    //    extracted.
    //
    // HISTORY
    // 11 September 2003
    //  o Initial design and coding.
    //

    debug_push("dataMemory_volumeExtract()");

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc_kSpace->volume_extract(a_repetition, a_echo);
        break;
        case e_phaseCorrectedKSpace:
            pCadc_phaseCorrected->volume_extract(a_repetition, a_echo);
        break;
    }
    debug_pop();
}

void
C_adcPack::dataMemory_volumeConstruct(
    e_KSPACEDATATYPE        ae_kspace       /*= e_normalKSpace*/)
{
    //
    // ARGS
    //  ae_kspace           in/opt      the kspace data set to process
    //
    // DESC
    //  This method "constructs" a volume extracted by a call to
    //  _volumeExtract(...). Note, that the actual data is "contained"
    //  (or rather, pointed to) by the C_adc class.
    //
    // PRECONDITIONS
    //
    // POSTCONDITIONS
    //  o The spec'd (ae_kspace) C_adc class has its internal volume
    //    constructed.
    //
    // HISTORY
    // 23 October 2003
    //  o Initial design and coding.
    //

    debug_push("dataMemory_volumeConstruct()");

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc_kSpace->volume_construct();
        break;
        case e_phaseCorrectedKSpace:
            pCadc_phaseCorrected->volume_construct();
        break;
    }
    debug_pop();
}

void
C_adcPack::dataMemory_volumeDestruct(
    e_KSPACEDATATYPE        ae_kspace       /*= e_normalKSpace*/)
{
    //
    // ARGS
    //  ae_kspace           in/opt      the kspace data set to process
    //
    // DESC
    //  This method "destructs" a volume extracted by a call to
    //  _volumeExtract(...). Note, that the actual data is "contained"
    //  (or rather, pointed to) by the C_adc class.
    //
    // PRECONDITIONS
    //  o Volume must have been previously "constructed" with a call
    //    to _volumeExtract(...).
    //
    // POSTCONDITIONS
    //  o The spec'd (ae_kspace) C_adc class has its internal volume
    //    destroyed.
    //
    // HISTORY
    // 17 September 2003
    //  o Initial design and coding.
    //

    debug_push("dataMemory_volumeDestruct()");

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc_kSpace->volume_destruct();
        break;
        case e_phaseCorrectedKSpace:
            pCadc_phaseCorrected->volume_destruct();
        break;
    }
    debug_pop();
}

CMatrix<int>
C_adcPack::dimension_zeroPad(
    CMatrix<int>&	M_orig
) {
    //
    // ARGS
    //	M_orig		in		row vector specifying the original
    //						volume dimensions
    //
    // DESC
    //	Given the volume dimensions as read from disk, this method
    //	returns a row vector with the dimensions of the same volume,
    //	but zeroPadded (if necessary).
    //
    // PRECONDITIONS
    // o M_orig is [ row col slice ]
    //
    // POSTCONDITIONS
    // o return is [ row col slice ], where each dimension is "padded"
    //	 if necessary. If no padding is required, dimension is unchanged.
    // o set internal class variables, zeroPad_{row,column,slice} with 
    //	 an offset value
    //
    // HISTORY
    // 13 February 2004
    //	o Initial design and coding.
    //
    
    debug_push("dimension_zeroPad()");
    
    CMatrix<int>	M_padded(1, 3);
    
    M_padded.copy(M_orig);
    int     rows		= M_orig(0);
    int     cols		= M_orig(1);
    int     slices		= M_orig(2);
    
    int     dimension           = -1;
    int     dimensionLength     = -1;
    int     ones                = -1;
    int     highestPower        = -1;     // for powerOf2() analysis
    int     nextHighestPower    = -1;
    int     padTarget           = -1;
    int     padAmount           = -1;
        
    for(dimension=e_row; dimension<=e_slice; dimension++) {
        powersOf2(M_orig.val(dimension), ones, highestPower);
        if(ones>1) {
            // We need to zeroPad along this dimension
            //  - first create the new padded volume
	    nextHighestPower = highestPower+1;
	    padTarget   = (1<<nextHighestPower);
            padAmount   = (padTarget - M_orig.val(dimension))/2;
	    
	    if((e_DOMINANCE)dimension==e_row) {
		M_padded(0)	= rows + 2*padAmount;
		zeroPad_row	= padAmount;
	    }
	    if((e_DOMINANCE)dimension==e_column) { 
		M_padded(1)	= cols + 2*padAmount;
		zeroPad_column	= padAmount;
	    }
	    if((e_DOMINANCE)dimension==e_slice) { 
		M_padded(2)	= slices + 2*padAmount;
		zeroPad_slice	= padAmount;
	    }
        }
    }
    debug_pop();
    return M_padded;
}

void
C_adcPack::dataMemory_volumeZeroPad(
    e_KSPACEDATATYPE        ae_kspace       /*= e_normalKSpace*/
)
{
    //
    // ARGS
    //  ae_kspace           in/opt      the kspace data set to process
    //
    // DESC
    //  Performs a zero padding operation on the extracted volume. This is
    //  an automated process: each dimension of the volume is examined, and
    //  if not a power of two, this dimension is pre- and post-padded with enough
    //  elements to complete a power of two.
    //
    // PRECONDITIONS
    //  o Extracted volume.
    //  o Implicit assumption that each volume dimension is of even length.
    //
    // POSTCONDITIONS
    //  o Zero padded volume is created.
    //  o This method might be "expensive" in as much as zero padding requires
    //    new memory to be re-allocated and the old memory to be freed.
    //
    // HISTORY
    // 11 September 2003
    //  o Initial design and coding.
    //
    // 01 October 2003
    //	o Incorporated volume object class.
    //

    debug_push("dataMemory_volumeZeroPad()");

    CVol<GSL_complex_float>*          pVl_volume;
    CVol<GSL_complex_float>*          pVl_volumePadded   = NULL;
    int                         inputSlices;
    int                         dimensionLength;
    C_adc*                      pCadc;

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc   = pCadc_kSpace;
        break;
        case e_phaseCorrectedKSpace:
            pCadc   = pCadc_phaseCorrected;
        break;
    }

//    pVl_volume	= pCadc->pvolume();
    
    int     dimension           = -1;
    int     ones                = -1;
    int     highestPower        = -1;     // for powerOf2() analysis
    int     nextHighestPower    = -1;
    int     padTarget           = -1;
    int     padAmount           = -1;
    for(dimension=e_row; dimension<=e_slice; dimension++) {
	pVl_volume	        = pCadc->pvolume();
	int     rows		= pCadc->volume_get()->rows_get();
	int     cols		= pCadc->volume_get()->cols_get();
	int     slices		= pCadc->volume_get()->slices_get();
        switch((e_DOMINANCE)dimension) {
            case e_row:
                dimensionLength =   pCadc->volume_get()->rows_get();
            break;
            case e_column:
                dimensionLength =   pCadc->volume_get()->cols_get();
            break;
            case e_slice:
                dimensionLength =   pCadc->volume_get()->slices_get();
            break;
        }
        powersOf2(dimensionLength, ones, highestPower);
        if(ones>1) {
            // We need to zeroPad along this dimension
            //  - first create the new padded volume
            nextHighestPower = highestPower+1;
            padTarget   = (1<<nextHighestPower);
            padAmount   = (padTarget - dimensionLength)/2;
	    
	    if((e_DOMINANCE)dimension==e_row) 
		pVl_volumePadded = new CVol<GSL_complex_float>(rows+2*padAmount, cols, slices);
	    if((e_DOMINANCE)dimension==e_column) 
		pVl_volumePadded = new CVol<GSL_complex_float>(rows, cols+2*padAmount, slices);
	    if((e_DOMINANCE)dimension==e_slice) 
		pVl_volumePadded = new CVol<GSL_complex_float>(rows, cols, slices+2*padAmount);
	    
	    *pVl_volumePadded = pVl_volume->zeroPad((e_DOMINANCE)dimension, padAmount);
	    
	    //  - now, delete the old unpadded memory
	    pCadc->volume_destruct();
	    
	    //  - and clean up the pointers
	    pCadc->volume_copyConstruct(pVl_volumePadded);
	    
	    delete	pVl_volumePadded;
	    
	    if((e_DOMINANCE)dimension == e_slice)
                pCadc->linesSliceSelect_set(inputSlices+2*padAmount);
        }
    }

    debug_pop();

}

void
C_adcPack::dataMemory_volumeShift(
    e_KSPACEDATATYPE        ae_kspace,      /*= e_normalKSpace  */
    e_FFTSHIFTDIR           ae_shiftDir     /*= e_fftshift      */
 )
{
    //
    // ARGS
    //  ae_kspace           in/opt      the kspace data set to process
    //  ae_shiftDir         in/opt      the direction in which to shift
    //                                      - note that this only has
    //                                      impact along uneven-lengthed
    //                                      dimensions.
    //
    // DESC
    //  Dispatching method for ifft/fft shift operations.
    //
    // PRECONDITIONS
    //  o A valid volume must have been extracted and should be ready for
    //    processing.
    //          - valid in this context implies that any dimensions with
    //          non-power-of-2 lengths have been appropriately zero padded.
    //
    // POSTCONDITIONS
    //  o This operation is "destructive" from the C_adc point of view!
    //      The "extracted volume" member is replaced with its shift.
    //
    // HISTORY
    // 11 September 2003
    //  o Initial design and coding.
    //
    // 01 October 2003
    //	o Incorporated volume object class.
    //
    // 04 March 2004
    // 	o b_shiftInPlace processing
    //

    C_adc*                      pCadc;

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc   = pCadc_kSpace;
        break;
        case e_phaseCorrectedKSpace:
            pCadc   = pCadc_phaseCorrected;
        break;
    }
    
    CVol<GSL_complex_float>*	pVl_volume		= pCadc->volume_get();	
    
    if(!b_shiftInPlace_get()) {
	CVol<GSL_complex_float>*        pVl_volumeShifted       = new CVol<GSL_complex_float>(
                                                         pVl_volume->rows_get(),
							 pVl_volume->cols_get(),
							 pVl_volume->slices_get()
								    );
	*pVl_volumeShifted	= pVl_volume->shift(ae_shiftDir);
	//  - now, delete the old shifted memory
	pCadc->volume_destruct();
	    
	//  - and clean up the pointers
	pCadc->volume_copyConstruct(pVl_volumeShifted);
	delete	pVl_volumeShifted;
    } else {
	int		pivotOffset = (ae_shiftDir == e_ifftshift ? -1 : +1);
	pVl_volume->shiftInPlace(pivotOffset);
    }
}

void
C_adcPack::dataMemory_volumePreprocess(	
				int		    a_echoIndex,
    				int		    a_echoTarget,
				int		    a_repetitionIndex,
				int		    a_repetitionTarget,
    				e_KSPACEDATATYPE    ae_kspace
							/*= e_normalKSpace*/) {
    //
    // ARGS
    //	a_echoIndex		in		current echo in loop
    //	a_echoTarget		in		target echo passed by system
    //	a_repetitionIndex	in		current repetition in loop
    //	a_repetitionTarget	in		target rep. passed by system
    //  ae_kspace           	in/opt      	the kspace data set to process
    //
    // DESC
    //	This method provides an entry point from the main top-level
    //	processing thread to implement some final echo/target specific
    //	preprocessing before starting the reconstruction.
    //
    //	It arose from a need to have different MRI parameters (as a function
    //	of echo) saved in MGH format files.
    //
    // PRECONDITIONS
    //
    // POSTCONDITIONS
    //	o Entry to C_IO class-specific pre-processing
    //
    // HISTORY
    // 24 June 2004
    //	o Initial design and coding.
    //
    
    C_adc*                      pCadc;

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc   = pCadc_kSpace;
        break;
        case e_phaseCorrectedKSpace:
            pCadc   = pCadc_phaseCorrected;
        break;
    }
    pCadc->volume_preprocess(	a_echoIndex,		a_echoTarget, 
    				a_repetitionIndex, 	a_repetitionTarget);
		
}        

void
C_adcPack::dataMemory_volumeifft(
    e_KSPACEDATATYPE    ae_kspace           /*  = e_normalKSpace    */
) {
    //
    // ARGS
    //  ae_kspace           in/opt      the kspace data set to process
    //
    // DESC
    //  Dispatching method for inverse FFT operation.
    //
    // PRECONDITIONS
    //  o Zero padding.
    //  o (i)fft shifting.
    //
    // POSTCONDITIONS
    //  o This operation is "destructive" from the C_adc point of view!
    //      The "extracted volume" member is replaced with its ifft.
    //
    // HISTORY
    // 12 September 2003
    //  o Initial design and coding.
    //
    // 25 May 2006
    //  o Added logic for 2D volumetric iFFT
    //

    C_adc*                      pCadc;
    int                         slices;

    switch(ae_kspace) {
        case e_normalKSpace:
            pCadc   = pCadc_kSpace;
        break;
        case e_phaseCorrectedKSpace:
            pCadc   = pCadc_phaseCorrected;
        break;
    }
    slices					= pCadc->linesSliceSelect_get();
    CVol<GSL_complex_float>*	pVl_volume	= pCadc->volume_get();
        
    bool        b_MKLwrap   = true;
    if(flag3D)
    	pVl_volume->fft3D( e_inverse, b_MKLwrap);
    else
	pVl_volume->fft2D( e_slice, e_inverse, b_MKLwrap);
}

CVol<GSL_complex_float>*
C_adcPack::dataMemory_volumeGet(       
    e_KSPACEDATATYPE    ae_kspace       /* = e_normalKSpace*/)
{
    //
    // ARGS
    //  ae_kspace           in/opt      the kspace data set to process
    //
    // DESC
    //  Return a pointer to the selected volume.
    //
    // HISTORY
    // 04 November 2003
    //  o Due to forward declarations, this orignally in-line function
    //	  was made non in-line.
    //

    switch(ae_kspace) {
        case e_normalKSpace:
	    return pCadc_kSpace->volume_get();
	break;
	case e_phaseCorrectedKSpace:
	    return pCadc_phaseCorrected->volume_get();
	break;
    }
}

bool
C_adcPack::dataMemory_volumeSave(
        string          astr_fileName,
        e_IOTYPE        e_iotype) {
    //
    // ARGS
    //	astr_fileName	in		file to process
    //  e_iotype        in              type of save operation to perform
    //
    // DESC
    //  A generic front end to deeper level save routines.
    //
    //  Saves a volume in memory in a variety of formats (as spec'd by the e_iotype)
    //
    // PRECONDITIONS
    //  o Assumes operations on the kSpace volume.
    //
    // HISTORY
    // 12 Decemeber 2003
    //  o Initial design and coding.
    //

    pCadc_kSpace->e_iotype_set(e_iotype);
    pCadc_kSpace->save(astr_fileName);
}

bool
C_adcPack::dataMemory_volumeSave(
    string	        astr_fileName
) {
    // ARGS
    //	astr_fileName	in		file to process
    //
    // DESC
    //	Saves an extracted volume in native binary (CMatrix) format.
    //
    // HISTORY
    // 04 November 2003
    //  o Due to forward declarations, this orignally in-line function
    //	  was made non in-line.
    //

    pCadc_kSpace->volume_get()->saveBinary(astr_fileName);
    return true;
}

bool
C_adcPack::dataMemory_volumeLoad(
    string	        astr_fileName
) {
    // ARGS
    //	astr_fileName	in		file to process
    //
    // DESC
    //	Loads an extracted volume in native binary (CMatrix) format.
    //
    // HISTORY
    // 04 November 2003
    //  o Due to forward declarations, this orignally in-line function
    //	  was made non in-line.
    //

    pCadc_kSpace->volume_get()->loadBinary(astr_fileName);
    return true;
}

bool
C_adcPack::dataMemory_volumeSaveReal(
    string	        astr_fileName
) {
    // ARGS
    //	astr_fileName	in		file to process
    //
    // DESC
    //	Saves the real component of an extracted volume.
    //
    // HISTORY
    // 04 November 2003
    //  o Due to forward declarations, this orignally in-line function
    //	  was made non in-line.
    //

    pCadc_kSpace->e_iotype_set(e_real);
    pCadc_kSpace->save(astr_fileName);
    return true;
}


bool
C_adcPack::dataMemory_volumeSaveImag(
    string	        astr_fileName
) {
    // ARGS
    //	astr_fileName	in		file to process
    //
    // DESC
    //	Saves the imaginary component of an extracted volume.
    //
    // HISTORY
    // 04 November 2003
    //  o Due to forward declarations, this orignally in-line function
    //	  was made non in-line.
    //
    
    pCadc_kSpace->e_iotype_set(e_imaginary);
    pCadc_kSpace->save(astr_fileName);
    return true;
}


bool 
C_adcPack::dataMemory_volumeSaveNorm(
    string	        astr_fileName
) {
    // ARGS
    //	astr_fileName	in		file to process
    //
    // DESC
    //	Saves the normal of an extracted volume.
    //
    // HISTORY
    // 04 November 2003
    //  o Due to forward declarations, this orignally in-line function
    //	  was made non in-line.
    //
    
    pCadc_kSpace->e_iotype_set(e_magnitude);
    pCadc_kSpace->save(astr_fileName);
    return true;
}

//
//\\\***
// C_adcPack_mgh definitions ****>>>>
/////***
//

//
// constructor / destructor block
//
C_adcPack_mgh::C_adcPack_mgh(
	const string            astr_baseFileName,
	C_dimensionLists*       apC_dimension,
        const bool              a_isData3D,
	int                     a_echoTarget,
	int                     a_repetitionTarget,
	int                     a_channelTarget) :
    C_adcPack(astr_baseFileName, apC_dimension, a_isData3D, 
	      a_echoTarget, a_repetitionTarget, a_channelTarget)
{
    //
    // ARGS
    //  astr_baseFilename               in              path and base name of
    //                                                          raw data
    //  apC_dimension                   in              an amalgamated class
    //                                                          describing the
    //                                                          dimensions of
    //                                                          the scan
    //                                                          space
    //  a_isData3D                      in              is this a 3D scan?
    //
    // DESC
    //  C_adcPack constructor.
    //
    // 	Effectively, this falls through to the base class constructor, with the
    //	additional sub class parsing of the meta options file for the
    //	vox2ras and MRIParams data.
    //
    // PRECONDITIONS
    // o The options file does not contain the matrix, but the name of
    //	 another file which contains the matrix.
    //		The search term is "MGH_vox2ras"
    //
    // POSTCONDITIONS
    // o There is some residual redundancy in the astr_baseFileName and apC_dimension
    //	 parameters.
    // o Note that this method will create an underlying IO class. 
    // o If the options file does not contain mention of the mgh relevant matrices, then
    //	 construction aborts with an error.   
    //
    // HISTORY
    // 07 October 2003
    //	o Initial design and coding.
    //
    
        
    debug_push("C_adcPack_mgh(...)");

    CMatrix<double>*			pM_vox2ras;
    CMatrix<double>*			pV_MRIParams;
    
    string  str_vox2rasFileName         = "";
    string  str_ADCbaseDirectory        = "";
    string  str_mriParamsFileName       = "";
    string  str_optionsFileName	        = apC_dimension->str_optionsFileName_get();
    
    C_scanopt                   cso_optionsFile(    str_optionsFileName, e_EquLink);
    string                      str_value;

    // Parse the optionsFile for MGH_vox2ras
    if(cso_optionsFile.scanFor("ADCbaseDirectory",      &str_value))
        str_ADCbaseDirectory        = str_value;
    if(cso_optionsFile.scanFor("MGH_vox2ras",    &str_value)) {
        str_vox2rasFileName        = str_value;
	pM_vox2ras	= new CMatrix<double>((char*)	
	                 (str_ADCbaseDirectory+"/"+str_vox2rasFileName).c_str());
    } else 
	error("Could not find MGH_vox2ras variable in options file.");
    
    // Parse the optionsFile for MGH_MRIParameters
    if(cso_optionsFile.scanFor("MGH_MRIParameters",    &str_value)) {
        str_mriParamsFileName       = str_value;
	pV_MRIParams	= new CMatrix<double>((char*)	
	                 (str_ADCbaseDirectory+"/"+str_mriParamsFileName).c_str());
    } else 
	error("Could not find MGH_MRIParameters variable in options file.");
    
    CMatrix<int>		M_dimensionStructure(1, 5);
    CMatrix<int>                M_unity(1, 5, 1);
    
    //
    // Create the actual data holding objects (mgh specialisation)
    //
    
    M_dimensionStructure	= *(pC_dimension->pV_dimensionStructure_get());
    int linesPhaseCorrect       = pC_dimension->linesPhaseCorrect_get();
    pCadc_kSpace                = new C_adc_mgh(this,
	                                        M_dimensionStructure,
						*pM_vox2ras,
						*pV_MRIParams);
    if(b_phaseCorrect_get()) {
	if(linesPhaseCorrect>0) {
	    M_dimensionStructure(0, e_readOut)      = linesPhaseCorrect;
	    pCadc_phaseCorrected    = new C_adc_mgh(this,
	                                        M_dimensionStructure,
						*pM_vox2ras,
						*pV_MRIParams);
	} else {
	    pCadc_phaseCorrected    = new C_adc_mgh(this,
	                                        M_unity,
						*pM_vox2ras,
						*pV_MRIParams);
	}
    }	
    // Synchronise any relevant variables with the IO object
    pCadc_kSpace->pCIO_get()->envSynchronise(this);
    
    debug_pop();
    
}
 
C_adcPack_mgh::~C_adcPack_mgh() {
    // 
    // Destructor
    //	
    //	Basically falls through to the base class destructor.
    //
    // HISTORY
    // 07 October 2003
    //	o Initial design and coding.
    //
        
}

//
//\\\***
// C_adcPack_analyze75 definitions ****>>>>
/////***
//

//
// constructor / destructor block
//
C_adcPack_analyze75::C_adcPack_analyze75(      
	const string            astr_baseFileName,
	C_dimensionLists*       apC_dimension,
        const bool              a_isData3D,
	int                     a_echoTarget,
	int                     a_repetitionTarget,
	int                     a_channelTarget) :
    C_adcPack(astr_baseFileName, apC_dimension, a_isData3D, 
	      a_echoTarget, a_repetitionTarget, a_channelTarget)
{
    //
    // ARGS
    //  astr_baseFilename               in              path and base name of
    //                                                          raw data
    //  apC_dimension                   in              an amalgamated class
    //                                                          describing the
    //                                                          dimensions of
    //                                                          the scan
    //                                                          space
    //  a_isData3D                      in              is this a 3D scan?
    //
    // DESC
    //  C_adcPack constructor.
    //
    // 	Effectively, this falls through to the base class constructor, with the
    //	additional sub class parsing of the meta options file for the
    //	vox2ras and MRIParams data.
    //
    // PRECONDITIONS
    // o The options file does not contain the matrix, but the name of
    //	 another file which contains the matrix.
    //		The search term is "MGH_vox2ras"
    //
    // POSTCONDITIONS
    // o Note that this method will create an underlying IO class. 
    // o If the options file does not contain mention of the mgh relevant matrices, then
    //	 construction aborts with an error.
    //
    // HISTORY
    // 07 October 2003
    //	o Initial design and coding.
    //
    // 24 October 2003
    //	o Added intensity scale factor to core class definition.
    //
    
        
    debug_push("C_adcPack_analyze75(...)");

    CMatrix<double>*			pV_voxelDimensions;
    short				s_orientation           = 0;;
    double				v_intensityScale	= 1e12;
    bool				b_readOutFlip		= false;
    
    string  str_voxelDimensionFileName  = "";
    string  str_ADCbaseDirectory        = "";
    string  str_optionsFileName	        = apC_dimension->str_optionsFileName_get();
    
    C_scanopt                   cso_optionsFile(    str_optionsFileName, e_EquLink);
    string                      str_value;

    // Parse the optionsFile for A75_voxelDimensions
    if(cso_optionsFile.scanFor("ADCbaseDirectory",      &str_value))
        str_ADCbaseDirectory        = str_value;
    if(cso_optionsFile.scanFor("A75_voxelDimensions",    &str_value)) {
        str_voxelDimensionFileName  = str_value;
	pV_voxelDimensions	    = new CMatrix<double>((char*)	
	                 (str_ADCbaseDirectory+"/"+str_voxelDimensionFileName).c_str());
    } else 
	error("Could not find A75_voxelDImensions variable in options file.");
    
    // Parse the optionsFile for orientation
    if(cso_optionsFile.scanFor("orientation",    &str_value)) {
	s_orientation		    = atoi(str_value.c_str());
    } else 
	error("Could not find orientation variable in options file.");
    
    // Parse the optionsFile for intensityScale
    if(cso_optionsFile.scanFor("intensityScale",    &str_value)) {
	v_intensityScale	    = atof(str_value.c_str());
    } else 
	error("Could not find intensityScale variable in options file.");

    // Parse the optionsFile for readOutFlip
    if(cso_optionsFile.scanFor("readOutFlip",    &str_value)) {
	b_readOutFlip	= atoi(str_value.c_str());
    }     
    
    CMatrix<int>		M_dimensionStructure(1, 5);
    CMatrix<int>                M_unity(1, 5, 1);
    
    //
    // Create the actual data holding objects (anlayze75 specialisation)
    //
    
    M_dimensionStructure	= *(pC_dimension->pV_dimensionStructure_get());
    int linesPhaseCorrect       = pC_dimension->linesPhaseCorrect_get();

    pCadc_kSpace                = new C_adc_analyze75(
	                                        this,
	                                        M_dimensionStructure,
						*pV_voxelDimensions,
						s_orientation,
						v_intensityScale,
						b_readOutFlip);

    if(b_phaseCorrect_get()) {
	if(linesPhaseCorrect>0) {
	    M_dimensionStructure(0, e_readOut)      = linesPhaseCorrect;
	    pCadc_phaseCorrected    = new C_adc_analyze75(
	                                        this,
	                                        M_dimensionStructure,
						*pV_voxelDimensions,
						s_orientation,
						v_intensityScale);
	} else {
	    pCadc_phaseCorrected    = new C_adc_analyze75(
	                                        this,
	                                        M_unity,
						*pV_voxelDimensions,
						s_orientation,
						v_intensityScale);
	}	
    }
    
    // Synchronise any relevant variables with the IO object
    pCadc_kSpace->pCIO_get()->envSynchronise(this);
    
    debug_pop();
    
}
 
C_adcPack_analyze75::~C_adcPack_analyze75() {
    // 
    // Destructor
    //	
    //	Basically falls through to the base class destructor.
    //
    // HISTORY
    // 07 October 2003
    //	o Initial design and coding.
    //
    
    
}
