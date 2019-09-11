// Tools.cpp: 

#include "stdafx.h"
#include "tools.h"

#pragma warning(disable:4996)

const unsigned char mungify_table[] =
{
	0x7A, 0x64, 0x05, 0xF1,
	0x1B, 0x9B, 0xA0, 0xB5,
	0xCA, 0xED, 0x61, 0x0D,
	0x4A, 0xDF, 0x8E, 0xC7
};

const unsigned char mungify_table2[] =
{
	0x05, 0x61, 0x7A, 0xED,
	0x1B, 0xCA, 0x0D, 0x9B,
	0x4A, 0xF1, 0x64, 0xC7,
	0xB5, 0x8E, 0xDF, 0xA0
};

unsigned char mungify_table3[] =
{
	0x20, 0x07, 0x13, 0x61,
	0x03, 0x45, 0x17, 0x72,
	0x0A, 0x2D, 0x48, 0x0C,
	0x4A, 0x12, 0xA9, 0xB5
};

template <typename T>
inline T DWordSwapC( T dw )
{
    unsigned int temp;
    
    temp  =   *((unsigned int *)&dw) >> 24;
    temp |= ((*((unsigned int *)&dw) & 0x00FF0000) >> 8);
    temp |= ((*((unsigned int *)&dw) & 0x0000FF00) << 8);
    temp |= ((*((unsigned int *)&dw) & 0x000000FF) << 24);
    
    return *((T*)&temp);
}

#define DWordSwap DWordSwapC

//Base func

//Output Info
int RusPrintf( char *pRusBuff )
{
	char pValidBuff[ 2048 ];
	memset( pValidBuff, 0, sizeof(pValidBuff) );

#ifdef _WIN32
	CharToOem( pRusBuff, pValidBuff );
	printf( "%s", pValidBuff );
#else
	printf( "%s", pRusBuff );
#endif

	return 1;
}

void LogPrintf( bool iserror, char *fmt, ... )
{
	char string[2048];
	memset( string, 0, sizeof(string) );

	time_t start;
	time(&start);

	tm* timeinfo;
	timeinfo = localtime ( &start );
	sprintf( string, "%d:%d  ", timeinfo->tm_hour, timeinfo->tm_min );

	va_list marker;
	va_start( marker, fmt );
	vsprintf( string + strlen(string), fmt, marker );
	va_end( marker );

#ifdef _WIN32
	HANDLE hConsole;

	if( iserror )
	{
		hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
		SetConsoleTextAttribute( hConsole, 12 );
	}
#endif

	RusPrintf( string );

#ifdef _WIN32
	if( iserror )
	{
		SetConsoleTextAttribute( hConsole, 7 );
	}
#endif

	FILE *fp = NULL;

	if( iserror )
	{
		fp = fopen( "error.log", "ab" );
	}
	else
	{
		fp = fopen( "info.log", "ab" );
	}

	fprintf( fp, "%s", string );
	fclose( fp );
}

char const* Sys_FindArg( char const *pArg, char const *pDefault )
{
	for( int i=0; i < __argc; i++ )
	{
		if( stricmp( __argv[i], pArg ) == 0 )
			return (i+1) < __argc ? __argv[i+1] : "";
	}

	return pDefault;
}

int Sys_FindArgInt( char const *pArg, int defaultVal )
{
	char const *pVal = Sys_FindArg( pArg, NULL );
	if( pVal )
		return atoi( pVal );
	else
		return defaultVal;
}

filedata_t FileRead( char *filename )
{
	filedata_t fpbuff;
	
	FILE *fp = fopen( filename, "rb" );

	if( !fp )
	{
		fpbuff.filelen = 0;
		fpbuff.filebuf = NULL;

		return fpbuff;
	}

	fseek( fp, 0, SEEK_END );
	int len = ftell( fp );
	rewind( fp );

	unsigned char *filebuff = ( unsigned char * )malloc( len );

	fread( filebuff, 1, len, fp );

	fclose( fp );

	fpbuff.filelen = len;
	fpbuff.filebuf = filebuff;

	return  fpbuff;
}

int FileWrite( char *filename, filedata_t filebuff )
{
	FILE *fp = fopen( filename, "ab" );

	if( !fp )
	{
		return 0;
	}

	fwrite(filebuff.filebuf, 1, filebuff.filelen, fp );

	fclose (fp);

	return 1;
}

int FileReWrite( char *filename, filedata_t filebuff )
{
	FILE *fp = fopen( filename, "wb" );

	if( !fp )
	{
		return 0;
	}

	fwrite(filebuff.filebuf, 1, filebuff.filelen, fp );

	fclose (fp);

	return 1;
}

