// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#if (defined(macintosh) || defined(__POWERPC__) || defined(__powerc)) || defined(__APPLE_CC__)
	#include <Carbon.r>
	#include "PIGeneral.r"
	#include "luce_photoshop_params.h"
#elif (defined(MSDOS) || defined(WIN32) || defined(_WIN32) || (defined(__INTEL__) && !defined(__MACH__)) || defined(_MSC_VER))
	#define Rez
	#include "luce_photoshop_params.h"
	#include "PIGeneral.h"
#endif

#include "PIActions.h"

resource 'PiPL' ( 16000, "Luce", purgeable )
{
	{
		Kind { Filter },
		Name { plugInName "..." },
		Category { vendorName },
		Version { (latestFilterVersion << 16 ) | latestFilterSubVersion },

		#ifdef __PIMac__
			
			#if (defined(__i386__))
				
				CodeMacIntel32 { "PluginMain" },
				
				/* If your plugin uses Objective-C, Cocoa, for UI it should not be
				   unloaded, this flag is valid for 32 bit plug-ins only and
			       should not be used in any windows section */
			       
				// off for now as this plug-in has no Objective-C Cocoa {}, 
			
			#endif
			#if (defined(__x86_64__))
				CodeMacIntel64 { "PluginMain" },
			#endif
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "PluginMain" },
			#else
				CodeWin32X86 { "PluginMain" },
			#endif
		#endif

		SupportedModes
		{
			noBitmap, doesSupportGrayScale,
			noIndexedColor, doesSupportRGBColor,
			noCMYKColor, noHSLColor,
			noHSBColor, noMultichannel,
			noDuotone, noLABColor
		},

		HasTerminology
		{
			plugInClassID,
			plugInEventID,
			16000,
			plugInUniqueID
		},
		
		EnableInfo { "in (PSHOP_ImageMode, RGBMode, GrayScaleMode) ||"
					 "PSHOP_ImageDepth == 16 ||"
					 "PSHOP_ImageDepth == 32" },

		PlugInMaxSize { 2000000, 2000000 },
		
		FilterLayerSupport {doesSupportFilterLayers},
		
		FilterCaseInfo
		{
			{
				/* Flat data, no selection */
				inWhiteMat, outWhiteMat,
				doNotWriteOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination,
					
				/* Flat data with selection */
				inWhiteMat, outWhiteMat,
				writeOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination,
				
				/* Floating selection */
				inWhiteMat, outWhiteMat,
				writeOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination,
					
				/* Editable transparency, no selection */
				inWhiteMat, outWhiteMat,
				doNotWriteOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination,
					
				/* Editable transparency, with selection */
				inWhiteMat, outWhiteMat,
				writeOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination,
					
				/* Preserved transparency, no selection */
				inWhiteMat, outWhiteMat,
				doNotWriteOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination,
					
				/* Preserved transparency, with selection */
				inWhiteMat, outWhiteMat,
				writeOutsideSelection,
				filtersLayerMasks, worksWithBlankData,
				copySourceToDestination
			}
		}	
	}
};

