/******************************************************************************
* File Name          : filters.h
* Date First Issued  : 03/09/2019
* Board              : DiscoveryF4
* Description        : Various filters for filtering readings
*******************************************************************************/

#ifndef __FILTERS
#define __FILTERS

#define ADCFILTERTYPE_NONE	0	// Skip filtering
#define ADCFILTERTYPE_IIR1	1	// Single pole IIR
#define ADCFILTERTYPE_IIR2	2	// Second order IIR

#define ADCFILT_Z1 0	// Index in ADCFILTWORK for iir "z-1"
#define ADCFILT_Z2 1	// Index in ADCFILTWORK for iir "z-2"

#define ADCFILT_COEF1 0 // Index in ADCFILTCOEF for first coefficient
#define ADCFILT_COEF2 1 // Index in ADCFILTCOEF for second coefficient

/* Filtered return value. */
union ADCFILTEREDVAL
{
	float	f;
	uint32_t ui;
	int32_t  s;
};

/* Intermediate, temporary, values for filter computation. */
#define ADCFILTWORKSZ 2  // Number of 8 byte items in work union
union ADCFILTWORK
{
	float     f[ADCFILTWORKSZ*2];
	uint32_t ui[ADCFILTWORKSZ*2];
	 int32_t  s[ADCFILTWORKSZ*2];
	double     d[ADCFILTWORKSZ];
	uint64_t ull[ADCFILTWORKSZ];
	 int64_t  ll[ADCFILTWORKSZ];
};

/* Fixed coefficients, i.e. parameters, for the filter. */
#define ADCFILTCOEFSZ 4 // Number of coefficients
union ADCFILTCOEF
{
	float     f[ADCFILTCOEFSZ];
	uint32_t ui[ADCFILTCOEFSZ];
	 int32_t  s[ADCFILTCOEFSZ];
};


/* With this struct one pointer will convey everything necessary. */
struct FILTERCOMPLETE
{
	union ADCFILTCOEF coef;
	union ADCFILTWORK work;
	uint8_t skipctr;
};

#endif
