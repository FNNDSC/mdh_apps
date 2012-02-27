/***************************************************************************
                          promasc.cpp  -  description
                             -------------------
    begin                : Tue Nov 18 2003
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
//      promasc.cpp
//
// SYNOPSIS
//
//	(see synopsis_show function).
//
// DESC
//
//      `promasc' (or PROcess Meas.ASC) parses a meas.asc Siemens header
//	file and extracts/creates meta data necessary for "mdh_process"
//
//
// NOTES / WARNINGS
//
//      Under active development! Behaviour subject to change!
//
// PRECONDITIONS
//
//      o Siemens scanner meas.asc header file
//
// POSTCONDITIONS
//
//	o meta data files needed for "mdh_process"
//
//
// HISTORY
//
// 18 November 2003
// o Initial design and development
//
// 17 June 2004
// o vox2ras back-ports from MatLAB
// o Re-design of variable scoping
//
// 24 November 2004
// o AMD64 build.
//
// 16 May 2006
// o 2D / 3D sequence parsing logic. In order to preserve backwards
//   compatibility, an extra file consisting of a single valued 
//   matrix is output to the filesystem. This file, '3D.cmt' contains
//   a '1' if the sequence is a 3D scan, or a '0' if it is a 2D scan.
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>

#include <getopt.h>

#include <sys/times.h>
#include <unistd.h>
#include <stdlib.h>

#include "SeqDefines.h"
// Siemens header for Partial Fourier flag lookups

#include "scanopt.h"
#include "cmatrix.h"
#include "c_SMessage.h"

using namespace std;

// Using a pseudo-class type structuring, the following "global" variables
//	should really be interpreted as class members, where the class
//	has been abstracted to this entire file.
//
//
//////////////////////////////////////////////////
// Miscellaneous housekeeping
//////////////////////////////////////////////////	
string		G_SELF          = "";		// "My" name
string          G_VERSION       = 		// version
	"$Id: promasc.cpp 3 2012-02-10 14:30:22Z rudolph $";
//////////////////////////////////////////////////
// Files that are accessed
//////////////////////////////////////////////////
string		Gstr_meas;			// Siemens meas.asc
string		Gstr_meta;			// "Meta" data file
						//	typically 
						//	"measMetaData.asc"
//////////////////////////////////////////////////
// Output to user
//////////////////////////////////////////////////
char		Gpch_msg[1024];			// Misc text buffer
C_SMessage*	Gpcsm;				// output message handling
						//	object used to dump
						//	syslog-type messages
						//	to user spec'd sink 
bool		Gb_syslogPrepend    = true;	// prepend with syslog stamp
string	        Gstr_runID	    = "RDP";	// runID is global since it 
						//	also serves as a file
						//	save prefix.
//////////////////////////////////////////////////						
// Data about the scan
//////////////////////////////////////////////////
bool		Gb_3D		    = true;	// bool flag: 3D or 2D scan
string		Gstr_orientation;		// string describing the
						//	orientation of the
						//	main slab
short		G_orientationA75;		// orientation code for 
						//	Analyze 7.5 format
CMatrix<float>	GV_slicePosition(1,3);		// slice position of main slab
CMatrix<float>	GV_sliceNormal(1,3);		// slice normal
CMatrix<float>	GV_logicalSpaceSize(1, 3);	// logical size of the volume
						//	i.e. 256 x 256 x 128
CMatrix<float>	GV_voxelDimension(1, 3);	// Physical size of voxel
						//	in mm
CMatrix<float>	GV_FOV(1, 3);			// Field of view of scan
CMatrix<float>	GM_vox2ras(4, 4);		// The vox2ras matrix
//////////////////////////////////////////////////						

// The following is an ugly but effective hack, and assuming the above
//	global variables, simplifies some operations
#define	COUT( msg )	Gpcsm->dump(Gb_syslogPrepend, ( msg ) );
#define	COUTnl( msg )	Gpcsm->dump(false, ( msg ) );

static struct option const longopts[] =
{
  {"baseDir", 		required_argument, 	NULL, 'b'},
  {"runID",             required_argument, 	NULL, 'd'},
  {"meas",		required_argument, 	NULL, 'm'},
  {"meta",		required_argument, 	NULL, 'M'},
  {"version",           no_argument,            NULL, 'v'},
  {NULL, 0, NULL, 0}
};

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

void warn(
    string              str_action,
    string              str_errorMsg,
    int                 errorCode) {

    //
    // ARGS
    //  str_action      in              action that failed
    //  str_errorMsg    in              error message to dump to stderr
    //  errorCode       in              code to echo to stdout
    //
    // DESC
    //  Dumps a simple error message and then warns with
    //  errorCode.
    //
    // HISTORY
    // 13 August 2003
    //  o Initial design and coding.
    //

    cerr << endl << G_SELF;
    cerr << endl << "\tWarning, Will Robinson!";
    cerr << endl << "\tWhile I was "    << str_action;
    cerr << endl << "\t"                << str_errorMsg;
    cerr << endl;
    cerr << endl << "\tWarning code " << errorCode;
    cerr << endl;
}

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
    cout << endl << "\tpromasc.cpp";
    cout << endl << "\t\t" << G_VERSION;
    cout << endl << "";
    cout << endl << "SYNOPSIS";
    cout << endl << "";
    cout << endl << "\tpromasc [OPTIONS]";
    cout << endl << "";
    cout << endl << "DESC";
    cout << endl << "";
    cout << endl << "\t`promasc' (or PROcess Meas.ASC) parses a meas.asc Siemens header";
    cout << endl << "\tfile and extracts/creates meta data necessary for \"mdh_process\"";
    cout << endl << "";
    cout << endl << "\tIts behaviour is governed largely by the following command-line options:";
    cout << endl << "";
    cout << endl << "OPTIONS:";
    cout << endl << "";
    cout << endl << "\t--baseDir=<baseDir>, -b <baseDir>";
    cout << endl << "\tThis defines the base directory that contains the original Siemens";
    cout << endl << "\traw data file <meas.out> and its text header <meas.asc>.";
    cout << endl << "";
    cout << endl << "\t--runID=<string>, -d <string>";
    cout << endl << "\tAn optional string can be defined that describes a current process run. This";
    cout << endl << "\tstring will be used as a prefix for any output files that are created. It is";
    cout << endl << "\tprobably a good idea to *not* include spaces in this string.";
    cout << endl << "";
    cout << endl << "NOTES / WARNINGS";
    cout << endl << "";
    cout << endl << "Under active development! Behaviour subject to change!";
    cout << endl << "";
    cout << endl << "PRECONDITIONS";
    cout << endl << "";
    cout << endl << "o Siemens scanner meas.asc header file.";
    cout << endl << "";
    cout << endl << "POSTCONDITIONS";
    cout << endl << "";
    cout << endl << "o meta data files needed for \"mdh_process\"";
    cout << endl;

}

double
partialFourier_scale(
    string	str_PFF
) {
    //
    // ARGS
    //	a_PFF			in		Partial Fourier flag
    //
    // DESC
    //	This simple function merely returns the partial fourier scaling
    //	corresponding to the lookup flag, a_PFF.
    //
    //	The flags are defined in the Siemens header, SeqDefines.h
    //
    
    //cout << "str_PFF " << str_PFF << endl;
    if(str_PFF=="0x01")	return 0.5;
    if(str_PFF=="0x2")	return (5.0 / 8.0);
    if(str_PFF=="0x4")	return (6.0 / 8.0);
    if(str_PFF=="0x8")	return (7.0 / 8.0);
    if(str_PFF=="0x10")	return 1.0;
    
    return 1.0;
}

double
partialFourier_scale(
    int		a_PFF
) {
    //
    // ARGS
    //	a_PFF			in		Partial Fourier flag
    //
    // DESC
    //	This simple function merely returns the partial fourier scaling
    //	corresponding to the lookup flag, a_PFF.
    //
    //	The flags are defined in the Siemens header, SeqDefines.h
    //
    
    //cout << "a_PFF " << a_PFF << endl;
    switch(a_PFF) {
        case SEQ::PF_HALF:
            return 0.5;
        case SEQ::PF_5_8:
            return (5.0 / 8.0);
        case SEQ::PF_6_8:
            return (6.0 / 8.0);
        case SEQ::PF_7_8:
            return (7.0 / 8.0);
        default:
            return 1.0;
    }
    
}

void
sliceNormal_parse(
	C_scanopt&	cso_meas
)
{
//
// ARGS
//	cso_meas	in		reference to scanopt object 
//						constructed around
//						the Siemens meas.asc file
//
// DESC
//	'sliceNormal_parse' populates the "global" member GV_sliceNormal
//	vector. It targets the 
//
//		sSliceArray.asSlice[0].sNormal.* values
//
// PRECONDTIONS
// o Should be called *before* the vox2ras functions.
//
// POSTCONDITIONS
// o Changes the "global" member GV_sliceNormal vector
// o Changes the "global" Gstr_orientation and G_orientationA75 values.
// o Writes (by shell escape) G_orientationA75 to meta file.
// o Uses the "global" Gstr_meta value.
//
// HISTORY
// 18 June 2004
// o Initial design and coding.
//
// 21 June 2004
// o Added code to output orientationA75
//
// 22 July 2004
// o Changed error_exit to warn for "sNormal"
//

    string	str_value;		// for scanopt
    char	pch_value[32];		// for awk cmd string
    
    float	f_sag	= 0.0;
    float	f_cor	= 0.0;
    float	f_tra	= 0.0;
    
    COUT("\tParsing for *.sNormal.dSag:...");
    str_value	= "0.0";
    if(cso_meas.scanFor("sSliceArray.asSlice[0].sNormal.dSag", 	&str_value))
	f_sag	= atof(str_value.c_str());
    else
	warn(	"parsing for *.sNormal.dSag",
		"I couldn't find sSliceArray.asSlice[0].sNormal.dSag!",
		3);
    COUTnl("\t\t["+str_value+"]\n");
    
    COUT("\tParsing for *.sNormal.dCor:...");
    str_value	= "0.0";
    if(cso_meas.scanFor("sSliceArray.asSlice[0].sNormal.dCor", 	&str_value))
	f_cor	= atof(str_value.c_str());
    else
	warn(	"parsing for *.sNormal.dCor",
		"I couldn't find sSliceArray.asSlice[0].sNormal.dCor!",
		3);
    COUTnl("\t\t["+str_value+"]\n");

    COUT("\tParsing for *.sNormal.dTra:...");
    str_value	= "0.0";
    if(cso_meas.scanFor("sSliceArray.asSlice[0].sNormal.dTra", 	&str_value))
	f_tra	= atof(str_value.c_str());
    else
	warn(	"parsing for *.sNormal.dTra",
		"I couldn't find sSliceArray.asSlice[0].sNormal.dTra!",
		3);
    COUTnl("\t\t["+str_value+"]\n");
        
    GV_sliceNormal(0)	= f_sag;
    GV_sliceNormal(1)	= f_cor;
    GV_sliceNormal(2)	= f_tra;
        
    float	f_max		= GV_sliceNormal.abs().max();
    int		occurrences 	= 0;
    bool	b_inPlace	= true;
    
    CMatrix<float>	V_position(1,2);
    V_position		= GV_sliceNormal.abs().findAll(f_max, occurrences);
    
    // The following is Siemens Sag/Cor/Tra dependent!
    switch((int)V_position(1)) {
    	case 0:
		Gstr_orientation	= "sagittal";
		G_orientationA75	= 2;
	break;
	case 1:
		Gstr_orientation	= "coronal";
		G_orientationA75	= 1;
	break;
	case 2:
		Gstr_orientation	= "transverse";
		G_orientationA75	= 0;
	break;
    } 
    
    // and parse the meta file, writing the correct orientation
    sprintf(pch_value, "%d", G_orientationA75);
    string	str_orientation(pch_value);
    string	str_cmd = "cat ";
    str_cmd 		+= Gstr_meta + "| awk '";
    str_cmd		+= "{";
    str_cmd		+=     "if($1==\"orientation\")";
    str_cmd		+=	"{printf(\"\torientation\t\t= %d #edited by promasc\\n\",";
    str_cmd		+=		str_orientation + ");";
    str_cmd		+=      "} else ";
    str_cmd		+=  	    "print;";
    str_cmd		+= "}'";
    str_cmd 	+= ">meta.bak ; mv meta.bak " + Gstr_meta;
    COUT("\tDominant slab orientation:...");
    COUTnl("\t\t["+Gstr_orientation+"]\n");
    COUT("\tEditing Analyze75 orientation:...");
    system(str_cmd.c_str());
    COUTnl("\t["+str_orientation+"]\n");
    
}

void
slicePosition_parse(
	C_scanopt&	cso_meas
)
{
//
// ARGS
//	cso_meas	in		reference to scanopt object 
//						constructed around
//						the Siemens meas.asc file
//
// DESC
//	'slicePosition_parse' populates the "global" member GV_slicePosition
//	vector. It targets the 
//
//		sSliceArray.asSlice[0].sPosition.* values
//
// PRECONDTIONS
// o Should be called *before* the vox2ras functions.
//
// POSTCONDITIONS
// o Changes the "global" member GV_slicePosition vector.
//
// HISTORY
// 18 June 2004
// o Initial design and coding.
//

    string	str_value;		// for scanopt
    
    float	f_sag;
    float	f_cor;
    float	f_tra;
    
    COUT("\tParsing for *.sPosition.dSag:...");
    if(cso_meas.scanFor("sSliceArray.asSlice[0].sPosition.dSag", 	&str_value))
	f_sag	= atof(str_value.c_str());
    else
	error_exit("parsing for *.sPosition.dSag",
		   "I couldn't find sSliceArray.asSlice[0].sPosition.dSag!",
		   3);
    COUTnl("\t["+str_value+"]\n");
    
    COUT("\tParsing for *.sPosition.dCor:...");
    if(cso_meas.scanFor("sSliceArray.asSlice[0].sPosition.dCor", 	&str_value))
	f_cor	= atof(str_value.c_str());
    else
	error_exit("parsing for *.sPosition.dCor",
		   "I couldn't find sSliceArray.asSlice[0].sPosition.dCor!",
		   3);
    COUTnl("\t["+str_value+"]\n");

    COUT("\tParsing for *.sPosition.dTra:...");
    if(cso_meas.scanFor("sSliceArray.asSlice[0].sPosition.dTra", 	&str_value))
	f_tra	= atof(str_value.c_str());
    else
	error_exit("parsing for *.sPosition.dTra",
		   "I couldn't find sSliceArray.asSlice[0].sPosition.dTra!",
		   3);
    COUTnl("\t["+str_value+"]\n");
    
    GV_slicePosition(0)	= f_sag;
    GV_slicePosition(1)	= f_cor;
    GV_slicePosition(2)	= f_tra;
    
}

CMatrix<float>
vox2ras_ksolve(
	CMatrix<float>&	M_voxRot
) {
//
// ARGS
//	M_voxRot	in		4x4 matrix containing a 3x3 rotational
//						sub-component
//
// DESC
//	'vox2ras_ksolve' uses the given rotational components of M_voxRot
//	as well as a starting position, GV_slicePosition, and determines
//	the center of k-space and thus creating a fully-qualified vox2ras
//	matrix.
//
// PRECONDTIONS
// o Make sure that the following have been defined:
//	GV_voxelDimension
//	GV_slicePosition
//	GV_sliceNormal
// o It is assumed that GV_sliceNormal is itself normalised.
// o Make sure that the rotational components of M_voxRot have been
//   properly defined (either by direct parsing or a call to vox2ras_rsolveAA)
//
// POSTCONDITIONS
// o Returns a final 4x4 vox2ras matrix
//
// HISTORY
// 22 June 2004
// o Initial design and coding.
//

    CMatrix<float>	M_vox2ras(4, 4);
    M_vox2ras.copy(M_voxRot);

    // First, read the direction cosines
    CMatrix<float>*	pV_RO	= new CMatrix<float>(3, 1);
    CMatrix<float>*	pV_PE	= new CMatrix<float>(3, 1);
    CMatrix<float>*	pV_SS	= new CMatrix<float>(3, 1);
    
    M_voxRot.matrix_remove(pV_PE, 0, 0, 3, 1);
    M_voxRot.matrix_remove(pV_RO, 0, 1, 3, 1);
    M_voxRot.matrix_remove(pV_SS, 0, 2, 3, 1);
    
    CMatrix<float>	V_PE(*pV_PE);
    CMatrix<float>	V_RO(*pV_RO);
    CMatrix<float>	V_SS(*pV_SS);
    
    delete		pV_RO, pV_PE, pV_SS;
   
    int			xoff	= (int)GV_logicalSpaceSize(0)/2;
    int			yoff	= (int)GV_logicalSpaceSize(1)/2;
    int			zoff	= (int)GV_logicalSpaceSize(2)/2;
    
    CMatrix<float>	V_K1(3, 1);
    CMatrix<float>	V_K2(3, 1);
    CMatrix<float>	V_K(4, 1, 1);	// Create the k-space column vector 
    					//	(filled with ones)
    
    // The first two components:
    V_K1		= -!GV_slicePosition -(V_PE*xoff+V_RO*yoff+V_SS*zoff);
    V_K2		=  !GV_slicePosition -(V_PE*xoff+V_RO*yoff+V_SS*zoff);
    
    // Use the voxelDimension offset to correct a Siemens bug 
    V_K(0)		= V_K1(0) - GV_voxelDimension(2)/2;
    V_K(1)		= V_K1(1);
    V_K(2)		= V_K2(2);
    V_K(3)		= 1;
    
    M_vox2ras.col_replace(V_K, 3);
    
    return M_vox2ras;
   
}

CMatrix<float>
vox2ras_rsolveAA(
	C_scanopt&	cso_meas
)
{
//
// ARGS
//	cso_meas	in		reference to scanopt object 
//						constructed around
//						the Siemens meas.asc file
//
// DESC
//	'vox2ras_rsolveAA' is a straightforward port of a similarly named 
//	MatLAB function. There are some minor changes for insertion into
//	promasc.
//
//	Effectively, 'vox2ras_rsolveAA' solves for the vox2ras rotation
//	matrix using Siemens-reference vectors.
//
// PRECONDTIONS
// o Make sure that the following have been defined:
//	GV_voxelDimension
//	GV_slicePosition
//	GV_sliceNormal
// o It is assumed that GV_sliceNormal is itself normalised.
//
// POSTCONDITIONS
// o Returns a 4x4 vox2ras matrix of which only the 3x3 rotational
//   component is valid.
//
// HISTORY
// 18 June 2004
// o Initial design and coding.
//

    string		str_value;			// for scanopt
    float		f_inPlaneRotation	= 0.;	// inPlaneRotation
    CMatrix<float>	M_vox2ras(4, 4);
    
    COUT("\tParsing for in-plane rotation:...");
    if(cso_meas.scanFor("sSliceArray.asSlice[0].dInPlaneRot", 	&str_value))
	f_inPlaneRotation	=	atof(str_value.c_str());
    else
	warn(	"parsing for in-plane rotation",
		"I couldn't find sSliceArray.asSlice[0].dInPlaneRot!",
		3);
    COUTnl("\t["+str_value+"]\n");
    
    CMatrix<float>	V_PE(3, 1);
    CMatrix<float>	V_SSn(3, 1);
    CMatrix<float>	V_SS(3, 1);
    // Swap the direction sense of the first two normals (LPS to RAS)
    GV_sliceNormal(0)	= -GV_sliceNormal(0);
    GV_sliceNormal(1)	= -GV_sliceNormal(1);
    V_SS		= !GV_sliceNormal;
    V_SSn		= !GV_sliceNormal;
    // Swap the direction sense of the first two normals (LPS to RAS)
    GV_sliceNormal(0)	= -GV_sliceNormal(0);
    GV_sliceNormal(1)	= -GV_sliceNormal(1);
    // and scale the vector with the Slice Select voxel dimension
    V_SS.scale(GV_voxelDimension(2));
    switch(Gstr_orientation.c_str()[0]) {
    	case 's':
		V_PE(0)	= -V_SS(1) * sqrt(1/(V_SS(0)*V_SS(0)+V_SS(1)*V_SS(1)));
		V_PE(1)	=  V_SS(0) * sqrt(1/(V_SS(0)*V_SS(0)+V_SS(1)*V_SS(1)));
		V_PE(2)	=  0;
	break;
    	case 'c':
		V_PE(0)	=  V_SS(1) * sqrt(1/(V_SS(0)*V_SS(0)+V_SS(1)*V_SS(1)));
		V_PE(1)	= -V_SS(0) * sqrt(1/(V_SS(0)*V_SS(0)+V_SS(1)*V_SS(1)));
		V_PE(2)	=  0;
	break;
    	case 't':
		V_PE(0)	=  0;
		V_PE(1)	=  V_SS(2) * sqrt(1/(V_SS(1)*V_SS(1)+V_SS(2)*V_SS(2)));
		V_PE(2)	= -V_SS(1) * sqrt(1/(V_SS(1)*V_SS(1)+V_SS(2)*V_SS(2)));
	break;
    }
    
    // The ReadOut vector is the cross product between the SliceSelect 
    //	and the PhaseEncode
    CMatrix<float>	V_RO(3, 1);
    V_RO	= V_SSn.cross(V_PE);
    
    // Construct the 3x3 rotational component of the vox2ras
    CMatrix<float>	M_voxRot(3, 3);
    M_voxRot.col_replace(V_PE, 0);
    M_voxRot.col_replace(V_RO, 1);
    M_voxRot.col_replace(V_SS, 2);
    
    // Construct a 3x3 rotational matrix as a function of f_inPlaneRotation
    float 		pf_Mu[] = {
    			 cos(f_inPlaneRotation),	sin(f_inPlaneRotation),	0,
			-sin(f_inPlaneRotation),	cos(f_inPlaneRotation), 0,
			 0,				0,			1
    };
    CMatrix<float>	M_Mu(3, 3, pf_Mu);
    CMatrix<float>	M_voxRotated(3, 3);
    M_voxRotated	= M_voxRot * M_Mu;
    
    // Change the sign of the RO dimension
    float	    	pf_Rinv[] = {
    			1,  0, 0, 0,
			0, -1, 0, 0,
			0,  0, 1, 0,
			0,  0, 0, 1
    };
    CMatrix<float>	M_Rinv(4, 4, pf_Rinv);
    CMatrix<float>	M_voxRot44(4, 4);
    M_voxRot44.matrix_replace(0, 0, M_voxRotated);
    M_vox2ras		= M_voxRot44	* M_Rinv;
    return M_vox2ras;
}

CMatrix<float>
vox2ras_dfmeas(
	C_scanopt&	cso_meas,
	C_scanopt&	cso_meta
)
{
//
// ARGS
//	cso_meas	in		reference to scanopt object 
//						constructed around
//						the Siemens meas.asc file
//	cso_meta	in		reference to scanopt object 
//						constructed around
//						the meta data file
//
// DESC
//	'vox2ras_dfmeas' is a straightforward port of a similarly named 
//	MatLAB function. There are some minor changes for insertion into
//	promasc.
//
//	Effectively, 'vox2ras_dfmeas' is the entry point for calculating
//	a vox2ras matrix from the given scanner (meas.asc) file.
//	
//
// PRECONDTIONS
// o Make sure that the following have been defined:
//	GV_voxelDimension
//	GV_slicePosition
//	GV_sliceNormal
//
// HISTORY
// 17 June 2004
// o Initial design and coding.
//
// 22 June 2004
// o Added cso_meta
//

    // Coordinate transform: scanner physical (x, y, z) coordinates
    //	to ras
    float		pf_xyz2ras1[] 	= {
				 1,	0,	 0,	0,
				 0,     1,	 0,	0,
				 0,	0,	-1,	0,
				 0,	0,	 0,	1    
    };
    float		pf_xyz2ras2[] 	= {
				-1,	0,	 0,	0,
				 0,     1,	 0,	0,
				 0,	0,	-1,	0,
				 0,	0,	 0,	1    
    };
    float               pf_default[]    = {
				 0,	0,	-1,	0,
				-1,	0,	 0,	0,
				 0,    -1,	 0,	0,
				 0,	0,	 0,	1    
    };
    CMatrix<float>	M_xyz2ras1(4, 4, pf_xyz2ras1);
    CMatrix<float>      M_xyz2ras2(4, 4, pf_xyz2ras2);
    CMatrix<float>      M_default(4, 4, pf_default);
    CMatrix<float>	M_voxScale("I", 4);
    CMatrix<float>	M_adRM("I", 4);
    
    float		f_bandWidthPerPixel	= 0.0;
    float		f_voxDimPE		= 0.0;
    float		f_voxDimRO		= 0.0;
    float		f_voxDimSS		= 0.0;
    
    string	str_value;		// for scanopt
    
    COUT("Calculating voxel sizes:...");
    COUTnl("\t\t\t[..]\n");
    f_voxDimRO	= GV_voxelDimension(0);
    sprintf(Gpch_msg, "\tVoxel size along ReadOut dimension:\t[%f] (mm)\n", f_voxDimRO);
    COUT(Gpch_msg);
    f_voxDimPE	= GV_voxelDimension(1);
    sprintf(Gpch_msg, "\tVoxel size along PhaseEncode dimension:\t[%f] (mm)\n", f_voxDimPE);
    COUT(Gpch_msg);
    f_voxDimSS	= GV_voxelDimension(2);
    sprintf(Gpch_msg, "\tVoxel size along SliceSelect dimension:\t[%f] (mm)\n", f_voxDimSS);
    COUT(Gpch_msg);
    
    M_voxScale(0, 0)			= 	GV_voxelDimension(0);
    M_voxScale(1, 1)			=	GV_voxelDimension(1);
    M_voxScale(2, 2)			=	GV_voxelDimension(2);
    
    string str_vox2rasDetermine		= "normal";
    if(cso_meta.scanFor("vox2rasDetermine", &str_value))
    	str_vox2rasDetermine		= str_value;
    if(	str_vox2rasDetermine != "default" 	&&
    	str_vox2rasDetermine != "calculate"	&&
	str_vox2rasDetermine != "normal")
	str_vox2rasDetermine = "normal";	
    bool		b_adRMFound	= true;
        
    // adRM
    //	Check if "Bandwidth_per_pixel_for_ADC[0]" is defined in file.
    //	If true, assume that adRM also exists, otherwise default to unity
    //	matrix for GM_vox2ras
    
    CMatrix<float>	M_voxRot(4, 4);
    CMatrix<float>	M_vox2ras(4, 4);
    if(str_vox2rasDetermine=="normal") {
    	COUT("Parsing for Siemens rotation matrix:...");
    	if(cso_meas.scanFor("Bandwidth_per_pixel_for_ADC[0]",	&str_value)) {
    	    f_bandWidthPerPixel	= 	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[0][0]",	&str_value))
	    	M_adRM(0, 0)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[0][1]",	&str_value))
	    	M_adRM(0, 1)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[0][2]",	&str_value))
	    	M_adRM(0, 2)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[1][0]",	&str_value))
	    	M_adRM(1, 0)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[1][1]",	&str_value))
	    	M_adRM(1, 1)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[1][2]",	&str_value))
	    	M_adRM(1, 2)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[2][0]",	&str_value))
	    	M_adRM(2, 0)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[2][1]",	&str_value))
	    	M_adRM(2, 1)	=	atof(str_value.c_str());
    	    if(cso_meas.scanFor("adRM[2][2]",	&str_value))
	    	M_adRM(2, 2)	=	atof(str_value.c_str());
	    COUTnl("\t\t[OK]\n");
	    M_voxRot		= M_xyz2ras1 * !M_adRM * M_xyz2ras2 * M_voxScale;      
    	} else {
	    COUTnl("\t\t[Not Found]\n");
    	    b_adRMFound	= false;
    	}
    }
    if(str_vox2rasDetermine=="calculate" || !b_adRMFound) {
    	COUT("Calculating rotation matrix:...")
	COUTnl("\t\t\t[..]\n");
	M_voxRot		= vox2ras_rsolveAA(cso_meas);
    }
    if(str_vox2rasDetermine=="default") {
    	COUT("Defaulting rotation matrix:...")
    	M_voxRot		= M_default * M_voxScale;
	COUTnl("\t\t\t[OK]\n");
    }
	
    COUT("Calculating k-space center:...");
    M_vox2ras	= vox2ras_ksolve(M_voxRot);
    COUTnl("\t\t\t[OK]\n");
    
    return M_vox2ras;
}

int 
main(int argc, char** ppch_argv) {
    //
    // HISTORY
    // 1 September 2004
    //	o Added initial Avanto support:
    //		- lContrasts 		(for echoes)
    //		- adFlipAngleDegree[0] 	(for flip angle)
    //
    // 16 May 2006
    // 	o Added logic for 2D sequences.
    //		- file: 3D.cmt
    //		- slices variable
    //
    
    G_SELF				= ppch_argv[0];
    Gstr_meas				= "meas.asc";
    Gstr_meta				= "measMetaData.asc";
    
    string	str_echoListFile	= "echolist.cmt";
    string	str_kListFile	        = "klist.cmt";
    string	str_repListFile	        = "replist.cmt";
    string	str_mriParamFile	= "mriparam.cmt";
    string	str_voxelDimFile	= "voxelDimension.cmt";
    string	str_rotationFile	= "vox2ras.cmt";
    string	str_ROpePCFile		= "ROpePC.cmt";
    string 	str_3DFile		= "3D.cmt";
    string	str_baseDir		= "./";
    string	str_runID		= G_SELF;
    
    int         PFF;

    ////////////////////////////////
    // debugging hard-coded settings
    //str_baseDir = "/space/okapi/2/data/rudolph/data/recon/gleek/pfizer/apr17_04/test";
    ////////////////////////////////
	    
    unsigned short	echoes		= 0;
    unsigned short	slices		= 0;
    unsigned short	reps		= 0;
    
    unsigned int	MRI_TR		= 0;
    unsigned int	MRI_TI		= 0;
    unsigned int	MRI_TE		= 0;
    double		v_flipAngle	= 0.0;
    
    int			PELines		= 0;
    int			ROLines		= 0;
    int			phaseCorrect	= 0;
    
    float		f_readOutFOV	= 0.0;
    float		f_phaseFOV	= 0.0;
    float		f_thickness	= 0.0;
    
    int			i;
    
    // Process command line options
    while(1) {
        int opt;
        int optionIndex = 0;
        opt = getopt_long(argc, ppch_argv, "", longopts, &optionIndex);
        if( opt == -1)
            break;

        switch(opt) {
            case 'b':
	        str_baseDir.assign(optarg, strlen(optarg));
            break;
            case 'd':
	        Gstr_runID.assign(optarg, strlen(optarg));
            break;
            case 'm':
	        Gstr_meas.assign(optarg, strlen(optarg));
            break;
            case 'M':
	        Gstr_meta.assign(optarg, strlen(optarg));
            break;
            case '?':
                synopsis_show();
                exit(1);
            break;
            case 'v':
                version_show();
            break;            
	    default:
                cout << "?? getopt returned character code " << opt << endl;
        }
    }
    
    if(str_baseDir == "/dev/null")
	error_exit("checking for a base directory",
		   str_baseDir+" seems invalid!",
		   1);
    
    Gpcsm	= new C_SMessage("",
			eSM_raw,
			"stdout",
			eSM_cpp);
    Gpcsm->str_syslogID_set(str_runID);
    COUT(G_SELF+" startup\n");
    stringstream        sout("");

    // Create scanopt objects to parse the meas and meta files
    C_scanopt   cso_measAscFile(str_baseDir+"/"+Gstr_meas, e_EquLink);
    C_scanopt   cso_metaAscFile(str_baseDir+"/"+Gstr_meta, e_EquLink);
    string	str_value;

    // First, check if this is a 3D scan. This is something of a hack, since
    //	I check for the presence of a slice thickness index that is non-zero.
    //	3D scans only have one array index, 0, while 2D scans have an index for
    //	each slice.
    COUT("Checking for scan dimensionality:...");
    string	str_dim 	= "3D";
    if(cso_measAscFile.scanFor("sSliceArray.asSlice[1].dThickness", &str_value)) {
	Gb_3D	= false;
	str_dim	= "2D";
    } else {
	Gb_3D	= true;
	str_dim	= "3D";
    }
    COUTnl("\t\t[ "+ str_dim +" ]\n");
    
    // readout lines
    COUT("Parsing for readout lines:...");
    if(cso_measAscFile.scanFor("sKSpace.lBaseResolution", 	&str_value))
	ROLines             =	2*atoi(str_value.c_str());
    else
	error_exit("parsing for readout lines",
		   "I couldn't find sKSpace.lBaseResolution!",
		   3);
    COUTnl("\t\t\t[2 X "+str_value+"]\n");
    
    // phase encode lines
    COUT("Parsing for phase encode lines:...");
    if(cso_measAscFile.scanFor("sKSpace.lPhaseEncodingLines", 	&str_value))
	PELines             =	atoi(str_value.c_str());
    else
	error_exit("parsing for phase encode",
		   "I couldn't find sKSpace.lPhaseEncodingLines!",
		   3);
    COUTnl("\t\t["+str_value+"]\n");
        
    // phase encode partial fourier
    COUT("Parsing for phase partial fourier:...");
    if(cso_measAscFile.scanFor("sKSpace.ucPhasePartialFourier", 	&str_value))
	PFF             =	atoi(str_value.c_str());
    else
	error_exit("parsing for phase partial fourier",
		   "I couldn't find sKSpace.ucPhasePartialFourier!",
		   3);
    COUTnl("\t\t["+str_value+"]\n");
    sscanf(str_value.c_str(), "%x", &PFF);
    PELines                 = (int) (PELines * partialFourier_scale(PFF));
    sprintf(Gpch_msg, "\tScaled phase encode lines:...\t\t[%d]\n", PELines);
    COUT(Gpch_msg);
        
    // Number of k space partitions...
    COUT("Parsing for k space partitions:...");
    if(Gb_3D) {
	// 3D scan
    	if(cso_measAscFile.scanFor("sKSpace.lPartitions", 	&str_value))
	    slices		= atoi(str_value.c_str());
        else
	    error_exit("parsing for partitions",
		   	"I couldn't find sKSpace.lPartitions!",
		   	3);
    } else {
	// 2D scan
    	if(cso_measAscFile.scanFor("sSliceArray.lSize", 	&str_value))
	    slices		= atoi(str_value.c_str());
        else
	    error_exit("parsing for partitions",
		   	"I couldn't find sSliceArray.lSize!",
		   	3);
    }
    COUTnl("\t\t["+str_value+"]\n");
    
    // partition encode partial fourier
    COUT("Parsing for slice partial fourier:...");
    if(cso_measAscFile.scanFor("sKSpace.ucSlicePartialFourier", 	&str_value))
	PFF             =	atoi(str_value.c_str());
    else
	error_exit("parsing for phase partial fourier",
		   "I couldn't find sKSpace.ucSlicePartialFourier!",
		   3);
    COUTnl("\t\t["+str_value+"]\n");
    sscanf(str_value.c_str(), "%x", &PFF);
    slices                  = (int) (slices * partialFourier_scale(PFF));
    sprintf(Gpch_msg, "\tScaled slice encode lines:...\t\t[%d]\n", slices);
    COUT(Gpch_msg);
    
    // Number of echoes... HACK ALERT!
    //	The new avanto scanners do not use sWiPMemBlock.alFree[10], so if no
    //	value is found, look for "lContrasts" instead.
    COUT("Parsing for echoes:...");
    if(cso_measAscFile.scanFor("sWiPMemBlock.alFree[10]", 	&str_value)) 
	echoes			=	atoi(str_value.c_str());
    else {
    	if(cso_measAscFile.scanFor("lContrasts", 	&str_value)) 
		echoes		=	atoi(str_value.c_str());
	else
		warn("parsing for echoes",
		   "I couldn't find any sWiPMemBlock.alFree[10] or lContrasts!",
		   3);
    }
    CMatrix<float>	M_mriParam(1, 3+echoes);
    COUTnl("\t\t\t\t["+str_value+"]\n");
    int TE = 0;
    string	str_echoText;
    string	str_echoNum;
    for(i=0; i<echoes; i++) {
	sout << i;
	str_echoNum	= sout.str();
	str_echoText	= "\tEcho time " + str_echoNum + "...";
	COUT(str_echoText);
	str_echoText	= "alTE[" + str_echoNum +"]";
    	if(cso_measAscFile.scanFor(str_echoText, 	&str_value)) {
	    TE			=	atoi(str_value.c_str());
	    COUTnl("\t\t\t\t["+str_value+"]\n");
	} else
	    COUTnl("\t\t\t\t[Not found]\n");
	M_mriParam(3+i)	= TE;
	sout.str("");
    }
    
    // Number of repetitions...
    COUT("Parsing for repetitions:...");
    if(cso_measAscFile.scanFor("lRepetitions", 	&str_value)) {
	reps                    =	atoi(str_value.c_str());
    	COUTnl("\t\t\t["+str_value+"]\n");
    } else
    	COUTnl("\t\t\t[0]\n");
    
    // TR...
    COUT("Parsing for TR:...");
    if(cso_measAscFile.scanFor("alTR[0]", 	&str_value))
	MRI_TR                   =	atoi(str_value.c_str());
    else
	error_exit("parsing for TR",
		   "I couldn't find alTR[0]!",
		   3);
    COUTnl("\t\t\t\t["+str_value+"]\n");
    
    // TE...
    COUT("Parsing for TE:...");
    if(cso_measAscFile.scanFor("alTE[0]", 	&str_value))
	MRI_TE                   =	atoi(str_value.c_str());
    else
	error_exit("parsing for TE",
		   "I couldn't find alTE[0]!",
		   3);
    COUTnl("\t\t\t\t["+str_value+"]\n");
 
    // TI...
    COUT("Parsing for TI:...");
    if(cso_measAscFile.scanFor("alTI[0]", 	&str_value))
	MRI_TI                   =	atoi(str_value.c_str());
    else
	error_exit("parsing for TI",
		   "I couldn't find alTI[0]!",
		   3);
    COUTnl("\t\t\t\t["+str_value+"]\n");
     
    // flip angle
    // 	The new (July 2004 Avanto) scanner records information slightly
    //	differently than the old regime.
    COUT("Parsing for flip angle:...");
    if(cso_measAscFile.scanFor("dFlipAngleDegree", 		&str_value))
	v_flipAngle             =	atof(str_value.c_str());
    else {
    	if(cso_measAscFile.scanFor("adFlipAngleDegree[0]",	&str_value))
		v_flipAngle	=	atof(str_value.c_str());
	else
		warn("parsing for flip angle data",
			"I couldn't find any information",
		   3);
    }
    COUTnl("\t\t\t["+str_value+"]\n");
    
    // RO FOV
    COUT("Parsing for readOut FOV:...");
    if(cso_measAscFile.scanFor("sSliceArray.asSlice[0].dReadoutFOV", 	&str_value))
	f_readOutFOV             =	atof(str_value.c_str());
    else
	error_exit("parsing for readOut FOV",
		   "I couldn't find sSliceArray.asSlice[0].dReadoutFOV!",
		   3);
    COUTnl("\t\t\t["+str_value+"]\n");
    
    // PE FOV
    COUT("Parsing for phase FOV:...");
    if(cso_measAscFile.scanFor("sSliceArray.asSlice[0].dPhaseFOV", 	&str_value))
	f_phaseFOV             =	atof(str_value.c_str());
    else
	error_exit("parsing for phase FOV",
		   "I couldn't find sSliceArray.asSlice[0].dPhaseFOV!",
		   3);
    COUTnl("\t\t\t["+str_value+"]\n");
            
    // thickness
    COUT("Parsing for partition thickness:...");
    if(cso_measAscFile.scanFor("sSliceArray.asSlice[0].dThickness", 	&str_value))
	f_thickness             =	atof(str_value.c_str());
    else
	error_exit("parsing for partition thickness",
		   "I couldn't find sSliceArray.asSlice[0].dThickness!",
		   3);
    COUTnl("\t\t["+str_value+"]\n");
    
    // The following calculates the voxel sizes. Note that this is determined from
    //	the FOV along each logical dimension, divided by the number of lines in that
    //	dimension. Also, note that the lines as read from meas.asc (and accounting for
    //	partial Fourier and resolution scaling) are also scaled upwards if necessary to
    //	the next highest power of two. The voxel dimensions are those of the *final*
    //	volume, not necessarily those of the scanned k-space volume.
    GV_logicalSpaceSize(0)	=	ROLines/2;			// Twice oversampled
    GV_logicalSpaceSize(1)	=	PELines;
    GV_logicalSpaceSize(2)	=	Gb_3D ? slices : 1.;		// For normalisation
    GV_logicalSpaceSize.nextPowerOf2();
        
    GV_FOV(0)			=	f_readOutFOV;
    GV_FOV(1)			=	f_phaseFOV;
    GV_FOV(2)			=	f_thickness;
    
    GV_voxelDimension		=	GV_FOV / GV_logicalSpaceSize;
    
    COUT("Parsing for Siemens normal vector:...");
    COUTnl("\t\t[..]\n");
    sliceNormal_parse(cso_measAscFile);
    
    COUT("Parsing for Siemens position vector:...");
    COUTnl("\t\t[..]\n");
    slicePosition_parse(cso_measAscFile);
    
    GM_vox2ras	= vox2ras_dfmeas(	cso_measAscFile, 
    					cso_metaAscFile);    
                
    COUT("Writing " + str_3DFile + ":...");
    CMatrix<int>	M_3D(1, 1);
    M_3D(0, 0)		= (int) Gb_3D;
    M_3D.fprint(str_baseDir+"/"+str_3DFile, "Boolean: Is this a 3D sequence");
    COUTnl("\t\t\t\t[OK]\n");	

    COUT("Writing " + str_echoListFile + ":...");
    CMatrix<int>	M_echoList(1, 2);
    M_echoList(1)	= echoes-1;
    M_echoList.fprint(str_baseDir+"/"+str_echoListFile, "Number of echoes in sequence");
    COUTnl("\t\t\t[OK]\n");	
    
    COUT("Writing " + str_repListFile + ":...");
    CMatrix<int>	M_repList(1, 2);
    M_repList(1)	= reps;
    M_repList.fprint(str_baseDir+"/"+str_repListFile, "Number of repetitions (measurements) in sequence");
    COUTnl("\t\t\t\t[OK]\n");
    
    COUT("Writing " + str_kListFile + ":...");
    CMatrix<int>	M_kList(1, 2);
    M_kList(1)	        = slices-1;
    M_kList.fprint(str_baseDir+"/"+str_kListFile, "Number of slices (partitions) in sequence");
    COUTnl("\t\t\t\t[OK]\n");
    
    COUT("Writing " + str_ROpePCFile + ":...");
    CMatrix<int>	M_ROpePC(1, 3);
    M_ROpePC(0)	        = ROLines;
    M_ROpePC(1)	        = PELines;
    M_ROpePC.fprint(str_baseDir+"/"+str_ROpePCFile, "Readout / Phase Encoding / Phase Correction");
    COUTnl("\t\t\t\t[OK]\n");
    
    COUT("Writing " + str_rotationFile + ":...");
    GM_vox2ras.fprint(str_baseDir+"/"+str_rotationFile, "rotation matrix in RAS (scanner rotation matrix is LPS2XYZ)\n#\t\tIf matrix is an unscaled linear combination of the voxelDimensions,\n#\t\tthen no rotation matrix data was found in meas.asc");
    COUTnl("\t\t\t\t[OK]\n");
    
    COUT("Writing " + str_voxelDimFile + ":...");
    GV_voxelDimension.fprint(str_baseDir+"/"+str_voxelDimFile, "voxel dimensions in scanner logical coordinate system\n#\t\t\t\t[ReadOut PhaseEncode SliceSelect] mm");
    COUTnl("\t\t\t[OK]\n");
    
    COUT("Writing " + str_mriParamFile + ":...");
    M_mriParam(0)	= MRI_TR;
    M_mriParam(1)	= v_flipAngle;
    M_mriParam(2)	= MRI_TI;
    M_mriParam.fprint(str_baseDir+"/"+str_mriParamFile,
    			"TR[0],dFlipAngleDegree,alTI[0],alTE[0],...,alTE[echoes-1],");
    COUTnl("\t\t\t[OK]\n");
    
    COUT("normal termination\n");
    
    return 0;    
}
