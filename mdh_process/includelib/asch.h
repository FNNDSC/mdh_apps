/***************************************************************************
 *   Copyright (C) 2003 by Rudolph Pienaar                                 *
 *   rudolph@nmr.mgh.harvard.edu                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __ASCH_H__
#define __ASCH_H__

#include <iostream>
#include <string>
#include <map>
#include <list>
using namespace std;

#include <cmatrix.h>

namespace mdh {

const int       C_asch_STACKDEPTH      = 64;


class C_asch {

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

        string  str_proc[C_asch_STACKDEPTH];    // execution procedure stack


        string                  str_sequenceFileName;

        int                     TXFrequency;          // NB! not in test data

        int                     sliceArraySize;

        CMatrix<int>*           pMi_TR;
        CMatrix<int>*           pMi_TI;
        CMatrix<int>*           pMi_TE;

        CMatrix<double>*        pMv_slicePositionSag;
        CMatrix<double>*        pMv_slicePositionCor;
        CMatrix<double>*        pMv_slicePositionTra;
        CMatrix<double>*        pMv_sliceNormalSag;
        CMatrix<double>*        pMv_sliceNormalCor;     // NB! not in test data
        CMatrix<double>*        pMv_sliceNormalTra;     // NB! not in test data

        CMatrix<double>*        pMv_thickness;
        CMatrix<double>*        pMv_phaseFOV;
        CMatrix<double>*        pMv_readoutFOV;
        CMatrix<double>*        pMv_inPlaneRot;         // NB! not in test data

        int                     baseResolution;
        int                     phaseEncodingLines;
        double                  v_phaseResolution;
        int                     partitions;

        double                  v_flipAngleDegree;

        CMatrix<double>*        pMv_centerSlicePosition;

        // methods


    public:
        //
        // constructor / destructor block
        //
        C_asch(         string          astr_aschFileName);
                // constructor reading options from file

        void    core_construct( int     a_sliceArraySize,
                                string  astr_name               = "unnamed",
                                int     a_id                    = -1,
                                int     a_iter                  = 0,
                                int     a_verbosity             = 0,
                                int     a_warnings              = 0,
                                int     a_stackDepth            = 0,
                                string  astr_proc               = "noproc");
        ~C_asch();

        //
        // error / warn / print block
        //
        void        debug_push(         string astr_currentProc);
        void        debug_pop();

        void        error(              string  astr_msg        = "Some error has occured",
                                        int     code            = -1);
        void        warn(               string  astr_msg        = "",
                                        int     code            = -1);
        void        function_trace(     string  astr_msg,
                                        string  astr_separator);
        void        function_trace(     string  astr_msg)
                        { function_trace(astr_msg, ""); };

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

};


}

#endif //__ASCH_H__


