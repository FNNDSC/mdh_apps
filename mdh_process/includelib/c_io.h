/***************************************************************************
 *   Copyright (C) 2003 by Rudolph Pienaar                                 *
 *   rudolph@nmr.mgh.harvard.edu                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
//
// NAME
//
//  c_io.h       $Id: c_io.h 2 2006-05-19 18:31:02Z rudolph $
//
// DESCRIPTION
//
//  `c_io.h' contains the class declarations for various I/O related
//   classes in the mdh_process project.
//
// HISTORY
// o see `README' for detailed history information.
//

#ifndef __C_IO_H__
#define __C_IO_H__

#include <iostream>
#include <string>
#include <complex>
using namespace std;

#ifdef __x86_64__
#include "mdh64.h"
#else
#include "mdh.h"
#endif

#include "cmatrix.h"
#include "machine.h"
#include "analyze.h"

namespace mdh {
    
const int       C_IO_STACKDEPTH      = 64;

    typedef enum _iotype {
	e_complex       = 0,
	e_real          = 1,
	e_imaginary     = 2,
	e_magnitude     = 3,
	e_phase	        = 4
    } e_IOTYPE;

    typedef enum _byteOrder {
        e_littleEndian  = 0,
        e_bigEndian     = 1
    } e_BYTEORDER;
  
class	C_adcPack;
    
class   C_IO {

        // data structures

    protected:
        //
        // generic object structures - used for internal bookkeeping
        // and debugging / automated tracing methods. The stackDepth
        // and str_proc[] variables are maintained by the debug_push|pop
        // methods
        //
        string  str_obj;                    // name of object class
        string  str_name;                   // name of object variable
        int     id;                         // id of agent
        int     iter;                       // current iteration in an
                                            //      arbitrary processing scheme
        int     verbosity;                  // debug related value for object
        int     warnings;                   // show warnings (and warnings level)
        int     stackDepth;                 // current pseudo stack depth

        string  str_proc[C_IO_STACKDEPTH];  // execution procedure stack

	
	string	str_saveFileName;               // save filename
	string 	str_loadFileName;               // load filename
	
	e_IOTYPE	e_iotype;	        // type of IO to perform for load/save.
	                                        //	Interpretation depends on particular
	                                        //	derived class.

        e_BYTEORDER     e_byteOrder;            // byte order for binary save data.
                                                //  On x86:                 little endian
	                                        //  everything else:        big endian
			
	// A pointer to the actual volume that is to be I/O'd
	CVol<GSL_complex_float>*	pVl_extracted;		// CVol object that
	                                        		//  contains the
	                                        		//  extracted vol.
	
	// A pointer back to the parent "root" of all the system objects.
	C_adcPack*		pCadcPack;	// parent object that contains
	                                        //	all system data.


    // methods


    public:
        //
        // constructor / destructor block
        //
	C_IO();
	C_IO(CVol<GSL_complex_float>* apVl_extracted);	

        void    core_construct( string  astr_name               = "unnamed",
                                int     a_id                    = -1,
                                int     a_iter                  = 0,
                                int     a_verbosity             = 0,
                                int     a_warnings              = 0,
                                int     a_stackDepth            = 0,
                                string  astr_proc               = "noproc");
        ~C_IO();

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

	string	str_saveFileName_get()	const {return str_saveFileName;};
	void	str_saveFileName_set(string	str_name)
                        { str_saveFileName = str_name;};
	string	str_loadFileName_get()	const {return str_loadFileName;};
	void	str_loadFileName_set(string	str_name)
                        { str_loadFileName = str_name;};


	CVol<GSL_complex_float>*&  pvolume()			// Get and set volume pointers
	                {return pVl_extracted;};

	CVol<GSL_complex_float>*&         volume_get()
                        {return pVl_extracted;};
	void                        volume_set(CVol<GSL_complex_float>*   pVl)
                        {pVl_extracted = pVl;};

        //
        // miscellaneous block
        //
	void		envSynchronise(	C_adcPack*	apCadcPack);
	void		envSynchronise();
	
	void		pCadcPack_set(	C_adcPack*	pack)
	                    { pCadcPack = pack;};
	
	e_IOTYPE	e_iotype_get()
	                    const {return e_iotype;};
	void		e_iotype_set(e_IOTYPE ae_iotype)
	                    { e_iotype = ae_iotype;};
	e_BYTEORDER	e_byteOrder_get()
	                    const {return e_byteOrder;};
	void		e_byteOrder_set(e_BYTEORDER ae_byteOrder)
	                    { e_byteOrder = ae_byteOrder;};

	void	volume_copyConstruct(	CVol<GSL_complex_float>*	pVl) {
	            pVl_extracted	= new CVol<GSL_complex_float>(*pVl);
	        };
        void    volume_destruct();
	void	volume_replaceWith(	CVol<GSL_complex_float>*	pVl) {
	            pVl_extracted->coreVolume_replaceWith(pVl);
	        };
	void    volume_reconstruct(     CVol<GSL_complex_float>*      pVl);

	//
	// Overloads
	//
	virtual bool	save(string str_fileName) {};
	virtual bool	load(string str_fileName) {};

};

class C_IO_mgh : public C_IO {

    protected:


    CMatrix<double>*		pM_vox2ras;	    	// transformation matrix
    CMatrix<double>*		pV_MRIParams;	    	// *all* MRI parameters
    CMatrix<double>*		pV_MRIParamsEcho;	// MRI paramaters for
    							//	current echo

    public:

    //
    // constructor / destructor block
    //
    C_IO_mgh(
	        CVol<GSL_complex_float>*        pVl_extracted,
		CMatrix<double>&                M_vox2ras,
	        CMatrix<double>& 		V_MRIParams
		  );
    C_IO_mgh(
	        CMatrix<double>&            	M_vox2ras,
	        CMatrix<double>&            	V_MRIParams
		  );
    
    ~C_IO_mgh();

    //
    // Misc access block
    //
    CMatrix<double>*	pV_MRIParams_get() const
    			{ return pV_MRIParams;};
    CMatrix<double>*	pV_MRIParamsEcho_copy(CMatrix<double>& aV_target)
    			{pV_MRIParamsEcho->copy(aV_target);};

    virtual bool	save(string astr_fileName);
    virtual bool	load(string astr_fileName);

};

class C_IO_analyze75 : public C_IO {

    protected:


    CMatrix<double>*		pV_voxelDimensions;     // transformation matrix
    short                       s_orientation;          // additional MRI parameters
    double			v_intensityScale;	// factor for scaling intensity values
    struct dsr*			mpdsr_header;		// the analyze format header
    int				m_minval;		// the min value of the volume
    int				m_maxval;		// the max value of the volume
    bool			b_readOutFlip;		// boolean flag for "inverting" the
    							//	save order of the readout
							//	dimension

    public:

    //
    // constructor / destructor block
    //
    C_IO_analyze75(
	        CVol<GSL_complex_float>*        apVl_extracted,
	        CMatrix<double>&                aV_voxelDimensions,
	        short			        a_orientation,
		double			        av_intensityScale,
		bool				ab_readOutFlip	= false
		  );
    C_IO_analyze75(
	        CMatrix<double>&                aV_voxelDimensions,
	        short			        a_orientation,
		double			        av_intensityScale,
		bool				ab_readOutFlip	= false
		  );
    
    ~C_IO_analyze75();

    //
    // Misc access block
    //

    virtual bool	save(string astr_fileName);
    virtual bool	load(string astr_fileName);
    
    bool		headerSave(string astr_fileName);
    bool		headerLoad(string astr_fileName);
    bool		header_byteSwap();

};


}

#endif //__C_ADC_H__
