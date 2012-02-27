/*---------------------------------------------------------------------------*/
/*  Copyright (C) Siemens AG 1998  All Rights Reserved.  Confidential        */
/*---------------------------------------------------------------------------*/
/*
 * Project: NUMARIS/4
 *    File: comp\Measurement\Sequence\SeqDefines.h@@\main\97
 * Version: 
 *  Author: STOESTF5
 *    Date: Mi 12.06.2002 13:54 
 *
 *    Lang: CPP
 *
 * Descrip: 
 *
 * Functns: n.a.
 *
 *---------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* Include control                                                          */
/*--------------------------------------------------------------------------*/
#ifndef __SeqDefines
#define __SeqDefines

//-----------------------------------------------------------------------------
// Class prototypes and local typedefs for it.
// (see also the NUCLEUS_xxx defines in this file)
//-----------------------------------------------------------------------------
#ifdef __cplusplus
  class   MeasNucleus;
  class   MeasNucleusSet;
  class   MeasKnownNuclei;
  typedef MeasNucleus      SEQ_MEASNUCLEUS;
  typedef MeasNucleusSet   SEQ_MEASNUCLEUSSET;
  typedef MeasKnownNuclei  SEQ_MEASKNOWNNUCLEI;
#endif

//-----------------------------------------------------------------------------
// Structure to provide the "namespace" SEQ
//-----------------------------------------------------------------------------
struct SEQ
{
  //---------------------------------------------------------------------------
  //  Enums for sequence parameter                                           
  //---------------------------------------------------------------------------
  enum Switch     
  {
    NO      = 0x1,
    OFF     = 0x1,
    DISABLE = 0x1,
    YES     = 0x2,
    ON      = 0x2,
    ENABLE  = 0x2,
    ALLOWED = 0x2
  };

  enum DisplayMode 
  {
    DM_OFF  = 0x01,
    DM_SHOW = 0x02,
    DM_EDIT = 0x04
  };

  enum UsedADC
  {
    ADC1 = 0x01,
    ADC2 = 0x02,
    ADC3 = 0x04,
    ADC4 = 0x08,
    ADC5 = 0x10,
    ADC6 = 0x20,
    ADC7 = 0x40,
    ADC8 = 0x80
  };

  enum AveragingMode
  {
    INNER_LOOP = 0x01,
    OUTER_LOOP = 0x02
  };
  
  enum Dimension
  {
    DIM_1 = 0x01,
    DIM_2 = 0x02,
    DIM_3 = 0x04
  };

  enum MultiSliceMode
  {
    MSM_SEQUENTIAL  = 0x01,
    MSM_INTERLEAVED = 0x02,
    MSM_SINGLESHOT  = 0x04
  };

  enum SimultaneousExcitation
  {
    HADAMARD_OFF     = 0x01,
    HADAMARD_2SLICES = 0x02
  };
  
  enum PartialFourierFactor
  {
    PF_HALF = 0x01,
    PF_5_8  = 0x02,
    PF_6_8  = 0x04,
    PF_7_8  = 0x08,
    PF_OFF  = 0x10
  };

  enum FatSuppression
  {
    FAT_SATURATION         = 0x01,
    WATER_EXCITATION       = 0x02,
    FAT_SUPPRESSION_OFF    = 0x04,
    FAT_SATURATION_QUICK   = 0x08
  };

  enum FatSatMode
  {
    FAT_SAT_WEAK         = 0x01,
    FAT_SAT_STRONG       = 0x02
  };
  
  enum WaterSuppression
  {
    WATER_SATURATION          = 0x01,
    FAT_EXCITATION            = 0x02,
    WATER_SUPPRESSION_OFF     = 0x04,
    WATER_SATURATION_QUICK    = 0x08,
    WATER_SUPPRESSION_PARTIAL = 0x10
  };

  enum Dixon
  {
    DIXON_NONE             = 0x01,
    DIXON_WATER_IMAGE      = 0x02,
    DIXON_FAT_IMAGE        = 0x04,
    DIXON_WATER_FAT_IMAGES = 0x08
  };
  
  enum Inversion
  {
    SLICE_SELECTIVE  = 0x01,
    VOLUME_SELECTIVE = 0x02,
    INVERSION_OFF    = 0x04
  };
  
  enum SeriesMode
  {
    ASCENDING   = 0x01,
    DESCENDING  = 0x02,
    INTERLEAVED = 0x04
  };
  
