
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

//Packet decode func
void COM_Munge( unsigned char *data, int len, int seq );
void COM_UnMunge( unsigned char *data, int len, int seq );
void COM_Munge2( unsigned char *data, int len, int seq );
void COM_UnMunge2( unsigned char *data, int len, int seq );
void COM_Munge3( unsigned char *data, int len, int seq );
void COM_UnMunge3( unsigned char *data, int len, int seq );

//Half-Life opcodes
#define SVC_BAD 			0
#define SVC_NOP				1
#define SVC_DISCONNECT			2
#define SVC_EVENT			3
#define SVC_VERSION			4
#define SVC_SETVIEW			5
#define SVC_SOUND			6
#define SVC_TIME			7
#define SVC_PRINT			8
#define SVC_STUFFTEXT			9
#define SVC_SETANGLE			10
#define SVC_SERVERINFO			11
#define SVC_LIGHTSTYLE			12
#define SVC_UPDATEUSERINFO		13
#define SVC_DELTADESCRIPTION		14
#define SVC_CLIENTDATA			15
#define SVC_STOPSOUND			16
#define SVC_PINGS			17
#define SVC_PARTICLE			18
#define SVC_DAMAGE			19
#define SVC_SPAWNSTATIC			20
#define SVC_EVENT_RELIABLE		21
#define SVC_SPAWNBASELINE		22
#define SVC_TEMPENTITY			23
#define SVC_SETPAUSE			24
#define SVC_SIGNONNUM			25
#define SVC_CENTERPRINT			26
#define SVC_KILLEDMONSTER		27
#define SVC_FOUNDSECRET			28
#define SVC_SPAWNSTATICSOUND		29
#define SVC_INTERMISSION		30
#define SVC_FINALE			31
#define SVC_CDTRACK			32
#define SVC_RESTORE			33
#define SVC_CUTSCENE			34
#define SVC_WEAPONANIM			35
#define SVC_DECALNAME			36
#define SVC_ROOMTYPE			37
#define SVC_ADDANGLE			38
#define SVC_NEWUSERMSG			39
#define SVC_PACKETENTITIES		40
#define SVC_DELTAPACKETENTITIES		41
#define SVC_CHOKE			42
#define SVC_RESOURCELIST		43
#define SVC_NEWMOVEVARS			44
#define SVC_RESOURCEREQUEST		45
#define SVC_CUSTOMIZATION		46
#define SVC_CROSSHAIRANGLE		47
#define SVC_SOUNDFADE			48
#define SVC_FILETXFERFAILED		49
#define SVC_HLTV			50
#define SVC_DIRECTOR			51
#define SVC_VOICEINIT			52
#define SVC_VOICEDATA			53
#define SVC_SENDEXTRAINFO		54
#define SVC_TIMESCALE			55
#define SVC_RESOURCELOCATION 		56
#define SVC_SENDCVARVALUE 		57
#define SVC_SENDCVARVALUE2 		58

// client to server
typedef enum
{
	clc_bad = 0, //0
	clc_nop, //1
	clc_move, //2
	clc_stringcmd, //3
	clc_delta, //4
	clc_resourcelist, //5
	clc_tmove, //6
	clc_fileconsistency, //7
	clc_voicedata, //8
	clc_hltv, //9
	clc_cvarvalue, //10
	clc_cvarvalue2, //11
	clc_endoflist = 255, //12
} clientmessages;