resource 'aete' (16000, "Dissolve dictionary", purgeable)
{
	1, 0, english, roman,									/* aete version and language specifiers */
	{
		vendorName,											/* vendor suite name */
		"Adobe example plug-ins",							/* optional description */
		plugInSuiteID,										/* suite ID */
		1,													/* suite code, must be 1 */
		1,													/* suite level, must be 1 */
		{													/* structure for filters */
			plugInName,										/* unique filter name */
			plugInAETEComment,								/* optional description */
			plugInClassID,									/* class ID, must be unique or Suite ID */
			plugInEventID,									/* event ID, must be unique to class ID */
			
			NO_REPLY,										/* never a reply */
			IMAGE_DIRECT_PARAMETER,							/* direct parameter, used by Photoshop */
			{												/* parameters here, if any */
			
				"type",										// parameter name
				keyType,									// parameter key ID
				typeLight,									// parameter type ID
				"type of light",							// optional description
				flagsEnumeratedParameter,					// parameter flags
				
				"xPos",										// parameter name
				keyXPos,									// parameter key ID
				typeFloat,									// parameter type ID
				"x position of light, 1 is right border",	// optional description
				flagsSingleParameter,						// parameter flags

				"yPos",										// parameter name
				keyYPos,									// parameter key ID
				typeFloat,									// parameter type ID
				"y position of light, 1 is bottom border",	// optional description
				flagsSingleParameter,						// parameter flags

				"angle",									// parameter name
				keyAngle,									// parameter key ID
				typeFloat,									// parameter type ID
				"angle of light in degree",					// optional description
				flagsSingleParameter,						// parameter flags

				"attenuation",								// parameter name
				keyAttenuation,								// parameter key ID
				typeAttenuation,							// parameter type ID
				"type of attenuation to apply",				// optional description
				flagsEnumeratedParameter,					// parameter flags
				
				"algorithm",								// parameter name
				keyAlgorithm,								// parameter key ID
				typeAlgorithm,								// parameter type ID
				"algorithm",								// optional description
				flagsEnumeratedParameter,					// parameter flags
				
				"LightScale",								// parameter name
				keyPositiveScale,							// parameter key ID
				typeFloat,									// parameter type ID
				"scale to apply to positive data",			// optional description
				flagsSingleParameter,						// parameter flags
				
				"ShadowScale",								// parameter name
				keyNegativeScale,							// parameter key ID
				typeFloat,									// parameter type ID
				"scale to apply to negative data",			// optional description
				flagsSingleParameter,						// parameter flags

				"normalizePositive",						// parameter name
				keyPositiveNormalize,						// parameter key ID
				typeBoolean,								// parameter type ID
				"if true the value between zero and white "
				"are scale to be between black and white",	// optional description
				flagsSingleParameter,						// parameter flags

				"normalizeNegative",						// parameter name
				keyNegativeNormalize,						// parameter key ID
				typeBoolean,								// parameter type ID
				"if true the value between black and zero are scale "
				"to be between black and negative white",	// optional description
				flagsSingleParameter,						// parameter flags
				
				"addSource",								// parameter name
				keyAddSource,								// parameter key ID
				typeBoolean,								// parameter type ID
				"If true the source will be added to luce"
				"result",									// optional description
				flagsSingleParameter,						// parameter flags
				
				"sourceZero",								// parameter name
				keySourceZero,								// parameter key ID
				typeBoolean,								// parameter type ID
				"If true, when add the source the zero color (and negatives)"
				"is converted to black",					// optional description
				flagsSingleParameter,						// parameter flags

				"sourcePositiveNormalize",					// parameter name
				keySourcePositiveNormalize,					// parameter key ID
				typeBoolean,								// parameter type ID
				"If true, when add the source the color between zero color and "
				"white is scaled to be black to white",		// optional description
				flagsSingleParameter,						// parameter flags

				"sourceNegativeNormalize",					// parameter name
				keySourceNegativeNormalize,					// parameter key ID
				typeBoolean,								// parameter type ID
				"If true, when add the source the color between black and zero "
				"color is scaled to be black to white negative",
															// optional description
				flagsSingleParameter,						// parameter flags

				"zero0",									// parameter name
				keyZero0,									// parameter key ID
				typeFloat,									// parameter type ID
				"First zero value",							// optional description
				flagsSingleParameter,						// parameter flags

				"zero1",									// parameter name
				keyZero1,									// parameter key ID
				typeFloat,									// parameter type ID
				"Second zero value",							// optional description
				flagsSingleParameter,						// parameter flags

				"zero2",									// parameter name
				keyZero2,									// parameter key ID
				typeFloat,									// parameter type ID
				"Third zero value",							// optional description
				flagsSingleParameter,						// parameter flags

				"zero3",									// parameter name
				keyZero3,									// parameter key ID
				typeFloat,									// parameter type ID
				"Fourth zero value",							// optional description
				flagsSingleParameter,						// parameter flags

				"zero4",									// parameter name
				keyZero4,									// parameter key ID
				typeFloat,									// parameter type ID
				"Fifth zero value",							// optional description
				flagsSingleParameter						// parameter flags				
			}
		},
		{													/* non-filter plug-in class here */
		},
		{													/* comparison ops (not supported) */
		},
		{													/* any enumerations */
			typeLight,
			{
				"point",									/* first value */
				typePoint,									
				"point light source",						/* optional description */
				
				"directional",								/* second value */
				typeDirectional,							
				"directional light",						/* optional description */
			},
			typeAttenuation,
			{
				"linear",									/* first value */
				typeLinear,									
				"linear",									/* optional description */
				
				"quadratic",								/* second value */
				typeQuadratic,							
				"quadratic",								/* optional description */
			},
			typeAlgorithm,
			{
				"zero",										/* first value */
				typeZero,									
				"zero algorithm",							/* optional description */
				
				"lightForm",								/* second value */
				typeLightForm,							
				"light form algorithm",						/* optional description */
			}
		}
	}
};