  enum SliceOrientation
  {
    ORTHOGONAL     = 0x01,
    SINGLE_OBLIQUE = 0x02,
    DOUBLE_OBLIQUE = 0x04
  };
  
  enum RFPulseType
  {
    RF_FAST      = 0x01,
    RF_NORMAL    = 0x02,
    RF_LOW_SAR   = 0x04,
    RF_OPTIMIZED = 0x08
  };
  
  enum RFPulseShape
  {
    RF_BINOMIAL     = 0x01,
    RF_HYPERSECANT  = 0x02,
    RF_RECTANGULAR  = 0x04
  };
  
  enum Gradients
  {
    GRAD_FAST                  = 0x01,
    GRAD_NORMAL                = 0x02,
    GRAD_WHISPER               = 0x04,
    GRAD_FAST_GSWD_RISETIME    = (GRAD_FAST    | 0x10),
    GRAD_NORMAL_GSWD_RISETIME  = (GRAD_NORMAL  | 0x10),
    GRAD_WHISPER_GSWD_RISETIME = (GRAD_WHISPER | 0x10)
  };

  enum GradientAxis
  {
    AXIS_UNDEFINED = 0x00,
    AXIS_PHASE     = 0x01,
    AXIS_READOUT   = 0x02,
    AXIS_SLICE     = 0x04
  };

  enum Direction
  {
    DIR_ASCENDING  = 0x01,
    DIR_DESCENDING = 0x02
  };

  enum ReconstructionMode
  {
    RECONMODE_MAGNITUDE  = 0x01,
    RECONMODE_PHASE      = 0x02,
    RECONMODE_REAL_PART  = 0x04,
    RECONMODE_MAGN_PHASE = 0x08,
    RECONMODE_REAL_PHASE = 0x10
  };

  enum RegriddingMode
  {
    REGRID_NONE        = 0x01,
    REGRID_TRAPEZOIDAL = 0x02,
    REGRID_SINUSOIDAL  = 0x04
  };

  enum FilterMode
  {
    FILTER_WEAK   = 0x01,
    FILTER_MEDIUM = 0x02,
    FILTER_STRONG = 0x04,
    FILTER_FREE   = 0x08
  };

  enum EllipticalFilterMode
  {
    ELLIPTICAL_FILTER_INPLANE   = 0x01,
    ELLIPTICAL_FILTER_VOLUME    = 0x02
  };

  enum Base
  {
    BASE2 = 1
  };

  enum Increment
  {
    INC_NORMAL           = 1,
    INC_BASE2            = 2,
    INC_FIX              = 3,
    INC_TURBO_FACTOR     = 4,
    INC_EPI_FACTOR       = 5,
    INC_SEGMENTED        = 6,
    INC_TGSE_FACTOR      = 7,
    INC_GRE_SEGMENTS     = 8,
    INC_SINGLESHOT       = 9,
    INC_TWICE_EPI_FACTOR = 10,
    INC_64 = 11

  };

  enum SequenceCard
  {
    SEQUENCE_CARD_NONE         = 1,
    SEQUENCE_CARD_IMAGING      = 2,
    SEQUENCE_CARD_SPECTROSCOPY = 3
  };

  enum ApplicationCard
  {
    APPLICATION_CARD_NONE         = 1,
    APPLICATION_CARD_FMRI         = 2,
    APPLICATION_CARD_DIFF         = 3,
    APPLICATION_CARD_TOF_CE       = 4,
    APPLICATION_CARD_PERF         = 5,
    APPLICATION_CARD_PC_FLOW      = 6,
    APPLICATION_CARD_TOF_PCANGIO  = 7,
    APPLICATION_CARD_INLINE       = 8,
    APPLICATION_CARD_EVA_CUSTOM   = 9
  };

  enum ApplicationCardName
  {
    APPLICATION_CARD_NAME_NONE = 1,
    APPLICATION_CARD_NAME_BOLD = 2,
    APPLICATION_CARD_NAME_FMRI = 3,
    APPLICATION_CARD_NAME_PERF = 4,
    APPLICATION_CARD_NAME_DIFF = 5,
    APPLICATION_CARD_NAME_TOF  = 6,
    APPLICATION_CARD_NAME_PC   = 7,
    APPLICATION_CARD_NAME_FLOW = 8,
    APPLICATION_CARD_NAME_CE   = 9,
    APPLICATION_CARD_NAME_MRA  = 10,
    APPLICATION_CARD_NAME_INLINE  = 11,
    APPLICATION_CARD_NAME_EVA_CUSTOM  = 12
  };

