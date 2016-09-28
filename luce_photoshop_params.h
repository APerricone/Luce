// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#pragma once 

#include "PIActions.h"
#include "PITerminology.h"

#define vendorName			"Amico Perry"
#define plugInName			"Luce"
#define plugInAETEComment	"Amico Perry luce is beauty and cool"
#define plugInSuiteID		'AmicoPerry'
#define plugInClassID		plugInSuiteID
#define plugInEventID		plugInClassID
#define plugInUniqueID		"84BFBC73-7D64-472b-899C-6C15E127DD58"
// use greatGUID inside tools menu of Visual studio ^^

#define plugInCopyrightYear  "2012"
#define plugInDescription "Amico Perry luce is beauty and cool"

// Parameters
//#define keyType					'Type'
#define keyXPos						'XPos'
#define keyYPos						'YPos'
//#define keyAngle					'Angl'
#define keyAttenuation				'Atte'
#define keyAlgorithm				'Algo'
#define keyPositiveScale			'bPsS'
#define keyNegativeScale			'bNgS'
#define keyPositiveNormalize		'bPsN'
#define keyNegativeNormalize		'bNgN'
#define keyAddSource				'bASr'
#define keySourceZero				'bSZr'
#define keySourcePositiveNormalize	'bSPN'
#define keySourceNegativeNormalize	'sBNN'
#define keyZero0					'Zer0'
#define keyZero1					'Zer1'
#define keyZero2					'Zer2'
#define keyZero3					'Zer3'
#define keyZero4					'Zer4'
#define keyNumThread				'numT'

#define typeLight			'lght'
#define typePoint			'lgtP'
#define typeDirectional		'lgtD'

#define typeAttenuation		'Attn'
#define typeLinear			'Line'
#define typeQuadratic		'Quad'

#define typeAlgorithm		'Algo'
#define typeZero			'Zero'
#define typeLightForm		'Lgfr'

#define ALLKEYS	{ keyType,keyXPos,keyYPos,keyAngle,						\
			keyAttenuation,keyAlgorithm,								\
			keyPositiveScale,keyNegativeScale,							\
			keyPositiveNormalize,keyNegativeNormalize,					\
			keyAddSource,keySourceZero,									\
			keySourcePositiveNormalize,keySourceNegativeNormalize,		\
			keyZero0,keyZero1,keyZero2,keyZero3,keyZero4 }
