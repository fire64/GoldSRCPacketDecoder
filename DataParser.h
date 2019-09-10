#pragma once

class CDataParser
{
public:
	CDataParser(void);
	CDataParser( unsigned char *pBuff, int bufflen );
	~CDataParser(void);

	byte GetByte();
	short GetShort();
	int GetInt();
	long GetLong();
	long long GetLongLong();
	char *GetString();

	int GetOffset();
	int MoveOffset(int changeoffs);

	int offset;
	unsigned char *pBuffLink;
};