  enum DiffusionMode
  {
    DIFFMODE_NONE             = 0x01,
    DIFFMODE_ONE_SCAN_TRACE   = 0x02,
    DIFFMODE_ORTHOGONAL       = 0x04,
    DIFFMODE_SLICE            = 0x08,
    DIFFMODE_READ             = 0x10,
    DIFFMODE_PHASE            = 0x20,
    DIFFMODE_THREE_SCAN_TRACE = 0x40,
    DIFFMODE_FREE             = 0x80,
    DIFFMODE_TENSOR           = 0x100,
    DIFFMODE_DIAGONAL         = 0x200
  };

  enum DiffReconMode
  {
    DIFFRECON_NONE                = 0x01,
    DIFFRECON_DIFF_WEIGHTED_IMAGE = 0x02,
    DIFFRECON_ADC_MAP             = 0x04,
    DIFFRECON_TRACE               = 0x08,
    DIFFRECON_AVERAGE_ADC_MAP     = 0x10
  };

  ////////////////////////////////////////////////////////////////////////////
  //
  // Values for the bit-field ucBOLDParadigmArray[].
  // For each repetition (within index range [0,1023], 1st measurement corresponds to repetition index 0) 
  // there are two element-bits representing the paradigm for the corresponding repetition.
  // 
  // The paradigm is periodic after a number of initial measurements, this periodicity 
  // (MrProt::ParadigmPeriodicity())can therefore be used
  // to determine the paradigm also for repetition indices > 1023.
  //
  // MrProt has two methods to get/set the element-bits:
  //
  //	SEQ::ParadigmElem MrProt::ParadigmByRepetition(long lRepetition)
  //		returns the paradigm-element-bits 
  //		corresponding to the given repetition
  //
  //	SEQ::ParadigmElem MrProt::ParadigmByRepetition(long lRepetition, 
  //													SEQ::ParadigmElem eParadigm)
  //		will set the paradigm-element-bits 
  //		corresponding to the given repetition
  //
  // MrProt has a method to determine if the actual protocol 
  // has an active BOLD measurement
  //	bool isBOLDMeasurement()
  //		returns true, if lParadigmPeriodicity != 0
  //	
  ////////////////////////////////////////////////////////////////////////////
  enum ParadigmElem
  {
    PARADIGM_IGNORE,            // repetition is ignored
	PARADIGM_ACTIVATED,         // repetition is baseline
	PARADIGM_BASELINE           // repetition is activated
  };

  enum AdjShim
  {
    ADJSHIM_TUNEUP      = 0x01,    // use tuneup shim setting
    ADJSHIM_STANDARD    = 0x02,    // perform standard shim procedure
    ADJSHIM_ADVANCED    = 0x04,    // perform advanced shim procedure
    ADJSHIM_INTERACTIVE = 0x20,    // perform interactive shim procedure (internal use)
    ADJSHIM_MEAS_ONLY   = 0x80,    // measurement only, no calculation (internal use)
    ADJSHIM_CALC_ONLY   = 0x40     // calculation only, no measurement (internal use)
  };

  enum AdjFre
  {
    ADJFRE_FINE           = 0x01,  // use narrow band adj/fre for exact results
    ADJFRE_COARSE         = 0x02,  // use wide band adj/fre for coarse results
    ADJFRE_FINE_NOTVOLSEL = 0x04   // use narrow band adj/fre for exact results (not volume selective)
  };

  enum MainOrientation
  {
    SAGITTAL   = 0,
    CORONAL    = 1,
    TRANSVERSE = 2
  };

  enum MainDirection
  {
    R_TO_L = SAGITTAL,         // (R)ight to (L)eft
    L_TO_R = 4,                // (L)eft to (R)ight
    A_TO_P = CORONAL,          // (A)nterior to (P)osterior
    P_TO_A = 8,                // (P)osterior to (A)nterior
    F_TO_H = TRANSVERSE,       // (F)eet to (H)ead
    H_TO_F = 16                // (H)ead to (F)eet   
  };
  
  enum RotationCode
  {
    SAG_TO_TRA_TO_COR = 1,
    SAG_TO_COR_TO_TRA = 2,
    COR_TO_SAG_TO_TRA = 3,
    COR_TO_TRA_TO_SAG = 4,
    TRA_TO_COR_TO_SAG = 5,
    TRA_TO_SAG_TO_COR = 6
  };

