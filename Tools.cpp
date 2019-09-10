// Tools.cpp: 
//

#include "stdafx.h"
#include "tools.h"

//Output Info
//Base func
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
