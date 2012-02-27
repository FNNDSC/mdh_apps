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
#include <c_io.h>
using namespace std;

#include "c_adcpack.h"
using namespace mdh;

#include "math_misc.h"

//
//\\\***
// C_IO definitions ****>>>>
/////***
//

void
C_IO::debug_push(
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

    if(stackDepth_get() >= C_IO_STACKDEPTH-1)
        error(  "Out of str_proc stack depth");
    stackDepth_set(stackDepth_get()+1);
    str_proc_set(stackDepth_get(), astr_currentProc);
}

void
C_IO::debug_pop() {
    //
    // DESC
    //  "pop" the stack. Since the previous name has been
    //  overwritten, there is no restoration, per se. The
    //  only important parameter really is the stackDepth.
    //

    stackDepth_set(stackDepth_get()-1);
}

void
C_IO::error(
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
    cerr << "\tC_IO object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc << "\n";
    cerr << "\t" << astr_msg << "\n";
    cerr << "Throwing an exception to (this) with code " << code << "\n\n";
    throw(this);
}

void
C_IO::warn(
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
    cerr << "\tC_IO object `" << str_name << "' (id: " << id << ")\n";
    cerr << "\tCurrent function: " << str_obj << "::" << str_proc << "\n";
    cerr << "\t" << astr_msg << "(code: " << code << ")\n";
}

