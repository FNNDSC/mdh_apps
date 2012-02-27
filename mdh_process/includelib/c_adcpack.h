/***************************************************************************
 *   Copyright (C) 2003 by Rudolph Pienaar                                 *
 *   rudolph@nmr.mgh.harvard.edu                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __C_ADCPACK_H__
#define __C_ADCPACK_H__

#include <iostream>
#include <string>
#include <map>
#include <list>
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

#include "c_adc.h"
#include "c_io.h"

namespace mdh {
        
const int       C_adcPack_STACKDEPTH            = 64;       // STACKDEPTH
const int       C_dimensionLists_STACKDEPTH     = 64;       //  for debugging

    typedef enum {
        e_normalKSpace,
        e_phaseCorrectedKSpace
    } e_KSPACEDATATYPE;


// Some forward declarations
//class C_dimensioLists;
//class C_adcPack;

//class C_adc;
//class C_IO;

class C_dimensionLists {

        //  A small auxillary class that contains vectors specifying numbered
        //      sequences of dimension lists. This class was originally small
        //      enough to be completely defined within this header file.
        //
        //  Subsequent expansion to allow for meta-data processing for its
        //      constructors resulted in a clean header/definition split into
        //      h/cpp files.
        //
        //  The main purpose of this class is to hold the relevant dimension
        //      sizes that are used by the primary data-unpack classes. This
        //	class describes a single channel collection of echoes/volumes.
        //	Multiple channel information can not be stored in active memory
        //	due to size constraints - hence a C_dimensionLists object
        //	can be thought of as describing the information relevant
        //	to a *single* channel.
        //
        //  Most of the apparent "bloat" in this class results from my standard
        //      class "template" meta tags / primitive error handling.
        //

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

        string  str_proc[C_dimensionLists_STACKDEPTH];    // execution procedure stack

        CMatrix<int>*   pM_sliceSelectList;     // Enumerated sequence of slices
        CMatrix<int>*   pM_repetitionList;      // Enumerated sequence of repetitions
        CMatrix<int>*   pM_echoList;            // Enumerated sequence of echos

        CMatrix<int>*   pM_ROpePC;              // Single 1x3 vector defining the
                                                //      ReadOut
                                                //      PhaseEncode
 	                                        //      PhaseCorrect dimensions
	
	CMatrix<int>*	pV_dimensionStructure;	// A vector defining the actually 
	                                        // implemented dimension structure
	                                        //	NOTE: this structure does not
	                                        //	include channel information!
	

	// Additional "meta" data that is contained in the
	//	options file. Note that the following is class
	//	generic. Specific sub-classes might have their
	//	own meta data and provide sub-class level
	//	access.
	
        string          str_ADCfileBaseName;    // Base fully qualified name for
                                                //      Siemens raw data, i.e.
	                                        //      "/some/path/meas"
	string		str_optionsFileName;	// The options file itself
	e_BYTEORDER     e_byteOrder;            // byte order for binary save data.
                                                //  On x86:                 little endian
                                                //  everything else:        big endian
	
	bool		b_unpackWpadShift;	// If true, implement a zero-padding
	                                        //	and ifftshift operation on data
						//	elements as they are read from
	                                        //	disk. This improves overall
	                                        //	execution speed and memory use
	                                        //	of the whole program.
	                                        // If false, unpack volumes exactly as are
	                                        //	and implement a zeroPad / shift
	                                        //	during the recon loop.
	bool		b_packAdditionalData;	// If true, create and populate several
	                                        //	additional data structures in
	                                        //	the core unpack classes (i.e.
						//	in addition to unpacking raw
	                                        //	k-space data, also unpack
						//	bit-reverse and timestamp info.
	                                        // If false, unpack and track *only* the
	                                        //	raw image data.
	bool		b_readOutCrop;		// If true, crop the read out direction
	                                        //	of image volumes by throwing away
	                                        //	the first and last quarters. This
	                                        //	only affects the volume_save(...)
	                                        // If false, save images without discarding
	                                        //	any information. This means that
	                                        //	the readOut FOV is double what
	                                        //	the scanner usually shows.
	bool		b_shiftInPlace;		// If true, implement the ((i)fft)shift
	                                        //	function "in place", i.e. do not
						//	create an intermediate volume. This
	                                        //	saves on memory, but takes longer
	                                        //	to complete.
	                                        // If false, create temp scratch volume
						//	for shift operation. As long as
	                                        //	main system does not start swapping
	                                        //	virtual memory, *false* will be
	                                        //	faster than *true*. Uses more memory.
	bool		b_phaseCorrect;		// If true, interpret image data as
	                                        //	containing phase corrected
	                                        //	information (not implemented in
	                                        //	recon yet: 2/25/04).
	

    public:
        C_dimensionLists();
        C_dimensionLists(
                        const CMatrix<int>&     aM_sliceSelectList,
                        const CMatrix<int>&     aM_repetitionList,
                        const CMatrix<int>&     aM_echoList,
                        int                     a_linesReadOut,
                        int                     a_linesPhaseEncode,
                        int                     a_linesPhaseCorrect);

        C_dimensionLists(           string      astr_optionsFileName);

        void    core_construct(     string  astr_name               = "unnamed",
                                    int     a_id                    = -1,
                                    int     a_iter                  = 0,
                                    int     a_verbosity             = 0,
                                    int     a_warnings              = 0,
                                    int     a_stackDepth            = 0,
                                    string  astr_proc               = "noproc");

        ~C_dimensionLists();
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

        CMatrix<int> M_sliceSelectionList_get()
            const {return   *pM_sliceSelectList;};
        CMatrix<int> M_repetitionList_get()
            const {return   *pM_repetitionList;};
        CMatrix<int> M_echoList_get()
            const {return   *pM_echoList;};
        CMatrix<int> M_ROpePC_get()
            const {return   *pM_ROpePC;};

        int     linesReadOut_get()
            const {return   pM_ROpePC->val(0, 0);};
        int     linesPhaseEncode_get()
            const {return   pM_ROpePC->val(0, 1);};
        int     linesPhaseCorrect_get()
            const {return   pM_ROpePC->val(0, 2);};
	int	linesSliceSelect_get()
	    const {return   pM_sliceSelectList->cols_get();};

        void    str_ADCfileBaseName_set(    string  astr_name)
            { str_ADCfileBaseName = astr_name;};
        string  str_ADCfileBaseName_get()
            const {return   str_ADCfileBaseName;};

        void    str_optionsFileName_set(    string  astr_name)
            { str_optionsFileName = astr_name;};
        string  str_optionsFileName_get()
            const {return   str_optionsFileName;};
	
	int     repetitionIndex_find(   int     a_index);
        int     sliceSelectIndex_find(  int     a_index);
        int     echoIndex_find(         int     a_index);
	
	CMatrix<int>*		pV_dimensionStructure_get()
	    const {return	pV_dimensionStructure;};
	
	void pV_dimensionStructure_set( CMatrix<int>*	pV) 
	    { *pV_dimensionStructure	= *pV;};
   
	e_BYTEORDER	e_byteOrder_get()
	                    const {return e_byteOrder;};
	void		e_byteOrder_set(e_BYTEORDER ae_byteOrder)
	                    { e_byteOrder = ae_byteOrder;};
	
	bool            b_unpackWpadShift_get()
	                    const {return b_unpackWpadShift;};
	bool		b_packAdditionalData_get()
	                    const {return b_packAdditionalData;};
	bool		b_readOutCrop_get()
	                    const {return b_readOutCrop;};
	bool		b_phaseCorrect_get()
	                    const {return b_phaseCorrect;};
	bool		b_shiftInPlace_get()
	                    const {return b_shiftInPlace;};
	
	void		metaData_parse();

};

class C_adcPack {

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

        string  str_proc[C_adcPack_STACKDEPTH];    // execution procedure stack

        // files that are processed by the class
        string                          str_baseFileName;       // base file
        string                          str_ascFileName;        // the ascii header
        string                          str_adcFileName;        // the binary data

        // some meta data
        bool                            flag3D;                 // is this a 3D scan?
	int				zeroPad_row;		// Zero-padding offsets
	int				zeroPad_column;		//	along each image
	int				zeroPad_slice;		//	volume dimension

        // header processing object
        C_asch*                         pcasch_measASCfile;

        // Siemens data structures
        sMDH                            s_MDH;

        // Scanner raw data classes and support structures
        C_dimensionLists*               pC_dimension;           // lists and dimensions
                                                                //      of raw data. This
	                                                        //	describes the data
	                                                        //	structure of the
	                                                        //	C_adcPack object
	                                                        //	which may be different
	                                                        //	from that of the
	                                                        //	problem - esp if 
	                                                        //	targetted echo/reps
	                                                        //	are unpacked.
	CMatrix<int>*			pV_echoesUnpacked;	// a "monitor" structure that
								//	checks which echoes
								//	have been unpacked
								//	from the raw data.
								//	Used as a sanity
								//	check.
        C_adc*                          pCadc_kSpace;           // K space data
        C_adc*                          pCadc_phaseCorrected;   // Phase corrected data
	
	// The following variables are used to specify if *all* channels/echoes/repetitions
	//	have been unpacked (all are -1) or if a specific channel/echo/repetition
	//	combination has been filtered out
	int				channelTarget;
	int				echoTarget;
	int				repetitionTarget;
	
        // methods


    public:
        //
        // constructor / destructor block
        //
        C_adcPack(      const string            astr_baseFileName,
                        C_dimensionLists*       apC_dimension,
                        const bool              isData3D,
			int                     a_echoTarget        = -1,
			int                     a_repetitionTarget  = -1,
			int			a_channelTarget	    = -1
			);
                // constructor reading options from file

        void    core_construct(     string  astr_name               = "unnamed",
                                    int     a_id                    = -1,
                                    int     a_iter                  = 0,
                                    int     a_verbosity             = 0,
                                    int     a_warnings              = 0,
                                    int     a_stackDepth            = 0,
                                    string  astr_proc               = "noproc");
        ~C_adcPack();

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
	
	void	channelTarget_set(	int	val)
	                { channelTarget = val;};
	
	int	channelTarget_get() 	const
	                { return channelTarget;};
	int	echoTarget_get() 	const
	                { return echoTarget;};
	int	repetitionTarget_get() 	const
	                { return repetitionTarget;};
	
	bool	flag3D_get()		        const
                   	{ return flag3D;};
	bool	b_unpackWpadShift_get()	        const
                	{return pC_dimension->b_unpackWpadShift_get();};
	bool	b_packAdditionalData_get() 	const
	                {return pC_dimension->b_packAdditionalData_get();};
	bool	b_readOutCrop_get()		const
	                {return pC_dimension->b_readOutCrop_get();};
	bool	b_phaseCorrect_get()		const
	                {return pC_dimension->b_phaseCorrect_get();};
	bool	b_shiftInPlace_get()		const
	                {return pC_dimension->b_shiftInPlace_get();};

        sMDH*                   ps_MDH_get()
	          {return &s_MDH;};
        C_adc*                  pCadc_kSpace_get()              
	    const {return pCadc_kSpace;};
	C_adc*                  pCadc_phaseCorrected_get()      
	    const {return pCadc_phaseCorrected;};
	C_dimensionLists*	pc_dimension_get()
	    const { return pC_dimension;};
	CMatrix<int>*		pV_echoesUnpacked_get()
	    const { return pV_echoesUnpacked;};

        //
        // Scanner data processing
        //
        void    headerFile_process(     bool    b_headerDump = false);
        int     dataFile_process();
	bool    disk2memory_voxelMap(
	    int			                indexChannel,
	    int			                indexSlicePartition,
	    int			                indexRepetition,
	    int			                indexEcho,
	    int		                        loopCounter,
	    int&		                slicePartitionIndex,
	    int&		                repetitionIndex,
	    int&		                echoIndex
	    );	
	int     phaseCorrect_unpack(
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
	    );
	int     kSpace_unpack(
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
	    );

	
	CMatrix<int> 
	        C_adcPack::dimension_zeroPad(
	                                   CMatrix<int>&	M_orig);
	
	
	void	dataMemory_volumePreprocess(	
				int		    a_echoIndex,
    				int		    a_echoTarget,
				int		    a_repetitionIndex,
				int		    a_repetitionTarget,
    				e_KSPACEDATATYPE    ae_kspace
							= e_normalKSpace);        
	
	void    dataMemory_volumeExtract(
				int                 a_repetition,
                                int                 a_echo,
                                e_KSPACEDATATYPE    ae_kspace       
					    		= e_normalKSpace);
        void    dataMemory_volumeConstruct( 
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace);
        void    dataMemory_volumeDestruct( 
				 e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace);

        void    dataMemory_volumeZeroPad(   
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace);

        void    dataMemory_volumeShift(
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace,
                                e_FFTSHIFTDIR       ae_shiftDir     
					    		= e_fftshift);

        void    dataMemory_volumefftShift(  
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace)
                        {dataMemory_volumeShift(    ae_kspace, e_fftshift);};
        void    dataMemory_volumeIfftShift( 
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace)
                        {dataMemory_volumeShift(    ae_kspace,  e_ifftshift);};

        void    dataMemory_volumeifft(      
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace);

        CVol<GSL_complex_float>*
                dataMemory_volumeGet(
				e_KSPACEDATATYPE    ae_kspace       
							= e_normalKSpace);
	
	
	// File I/O access front ends.
        bool    dataMemory_volumeSave(          string          astr_fileName,
                                                e_IOTYPE        e_iotype);
	bool    dataMemory_volumeSave(          string          astr_fileName);
	bool    dataMemory_volumeLoad(          string          astr_fileName);
	bool    dataMemory_volumeSaveReal(      string          astr_fileName);
	bool    dataMemory_volumeSaveImag(      string          astr_fileName);
	bool    dataMemory_volumeSaveNorm(      string          astr_fileName);
}; // class


class C_adcPack_mgh : public C_adcPack {
    
    //
    // This class really only implements a "thin interface" allowing the
    //	end user to control the C_IO_mgh object. It keeps no state
    //	information.
    //

    protected:
    
    public:

    //
    // constructor / destructor block
    //
    C_adcPack_mgh(      
	const string            astr_baseFileName,
	C_dimensionLists*       apC_dimension,
	const bool              isData3D,
	int                     a_echoTarget            = -1,
	int                     a_repetitionTarget      = -1,
        int			a_channelTarget	        = -1
	);		
 
    ~C_adcPack_mgh();
    
    //		
    // Misc access block
    //	
};

class C_adcPack_analyze75 : public C_adcPack {
    
    //
    // This class really only implements a "thin interface" allowing the
    //	end user to control the C_IO_mgh object. It keeps no state
    //	information.
    //

    protected:
    
    public:

    //
    // constructor / destructor block
    //
    C_adcPack_analyze75(      
	const string            astr_baseFileName,
	C_dimensionLists*       apC_dimension,
	const bool              isData3D,
	int                     a_echoTarget            = -1,
	int                     a_repetitionTarget      = -1,
	int			a_channelTarget         = -1
	);
 
    ~C_adcPack_analyze75();
    
    //		
    // Misc access block
    //	
};

} // namespace

#endif //__ADCPACK_H__


