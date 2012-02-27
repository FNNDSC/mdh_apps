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

#include "math_misc.h"
#include "machine.h"

#include "c_adcpack.h"

using namespace std;
using namespace mdh;

//
//\\\***
// C_adc definitions ****>>>>
/////***
//

void
C_adc::debug_push(
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

    if(stackDepth_get() >= C_adc_STACKDEPTH-1)
        error(  "Out of str_proc stack depth");
    stackDepth_set(stackDepth_get()+1);
    str_proc_set(stackDepth_get(), astr_currentProc);
}

void
C_adc::debug_pop() {
    //
    // DESC
    //  "pop" the stack. Since the previous name has been
    //  overwritten, there is no restoration, per se. The
    //  only important parameter really is the stackDepth.
    //

    stackDepth_set(stackDepth_get()-1);
}

void
C_adc::error(
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
    cerr << "\tC_adc object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc << "\n";
    cerr << "\t" << astr_msg << "\n";
    cerr << "Throwing an exception to (this) with code " << code << "\n\n";
    throw(this);
}

void
C_adc::warn(
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
    cerr << "\tC_adc object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc << "\n";
    cerr << "\t" << astr_msg << "(code: " << code << ")\n";
}

void
C_adc::function_trace(
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
            cerr << "\nC_adc`" << str_name;
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
C_adc::core_construct(
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
    // 22 September 2003
    //	o pVl_extracted.
    //
    // 30 September 2003
    //	o Removed pVl_extracted. volume must be explicitly constructed!
    //
    // 04 March 2004
    //	o Temp toggle switch, b_libCMatrixUse
    //

    str_name                    = astr_name;
    id                          = a_id;
    iter                        = a_iter;
    verbosity                   = a_verbosity;
    warnings                    = a_warnings;
    stackDepth                  = a_stackDepth;
    str_proc[stackDepth]        = astr_proc;

    str_obj                     = "C_adc";
    
    b_libCMatrixUse		= true;
    
}

C_adc::C_adc(
	C_adcPack*			apParent,
	CMatrix<int>&                   aM_spaceVolume
) {
    //
    // ARGS
    //  apParent			in		pointer back to parent
    //  aM_kSpaceVolume                 in              dimensions of data space
    //                                                          see
    //                                                          e_SCANDIMENSION
    //                                                          for enumeration
    // DESC
    //  C_adc constructor.
    //
    // POSTCONDITIONS
    // o Internal data structures are created and initialized according to
    //   passed parameters.
    //
    // HISTORY
    // 15 May 2003
    //  o Initial design and coding.
    //
    // 02 March 2004
    //	o Data index re-ordering.
    //
    // 04 March 2004
    //	o libCMatrix structures
    //

    core_construct();
    stackDepth = 0;
    debug_push("C_adc");
    
    pParent			= apParent;
    pCIO			= NULL;

    linesReadOut                = aM_spaceVolume(0, e_readOut);
    linesPhaseEncode            = aM_spaceVolume(0, e_phaseEncode);
    linesSliceSelect            = aM_spaceVolume(0, e_sliceSelect);
    numRepetitions              = aM_spaceVolume(0, e_numRepetitions);
    numEchoes                   = aM_spaceVolume(0, e_numEchoes);

    //aM_spaceVolume.print("spaceVolume");
    //cout << endl;
    //cout << "linesReadOut\t\t"		<< linesReadOut		<< endl;
    //cout << "linesPhaseEncode\t"	<< linesPhaseEncode	<< endl;
    //cout << "linesSliceSelect\t"	<< linesSliceSelect	<< endl;
    //cout << "numRepetitions\t\t"	<< numRepetitions	<< endl;
    //cout << "numEchoes\t\t"		<< numEchoes		<< endl;

    complex<float>              z_init(0, 0);
    bool                        b_init          = false;
    unsigned long               ul_init         = 0;
    int                         i_init          = 0;

    // First create the data structures
    if(!b_libCMatrixUse) {
	pAz_data		= new MArray<complex<float>, 5>(
                                                        linesReadOut,
                                                        linesPhaseEncode,
                                                        linesSliceSelect,
                                                        numRepetitions,
                                                        numEchoes
                                                        );
	pAz_data->setBase(0, 0, 0, 0, 0);
	(*pAz_data)         = z_init;
    } else {
	pMz_data		=  new CVol5D<GSL_complex_float>(
                                                        linesReadOut,
                                                        linesPhaseEncode,
                                                        linesSliceSelect,
                                                        numRepetitions,
                                                        numEchoes
                                                        );
    }
    
    if(pParent->b_packAdditionalData_get()) {
	pAb_reverse                 = new MArray<bool, 4>(  
                                                        linesReadOut,
	                                                linesSliceSelect,
                                                        numRepetitions,
                                                        numEchoes
                                                        );
	pAb_reverse->setBase(0, 0, 0, 0);

	pAul_timeStamp              = new MArray<unsigned long, 4>(
                                                        linesReadOut,
                                                        linesSliceSelect,
                                                        numRepetitions,
                                                        numEchoes
                                                        );
	pAul_timeStamp->setBase(0, 0, 0, 0);
	(*pAb_reverse)      = b_init;
	(*pAul_timeStamp)   = ul_init;
    }
    
    if(apParent->b_phaseCorrect_get()) {
	pA_ssIndex                  = new MArray<int, 3>(
                                                        linesSliceSelect,
                                                        numRepetitions,
                                                        numEchoes
                                                        );
	pA_ssIndex->setBase(0, 0, 0);
	(*pA_ssIndex)       = i_init;
    }

    /*
        if(!pAz_data)   error("pAz_data creation error");
        pAz_data->describeSelf(); cout << endl;
        if(!pAb_reverse)   error("pAb_reverse creation error");
        pAb_reverse->describeSelf(); cout << endl;
        if(!pAul_timeStamp)   error("pAul_timeStamp creation error");
        pAul_timeStamp->describeSelf(); cout << endl;
        if(!pA_ssIndex)   error("pA_ssIndex creation error");
        pA_ssIndex->describeSelf(); cout << endl;

    // NB NB NB
    //      The default base index is 1, not zero!
    cout << "pAz element:\t"        << (*pAz_data)(1, 1, 1, 1, 1)   << endl;
    cout << "Az element:\t"         << Az_data()(0, 0, 0, 0, 0)     << endl;
    */
    debug_pop();
    pVl_extracted	= NULL;
}

C_adc::~C_adc() {
    //
    // DESC
    //  Destructor
    //
    // PRECONDITIONS
    //	o All the pointers "delete"d here must point to already
    //	  created objects!
    //  o NB!! Make sure that pCIO has been properly created! This C_IO
    //	  object needs to be explicitly created!
    //
    // HISTORY
    // 15 May 2003
    //  o Initial design and coding.
    //
    // 31 September 2003
    //	o Added volume class.
    //
    // 05 October 2003
    //	o Added mdh::C_IO class
    //
    // 08 October 2003
    // 	o Since the constructor does *not* create the pVl_extracted volume,
    //	  it is *not* destructed here. The user must take responsibility for the
    //	   deletion of this memory!
    //
    // 04 March 2004
    //	o libCMatrix structures
    //

    debug_push("~C_adc()");
    
    if(!b_libCMatrixUse) {
	delete pAz_data;
    } else {
	delete pMz_data;
    }
    delete pAb_reverse;
    delete pAul_timeStamp;
    delete pA_ssIndex;
    
    if(pVl_extracted!=NULL)
	delete pVl_extracted;
    
    if(pCIO==NULL)
	error("It seems as though no pcIO object exists", -1);
    delete pCIO;
    
    debug_pop();

}

void
C_adc::volume_destruct()
{
    //
    // DESC
    //  Simply destroys the memory pointed to by the member extracted volume.
    //
    // PRECONDITIONS
    //  o Make sure that ppzM_volume has been initialised and created!
    //	o Make sure that pVl_extracted has been constructed!
    //
    // POSTCONDITIONS
    //  o extracted volume's memory is free'd and pointer is explicitly set
    //    to NULL.
    //
    // HISTORY
    // 11 September 2003
    //  o Initial design and coding.
    //
    // 25 September 2003
    //	o Shifted destruct away from volume arrays to volume classes.
    //

    debug_push("volume_destruct()");
    
    if(pVl_extracted != NULL)
	delete pVl_extracted;

    debug_pop();
}

void
C_adc::volume_construct() {
    //
    // DESC
    //  This method constructs a "dummy" 1x1x1 volume It's primary purpose is to 
    //  provide the functionality for constructing an emtpy data holder that can 
    //  be filled at a later stage, most likely as the result of reading a volume 
    //  from file.
    //
    // PRECONDITIONS
    //
    // POSTCONDITIONS
    //
    // HISTORY
    // 10 September 2003
    //  o Initial design and coding.
    //
    // 11 September 2003
    //  o Added ppzM_volume as a member element of class - removed
    //      from argument list.
    //
    // 23 October 2003
    //	o Adapted from volume_extract()
    //
    // 24 October 2003
    //	o Removed any dependency on pAz data lengths and structures. The volume is
    //	  simply a 1x1x1 volume.
    //

    debug_push("volume_construct(...)");
        
    // and create a new volume for the soon to be extracted data
    pVl_extracted	= new CVol<GSL_complex_float>(1, 1, 1);
    
    debug_pop();
}


void
C_adc::volume_extract(
    int                     repetition,
    int                     echo
) {
    //
    // ARGS
    //  repetition          in              the repetition to extract
    //  echo                in              the echo to extract
    //
    // DESC
    //  This method is arguably the most important function of this class - at
    //  least from an external point of view. It provides the main mechanism
    //  of extracting unpacked volumes in a format that can be processed
    //  by the CMatrix library.
    //
    // PRECONDITIONS
    //  o pVl_extracted MUST exist.
    //  o echo and repetition must be valid indices.
    //
    // POSTCONDITIONS
    //  o p_depth contains the number of slices.
    //  o the volume is returned in the ppzM_volume pointer.
    //
    // HISTORY
    // 10 September 2003
    //  o Initial design and coding.
    //
    // 11 September 2003
    //  o Added ppzM_volume as a member element of class - removed
    //      from argument list.
    //
    // 01 October 2003
    //	o Completely removed ppzM_volume. Replaced with volume class
    //	  object, pVl_extracted.
    //
    // 02 March 2004
    //	o Data index re-ordering.
    //
    // 04 March 2004
    //	o libCMatrix structures - in this case, the extracted volume evaluation reduces to
    //	  a simple call on the 5D CVol pMz_data structure.
    //

    debug_push("volume_extract(...actual data holding objects)");

    int		row		= 0;
    int         col             = 0;
    int         slice           = 0;
    
    int 	totalRows	= 0;
    int		totalCols	= 0;
    int		totalSlices	= 0;
    
    if(!b_libCMatrixUse) {
    totalRows   =   pAz_data->length(e_readOut          +1);
    totalCols   =   pAz_data->length(e_phaseEncode      +1);
    totalSlices =   pAz_data->length(e_sliceSelect      +1);
    // The '+1' here is necessary since MArrays dimensions are counted from '1'!
    
    // Keep internal data consistent!
    linesSliceSelect_set(totalSlices);

    // If some other volume has already been extracted, destroy it
    //volume_destruct();
    
    // and create a new volume for the soon to be extracted data
    pVl_extracted	= new CVol<GSL_complex_float>(totalRows, totalCols, totalSlices);
    
    // Copy data from the 5 dimensional hyper cube to the local volume
    GSL_complex_float     zv_val(0, 0);
    complex<float>  z_val;
    for(slice=0; slice<totalSlices; slice++) {
	for(row=0; row<totalRows; row++) {
            for(col=0; col<totalCols; col++) {
                z_val   = Az_data()(row, col, slice, repetition, echo);
                GSL_SET_COMPLEX(&zv_val, z_val.real(), z_val.imag());
		pVl_extracted->val(row, col, slice)	= zv_val;
            }
	}
    }
    
    } else {
	pVl_extracted	= &(pMz_data->vol3D(repetition, echo));
    }
    
    debug_pop();
}

e_IOTYPE
C_adc::e_iotype_get() const
{
    return pCIO->e_iotype_get();
}

void
C_adc::e_iotype_set(
    e_IOTYPE		ae_iotype)
{
    pCIO->e_iotype_set(ae_iotype);
}
	
bool 
C_adc::save(
    string		astr_fileName)
{
    // This "reconstruct" is necessary to keep memory handling clean
    pCIO->volume_reconstruct(pVl_extracted);
    pCIO->save(astr_fileName);
}
	
bool 
C_adc::load(
    string		astr_fileName)
{
    // This "reconstruct" is necessary to keep memory handling clean
    pCIO->volume_reconstruct(pVl_extracted);
}

void
C_adc::volume_preprocess(
	int		    a_echoIndex,
    	int		    a_echoTarget,
	int		    a_repetitionIndex,
	int		    a_repetitionTarget) {
    //
    // ARGS
    //	a_echoIndex	
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
    // 25 June 2004
    //	o Initial design and coding.
    
    cout << "In C_adc::volume_preprocess(...)" << endl;
    
}


//
//\\\***
// C_adc_mgh definitions ****>>>>
/////***
//

//
// constructor / destructor block
//

C_adc_mgh::C_adc_mgh(
    C_adcPack*			apParent,
    CMatrix<int>&               aM_spaceVolume,
    CMatrix<double>&            M_vox2ras,
    CMatrix<double>&            V_MRIParams
) :
C_adc(apParent, 
      aM_spaceVolume) 
{
    //
    // ARGS
    //  apParent			in		pointer back to parent
    //  aM_kSpaceVolume                 in              dimensions of data space
    //                                                          see
    //                                                          e_SCANDIMENSION
    //                                                          for enumeration
    //	M_vox2ras			in		mgh specific transformation
    //								matrix
    //	V_MRIParams			in		mgh format MRI parameters
    //
    // DESC
    //  C_adc_mgh constructor.
    //
    // POSTCONDITIONS
    // o Internal data structures are created and initialized according to
    //   passed parameters.
    //
    // HISTORY
    // 08 October 2003
    //	o Initial design and coding.
    //
    
    str_obj             = "C_adc_mgh";
    pCIO		= new C_IO_mgh(M_vox2ras, V_MRIParams);
    
}

C_adc_mgh::~C_adc_mgh() {
}
    
void
C_adc_mgh::volume_preprocess(
	int		    a_echoIndex,
    	int		    a_echoTarget,
	int		    a_repetitionIndex,
	int		    a_repetitionTarget) {
    //
    // ARGS
    //	a_echoIndex	
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
    // 25 June 2004
    //	o Initial design and coding.
    
    // Fine-tune the MRI parameters.
    CMatrix<double>*	pV_MRIparams;
    pV_MRIparams	= ((C_IO_mgh*)pCIO)->pV_MRIParams_get();
    CMatrix<double>	V_MRIParamsEcho(1, 4);
    int	echo		= 0;
    
    if(a_echoTarget == -1)	
    	echo	= a_echoIndex;
    else
    	echo	= a_echoTarget;
    
    V_MRIParamsEcho(0)	= pV_MRIparams->val(0); 
    V_MRIParamsEcho(1)	= pV_MRIparams->val(1); 
    V_MRIParamsEcho(3)	= pV_MRIparams->val(2);
    V_MRIParamsEcho(2)	= pV_MRIparams->val(3+echo);
    
    ((C_IO_mgh*)pCIO)->pV_MRIParamsEcho_copy(V_MRIParamsEcho);
}
    
    
//
//\\\***
// C_adc_analyze75 definitions ****>>>>
/////***
//

//
// constructor / destructor block
//

C_adc_analyze75::C_adc_analyze75(
    C_adcPack*			apParent,
    CMatrix<int>&               aM_spaceVolume,
    CMatrix<double>&            aV_voxelDimensions,
    int                         a_orientation,
    double		        av_intensityScale,
    bool			ab_readOutFlip
) :
C_adc(apParent, 
      aM_spaceVolume) 
{
    //
    // ARGS
    //	apParent			in		pointer back to parent
    //  aM_kSpaceVolume                 in              dimensions of data space
    //                                                          see
    //                                                          e_SCANDIMENSION
    //                                                          for enumeration
    //	aV_voxelDimensions		in		voxel dimensions in mm
    //	a_orientation			in		orientation of slice
    //	av_intensityScale		in		factor by which each voxel
    //								value is multiplied
    //	ab_readOutFlip			in		flag for "inverting" the
    //								save order for
    //								readout dim
    //
    // DESC
    //  C_adc_analyze75 constructor.
    //
    // POSTCONDITIONS
    // o Internal data structures are created and initialized according to
    //   passed parameters.
    //
    // HISTORY
    // 13 October 2003
    //	o Initial design and coding.
    //
    // 24 October 2003
    //	o Added v_intensityScale
    //
    // 28 June 2004
    //	o Added ab_readOutFlip
    //
    
    str_obj             = "C_adc_analyze75";
    pCIO		= new C_IO_analyze75(	aV_voxelDimensions, 
    						a_orientation, 
    						av_intensityScale,
						ab_readOutFlip);
    
}

C_adc_analyze75::~C_adc_analyze75() {
}

void
C_adc_analyze75::volume_preprocess(
	int		    a_echoIndex,
    	int		    a_echoTarget,
	int		    a_repetitionIndex,
	int		    a_repetitionTarget) {
    //
    // ARGS
    //	a_echoIndex	
    //	a_echoIndex		in		current echo in loop
    //	a_echoTarget		in		target echo passed by system
    //	a_repetitionIndex	in		current repetition in loop
    //	a_repetitionTarget	in		target rep. passed by system
    //	ab_readOutFlip		in		flag for "inverting" the
    //							save order for
    //							readout dim
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
    // 25 June 2004
    //	o Initial design and coding.
    
    //cout << "In C_adc_analyze75::volume_preprocess(...)" << endl;
    
}