  enum PhaseCyclingType
  {
    PHASE_CYCLING_NONE             = 0x01,
    PHASE_CYCLING_AUTO             = 0x02,
    PHASE_CYCLING_TWOSTEP          = 0x04,
    PHASE_CYCLING_EIGHTSTEP        = 0x08,
    PHASE_CYCLING_EXORCYCLE        = 0x10,
    PHASE_CYCLING_SIXTEENSTEP_EXOR = 0x20,
    PHASE_CYCLING_SMART_AVERAGE    = 0x40
  };

  enum PhapsMode
  {
    PHAPS_NONE       = 0x01,
    PHAPS_ON         = 0x02,
    PHAPS_SUM_ONLY   = 0x04,
    PHAPS_DIFF_ONLY  = 0x08
  };

  enum PhaseEncodingType
  {
    PHASE_ENCODING_FULL       = 0x01,
    PHASE_ENCODING_ELLIPTICAL = 0x02,
    PHASE_ENCODING_WEIGHTED   = 0x04
  };

  enum RespirationPhase
  {
    PHASE_INSPIRATION = 0x01,
    PHASE_EXPIRATION  = 0x02
  };

  enum PhysioSignal
  {
    SIGNAL_NONE        = 0x01,
    SIGNAL_EKG         = 0x02,
    SIGNAL_PULSE       = 0x04,
    SIGNAL_EXT         = 0x08,
    SIGNAL_CARDIAC     = 0x0E,  /* the sequence usually takes this */
    SIGNAL_RESPIRATION = 0x10,
    SIGNAL_ALL         = 0x1E
  };

  enum PhysioMethod
  {
    METHOD_NONE        = 0x01,
    METHOD_TRIGGERING  = 0x02,
    METHOD_GATING      = 0x04,
    METHOD_RETROGATING = 0x08,
    METHOD_SOPE        = 0x10,
    METHOD_ALL         = 0x1E
  };

  enum AcquisitionWindowCalculationMode
  {
    AWCM_STANDARD            = 0x01,
    AWCM_CONSIDER_LINES      = 0x02,
    AWCM_CONSIDER_PARTITIONS = 0x04
  };

  enum ExcitationPulse
  {
    EXCITATION_SLICE_SELECTIVE  = 0x01,
    EXCITATION_VOLUME_SELECTIVE = 0x02
  };

  enum SaturationRecovery
  {
    SATREC_NONE             = 0x01,
    SATREC_SLICE_SELECTIVE  = 0x02,
    SATREC_VOLUME_SELECTIVE = 0x04
  };

  enum Device
  {
    DEVICE_DC  = 0x00,
    DEVICE_RX  = 0x01,
    DEVICE_TX  = 0x02,
    DEVICE_ALL = 0xff
  };

  enum ArrhythmiaDetection
  {
    AD_NONE         = 0x01,
    AD_TIMEBASED    = 0x02,
    AD_PATTERNBASED = 0x04
  };

  enum FlowSensitivity
  {
    FLOW_SENSITIVITY_SLOW   = 0x01,
    FLOW_SENSITIVITY_MEDIUM = 0x02,
    FLOW_SENSITIVITY_FAST   = 0x04
  };

  enum SharedDimension
  {
    SHARED_DIM_NONE         = 1,
    SHARED_DIM_PHASES       = 2,
    SHARED_DIM_SETS         = 3,
    SHARED_DIM_REPETITIONS  = 4,
    SHARED_DIM_ECHOES       = 5,
    SHARED_DIM_FREE         = 6,
    SHARED_DIM_ACQUISITIONS = 7,
    SHARED_DIM_PARTITIONS   = 8
  };

  enum ICEMode
  {
    ICE_MODE_MIP_SLAB_OVERLAP  = 0x01,
    ICE_MODE_CISS              = 0x02,
    ICE_MODE_DESS              = 0x04,
    ICE_MODE_RE_DEPHASED       = 0x08
  };

  enum FlowCompensation
  {
    FLOW_COMPENSATION_NO            = 0x01,
    FLOW_COMPENSATION_YES           = 0x02,
    FLOW_COMPENSATION_READOUT_ONLY  = 0x04,
    FLOW_COMPENSATION_SLICESEL_ONLY = 0x08,
  };

