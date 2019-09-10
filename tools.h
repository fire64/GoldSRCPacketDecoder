
struct filedata_t
{
	int filelen;
	unsigned char *filebuf;
};

int RusPrintf( char *pRusBuff );
void LogPrintf( bool iserror, char *fmt, ... );

char const* Sys_FindArg( char const *pArg, char const *pDefault );
int Sys_FindArgInt( char const *pArg, int defaultVal );


filedata_t FileRead( char *filename );
int FileWrite( char *filename, filedata_t filebuff );
int FileReWrite( char *filename, filedata_t filebuff );