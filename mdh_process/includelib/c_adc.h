/***************************************************************************
 *   Copyright (C) 2003 by Rudolph Pienaar                                 *
 *   rudolph@nmr.mgh.harvard.edu                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __C_ADC_H__
#define __C_ADC_H__

#include <iostream>
#include <string>
#include <complex>
using namespace std;

#include <ltl/marray.h>
#include <ltl/marray_io.h>
using namespace ltl;

#ifdef __x86_64__
#include "mdh64.h"
#else
#include "mdh.h"
#endif

#include "cmatrix.h"
#include "asch.h"

//#include "c_adcpack.h"
#include "c_io.h"

namespace mdh {
    
const int       C_adc_STACKDEPTH      = 64;

// These enumerations define the total data space volume
typedef enum {
        e_readOut           = 0,            // `i' dimension (also phase correct)
        e_phaseEncode       = 1,            // 'j' dimension
        e_sliceSelect       = 2,            // 'k' dimension
        e_numRepetitions    = 3,            // repetitions in scan cycle
        e_numEchoes         = 4             // echoes in scan cycle
} e_SCANDIMENSIONS;

//class C_IO;
class C_adcPack;

// NB! Note that when accessing scan dimensions from the MArray structures,
//  remember that MArrays start counting from '1', *not* '0'!

class C_adc {

        // data structures

    protected:
        //
        // generic object structures - used for internal bookkeeping
        // and debugging / automated tracing methods. The stackDepth
        // and str_proc[] variables are maintained by the debug_push|pop
        // methods
        //
        string  str_obj;                // name of object class
        string  str_name;               // name of object variable
        int     id;                     // id of agent
        int     iter;                   // current iteration in an
                                        //      arbitrary processing scheme
        int     verbosity;              // debug related value for object
        int     warnings;               // show warnings (and warnings level)
        int     stackDepth;             // current pseudo stack depth

        string  str_proc[C_adc_STACKDEPTH];    // execution procedure stack
	
	// Pointer back to "parent" object that contains C_adc
	C_adcPack*			pParent;


        // Data space meta data dimensions
        int                             linesReadOut;
        int                             linesPhaseEncode;
        int                             linesSliceSelect;
        int                             numRepetitions;         // Number of repeitions
        int                             numEchoes;              // Number of echoes

        // Raw scanner data is unpacked and stored in the following structures
	bool				b_libCMatrixUse;	// A temp switch for
	                                                        //	toggling between
	                                                        //	the ltl and 
								//	CMatrix usage
	
	MArray<complex<float>,  5>*     pAz_data;               // Fourier complex data
	CVol5D<GSL_complex_float>*	pMz_data;		// Fourier complex data
	
	MArray<bool,            4>*     pAb_reverse;            // Reverse bit tracking
        MArray<unsigned long,   4>*     pAul_timeStamp;         // Time stamp tracking
        MArray<int,             3>*     pA_ssIndex;             // Sub-space indexing
		
        // Interface volumes: raw scanner data, spec'd by echo and repetition, is
        //  extracted to the following volume
	CVol<GSL_complex_float>*	pVl_extracted;		// CVol object that
	                                                        //  contains the
	                                                        //  extracted vol.
	C_IO*                           pCIO;			// I/O object that will
	                                                        //  save/load extracted
	                                                        //  volume from file.


    // methods


    public:
        //
        // constructor / destructor block
        //
        C_adc(
	    C_adcPack*			pParent,
	    CMatrix<int>&               aM_spaceVolume);
                // constructor reading options from file

        void    core_construct( string  astr_name               = "unnamed",
                                int     a_id                    = -1,
                                int     a_iter                  = 0,
                                int     a_verbosity             = 0,
                                int     a_warnings              = 0,
                                int     a_stackDepth            = 0,
                                string  astr_proc               = "noproc");
        ~C_adc();

        //
        // error / warn / print block
        //
        void        debug_push(         string astr_currentProc);
        void        debug_pop();

        void        error(              string  astr_msg        = "Some error has occured",
                                        int     code            = -1);
        void        warn(               string  astr_msg        = "",
                                        int     code            = -1);
        void        function_trace(     string  astr_msg        = "Function trace",
                                        string  astr_separator  = "");

        //
        // access block
        //
        void    print();                        // print object
        int     stackDepth_get()        const {return stackDepth;};
        void    stackDepth_set(int anum)
                        { stackDepth = anum;};
        int     iter_get()              const {return iter;};
        void    iter_set(int anum)
                        { iter = anum;};
        int     id_get()                const {return id;};
        void    id_set(int anum)
                        { id = anum;};
        int     verbosity_get()         const {return verbosity;};
        void    verbosity_set(int anum)
                        { verbosity = anum;};
        int     warnings_get()          const {return warnings;};
        void    warnings_set(int anum)
                        { warnings = anum;};
        string  str_obj_get()           const {return str_obj;};
        void    str_obj_set(string astr)
                        { str_obj = astr;};
        string  str_name_get()          const {return str_name;};
        void    str_name_set(string astr)
                        { str_name = astr;};
        string  str_proc_get()          const {return str_proc[stackDepth_get()];};
        void    str_proc_set(int depth, string astr)
                        { str_proc[depth] = astr;};

        MArray<complex<float>,  5>                      Az_data()
                {return *pAz_data;};
        MArray<bool,            4>                      Ab_reverse()
                {return *pAb_reverse;};
        MArray<unsigned long,   4>                      Aul_timeStamp()
                {return *pAul_timeStamp;};
        MArray<int, 3>                                  A_ssIndex()
                {return *pA_ssIndex;};
	
	CVol5D<GSL_complex_float>*                      pMz_data_get()
	        {return pMz_data;};

        int     linesSliceSelect_get()  const
                        {return linesSliceSelect;};
        void    linesSliceSelect_set(   int a_num)
                        {linesSliceSelect = a_num;};
	
	CVol<GSL_complex_float>*&     pvolume()			// Get and set volume pointers
	                {return pVl_extracted;};
	
	CVol<GSL_complex_float>*&     volume_get()    
                        {return pVl_extracted;};
	void                    volume_set(CVol<GSL_complex_float>*   pVl)
                        {pVl_extracted = pVl;};

        //
        // miscellaneous block
        //
	virtual	void	volume_preprocess(
				int		    a_echoIndex,
    				int		    a_echoTarget,
				int		    a_repetitionIndex,
				int		    a_repetitionTarget);
	void	volume_construct();
        void    volume_destruct();
	void	volume_replaceWith(	CVol<GSL_complex_float>*	pVl) {
	            pVl_extracted->coreVolume_replaceWith(pVl);
		};
	void	volume_copyConstruct(	CVol<GSL_complex_float>*	pVl) {
	            pVl_extracted	= new CVol<GSL_complex_float>(*pVl);
		};
	    
        virtual
	void    volume_extract(
                        int                             repetition,
                        int                             echo
                );
	
	C_IO*	pCIO_get()	const
	    { return pCIO;}; 
	
	//
	// Convenience functions for simple access to some PCIO methods
	//
	e_IOTYPE	e_iotype_get()  const;
	void		e_iotype_set(   e_IOTYPE        ae_iotype);
		
	bool   save(    string          astr_fileName);
	bool   load(    string          astr_fileName);
};

class C_adc_mgh : public C_adc {

    //
    // This class really only implements a "thin interface", providing
    //	a "bridge" between the top level API (C_adcPack) and the C_IO 
    //	classes of a particular C_adc. 
    //
    
    protected:

    public:

    //
    // constructor / destructor block
    //
    C_adc_mgh(
	C_adcPack*			pParent,
	CMatrix<int>&                   aM_spaceVolume,
	CMatrix<double>&                M_vox2ras,
	CMatrix<double>&                V_MRIParams
		  );
    
    ~C_adc_mgh();

    //
    // Misc access block
    //
    
    // This method "extends" the method of the same name in the
    //	base class. Specifically, depending on the echo, we
    //	further process the C_IO_mgh internal member,
    //	pV_MRIParamsEcho
    virtual	void	volume_preprocess(
				int		    a_echoIndex,
    				int		    a_echoTarget,
				int		    a_repetitionIndex,
				int		    a_repetitionTarget);
};

class C_adc_analyze75 : public C_adc {

    //
    // This class really only implements a "thin interface", providing
    //	a "bridge" between the top level API (C_adcPack) and the C_IO 
    //	classes of a particular C_adc. 
    //
    
    protected:

    public:

    //
    // constructor / destructor block
    //
    C_adc_analyze75(
	C_adcPack*			pParent,
	CMatrix<int>&                   aM_spaceVolume,
	CMatrix<double>&                aV_voxelDimensions,
	int                             a_orientation,
	double			        av_intensityScale,
	bool				ab_readOutFlip	= false
		  );
    
    ~C_adc_analyze75();

    //
    // Misc access block
    //
    virtual	void	volume_preprocess(
				int		    a_echoIndex,
    				int		    a_echoTarget,
				int		    a_repetitionIndex,
				int		    a_repetitionTarget);

};


}

#endif //__C_ADCPACK_H__
