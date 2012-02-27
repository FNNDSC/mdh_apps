/***************************************************************************
 *   Copyright (C) 2003 by Rudolph Pienaar                                 *
 *   rudolph@nmr.mgh.harvard.edu                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include <iostream>
#include <string>
#include <asch.h>

#include <scanopt.h>

using namespace std;
using namespace mdh;


//
//\\\***
// C_asch definitions ****>>>>
/////***
//

void
C_asch::debug_push(
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

    if(stackDepth_get() >= C_asch_STACKDEPTH-1)
        error(  "Out of str_proc stack depth");
    stackDepth_set(stackDepth_get()+1);
    str_proc_set(stackDepth_get(), astr_currentProc);
}

void
C_asch::debug_pop() {
    //
    // DESC
    //  "pop" the stack. Since the previous name has been
    //  overwritten, there is no restoration, per se. The
    //  only important parameter really is the stackDepth.
    //

    stackDepth_set(stackDepth_get()-1);
}

void
C_asch::error(
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
    cerr << "\tC_asch object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc << "\n";
    cerr << "\t" << astr_msg << "\n";
    cerr << "Throwing an exception to (this) with code " << code << "\n\n";
    throw(this);
}

void
C_asch::warn(
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
    cerr << "\tc_asch object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc << "\n";
    cerr << "\t" << astr_msg << "(code: " << code << ")\n";
}

void
C_asch::function_trace(
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
	    cerr << "\nStochastic World `" << str_name;
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
C_asch::core_construct(
        int             a_sliceArraySize,
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

    str_name                    = astr_name;
    id                          = a_id;
    iter                        = a_iter;
    verbosity                   = a_verbosity;
    warnings                    = a_warnings;
    stackDepth                  = a_stackDepth;
    str_proc[stackDepth]        = astr_proc;

    str_obj                     = "C_asch";

    str_sequenceFileName        = " < void > ";
    TXFrequency                 = 0;

    sliceArraySize              = a_sliceArraySize;

    pMi_TR                      = new CMatrix<int>      (1, sliceArraySize);
    pMi_TI                      = new CMatrix<int>      (1, sliceArraySize);
    pMi_TE                      = new CMatrix<int>      (1, sliceArraySize);

    pMv_slicePositionSag        = new CMatrix<double>   (1, sliceArraySize);
    pMv_slicePositionCor        = new CMatrix<double>   (1, sliceArraySize);
    pMv_slicePositionTra        = new CMatrix<double>   (1, sliceArraySize);
    pMv_sliceNormalSag          = new CMatrix<double>   (1, sliceArraySize);
    pMv_sliceNormalCor          = new CMatrix<double>   (1, sliceArraySize);
    pMv_sliceNormalTra          = new CMatrix<double>   (1, sliceArraySize);

    pMv_thickness               = new CMatrix<double>   (1, sliceArraySize);
    pMv_phaseFOV                = new CMatrix<double>   (1, sliceArraySize);
    pMv_readoutFOV              = new CMatrix<double>   (1, sliceArraySize);
    pMv_inPlaneRot              = new CMatrix<double>   (1, sliceArraySize);

    baseResolution              = 0;
    phaseEncodingLines          = 0;
    v_phaseResolution           = 0.0;
    partitions                  = 0;

    v_flipAngleDegree           = 0.0;

    pMv_centerSlicePosition     = new CMatrix<double>   (1, 3);

}

C_asch::~C_asch() {
   //
   // DESC
   //   Destructor
   //
   // HISTORY
   // 05 May 2003
   //   o Initial design and coding.
   //

   delete pMi_TR;
   delete pMi_TI;
   delete pMi_TE;

   delete pMv_slicePositionSag;
   delete pMv_slicePositionCor;
   delete pMv_slicePositionTra;

   delete pMv_sliceNormalSag;
   delete pMv_sliceNormalCor;
   delete pMv_sliceNormalTra;

   delete pMv_thickness;
   delete pMv_phaseFOV;
   delete pMv_readoutFOV;
   delete pMv_inPlaneRot;

   delete pMv_centerSlicePosition;

}

C_asch::C_asch(
        string          astr_aschFileName
) {
    //
    // ARGS
    //  astr_aschFileName       in              *.asc header filename to process
    //
    // DESC
    //  C_asch constructor.
    //
    // PRECONDITIONS
    //  o astr_aschFileName *must* be a valid and complete filename
    //
    // POSTCONDITIONS
    //  o Internal class fields are filled with values read from asc file.
    //  o Values that are not found are initialized to zero.
    //
    // HISTORY
    // 03 May 2003
    //  o Initial design and coding.
    //

    stackDepth = 0;
    debug_push("C_asch");

    C_scanopt   cso_measAscFile(astr_aschFileName, e_EquLink);
    string      str_token;
    string      str_value;

    //cout << "Scanning..." << endl;

    sliceArraySize = 0;
    // First, scan for the slice array size and then call a core_construct
    if(cso_measAscFile.scanFor("sSliceArray.lSize",     &str_value))
        sliceArraySize  = atoi(str_value.c_str());

    if(sliceArraySize <= 0)
        error("Invalid sliceArraySize!", 1);
    core_construct(sliceArraySize);

    if(cso_measAscFile.scanFor("tSequenceFileName",     &str_value))
        str_sequenceFileName    = str_value;
    if(cso_measAscFile.scanFor("sTXSPEC.lfrequency",    &str_value))
        TXFrequency             = atoi(str_value.c_str());

    for(int slice=0; slice<sliceArraySize; slice++) {
        char    pch_slice[1024];
	sprintf(pch_slice, "[%d]", slice);
	string  str_suffix      = pch_slice;
	string  str_token       = "alTR" + str_suffix;
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMi_TR->val(0, slice) =             atoi(str_value.c_str());
	str_token      = "alTI" + str_suffix;
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMi_TI->val(0, slice) =             atoi(str_value.c_str());
	str_token      = "alTE" + str_suffix;
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMi_TE->val(0, slice) =             atoi(str_value.c_str());

	sprintf(pch_slice, "sSliceArray.asSlice[%d].", slice);
	string  str_prefix      = pch_slice;
        str_token               = str_prefix + "sPosition.dSag";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_slicePositionSag->val(0, slice) =       atof(str_value.c_str());
	str_token               = str_prefix + "sPosition.dCor";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_slicePositionCor->val(0, slice) =       atof(str_value.c_str());
	str_token               = str_prefix + "sPosition.dTra";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_slicePositionTra->val(0, slice) =       atof(str_value.c_str());

	str_token               = str_prefix + "sNormal.dSag";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_sliceNormalSag->val(0, slice)   =         atof(str_value.c_str());
	str_token               = str_prefix + "sNormal.dCor";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_sliceNormalCor->val(0, slice)   =         atof(str_value.c_str());
	str_token               = str_prefix + "sNormal.dTra";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_sliceNormalTra->val(0, slice)   =         atof(str_value.c_str());

	str_token               = str_prefix + "dThickness";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_thickness->val(0, slice)        =       atof(str_value.c_str());
	str_token               = str_prefix + "dPhaseFOV";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_phaseFOV->val(0, slice)         =       atof(str_value.c_str());
	str_token               = str_prefix + "dReadoutFOV";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_readoutFOV->val(0, slice)       =       atof(str_value.c_str());
	str_token               = str_prefix + "dInPlaneRot";
	if(cso_measAscFile.scanFor(str_token,           &str_value))
	    pMv_inPlaneRot->val(0, slice)       =       atof(str_value.c_str());
    }

    if(cso_measAscFile.scanFor("sKSpace.lBaseResolution",       &str_value))
        baseResolution                          =       atoi(str_value.c_str());
    if(cso_measAscFile.scanFor("sKSpace.lPhaseEncodingLines",   &str_value))
        phaseEncodingLines                      =       atoi(str_value.c_str());
    if(cso_measAscFile.scanFor("sKSpace.dPhaseResolution",      &str_value))
        v_phaseResolution                       =       atof(str_value.c_str());
    if(cso_measAscFile.scanFor("sKSpace.lPartitions",           &str_value))
        partitions                              =       atoi(str_value.c_str());
    if(cso_measAscFile.scanFor("dFlipAngleDegree",              &str_value))
        v_flipAngleDegree                       =       atof(str_value.c_str());

    // Determine center of slice position
    double      v_sag   = (pMv_slicePositionSag->val(0,0) +
                           pMv_slicePositionSag->val(0, sliceArraySize-1)) / 2;
    double      v_cor   = (pMv_slicePositionCor->val(0,0) +
                           pMv_slicePositionCor->val(0, sliceArraySize-1)) / 2;
    double      v_tra   = (pMv_slicePositionTra->val(0,0) +
                           pMv_slicePositionTra->val(0, sliceArraySize-1)) / 2;

    pMv_centerSlicePosition->val(0, 0)  = v_sag;
    pMv_centerSlicePosition->val(0, 1)  = v_cor;
    pMv_centerSlicePosition->val(0, 2)  = v_tra;

    debug_pop();

}

void
C_asch::print(void) {
    //
    // DESC
    //  Print the contents of the class members
    //
    // HISTORY
    // 05 May 2003
    //  o Initial design and coding.
    //

    cout << "ASHC header class dump"    << endl;

    cout << "Sequence file name\t = "     << str_sequenceFileName         << endl;
    cout << "TXFrequency\t\t = "          << TXFrequency                  << endl;
    cout << "sliceArraySize\t\t = "       << sliceArraySize               << endl;

    pMi_TR->print("TR\t\t\t");                                  cout << endl;
    pMi_TI->print("TI\t\t\t");                                  cout << endl;
    pMi_TE->print("TE\t\t\t");                                  cout << endl;

    pMv_slicePositionSag->print("slicePositionSag\t");          cout << endl;
    pMv_slicePositionCor->print("slicePositionCor\t");          cout << endl;
    pMv_slicePositionTra->print("slicePositionTra\t");          cout << endl;

    pMv_sliceNormalSag->print("sliceNormalSag\t\t");            cout << endl;
    pMv_sliceNormalCor->print("sliceNormalCor\t\t");            cout << endl;
    pMv_sliceNormalTra->print("sliceNormalTra\t\t");            cout << endl;

    pMv_thickness->print("thickness\t\t");                      cout << endl;
    pMv_phaseFOV->print("phaseFOV\t\t");                        cout << endl;
    pMv_readoutFOV->print("readoutFOV\t\t");                    cout << endl;
    pMv_inPlaneRot->print("inPlaneRot\t\t");                    cout << endl;

    pMv_centerSlicePosition->print("centerSlicePosition\t");    cout << endl;

    cout << "baseResolution\t\t = "     << baseResolution               << endl;
    cout << "phaseEncodingLines\t = "   << phaseEncodingLines           << endl;
    cout << "phaseResolution\t\t = "    << v_phaseResolution            << endl;
    cout << "partitions\t\t = "         << partitions                   << endl;
    cout << "flipAngleDegree\t\t = "    << v_flipAngleDegree            << endl;

}