  enum Tagging
  {
    TAGGING_NONE     = 0x01,
    TAGGING_GRID_TAG = 0x02,
    TAGGING_LINE_TAG = 0x04
  };

  enum OnlineFFT
  {
    ONLINE_FFT_NONE      = 1,
    ONLINE_FFT_PHASE     = 2,
    ONLINE_FFT_PARTITION = 3
  };

  enum FlowDir
  {
      FLOW_DIR_PHASE    = 0x01,
      FLOW_DIR_READ     = 0x02,
      FLOW_DIR_SLICESEL = 0x04,
      FLOW_DIR_FREE     = 0x08,
      FLOW_DIR_NONE     = 0x10        // not for protocol use!
  };

  enum PCAngioFlowMode
  {
      PCANGIO_MODE_SINGLE_VELOCITY  = 0x01,
      PCANGIO_MODE_SINGLE_DIRECTION = 0x02,
      PCANGIO_MODE_FREE = 0x04
  };

  enum PCAlgorithm
  {
    PC_ALGORITHM_NONE      = 1,
    PC_ALGORITHM_SUBMATRIX = 2,
    PC_ALGORITHM_MARGOSIAN = 3
  };

  enum NormalizeFilterAlgo
  {
    NORMALIZE_FILTER_ALGO_STANDARD       = 1,
    NORMALIZE_FILTER_ALGO_WANG_MEININGER = 2,
    NORMALIZE_FILTER_ALGO_PAT_ABDOMINAL  = 3,
    NORMALIZE_FILTER_ALGO_HEAD           = 4,
    NORMALIZE_FILTER_ALGO_EXPONENTIAL    = 5
  };

  enum ReorderingScheme
  {
    RS_LIN_IN_PAR          = 1,   // lines in partition
    RS_PAR_IN_LIN          = 2,   // partitions in lines
    RS_ARBITRARY           = 3    // arbitrary reordering schemes e.g. centric reordering
  };

  enum PSatMode
  {
    PSAT_NONE         = 0x01,
    PSAT_SINGLE_REG   = 0x02,
    PSAT_DOUBLE_REG   = 0x04,
    PSAT_SINGLE_QUICK = 0x08,
    PSAT_DOUBLE_QUICK = 0x10
  };

  enum RSatMode
  {
      RSAT_REG   = 0x01,
      RSAT_QUICK = 0x02
  };
  
  enum ShowOffline
  {
    SO_SHOW_YES          = 1,   
    SO_SHOW_NO           = 2
  };

  enum FilterType
  {
    FILTER_NONE  = 0x01,
    FILTER_RAW   = 0x02,        // RAW_FILTER clashes with prot.h
    LARGE_FOV    = 0x04,
    NORMALIZE    = 0x08,
    ELLIPTICAL   = 0x10,
    HAMMING      = 0x20
  };

  enum Reordering
  {
      REORDERING_LINEAR    = 0x01,
      REORDERING_CENTRIC   = 0x02,
      REORDERING_LINE_SEGM = 0x04,
      REORDERING_PART_SEGM = 0x08,
      REORDERING_FREE_0    = 0x10,
      REORDERING_FREE_1    = 0x20,
      REORDERING_FREE_2    = 0x40,
      REORDERING_FREE_3    = 0x80
  };

  enum PhaseStabScanPosition
  {
    AFTER        = 1,
    BEFORE       = 2
  };

  enum FlowDirDisplay
  {
    FLOW_DIR_R_TO_L     = R_TO_L,       // (R)ight (to) (L)eft
    FLOW_DIR_L_TO_R     = L_TO_R,       // (L)eft (to) (R)ight
    FLOW_DIR_A_TO_P     = A_TO_P,       // (A)nterior (to) (P)osterior
    FLOW_DIR_P_TO_A     = P_TO_A,       // (P)osterior (to) (A)nterior
    FLOW_DIR_F_TO_H     = F_TO_H,       // (F)eet (to) (H)ead
    FLOW_DIR_H_TO_F     = H_TO_F,       // (H)ead (to) (F)eet   
    FLOW_DIR_TP_IN      = 0x020,        // (T)hrough (P)lane (In)flow
    FLOW_DIR_TP_OUT     = 0x040,        // (T)hrough (P)lane (Out)flow
    FLOW_DIR_INVALID    = 0x080         // No flow encoding, not for Protocol use   
  };
  
  
  enum TriggerMode
  {
    TRIGGER_STANDARD     = 0x01,
    TRIGGER_STEADY_STATE = 0x02
  };

