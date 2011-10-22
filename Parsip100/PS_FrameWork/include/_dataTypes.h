#ifndef DATATYPES_H
#define DATATYPES_H

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p) {delete (p); (p) = NULL;}}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if( (p) ) {delete [] (p); (p) = NULL;}}
#endif

#ifndef COMFORT_TYPES
#define COMFORT_TYPES 

typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32;
typedef	char		I8;
typedef	short		I16;
typedef	 int		I32;

#if defined(_MSC_VER)
typedef unsigned __int64	U64;
typedef	__int64				I64;
#else
typedef unsigned long long	U64;
typedef	long long		I64;
#endif

typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;

#endif



//typedef unsigned int	U32;
//typedef int				S32;
//
//typedef unsigned	short	U16;
//typedef short				S16;
//
//typedef unsigned char	U8;
//typedef char			S8;	



//Overall progress with status message and percent of progress
typedef void (*OnOverallProgress)( const char* strStatus, float percent);

//Details stage progress
typedef void (*OnStageProgress)( int iMin, int iMax, int iVal);



#endif