//Packet tools
/*
================
COM_Munge

Anti-proxy/aimbot obfuscation code

  COM_UnMunge should reversably fixup the data
================
*/
void COM_Munge( unsigned char *data, int len, int seq )
{
	int i;
	int mungelen;

	int c;
	int *pc;
	unsigned char *p;

	int j;

	mungelen = len & ~3;
	mungelen /= 4;

	for ( i = 0; i < mungelen; i++ )
	{
		pc = (int *)&data[ i * 4 ];
		c = *pc;

		c ^= ~seq;

		c = DWordSwap( c );
		
		p = ( unsigned char *)&c;
		for ( j = 0 ; j < 4; j++ )
		{
			*p++ ^= ( 0xa5 | ( j << j) | j | mungify_table[ ( i + j ) & 0x0f ] );
		}

		c ^= seq;
		
		*pc = c;
	}
}

/*
================
COM_UnMunge

Anti-proxy/aimbot obfuscation code
================
*/
void COM_UnMunge( unsigned char *data, int len, int seq )
{
	int i;
	int mungelen;

	int c;
	int *pc;
	unsigned char *p;

	int j;

	mungelen = len & ~3;
	mungelen /= 4;

	for ( i = 0; i < mungelen; i++ )
	{
		pc = (int *)&data[ i * 4 ];
		c = *pc;

		c ^= seq;

		p = ( unsigned char *)&c;
		for ( j = 0 ; j < 4; j++ )
		{
			*p++ ^= ( 0xa5 | ( j << j) | j | mungify_table[ ( i + j ) & 0x0f ] );
		}

		c = DWordSwap( c );

		c ^= ~seq;
		
		*pc = c;
	}
}

/*
================
COM_Munge

Anti-proxy/aimbot obfuscation code

  COM_UnMunge should reversably fixup the data
================
*/
void COM_Munge2( unsigned char *data, int len, int seq )
{
	int i;
	int mungelen;

	int c;
	int *pc;
	unsigned char *p;

	int j;

	mungelen = len & ~3;
	mungelen /= 4;

	for ( i = 0; i < mungelen; i++ )
	{
		pc = (int *)&data[ i * 4 ];
		c = *pc;

		c ^= ~seq;

		c = DWordSwap( c );
		
		p = ( unsigned char *)&c;
		for ( j = 0 ; j < 4; j++ )
		{
			*p++ ^= ( 0xa5 | ( j << j) | j | mungify_table2[ ( i + j ) & 0x0f ] );
		}

		c ^= seq;
		
		*pc = c;
	}
}

/*
================
COM_UnMunge

Anti-proxy/aimbot obfuscation code
================
*/
void COM_UnMunge2( unsigned char *data, int len, int seq )
{
	int i;
	int mungelen;

	int c;
	int *pc;
	unsigned char *p;

	int j;

	mungelen = len & ~3;
	mungelen /= 4;

	for ( i = 0; i < mungelen; i++ )
	{
		pc = (int *)&data[ i * 4 ];
		c = *pc;

		c ^= seq;

		p = ( unsigned char *)&c;
		for ( j = 0 ; j < 4; j++ )
		{
			*p++ ^= ( 0xa5 | ( j << j) | j | mungify_table2[ ( i + j ) & 0x0f ] );
		}

		c = DWordSwap( c );

		c ^= ~seq;
		
		*pc = c;
	}
}

/*
================
COM_Munge

Anti-proxy/aimbot obfuscation code

  COM_UnMunge should reversably fixup the data
================
*/
void COM_Munge3( unsigned char *data, int len, int seq )
{
	int i;
	int mungelen;

	int c;
	int *pc;
	unsigned char *p;

	int j;

	mungelen = len & ~3;
	mungelen /= 4;

	for ( i = 0; i < mungelen; i++ )
	{
		pc = (int *)&data[ i * 4 ];
		c = *pc;

		c ^= ~seq;

		c = DWordSwap( c );
		
		p = ( unsigned char *)&c;
		for ( j = 0 ; j < 4; j++ )
		{
			*p++ ^= ( 0xa5 | ( j << j) | j | mungify_table3[ ( i + j ) & 0x0f ] );
		}

		c ^= seq;
		
		*pc = c;
	}
}

/*
================
COM_UnMunge

Anti-proxy/aimbot obfuscation code
================
*/
void COM_UnMunge3( unsigned char *data, int len, int seq )
{
	int i;
	int mungelen;

	int c;
	int *pc;
	unsigned char *p;

	int j;

	mungelen = len & ~3;
	mungelen /= 4;

	for ( i = 0; i < mungelen; i++ )
	{
		pc = (int *)&data[ i * 4 ];
		c = *pc;

		c ^= seq;

		p = ( unsigned char *)&c;
		for ( j = 0 ; j < 4; j++ )
		{
			*p++ ^= ( 0xa5 | ( j << j) | j | mungify_table3[ ( i + j ) & 0x0f ] );
		}

		c = DWordSwap( c );

		c ^= ~seq;
		
		*pc = c;
	}
}