  enum ProtocolPackage
  {
    PROTPACKAGE_NONE      = 0x00,
    PROTPACKAGE_MR_GUIDED = 0x01
  };
  
  enum RfasSelMode
  {
    RFAS_SEL_NORMAL       = 0x00,
    RFAS_SEL_TOGGLE_GAIN  = 0x01
  };


  //  Defines for dynamic image numbering
  enum ImageNumbSag
  {
      IMAGE_NUMB_R_TO_L = 0x0,
      IMAGE_NUMB_L_TO_R = 0x1
  };

  enum ImageNumbCor
  {
      IMAGE_NUMB_A_TO_P = 0x0,
      IMAGE_NUMB_P_TO_A = 0x1
  };

  enum ImageNumbTra
  {
      IMAGE_NUMB_F_TO_H = 0x0,
      IMAGE_NUMB_H_TO_F = 0x1
  };

  enum ImageNumbMSMA
  {
      IMAGE_NUMB_SCT = 0x0,      // sagittal images precede coronal images,
                                 // coronal images precede transverse images
      IMAGE_NUMB_STC = 0x1,      // sagittal images precede transverse images,
                                 // transverse images precede coronal images
      IMAGE_NUMB_CTS = 0x2,      // coronal images precede transverse images,
                                 // transverse images precede sagittal images
      IMAGE_NUMB_CST = 0x3,      // coronal images precede sagittal images,
                                 // sagittal images precede tranversal images
      IMAGE_NUMB_TSC = 0x4,      // tranversal images precede sagittal images,
                                 // sagittal images precede coronal images
      IMAGE_NUMB_TCS = 0x5       // tranversal images precede coronal images,
                                 // coronal images precede sagittal images
  };

  enum DecouplingType
  {
    DECOUPLING_NONE  = 0x1,      // No decoupling  
    DECOUPLING_WALTZ = 0x2,      // WALTZ decoupling
    DECOUPLING_MLEV  = 0x4,      // MLEV decoupling
    DECOUPLING_CW    = 0x8,      // CW decoupling
  };

  enum NOEType
  {
    NOE_NONE         = 0x1,      // No NOE 
    NOE_RECTANGULAR  = 0x2       // Rectangular NOE shapes
  };

  enum ExcitationType
  {
    EXCITATION_STANDARD  = 0x1,  // Standard excitation pulse type
    EXCITATION_ADIABATIC = 0x2   // Adiabatic excitation pulse type
  };

  enum PulseMode
  {
	EXCIT_MODE_2D_PENCIL = 0x1,		// The timing used is a gradient-echo sequence
    EXCIT_MODE_GRE       = 0x2		// The timing used is a 90 - 180 spin-echo sequence
  };

  enum ExamAnatomy
  {
	EXAM_ANATOMY_ABDOMEN = 0x01,    // selects an appropriate tracking factor for the abdomen
	EXAM_ANATOMY_APEX    = 0x02,	// selects an appropriate tracking factor for the apex
	EXAM_ANATOMY_LCA     = 0x03,	// selects an appropriate tracking factor for the LCA
	EXAM_ANATOMY_OTHER   = 0x04		// the user can alter the tracking factor
  };

  enum PATSelMode
  {
	PAT_MODE_NONE   = 0x01,
	PAT_MODE_GRAPPA = 0x02,
	PAT_MODE_SENSE  = 0x03
  };
  
  enum PATRefScanMode
  {
	PAT_REF_SCAN_UNDEFINED  = 0x01, // e.g. if no PAT is selected
	PAT_REF_SCAN_INPLACE    = 0x02, // sequence supplies inplace reference lines
	PAT_REF_SCAN_EXTRA      = 0x04, // sequence supplies extra reference lines
	PAT_REF_SCAN_PRESCAN    = 0x08  // sequence does not supply reference lines, the data must have been acquired with a previous measurement
  };

  enum ChronPos
  {
	CHRON_POS_BEFORE_ECHO_TRAIN				= 0x01,	// The navigator is actually executed before the image echo train
	CHRON_POS_AFTER_ECHO_TRAIN				= 0x02, // The navigator is actually executed after the image echo train
	CHRON_POS_BEFORE_AND_AFTER_ECHO_TRAIN   = 0x03  // The navigator is actually executed before and after the image echo train
  };