void
C_IO::function_trace(
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
            cerr << "\nC_IO`" << str_name;
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
C_IO::core_construct(
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
    // 04 November 2003
    //  o Added e_byteOrder
    //

    str_name                    = astr_name;
    id                          = a_id;
    iter                        = a_iter;
    verbosity                   = a_verbosity;
    warnings                    = a_warnings;
    stackDepth                  = a_stackDepth;
    str_proc[stackDepth]        = astr_proc;

    e_byteOrder                 = e_littleEndian;

    str_obj                     = "C_IO";
    
}

C_IO::C_IO() {
    //
    // DESC
    //	This is a parameterless constructor. Used in array creation, 
    //	or when an IO object needs to be constructed without a core
    //	volume to wrap around.
    //
    // POSTCONDITIONS
    //	o A "dummy" 1x1x1 volume is constructed. This volume must be
    //	  deleted before a "real" volume can be wrapped around.
    //
    // HISTORY
    // 07 October 2003
    //	o Initial design and coding.
    //
    
    core_construct();
    pVl_extracted	= new CVol<GSL_complex_float>(1, 1, 1);
     
}

C_IO::C_IO(
    CVol<GSL_complex_float>*    apVl_extracted
) {
    //
    // ARGS
    //  pVl_extracted		in		pointer to volume to be saved
    //
    // DESC
    //  C_IO constructor.
    //
    // PRECONDITIONS
    //	o The apVl_extracted pointer *must* point to an already created volume!
    //
    // POSTCONDITIONS
    //	o The internal pointer, pVl_extracted, is copy constructed using the
    //	  passed apVl_extracted argument. No extra memory is consumed, and this
    //	  class effectively "points" to an already constructed volume.
    //
    // HISTORY
    // 06 October 2003
    //  o Initial design and coding.
    //

    stackDepth = 0;
    debug_push("C_IO");

    core_construct();
    pVl_extracted	= new CVol<GSL_complex_float>(*apVl_extracted);

    debug_pop();

}

C_IO::~C_IO() {
    //
    // DESC
    //  Destructor
    //
    // HISTORY
    // 06 October 2003
    //  o Initial design and coding.
    //
    // POSTCONDITIONS
    //	o Note that since the internal pVl_extracted is copy constructed
    //	  from a passed pointer, it *must* be deleted here. This does not
    //	  delete the actual memory, merely decrements a ref counter to the
    //	  memory.
    //

//    cout << "In base C_IO destructor" << endl;
    delete	pVl_extracted;
}

void
C_IO::volume_reconstruct(
    CVol<GSL_complex_float>*		pVl
) {
    //
    // ARGS
    //  pVl_extracted		in		pointer to volume to be saved
    //
    // DESC
    //  C_IO (re)-constructor.
    //
    // 	Each time a new volume has been "extracted" for analysis, a corresponding
    //	IO object is re-initialised around this new volume. This implies that the
    //	IO object "destructs" its original volume pointer and re-constucts this
    //	pointer around the new volume.
    //
    //  Note that the naming is a bit clumsy: the "reconstruct" has nothing to do
    //  with volume reconstruction!
    //
    // PRECONDITIONS
    //	o The apVl_extracted pointer *must* point to an already created volume!
    //	o The internal pVl_extracted *must* exist.
    //
    // POSTCONDITIONS
    //	o The internal pointer, pVl_extracted, is copy constructed using the
    //	  passed apVl_extracted argument. No extra memory is consumed, and this
    //	  class effectively "points" to an already constructed volume.
    //
    // HISTORY
    // 08 October 2003
    //  o Initial design and coding.
    //

    stackDepth = 0;
    debug_push("volume_reconstruct");

    delete		pVl_extracted;
    pVl_extracted	= new CVol<GSL_complex_float>(*pVl);

    debug_pop();


}

void
C_IO::volume_destruct()
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
C_IO::envSynchronise(
    C_adcPack*		apCadcPack
) {
    //
    // ArGS
    //	apCadcPack		in		root env object with which to
    //							synchronise
    //
    // DESC
    //	Synchronises various internal variables with "top" root level
    //	C_adcPack object.
    //
    // POSTCONDITIONS
    //	o Updates relevant IO members to corresponding values spec'd in
    //	  the top level C_adcPack object (analogous to setting/propogating
    //	  "env"/operational variables that were read in from file (and exist
    //	  in C_adcPack)).
    //
    // HISTORY
    // 04 November 2003
    //	o Initial design and coding.
    //
    
    pCadcPack		= apCadcPack;
    e_byteOrder		= pCadcPack->pc_dimension_get()->e_byteOrder_get();

}

//
//\\\***
// C_IO_mgh definitions ****>>>>
/////***
//

//
// constructor / destructor block
//

C_IO_mgh::C_IO_mgh( 
    CVol<GSL_complex_float>*          	apVl_extracted,
    CMatrix<double>&            	aM_vox2ras,
    CMatrix<double>&            	aV_MRIParams
) : C_IO(apVl_extracted) 
{
    //
    // ARGS
    //  pVl_extracted		in		pointer to volume to be saved
    //	M_vox2ras		in		vox2ras transformation matrix
    //	V_MRIParams		in		Misc MRI parameters
    //
    // DESC
    //	Constructor for the MGH format I/O.
    //
    //	Note that the base IO class is passed a pointer to the extracted volume.
    //
    // HISTORY
    // 06 October 2003
    //	o Initial design and coding.
    //
    
    str_obj             = "C_IO_mgh";    
    
    pM_vox2ras		= new CMatrix<double>(aM_vox2ras.rows_get(), 
					      aM_vox2ras.cols_get());
    pM_vox2ras->copy(aM_vox2ras);
    pV_MRIParams         = new CMatrix<double>(aV_MRIParams.rows_get(), 
					       aV_MRIParams.cols_get());
    pV_MRIParams->copy(aV_MRIParams);
    pV_MRIParamsEcho	= new CMatrix<double>(1, 4);
}

C_IO_mgh::C_IO_mgh( 
    CMatrix<double>&            aM_vox2ras,
    CMatrix<double>&            aV_MRIParams
) : C_IO()
{
    //
    // ARGS
    //	M_vox2ras		in		vox2ras transformation matrix
    //	V_MRIParams		in		Misc MRI parameters
    //
    // DESC
    //	Constructor for the MGH format I/O.
    //
    //	This is a partial constructor that is used when the target volume
    //	is unknown. Note that the parameterless "dummy" base constructor
    //	is called!
    //
    // HISTORY
    // 06 October 2003
    //	o Initial design and coding.
    //
    
    str_obj             = "C_IO_mgh";    
    
    pM_vox2ras		= new CMatrix<double>(aM_vox2ras.rows_get(), 
					      aM_vox2ras.cols_get());
    pM_vox2ras->copy(aM_vox2ras);
    pV_MRIParams         = new CMatrix<double>(aV_MRIParams.rows_get(), 
					       aV_MRIParams.cols_get());
    pV_MRIParams->copy(aV_MRIParams);
    pV_MRIParamsEcho	= new CMatrix<double>(1, 4);
}

C_IO_mgh::~C_IO_mgh() {
    delete	pM_vox2ras;
    delete	pV_MRIParams;
    delete	pV_MRIParamsEcho;
}
	
//
// Misc access block
//

bool	
C_IO_mgh::save(string astr_fileName) 
{
    //
    // ARGS
    //	astr_fileName			in		fileName to save volume to
    //
    // DESC
    //	This method is pretty much a direct port of Doug Greve's save_mgh.m matlab
    //	script. It's main purpose is to save a volume to disk in "MGH" format.
    //
    // PRECONDITIONS
    //	pM_vox2ras			member		4x4 transformation matrix
    //	pM_MRIParams			member		MRI parameters:
    //								[tr flipangle te ti]
    //	b_realImag			member		flag defining whether or not
    //								to save real or imag
    //								component of the volume.
    //
    //	o The vox2ras and MRIParams matrices are calculated externally to this
    //	  method and simply accessed as class members
    //	o Note that within this class, volumes are spec'd as
    //		[row col slice]
    //	o Eventhough MatLAB saves matrices in COLUMN-order, data is saved by this
    //	  method in ROW-order - the assumption being that data streaming directly
    //	  off the scanner is in ROW-order.
    //	o The b_realImag flag denotes which part of the complex volume to save.
    //		false:	save the real part
    //		true:	save the imag part
    //	  
    //
    // POSTCONDITIONS
    //	o The "current" volume class is saved to disk in MGH format. Note that, unlike
    //	  the original MatLAB script, this method only saves individual 3D volumes. To
    //	  save an entire pAz_data type MArray class, extract target repetitions/echoes into
    //	  volumes and save the volumes individually.
    //
    // NOTE
    //	o fwrite is used instead of fprintf since fwrite streams BINARY while fprintf
    //	  would stream formatted (human readable).
    //	o Linux on intel is *little endian*!
    //
    // SEE ALSO
    //	save_mgh.m under /space/repo/1/dev/dev/matlab/save_mgh.m
    //
    // HISTORY
    // 22 September 2003
    //	o Initial design and coding.
    //
    // 25 September 2003
    //	o Added byte swap code (from machine.h/c). Each data value is either individually swapped 
    //	  or a whole buffer is swapped using the ByteSwapN(..) functions. Be aware that the number
    //	  of items refers to the actual single byte count!
    //
    // 05 October 2003
    //	o Encapsulation within C_IO_mgh class.
    //
    // 20 October 2003
    //	o After a long and labourious debugging effort, the indices of the "vol unpack"
    //	  loops were tweaked until the final mgh files were practically byte compatible
    //	  with the same files as filtered by MatLAB. There are minor differences, though.
    //	  In a typical test example (of say a 55 MB raw data file), about 12 bytes might be
    //	  different between two 4.1 MB output mgh files (one generated by MatLAB, one by
    //    the C++ unpack code). This is most likely to small rounding errors/differences
    //	  between MatLAB fft code and MKL code.
    //
    // 12 December 2003
    //  o Added magnitude / phase saving capability.
    //
    // 25 February 2004
    //	o Added readOut crop capability - NB! dimensions need to be changed if cropped!
    //
    //
    // NOTES
    //	NB! NB! NB!
    //	tkmedit assumes a very specific dimension ordering when parsing the mgh files!
    //	as well as the dimension labelling:
    //	o ndim1:	slices		( partitions )
    //	o ndim2:	rows		( ReadOut )
    //	o ndim3:	cols		( PhaseEncode )
    //

    debug_push("save(...)");
    
    if(!pM_vox2ras->compatible(4, 4))
	error("Passed vox2ras matrix is not 4x4! Has it been properly initialised?", 1);
    
    FILE*	pFILE_stream;
    const	int		MRI_UCHAR	= 0;
    const	int		MRI_INT	        = 1;
    const	int		MRI_LONG	= 2;
    const	int		MRI_FLOAT	= 3;
    const	int		MRI_SHORT	= 4;
    const	int		MRI_BITMAP	= 5;
    const	int		MRI_TENSOR	= 6;
    
    const	int		UNUSED_SPACE_SIZE	= 256;
    const	int 		USED_SPACE_SIZE		= (3*4 + 4*3*4);
    
    int				ndim1, ndim2, ndim3;
    int				i, j, k;
    

    // Note that the "binary" spec is ignored in POSIX systems
    //	from fopen(3):
    //	This is strictly for compatibility with ANSI X3.159-1989 (``ANSI C'') and has 
    //  no effect; the ``b'' is ignored on all POSIX conforming systems, including Linux.
    if((pFILE_stream = fopen(astr_fileName.c_str(), "wb")) == NULL) 
	error("Could not open MGH file:spaceVolume " + astr_fileName, 1);
    
    // Allocate an int buffer
    char*	pch_buffer	= new char      [1024];
    short*	ps_buffer	= new short	[1024];
    int*	p_buffer	= new int       [1024];
    float*	pf_buffer	= new float     [1024];
    double*	pv_buffer	= new double    [1024];
        
    //	Dump a magic number
    p_buffer[0]		= 1;
    p_buffer[0]		= swapInt(p_buffer[0]);
    fwrite(p_buffer, sizeof(int), 1, pFILE_stream);
    
    int			readOutStart	= 0;
    int			readOutEnd	= pVl_extracted->rows_get();
    float               f_readOutScale  = 1.0;
    if(pCadcPack->b_readOutCrop_get()) {
	readOutStart	= readOutEnd / 4;
	readOutEnd	= (int) (0.75 * readOutEnd);
	f_readOutScale  = 0.5;
    }
        
    // dimension 0
    ndim1		= pVl_extracted->cols_get();
    p_buffer[0]		= ndim1;
    p_buffer[0]		= swapInt(p_buffer[0]);
        fwrite(p_buffer, sizeof(int), 1, pFILE_stream);
        
    // dimension 1
    ndim2		= (int) (pVl_extracted->rows_get() * f_readOutScale);
    p_buffer[0]		= ndim2;
    p_buffer[0]		= swapInt(p_buffer[0]);
    fwrite(p_buffer, sizeof(int), 1, pFILE_stream);
    
    // dimension 2
    ndim3		= pVl_extracted->slices_get();
    p_buffer[0]		= ndim3;
    p_buffer[0]		= swapInt(p_buffer[0]);
    fwrite(p_buffer, sizeof(int), 1, pFILE_stream);

    // dimension 3 - not used. Write a '1' to tell MatLAB this is a singleton
    //	dimension
    p_buffer[0]		= 1;
    p_buffer[0]		= swapInt(p_buffer[0]);
    fwrite(p_buffer, sizeof(int), 1, pFILE_stream);

    // Float data to follow...
    p_buffer[0]		= MRI_FLOAT;
    p_buffer[0]		= swapInt(p_buffer[0]);
    fwrite(p_buffer, sizeof(int), 1, pFILE_stream);

    // dof
    p_buffer[0]		= 1;
    p_buffer[0]		= swapInt(p_buffer[0]);
    fwrite(p_buffer, sizeof(int), 1, pFILE_stream);

    CMatrix<double>*    pM_trns3x3              = new CMatrix<double>(3, 3);
    CMatrix<double>*    pM_trns3x3sq            = new CMatrix<double>(3, 3);
    CMatrix<double>	M_trns3x3Scaled(3, 3);
    CMatrix<double>	M_delta(1, 3);
    CMatrix<double>	V_crsC(4, 1);
    CMatrix<double>	V_xyzC(4, 1);
    CMatrix<double>*	pV_xyz                  = new CMatrix<double>(3, 1);

    pM_vox2ras->matrix_remove( pM_trns3x3,
			     0, 0,
			     3, 3);

    pM_trns3x3sq->copy(*pM_trns3x3);
    pM_trns3x3sq->functionApply(sqr);

    M_delta	= pM_trns3x3sq->sum(e_column);
    M_delta.functionApply(sqrt);
    M_delta.replicate(3, e_column);

    M_trns3x3Scaled = (*pM_trns3x3) / M_delta;

    V_crsC(0, 0)	= ndim1/2;
    V_crsC(1, 0)	= ndim2/2;
    V_crsC(2, 0)	= ndim3/2;
    V_crsC(3, 0)	= 1;

    V_xyzC	= (*pM_vox2ras) * V_crsC;
    V_xyzC.matrix_remove(	pV_xyz,
				0, 0,
				3, 1);
								
    /*
    M_delta.print("M_delta");
    M_trns3x3Scaled.print("M_trns3x3Scaled");
    V_xyzC.print("V_xyzC");
    */

    // ras_good_flag
    ps_buffer[0]		= 1;
    ps_buffer[0]		= swapShort(ps_buffer[0]);
    fwrite(ps_buffer, sizeof(short), 1, pFILE_stream);

    // M_delta matrix: Only the first three values along the first row
    for(i=0; i<3; i++)
	pf_buffer[i]    = M_delta(0, i);
    ByteSwap4(pf_buffer, 3*4);
    fwrite(pf_buffer, sizeof(float), 3, pFILE_stream);

    // M_trns3x3Scaled
    int	 rows	= M_trns3x3Scaled.rows_get();
    int	 cols	= M_trns3x3Scaled.cols_get();
    for(j=0; j<cols; j++)
	for(i=0; i<rows; i++)
	    pf_buffer[j*rows+i]	= M_trns3x3Scaled(i, j);
    ByteSwap4(pf_buffer, rows*cols*4);
    fwrite(pf_buffer, sizeof(float), rows*cols, pFILE_stream);

    // V_xyzC
    rows	= pV_xyz->rows_get();
    for(i=0; i<rows; i++)
	pf_buffer[i]		= pV_xyz->val(i, 0);
    ByteSwap4(pf_buffer, rows*4);
    fwrite(pf_buffer, sizeof(float), rows, pFILE_stream);

    // Unused space
    for(i=0; i<(UNUSED_SPACE_SIZE-2)-USED_SPACE_SIZE; i++)
	pch_buffer[i]	= 0;
    fwrite(pch_buffer, sizeof(char), i, pFILE_stream);

    int bk = 0;

    // Now, finally, the actual volume itself!
    //	These indices were selected to be as byte-identical as possible to
    //	the same files as produced by MatLAB.
    delete [] pf_buffer;
    int	                bufCount        = 0;
    double              v_mag           = 0.0;
    double              v_phase         = 0.0;
    pf_buffer           = new float [ndim1 * ndim2 * ndim3];

    //
    // The core "write" loop. The nesting order of the loops, as well as the order
    //  in which the dimension sizes (ndim1, ndim2, ndim3) are written to disk, is
    //  of *critical* importance.
    //  
    //          o Whichever order is used in this nested looping, the dimension
    //            sizes should be saved in inverse order. Thus, if the loop ordering  
    //            from outer to inner is 'k, i, j', the dimension size spec is 
    //            written in  'j, i, k' order.
    //
    //          o The slice 'k' and column 'j' data is written in "inverse" order.
    //            This is because the scanner operates in LPS coordinates, while
    //            we are interested in RAS order. Thus, L becomes -R (i.e. the 
    //            slices, k, are inverted) and P becomes -A (posterior/anterior,
    //            i.e. the columns, j, are inverted).
    //
        
    for(k=pVl_extracted->slices_get()-1; k>=0; k--) {
        for(i=readOutStart; i<readOutEnd; i++) {
            for(j=pVl_extracted->cols_get()-1; j>=0; j--) {
                switch(e_iotype) {
                    case e_imaginary:
		        pf_buffer[bufCount++]	=	GSL_IMAG(pVl_extracted->val(i, j, k));
                        break;
                    case e_real:
		        pf_buffer[bufCount++]	= 	GSL_REAL(pVl_extracted->val(i, j, k));
                        break;
                    case e_magnitude:
                        v_mag = sqrt(
		            GSL_REAL(pVl_extracted->val(i, j, k)) * GSL_REAL(pVl_extracted->val(i, j, k)) +
		            GSL_IMAG(pVl_extracted->val(i, j, k)) * GSL_IMAG(pVl_extracted->val(i, j, k))
		        );
                        pf_buffer[bufCount++]   = (float) v_mag;
                        break;
                    case e_phase:
                        if(GSL_REAL(pVl_extracted->val(i, j, k))) {
                            v_phase = atan(
                                GSL_IMAG(pVl_extracted->val(i, j, k)) /
                                GSL_REAL(pVl_extracted->val(i, j, k))
                            );
                        } else
                            v_phase     = 0.0;
                        pf_buffer[bufCount++]   = (float) v_phase;
                        break;
                }
	    }
	}
    }

    ByteSwap4(pf_buffer, bufCount*4);
    fwrite(pf_buffer, sizeof(float), bufCount, pFILE_stream);

    // And at the very end, the MRIParams
    for(i=0; i<pV_MRIParamsEcho->cols_get(); i++) {
	pf_buffer[i] = pV_MRIParamsEcho->val(0, i);
        // Need to convert units, as well as do a degrees->radian
        switch(i) {
            case 0:
            case 2:
            case 3:
                pf_buffer[i] /= 1000.0;
            break;
            case 1:
                pf_buffer[i]*=M_PI/180;
            break;
        }
    }
    ByteSwap4(pf_buffer, i*4);
    fwrite(pf_buffer, sizeof(float), i, pFILE_stream);

    fflush(pFILE_stream);
    fclose(pFILE_stream);

    delete []	pch_buffer;
    delete []	ps_buffer;
    delete []	p_buffer;
    delete [] 	pf_buffer;
    delete []	pv_buffer;
    delete      pM_trns3x3;
    delete	pM_trns3x3sq;
    delete 	pV_xyz;
    debug_pop();

}

bool
C_IO_mgh::load(string str_fileName) {
}

//
//\\\***
// C_IO_analyze75 definitions ****>>>>
/////***
//

//
// constructor / destructor block
//

C_IO_analyze75::C_IO_analyze75( 
	CVol<GSL_complex_float>*        apVl_extracted,
    	CMatrix<double>&            	aV_voxelDimensions,
    	short                       	a_orientation,
    	double				av_intensityScale,
	bool				ab_readOutFlip
) : C_IO(apVl_extracted) 
{
    //
    // ARGS
    //  pVl_extracted		in		pointer to volume to be saved
    //	aV_voxelDimensions	in		voxel dimensions in mm
    //	a_orientation		in		orientation
    //	av_intensityScale	in		scale factor for intensity values
    //	ab_readOutFlip		in		flip readOut dimension on save
    //
    // DESC
    //	Constructor for the analyze75 format I/O.
    //
    //	Note that the base IO class is passed a pointer to the extracted volume.
    //
    // HISTORY
    // 13 October 2003
    //	o Initial design and coding.
    //
    // 24 October 2003
    //	o Added av_intensityScale.
    //
    
    str_obj             = "C_IO_analyze75";    
    
    pV_voxelDimensions	= new CMatrix<double>(aV_voxelDimensions.rows_get(),
					      aV_voxelDimensions.cols_get());
    pV_voxelDimensions->copy(aV_voxelDimensions);
    
    s_orientation		= a_orientation;
    v_intensityScale		= av_intensityScale;
    b_readOutFlip		= ab_readOutFlip;
    mpdsr_header                = new struct dsr;
}

C_IO_analyze75::C_IO_analyze75( 
    CMatrix<double>&            aV_voxelDimensions,
    short                       a_orientation,
    double			av_intensityScale,
    bool			ab_readOutFlip
) : C_IO()
{
    //
    // ARGS
    //	aV_voxelDimensions	in		voxel dimensions in mm
    //	a_orientation		in		orientation
    //	av_intensityScale	in		scale factor for intensity values
    //	ab_readOutFlip		in		flip readOut dimension on save
    //
    // DESC
    //	Constructor for the analyze75 format I/O.
    //
    //	This is a partial constructor that is used when the target volume
    //	is unknown. Note that the parameterless "dummy" base constructor
    //	is called!
    //
    // HISTORY
    // 13 October 2003
    //	o Initial design and coding.
    //
    // 24 October 2003
    //	o Added av_intensityScale.
    //
    // 28 June 2004
    //	o Added b_readOutFlip
    //
    
    str_obj             = "C_IO_analyze75";    
    
    pV_voxelDimensions	= new CMatrix<double>(aV_voxelDimensions.rows_get(),
					      aV_voxelDimensions.cols_get());
    pV_voxelDimensions->copy(aV_voxelDimensions);
    
    s_orientation		= a_orientation;
    v_intensityScale		= av_intensityScale;
    b_readOutFlip		= ab_readOutFlip;
    mpdsr_header                = new struct dsr;
}

C_IO_analyze75::~C_IO_analyze75() {
    delete	pV_voxelDimensions;
    delete	mpdsr_header;
}
	
//
// Misc access block
//

bool	
C_IO_analyze75::save(string astr_fileName) 
{
    //
    // ARGS
    //	astr_fileName			in		fileName to save volume to
    //
    // DESC
    //	This method is derived from Avi Snyder's "write_cdble_as_analyze" and "Inithdr0"
    //	code. It saves the volume and an analyze 7.5 header.
    //
    // PRECONDITIONS
    //	pV_voxelDimensions              member		voxel dimensions in mm
    //	orientation                     member		orientation of image:
    //								0: transverse
    //                                                          1: coronal
    //                                                          2: sagittal
    //
    //	o The pV_voxelDimensions vector is calculated externally to this
    //	  method and simply accessed as a class member
    //	o Note that within this class, volumes are spec'd as
    //		[row col slice]
    //	o For now, this save method saves a short norm of the reconstructed data.
    //
    // POSTCONDITIONS
    //	o The "current" volume class is saved to disk in analyze 7.5 format.
    //	o If successful, return true, else return false.
    //
    // NOTE
    //	o fwrite is used instead of fprintf since fwrite streams BINARY while fprintf
    //	  would stream formatted (human readable).
    //	o Linux on intel is *little endian*!
    //
    // SEE ALSO
    //	Avi Snyder's original code.
    //
    // HISTORY
    // 14 October 2003
    //	o Initial port and integration.
    //
    // 05 November 2003
    //	o Added big/little endian support. Note that currently it is assumed that
    //	  the code will execute on a native little endian machine!
    //
    // 12 December 2003
    //  o Changed astr_fileName: it is now only base file name (without extension).
    //
    // 25 February 2004
    //	o Added readOUt crop capability
    //

    debug_push("save(...)");

    FILE		*fp;
    double		q;
    int		        status, i, ix, iy, iz;
    short		norm;
    bool		b_returnVal	= true;

    /**********************************************/
    /* write norm of complex array as short image */
    /**********************************************/
    if (!(fp = fopen ((const char*)(astr_fileName+".img").c_str(), "wb")))
	error("Some problem encountered accessing output file " + astr_fileName, 1);
    int cnt = 0;
    m_maxval = status = 0;
    m_minval = SHRT_MAX;
    int			readOutStart	= 0;
    int			readOutEnd	= pVl_extracted->rows_get();
    if(pCadcPack->b_readOutCrop_get()) {
	readOutStart	= readOutEnd / 4;
	readOutEnd	= (int) (0.75 * readOutEnd);
    }
	
    int	RObegin, ROend, ROdel;
    if(b_readOutFlip) {
    	RObegin	= readOutEnd-1;
	ROend	= readOutStart;
	ROdel	= -1;
    } else {
    	RObegin	= readOutStart;
	ROend	= readOutEnd;
	ROdel	= 1;
    }
    
    for (iz = 0; iz < pVl_extracted->slices_get();   iz++) {
//        for (iy = RObegin; iy < readOutEnd;   iy++) {
	for (iy = RObegin; b_readOutFlip ? iy >= ROend : iy < ROend; iy+=ROdel) {
//        for (iy = readOutEnd-1; iy >= readOutStart;   iy--) {
	    for (ix = 0; ix < pVl_extracted->cols_get(); ix++) {
		q = sqrt(
		      GSL_REAL(pVl_extracted->val(iy, ix, iz)) * GSL_REAL(pVl_extracted->val(iy, ix, iz)) +
		      GSL_IMAG(pVl_extracted->val(iy, ix, iz)) * GSL_IMAG(pVl_extracted->val(iy, ix, iz))
		      );
		//cout << q << " ";
		if (q > (double) SHRT_MAX) status = -1;
		norm = (short) (v_intensityScale*q + 0.5);
		//cout << norm << " | ";
		if (norm < m_minval) m_minval = norm;
		if (norm > m_maxval) m_maxval = norm;
		if(e_byteOrder==e_bigEndian)
		    norm = swapShort(norm);
		fwrite (&norm, sizeof (short), 1, fp);
		//cnt++; if(cnt==15) exit(1);
	    }
	}
    }
        
    if (fclose (fp))
	error("Some error encountered when trying to fclose() " + astr_fileName, 1);
    if(status)
	b_returnVal	= false;

    headerSave(astr_fileName + ".hdr");

    debug_pop();
    return b_returnVal;
}

bool
C_IO_analyze75::headerSave(string astr_fileName) {
    //
    // ARGS
    //	astr_fileName			in		fileName to save volume to
    //
    // DESC
    //	This method is derived from Avi Snyder's "write_cdble_as_analyze" and "Inithdr0"
    //	code. It saves the volume and an analyze 7.5 header.
    //
    // PRECONDITIONS
    //	pV_voxelDimensions              member		voxel dimensions in mm
    //	orientation                     member		orientation of image:
    //								0: transverse
    //                                                          1: coronal
    //                                                          2: sagittal
    //
    //	o The pV_voxelDimensions vector is calculated externally to this
    //	  method and simply accessed as a class member
    //	o Note that within this class, volumes are spec'd as
    //		[row col slice]
    //
    // POSTCONDITIONS
    //	o The "current" volume class is saved to disk in analyze 7.5 format.
    //	o If successful, return true, else return false.
    //
    // NOTE
    //	o fwrite is used instead of fprintf since fwrite streams BINARY while fprintf
    //	  would stream formatted (human readable).
    //	o Linux on intel is *little endian*!
    //
    // SEE ALSO
    //	Avi Snyder's original code - this incorporates the header write and Inithdr0() code.
    //
    // HISTORY
    // 14 October 2003
    //	o Initial port and integration.
    // 
    // 29 June 2004
    //	o Folded check for ByteSwap on header.
    //

    debug_push("headerSave(...)");
    
    char	userid[1024];
    int		j, k;
    int		debug = 0;
    time_t	time_sec;
    
    struct dsr*	phdr	= mpdsr_header;

    memset (phdr, '\0', sizeof (struct dsr));
    phdr->hk.sizeof_hdr     = sizeof (struct dsr); 	        /* required */
    phdr->hk.extents        = 16384;			        /* recommended */
    phdr->hk.regular        = 'r';				/* required */
    phdr->dime.datatype     = 4;			        /* short int */
    phdr->dime.bitpix       = 16;

    phdr->dime.dim[0]       = 4;				/* 4 dimensions always */
    
    int	readOutLines	    = pVl_extracted->rows_get();
    if(pCadcPack->b_readOutCrop_get())
	readOutLines /= 2;
    
    phdr->dime.dim[1]       = pVl_extracted->cols_get();
//    phdr->dime.dim[2]       = pVl_extracted->rows_get();    
    phdr->dime.dim[2]       = readOutLines;    
    phdr->dime.dim[3]       = pVl_extracted->slices_get();
    phdr->dime.dim[4]       = 1;
    phdr->dime.pixdim[1]    = pV_voxelDimensions->val(0);
    phdr->dime.pixdim[2]    = pV_voxelDimensions->val(1);
    phdr->dime.pixdim[3]    = pV_voxelDimensions->val(2);

    phdr->hist.orient       = s_orientation;
    phdr->dime.glmin        = m_minval;
    phdr->dime.glmax        = m_maxval;

    /* start optional history code */
    time (&time_sec);
    strcpy (userid, ctime (&time_sec));
    userid[24] = '\0';
    if (debug) printf ("%s\n", userid);
    for (j = k = 0; k < 10; k++) if (userid[k] != ' ') phdr->hist.exp_date[j++] = userid[k];
    strncpy (phdr->hist.exp_time,  userid + 11, 9);
    strncpy (phdr->hist.generated, userid + 20, 9);

    cuserid (userid);	
    strncpy (phdr->hist.originator, userid, 9);
    /*   end optional history code */    
    
    //BEGIN:: Added by Mohana R to swap header bytes depending on e_bigEndian
    if(e_byteOrder==e_bigEndian)
	header_byteSwap();
    //END:: Added by Mohana R
    
    
    FILE*	fp;
    if (!(fp = fopen (astr_fileName.c_str(), "wb")) || fwrite (phdr, sizeof (struct dsr), 1, fp) != 1
	|| fclose (fp)) 
	error("Some problem occurred when writing the header " + astr_fileName, 1); 
    
    debug_pop();
}

bool
C_IO_analyze75::header_byteSwap() {
    //
    // DESC
    //	Swaps the bytes in the Analyze75 header.
    //
    // HISTORY
    // 29 June 2004
    //	o Incorporated from Mohana R
    //

    register int i;
    
    struct dsr*	hdr	= mpdsr_header;

    //Header Key section
    hdr->hk.sizeof_hdr = swapInt(hdr->hk.sizeof_hdr);
    hdr->hk.extents = swapInt(hdr->hk.extents);
    hdr->hk.session_error = swapShort(hdr->hk.session_error);

    //Image Dimension Section
    //Byte Swap the dim array;
    for ( i=0; i<8; i++) 
    	hdr->dime.dim[i] = swapShort(hdr->dime.dim[i]);
    hdr->dime.unused8 = swapShort(hdr->dime.unused8);
    hdr->dime.unused9 = swapShort(hdr->dime.unused9);
    hdr->dime.unused10 = swapShort(hdr->dime.unused10);
    hdr->dime.unused11 = swapShort(hdr->dime.unused11);
    hdr->dime.unused12 = swapShort(hdr->dime.unused12);
    hdr->dime.unused13 = swapShort(hdr->dime.unused13);
    hdr->dime.unused14 = swapShort(hdr->dime.unused14);
    hdr->dime.datatype = swapShort(hdr->dime.datatype);

    hdr->dime.bitpix = swapShort(hdr->dime.bitpix);
    hdr->dime.dim_un0 = swapShort(hdr->dime.dim_un0);

    for (i=0; i<8; i++) 
    	hdr->dime.pixdim[i] = swapFloat(hdr->dime.pixdim[i]);

    hdr->dime.funused8 = swapFloat(hdr->dime.funused8);
    hdr->dime.funused9 = swapFloat(hdr->dime.funused9);
    hdr->dime.funused10 = swapFloat(hdr->dime.funused10);
    hdr->dime.funused11 = swapFloat(hdr->dime.funused11);
    hdr->dime.funused12 = swapFloat(hdr->dime.funused12);
    hdr->dime.funused13 = swapFloat(hdr->dime.funused13);
    hdr->dime.compressed = swapFloat(hdr->dime.compressed);
    hdr->dime.verified = swapFloat(hdr->dime.verified);
    hdr->dime.glmax = swapInt(hdr->dime.glmax);
    hdr->dime.glmin = swapInt(hdr->dime.glmin);

    //Data History section
    hdr->hist.views = swapInt(hdr->hist.views);
    hdr->hist.vols_added = swapInt(hdr->hist.vols_added);
    hdr->hist.start_field = swapInt(hdr->hist.start_field);
    hdr->hist.field_skip = swapInt(hdr->hist.field_skip);
    hdr->hist.omax = swapInt(hdr->hist.omax);
    hdr->hist.omin = swapInt(hdr->hist.omin);
    hdr->hist.smax = swapInt(hdr->hist.smax);
    hdr->hist.smin = swapInt(hdr->hist.smin);

    return true;
}


bool	
C_IO_analyze75::load(string str_fileName) {
}




