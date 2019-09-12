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
enum svc_commands_e
{
	svc_bad,
	svc_nop,
	svc_disconnect,
	svc_event,
	svc_version,
	svc_setview,
	svc_sound,
	svc_time,
	svc_print,
	svc_stufftext,
	svc_setangle,
	svc_serverinfo,
	svc_lightstyle,
	svc_updateuserinfo,
	svc_deltadescription,
	svc_clientdata,
	svc_stopsound,
	svc_pings,
	svc_particle,
	svc_damage,
	svc_spawnstatic,
	svc_event_reliable,
	svc_spawnbaseline,
	svc_temp_entity,
	svc_setpause,
	svc_signonnum,
	svc_centerprint,
	svc_killedmonster,
	svc_foundsecret,
	svc_spawnstaticsound,
	svc_intermission,
	svc_finale,
	svc_cdtrack,
	svc_restore,
	svc_cutscene,
	svc_weaponanim,
	svc_decalname,
	svc_roomtype,
	svc_addangle,
	svc_newusermsg,
	svc_packetentities,
	svc_deltapacketentities,
	svc_choke,
	svc_resourcelist,
	svc_newmovevars,
	svc_resourcerequest,
	svc_customization,
	svc_crosshairangle,
	svc_soundfade,
	svc_filetxferfailed,
	svc_hltv,
	svc_director,
	svc_voiceinit,
	svc_voicedata,
	svc_sendextrainfo,
	svc_timescale,
	svc_resourcelocation,
	svc_sendcvarvalue,
	svc_sendcvarvalue2,
	svc_startofusermessages = svc_sendcvarvalue2,
	svc_endoflist = 255,
};

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