  enum RspCompMode
  {
    RESP_COMP_BREATH_HOLD_AND_FOLLOW  = 0x01, // the navigator data is used to mitor the patient respiratory curve
    RESP_COMP_GATE_AND_FOLLOW         = 0x02, // navigator result is in a acceptance window, slices are shifted and measurement continues
    RESP_COMP_OFF                     = 0x04, // no respiratory compensation
    RESP_COMP_GATE                    = 0x08, // data is aquired only if diaphragma position is in a acceptance window
    RESP_COMP_BREATH_HOLD             = 0x10, // slices are aquired after the operator pressed the scan button of the online display
    RESP_COMP_BREATH_HOLD_AND_MONITOR = 0x20  // similar to BREATH_HOLD_AND_FOLLOW, the differnce here is that the position from one breathhold to the next  is not adjusted
  };

  enum BreathHoldNavigatorCapability
  {
    BREATHHOLD_NAVIGATOR_RERUN = 0x01, // wether sequence allows user to rerun breathhold
    BREATHHOLD_NAVIGATOR_SCAN  = 0x02  // wether sequence allows user to scan breathhold 
  };

  enum DiffusionDirectionality
  {
    DIFFDIR_NONE         = 0x01,    // none specifies diffusion conditions
    DIFFDIR_DIRECTIONAL  = 0x02,    // specifies whether diffusion conditions for the frame are directional with respect to direction
    DIFFDIR_ISOTROPIC    = 0x04     // specifies whether diffusion conditions for the frame are isotropic with respect to direction
  };

  enum TRFillPosition
  {
    BEFORE_ACQUISITION = 1,     // the delay time for each acquisition (TRFill) is before the acquisition
    AFTER_ACQUISITION  = 2      // the delay time for each acquisition (TRFill) is after the acquisition
  };

  enum PALIMode
  {
    PALI_MODE_NORMAL     = 1,       // normal PALI mode for patient examinations
    PALI_MODE_SERVICE    = 2        // PALI disabled (for some service sequences only, not for patient examinations!!!!)
  };


  //---------------------------------------------------------------------------
  // Defines can be used in two ways:
  // 
  // 1. SEQ::NUCLEUS_1H  -> SEQ::MeasNucleus("1H")
  //                     -> SEQ_MEASNUCLEUS("1H")
  //                     -> MeasNucleus("1H")
  // 2. NUCLEUS_1H       -> MeasNucleus("1H")
  //---------------------------------------------------------------------------
  #ifdef __cplusplus
    typedef SEQ_MEASNUCLEUS     MeasNucleus;
    typedef SEQ_MEASNUCLEUSSET  MeasNucleusSet;
    typedef SEQ_MEASKNOWNNUCLEI MeasKnownNuclei;
  #endif

  #define NUCLEUS_1H          MeasNucleus("1H")
  #define NUCLEUS_3HE         MeasNucleus("3He")
  #define NUCLEUS_7LI         MeasNucleus("7Li")
  #define NUCLEUS_13C         MeasNucleus("13C")
  #define NUCLEUS_17O         MeasNucleus("17O")
  #define NUCLEUS_19F         MeasNucleus("19F")
  #define NUCLEUS_23NA        MeasNucleus("23Na")
  #define NUCLEUS_31P         MeasNucleus("31P")
  #define NUCLEUS_129XE       MeasNucleus("129Xe")
  #define NUCLEUS_NONE        MeasNucleus("")        // Nucleus not defined

  #define NUCLEI_NONE         MeasNucleusSet("")     // Empty set
  #define NUCLEI_ALL          MeasKnownNuclei()      // All known nuclei
};


//-----------------------------------------------------------------------------
// Definitions for array lengths (used in SeqLim.h)
//-----------------------------------------------------------------------------
#define MAX_SEQ_LIM_FILENAMELENGTH    128
#define MAX_SEQ_LIM_INT_PAR           16
#define MAX_SEQ_LIM_FLOAT_PAR         16


//------------------------------------------------------------------------------
// Nametags for Breathhold Navigator used by Sequence and Online Display
//------------------------------------------------------------------------------
const char BREATHOLD_NAMETAG_SCAN[]  =   "BH_SCAN";
const char BREATHOLD_NAMETAG_RERUN[] =   "BH_RRUN";
#endif

/*---------------------------------------------------------------------------*/
/*  Copyright (C) Siemens AG 1998  All Rights Reserved.  Confidential        */
/*---------------------------------------------------------------------------*/
