/**********************************************************************************
    First Growtopia Private Server made with ENet.
    Copyright (C) 2018  Growtopia Noobs

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************************/


#include "stdafx.h"
#include <iostream>

#include "enet/enet.h"
#include <filesystem>
#include <experimental/filesystem>
#include <string>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif
#ifdef __linux__
#include <stdio.h>
char _getch() {
    return getchar();
}
#endif
#include <vector>
#include <sstream>
#include <chrono>
#include <fstream>
#include "json.hpp"
#ifdef _WIN32
#include "bcrypt.h"
#include "crypt_blowfish/crypt_gensalt.cpp"
#include "crypt_blowfish/crypt_blowfish.h"
#include "crypt_blowfish/crypt_blowfish.cpp"
#include "crypt_blowfish/ow-crypt.cpp"
#include "bcrypt.cpp"
#include <regex>
#else
#include "bcrypt.h"
#include "bcrypt.cpp"
#include "crypt_blowfish/crypt_gensalt.h"
#include "crypt_blowfish/crypt_gensalt.cpp"
#include "crypt_blowfish/crypt_blowfish.h"
#include "crypt_blowfish/crypt_blowfish.cpp"
#include "crypt_blowfish/ow-crypt.h"
#include "crypt_blowfish/ow-crypt.cpp"
#include "bcrypt.h"
#endif
#include <thread> // TODO
#include <mutex> // TODO

#pragma warning(disable : 4996)

using namespace std;
using json = nlohmann::json;
string newslist = "set_default_color|`o\n\nadd_label_with_icon|big|`wThe Growtopia Gazette``|left|5016|\n\nadd_spacer|small|\nadd_label_with_icon|small|`4WARNING:`` `5Worlds (and accounts)`` might be deleted at any time if database issues appear (once per day or week).|left|4|\nadd_label_with_icon|small|`4WARNING:`` `5Accounts`` are in beta, bugs may appear and they will be probably deleted often, because of new account updates, which will cause database incompatibility.|left|4|\nadd_spacer|small|\n\nadd_url_button||``Watch: `1Watch a video about GT Private Server``|NOFLAGS|https://www.youtube.com/watch?v=_3avlDDYBBY|Open link?|0|0|\nadd_url_button||``Channel: `1Watch Growtopia Noobs' channel``|NOFLAGS|https://www.youtube.com/channel/UCLXtuoBlrXFDRtFU8vPy35g|Open link?|0|0|\nadd_url_button||``Items: `1Item database by Nenkai``|NOFLAGS|https://raw.githubusercontent.com/Nenkai/GrowtopiaItemDatabase/master/GrowtopiaItemDatabase/CoreData.txt|Open link?|0|0|\nadd_url_button||``Discord: `1GT Private Server Discord``|NOFLAGS|https://discord.gg/8WUTs4v|Open the link?|0|0|\nadd_quick_exit|\n\nend_dialog|gazette|Close||";

//#define TOTAL_LOG
#define REGISTRATION
#include <signal.h>
#ifdef __linux__
#include <cstdint>
typedef unsigned char BYTE;
typedef unsigned char byte;
typedef unsigned char __int8;
typedef unsigned short __int16;
typedef unsigned int DWORD;
#endif
ENetHost * server;
int cId = 1;
BYTE* itemsDat = 0;
int itemsDatSize = 0;
int hasil = 0;
int prize = 0;
int resultnbr1 = 0;
bool serverIsFrozen = false;
bool DailyMaths = false;
int resultnbr2 = 0;
long long int quest = 0;
std::vector<std::thread> threads;
//Linux equivalent of GetLastError
#ifdef __linux__
string GetLastError() {
	return strerror(errno);
}
//Linux has no byteswap functions.
ulong _byteswap_ulong(ulong x)
{
	// swap adjacent 32-bit blocks
	//x = (x >> 32) | (x << 32);
	// swap adjacent 16-bit blocks
	x = ((x & 0xFFFF0000FFFF0000) >> 16) | ((x & 0x0000FFFF0000FFFF) << 16);
	// swap adjacent 8-bit blocks
	return ((x & 0xFF00FF0
		
		0FF00FF00) >> 8) | ((x & 0x00FF00FF00FF00FF) << 8);
}
#endif

//configs
int configPort = 17091;
string configCDN = "0098/78237/cache/"; 


/***bcrypt***/

bool has_only_digits(const string str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}
bool has_only_digits_wnegative(const string str) {
	return str.find_first_not_of("-0123456789") == string::npos;
}

string hashPassword(string password) {
	char salt[BCRYPT_HASHSIZE];
	char hash[BCRYPT_HASHSIZE];
	int ret;
	
	ret = bcrypt_gensalt(12, salt);
	assert(ret == 0);
	ret = bcrypt_hashpw(password.c_str(), salt, hash);
	assert(ret == 0);
	return hash;
}

/***bcrypt**/

void sendData(ENetPeer* peer, int num, char* data, int len)
{
	/* Create a reliable packet of size 7 containing "packet\0" */
	ENetPacket * packet = enet_packet_create(0,
		len + 5,
		ENET_PACKET_FLAG_RELIABLE);
	/* Extend the packet so and append the string "foo", so it now */
	/* contains "packetfoo\0"                                      */
	/* Send the packet to the peer over channel id 0. */
	/* One could also broadcast the packet by         */
	/* enet_host_broadcast (host, 0, packet);         */
	memcpy(packet->data, &num, 4);
	if (data != NULL)
	{
		memcpy(packet->data+4, data, len);
	}
	char zero = 0;
	memcpy(packet->data + 4 + len, &zero, 1);
	enet_peer_send(peer, 0, packet);
	enet_host_flush(server);
}

int getPacketId(char* data)
{
	return *data;
}

char* getPacketData(char* data)
{
	return data + 4;
}

string text_encode(char* text)
{
	string ret = "";
	while (text[0] != 0)
	{
		switch (text[0])
		{
		case '\n':
			ret += "\\n";
			break;
		case '\t':
			ret += "\\t";
			break;
		case '\b':
			ret += "\\b";
			break;
		case '\\':
			ret += "\\\\";
			break;
		case '\r':
			ret += "\\r";
			break;
		default:
			ret += text[0];
			break;
		}
		text++;
	}
	return ret;
}

int ch2n(char x)
{
	switch (x)
	{
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'A':
		return 10;
	case 'B':
		return 11;
	case 'C':
		return 12;
	case 'D':
		return 13;
	case 'E':
		return 14;
	case 'F':
		return 15;
	default:
		break;
	}
}


char* GetTextPointerFromPacket(ENetPacket* packet)
{
	char zero = 0;
	memcpy(packet->data + packet->dataLength - 1, &zero, 1);
	return (char*)(packet->data + 4);
}

BYTE* GetStructPointerFromTankPacket(ENetPacket* packet)
{
	unsigned int packetLenght = packet->dataLength;
	BYTE* result = NULL;
	if (packetLenght >= 0x3C)
	{
		BYTE* packetData = packet->data;
		result = packetData + 4;
		if (*(BYTE*)(packetData + 16) & 8)
		{
			if (packetLenght < *(int*)(packetData + 56) + 60)
			{
				cout << "Packet too small for extended packet to be valid" << endl;
				cout << "Sizeof float is 4.  TankUpdatePacket size: 56" << endl;
				result = 0;
			}
		}
		else
		{
			int zero = 0;
			memcpy(packetData + 56, &zero, 4);
		}
	}
	return result;
}

int GetMessageTypeFromPacket(ENetPacket* packet)
{
	int result;

	if (packet->dataLength > 3u)
	{
		result = *(packet->data);
	}
	else
	{
		cout << "Bad packet length, ignoring message" << endl;
		result = 0;
	}
	return result;
}


vector<string> explode(const string &delimiter, const string &str)
{
	vector<string> arr;

	int strleng = str.length();
	int delleng = delimiter.length();
	if (delleng == 0)
		return arr;//no change

	int i = 0;
	int k = 0;
	while (i<strleng)
	{
		int j = 0;
		while (i + j<strleng && j<delleng && str[i + j] == delimiter[j])
			j++;
		if (j == delleng)//found delimiter
		{
			arr.push_back(str.substr(k, i - k));
			i += delleng;
			k = i;
		}
		else
		{
			i++;
		}
	}
	arr.push_back(str.substr(k, i - k));
	return arr;
}

struct gamepacket_t
{
private:
	int index = 0;
	int len = 0;
	byte* packet_data = new byte[61];

public:
	gamepacket_t(int delay = 0, int NetID = -1) {

		len = 61;
		int MessageType = 0x4;
		int PacketType = 0x1;
		int CharState = 0x8;

		memset(packet_data, 0, 61);
		memcpy(packet_data, &MessageType, 4);
		memcpy(packet_data + 4, &PacketType, 4);
		memcpy(packet_data + 8, &NetID, 4);
		memcpy(packet_data + 16, &CharState, 4);
		memcpy(packet_data + 24, &delay, 4);
	};
	~gamepacket_t() {
		delete[] packet_data;
	}

	void Insert(string a) {
		byte* data = new byte[len + 2 + a.length() + 4];
		memcpy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x2;
		int str_len = a.length();
		memcpy(data + len + 2, &str_len, 4);
		memcpy(data + len + 6, a.data(), str_len);
		len = len + 2 + a.length() + 4;
		index++;
		packet_data[60] = (byte)index;
	}
	void Insert(int a) {
		byte* data = new byte[len + 2 + 4];
		memcpy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x9;
		memcpy(data + len + 2, &a, 4);
		len = len + 2 + 4;
		index++;
		packet_data[60] = (byte)index;
	}
	void Insert(unsigned int a) {
		byte* data = new byte[len + 2 + 4];
		memcpy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x5;
		memcpy(data + len + 2, &a, 4);
		len = len + 2 + 4;
		index++;
		packet_data[60] = (byte)index;
	}
	void Insert(float a) {
		byte* data = new byte[len + 2 + 4];
		memcpy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x1;
		memcpy(data + len + 2, &a, 4);
		len = len + 2 + 4;
		index++;
		packet_data[60] = (byte)index;
	}
	void Insert(float a, float b) {
		byte* data = new byte[len + 2 + 8];
		memcpy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x3;
		memcpy(data + len + 2, &a, 4);
		memcpy(data + len + 6, &b, 4);
		len = len + 2 + 8;
		index++;
		packet_data[60] = (byte)index;
	}
	void Insert(float a, float b, float c) {
		byte* data = new byte[len + 2 + 12];
		memcpy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x4;
		memcpy(data + len + 2, &a, 4);
		memcpy(data + len + 6, &b, 4);
		memcpy(data + len + 10, &c, 4);
		len = len + 2 + 12;
		index++;
		packet_data[60] = (byte)index;
	}
	void CreatePacket(ENetPeer* peer) {
		ENetPacket* packet = enet_packet_create(packet_data, len, 1);
		enet_peer_send(peer, 0, packet);
	}
};

struct GamePacket {
	BYTE* data;
	int len;
	int indexes;
};
GamePacket appendFloat(GamePacket p, float val) {
	BYTE* n = new BYTE[p.len + 2 + 4];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	n[p.len] = p.indexes;
	n[p.len + 1] = 1;
	memcpy(n + p.len + 2, &val, 4);
	p.len = p.len + 2 + 4;
	p.indexes++;
	return p;
}
GamePacket appendFloat(GamePacket p, float val, float val2) {
	BYTE* n = new BYTE[p.len + 2 + 8];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	n[p.len] = p.indexes;
	n[p.len + 1] = 3;
	memcpy(n + p.len + 2, &val, 4);
	memcpy(n + p.len + 6, &val2, 4);
	p.len = p.len + 2 + 8;
	p.indexes++;
	return p;
}
GamePacket appendFloat(GamePacket p, float val, float val2, float val3) {
	BYTE* n = new BYTE[p.len + 2 + 12];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	n[p.len] = p.indexes;
	n[p.len + 1] = 4;
	memcpy(n + p.len + 2, &val, 4);
	memcpy(n + p.len + 6, &val2, 4);
	memcpy(n + p.len + 10, &val3, 4);
	p.len = p.len + 2 + 12;
	p.indexes++;
	return p;
}
GamePacket appendInt(GamePacket p, int val) {
	BYTE* n = new BYTE[p.len + 2 + 4];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	n[p.len] = p.indexes;
	n[p.len + 1] = 9;
	memcpy(n + p.len + 2, &val, 4);
	p.len = p.len + 2 + 4;
	p.indexes++;
	return p;
}
GamePacket appendIntx(GamePacket p, int val) {
	BYTE* n = new BYTE[p.len + 2 + 4];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	n[p.len] = p.indexes;
	n[p.len + 1] = 5;
	memcpy(n + p.len + 2, &val, 4);
	p.len = p.len + 2 + 4;
	p.indexes++;
	return p;
}
GamePacket appendString(GamePacket p, string str) {
	BYTE* n = new BYTE[p.len + 2 + str.length() + 4];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	n[p.len] = p.indexes;
	n[p.len + 1] = 2;
	int sLen = str.length();
	memcpy(n + p.len + 2, &sLen, 4);
	memcpy(n + p.len + 6, str.c_str(), sLen);
	p.len = p.len + 2 + str.length() + 4;
	p.indexes++;
	return p;
}
GamePacket createPacket() {
	BYTE* data = new BYTE[61];
	string asdf = "0400000001000000FFFFFFFF00000000080000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	for (int i = 0; i < asdf.length(); i += 2) {
		char x = ch2n(asdf[i]);
		x = x << 4;
		x += ch2n(asdf[i + 1]);
		memcpy(data + (i / 2), &x, 1);
		if (asdf.length() > 61 * 2) throw 0;
	}
	GamePacket packet;
	packet.data = data;
	packet.len = 61;
	packet.indexes = 0;
	return packet;
}
GamePacket packetEnd(GamePacket p) {
	BYTE* n = new BYTE[p.len + 1];
	memcpy(n, p.data, p.len);
	delete p.data;
	p.data = n;
	char zero = 0;
	memcpy(p.data + p.len, &zero, 1);
	p.len += 1;
	*(int*)(p.data + 56) = p.indexes;
	*(BYTE*)(p.data + 60) = p.indexes;
	return p;
}

struct ItemSharedUID {
	int actual_uid = 1;
	int shared_uid = 1;
};

struct InventoryItem {
	__int16 itemID;
	__int8 itemCount;
};

struct PlayerInventory {
	vector<InventoryItem> items;
	int inventorySize = 100;
};

#define cloth0 cloth_hair
#define cloth1 cloth_shirt
#define cloth2 cloth_pants
#define cloth3 cloth_feet
#define cloth4 cloth_face
#define cloth5 cloth_hand
#define cloth6 cloth_back
#define cloth7 cloth_mask
#define cloth8 cloth_necklace
#define STR16(x, y) (*(uint16_t*)(&(x)[(y)]))
#define STRINT(x, y) (*(int*)(&(x)[(y)]))
#define cloth9 cloth_ances

struct PlayerInfo {
	bool isIn = false;
	int netID;
	int level = 1;
	int xp = 0;
	int adminLevel = 0;
	int premwl = 0;
	int buygems = 0;
	int buygacha = 0;
	bool haveGrowId = false;
	int characterState = 0;
	vector<string>friendinfo;
	vector<string>createfriendtable;
	int punchX;
	int punchY;
	int wrenchx;
	bool legend = false;
	int wrenchy;
	int droppeditemcount = 0;
	int wrenchsession = 0;
	string wrenchedplayer = "";
	int lasttrashitem = 0;
	int lasttrashitemcount = 0;
	int lastunt = 0;
	int lastuntc = 0;
	string wrenchdisplay = "";
	string lastInfo = "";
	string tankIDName = "";
	string tankIDPass = "";
	string requestedName = "";
	string rawName = "";
	long long int quest = 0;
	string displayName = "";
	string country = "";
	string currentWorld = "START";
	string lastInfoname = "";
	string lastgm = "";
	short currentInventorySize = 200;
	string lastgmname = "";
	string lastgmworld = "";
	string guildlast = "";
	vector<string>worldsowned;
	vector<string>createworldsowned;
	bool timedilation = false;
	bool DailyMaths = false;
	bool isinvited = false;
	int guildranklevel = 0;
	int guildBg = 0;
	int userID;
	int guildFg = 0;
	int petlevel = 0;
	//GUILD SYSTEM
	string guildStatement = "";
	string guildLeader = "";
	vector <string> guildmatelist;
	vector<string>guildMembers;
	vector<string>guildGE;
	vector<string>guildGC;
	int guildlevel = 0;
	int guildexp = 0;
	string createGuildName = "";
	string createGuildStatement = "";
	string createGuildFlagBg = "";
	string createGuildFlagFg = "";

	string guild = "";

	bool joinguild = false;
	bool radio = true;
	int x;
	int y;
	int lastPunchX;
	int lastPunchY;
	int posX;
	int posY;

	string lastfriend = "";
	int x1;
	int y1;
	bool isRotatedLeft = false;
	string charIP = "";
	int peffect = 8421376;
	bool isUpdating = false;
	bool joinClothesUpdated = false;
	
	bool hasLogon = false;
	
	bool taped = false;

	int cloth_hair = 0; // 0
	int cloth_shirt = 0; // 1
	int cloth_pants = 0; // 2
	int cloth_feet = 0; // 3
	int cloth_face = 0; // 4
	int cloth_hand = 0; // 5
	int cloth_back = 0; // 6
	int cloth_mask = 0; // 7
	int cloth_necklace = 0; // 8
	int cloth_ances=0; // 9

	bool canWalkInBlocks = false; // 1
	bool canDoubleJump = false; // 2
	bool isInvisible = false; // 4
	bool noHands = false; // 8
	bool noEyes = false; // 16
	bool noBody = false; // 32
	bool devilHorns = false; // 64
	bool goldenHalo = false; // 128
	bool isFrozen = false; // 2048
	bool isCursed = false; // 4096
	bool isDuctaped = false; // 8192
	bool haveCigar = false; // 16384
	int Ultrasound = 0;
	bool PatientHeartStopped = false;
	long long int SurgeryTime = 0;
	bool SurgeryCooldown = false;
	float PatientTemperatureRise = 0;
	bool FixIt = false;
	bool UnlockedAntibiotic = false;
	bool PerformingSurgery = false;
	int SurgerySkill = 0;
	bool RequestedSurgery = false;
	string TempColor = "";
	bool HardToSee = false;
	bool PatientLosingBlood = false;
	int SurgItem1 = 4320;
	int SurgItem2 = 4320;
	int SurgItem3 = 4320;
	int SurgItem4 = 4320;
	int SurgItem5 = 4320;
	int SurgItem6 = 4320;
	int SurgItem7 = 4320;
	int SurgItem8 = 4320;
	int SurgItem9 = 4320;
	int SurgItem10 = 4320;
	int SurgItem11 = 4320;
	int SurgItem12 = 4320;
	int SurgItem13 = 4320;
	string PatientDiagnosis = "";
	string PatientPulse = "";
	string PatientStatus = "";
	float PatientTemperature = 0;
	string OperationSite = "";
	string IncisionsColor = "";
	int PatientIncisions = 0;
	string PatientRealDiagnosis = "";
	bool isShining = false; // 32768
	bool isZombie = false; // 65536
	bool isHitByLava = false; // 131072
	bool haveHauntedShadows = false; // 262144
	bool haveGeigerRadiation = false; // 524288
	bool haveReflector = false; // 1048576
	bool isEgged = false; // 2097152
	bool havePineappleFloag = false; // 4194304
	bool haveFlyingPineapple = false; // 8388608
	bool haveSuperSupporterName = false; // 16777216
	bool haveSupperPineapple = false; // 33554432
	bool isGhost = false;
	int wrenchedBlockLocation = -1;
	//bool 
	int skinColor = 0x8295C3FF; //normal SKin color like gt!

	PlayerInventory inventory;

	long long int lastSB = 0;

	//hacky dropped item stuff :(
	vector<ItemSharedUID> item_uids;
	int last_uid = 1;
};


int getState(PlayerInfo* info) {
	int val = 0;
	val |= info->canWalkInBlocks << 0;
	val |= info->canDoubleJump << 1;
	val |= info->isInvisible << 2;
	val |= info->noHands << 3;
	val |= info->noEyes << 4;
	val |= info->noBody << 5;
	val |= info->devilHorns << 6;
	val |= info->goldenHalo << 7;
	val |= info->isFrozen << 11;
	val |= info->isCursed << 12;
	val |= info->isDuctaped << 13;
	val |= info->haveCigar << 14;
	val |= info->isShining << 15;
	val |= info->isZombie << 16;
	val |= info->isHitByLava << 17;
	val |= info->haveHauntedShadows << 18;
	val |= info->haveGeigerRadiation << 19;
	val |= info->haveReflector << 20;
	val |= info->isEgged << 21;
	val |= info->havePineappleFloag << 22;
	val |= info->haveFlyingPineapple << 23;
	val |= info->haveSuperSupporterName << 24;
	val |= info->haveSupperPineapple << 25;
	return val;
}

struct BlockVisual {
	int packetType;
	int characterState;
	int punchX;
	int punchY;
	float x;
	float y;
	int plantingTree;
	float XSpeed;
	float YSpeed;
	int charStat;
	int blockid;
	int visual;
	int signs;
	int backgroundid;
	int displayblock;
	int time;
	int netID;
	//int bpm;
};

struct WorldItem {
	__int16 foreground = 0;
	__int16 background = 0;
	int breakLevel = 0;
	long long int breakTime = 0;
	int PosFind = 0;
	bool isLocked = false;
	int displayblock;
	bool rotatedLeft = false;
	bool water = false;
	bool fire = false;
	bool glue = false;
	bool red = false;
	bool green = false;
	bool blue = false;

	int clothHair = 0;
	int clothHead = 0;
	int clothMask = 0;
	int clothHand = 0;
	int clothNeck = 0;
	int clothShirt = 0;
	int clothPants = 0;
	int clothFeet = 0;
	int clothBack = 0;

	int dropItem = 0;
	int amount = 0; // like this

	string text = "";
	string signn = "";

	vector<string> mailbox;

	int gravity = 0;
	bool flipped = false;
	bool active = false;
	bool silenced = false;
	int16_t lockId = 0;
	string label = "";
	string destWorld = "";
	string destId = "";
	string currId = "";
	string password = "";
	int intdata = 0;
	bool activated = false;
	int displayBlock = 0;
	bool isOpened = false;
};

struct DroppedItem { // TODO
	int id;
	int uid;
	int count;
	int x;
	int y;
};

struct WorldInfo {
	int width = 100;
	int height = 60;
	string name = "TEST";
	WorldItem* items;
	string owner = "";
	int stuffID = 2;
	int stuff_gravity = 50;
	bool stuff_spin = false;
	bool stuff_invert = false;
	bool isPublic = false;
	bool isNuked = false;
	int ownerID = 0;
	int droppeditemcount = 0;
	bool isCasino = false;
	int bgID = 14;
	vector<string> acclist;
	int weather = 0;
	bool ice = false;
	bool land = false;
	bool volcano = false;
	bool online = false;
	int droppedItemUid = 0;
	int droppedCount = 0;

	unsigned long currentItemUID = 1; //has to be 1 by default
	vector<DroppedItem> droppedItems;
};

WorldInfo generateWorld(string name, int width, int height)
{
	WorldInfo world;
	world.name = name;
	world.width = width;
	world.height = height;
	world.items = new WorldItem[world.width*world.height];
	for (int i = 0; i < world.width*world.height; i++)
	{
		if (i >= 3800 && i < 5400 && !(rand() % 50)){ world.items[i].foreground = 10; }
		else if (i >= 3700 && i < 5400) {
			if(i > 5000) {
				if (i % 7 == 0) { world.items[i].foreground = 4;}
				else { world.items[i].foreground = 2; }
			}
			else { world.items[i].foreground = 2; }
		}
		else if (i >= 5400) { world.items[i].foreground = 8; }
		if (i >= 3700)
			world.items[i].background = 14;
		if (i == 3650)
			world.items[i].foreground = 6;
		else if (i >= 3600 && i<3700)
			world.items[i].foreground = 0; //fixed the grass in the world!
		if (i == 3750)
			world.items[i].foreground = 8;
	}
	return world;
}

class PlayerDB {
public:
	static string getProperName(string name);
	static string fixColors(string text);
	static int playerLogin(ENetPeer* peer, string username, string password);
	static int playerRegister(ENetPeer* peer, string username, string password, string passwordverify, string email, string discord);
	static void showWrong(ENetPeer* peer, string itemFind, string listFull);
	static void OnTextOverlay(ENetPeer* peer, string text);
	static void OnSetCurrentWeather(ENetPeer* peer, int weather);
	static void PlayAudio(ENetPeer* peer, string audioFile, int delayMS);
	static void OnAddNotification(ENetPeer* peer, string text, string audiosound, string interfaceimage);
	static int guildRegister(ENetPeer* peer, string guildName, string guildStatement, string guildFlagfg, string guildFlagbg);
	static void OnTalkBubble(ENetPeer* peer, int netID, string message, bool stay) {
		if (message.length() == 0 || message.length() > 100) return;
		GamePacket p2;
		if (stay)
			p2 = packetEnd(appendIntx(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), message), 0), 1));
		else
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), message), 0));
		ENetPacket* packet2 = enet_packet_create(p2.data,
			p2.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet2);
		delete p2.data;
	}
};
int maxItems = 11454;
std::mutex m;
string PlayerDB::getProperName(string name) {
	string newS;
	for (char c : name) newS+=(c >= 'A' && c <= 'Z') ? c-('A'-'a') : c;
	string ret;
	for (int i = 0; i < newS.length(); i++)
	{
		if (newS[i] == '`') i++; else ret += newS[i];
	}
	string ret2;
	for (char c : ret) if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) ret2 += c;
	
	string username = ret2;
	if (username == "prn" || username == "con" || username == "aux" || username == "nul" || username == "com1" || username == "com2" || username == "com3" || username == "com4" || username == "com5" || username == "com6" || username == "com7" || username == "com8" || username == "com9" || username == "lpt1" || username == "lpt2" || username == "lpt3" || username == "lpt4" || username == "lpt5" || username == "lpt6" || username == "lpt7" || username == "lpt8" || username == "lpt9") {
		return "";
	}
	
	return ret2;
}

string PlayerDB::fixColors(string text) {
	string ret = "";
	int colorLevel = 0;
	for (int i = 0; i < text.length(); i++)
	{
		if (text[i] == '`')
		{
			ret += text[i];
			if (i + 1 < text.length())
				ret += text[i + 1];
			
			
			if (i+1 < text.length() && text[i + 1] == '`')
			{
				colorLevel--;
			}
			else {
				colorLevel++;
			}
			i++;
		} else {
			ret += text[i];
		}
	}
	for (int i = 0; i < colorLevel; i++) {
		ret += "``";
	}
	for (int i = 0; i > colorLevel; i--) {
		ret += "`w";
	}
	return ret;
}

struct Admin {
	string username;
	string password;
	int level = 0;
	long long int lastSB = 0;
};

vector<Admin> admins;

int PlayerDB::playerLogin(ENetPeer* peer, string username, string password) {
	std::ifstream ifs("players/" + PlayerDB::getProperName(username) + ".json");
	if (ifs.is_open()) {
		json j;
		ifs >> j;
		string pss = j["password"];
		int adminLevel = j["adminLevel"];
		if (password == pss) {
			((PlayerInfo*)(peer->data))->hasLogon = true;
			//after verify password add adminlevel not before
			bool found = false;
			for (int i = 0; i < admins.size(); i++) {
				if (admins[i].username == username) {
					found = true;
				}
			}
			if (!found) {//not in vector
				if (adminLevel != 0) {
					Admin admin;
					admin.username = PlayerDB::getProperName(username);
					admin.password = pss;
					admin.level = adminLevel;
					admins.push_back(admin);
				}
			}
			ENetPeer* currentPeer;

			for (currentPeer = server->peers;
				currentPeer < &server->peers[server->peerCount];
				++currentPeer)
			{
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
					continue;
				if (currentPeer == peer)
					continue;
				if (((PlayerInfo*)(currentPeer->data))->rawName == PlayerDB::getProperName(username))
				{
					{
						gamepacket_t p;
						p.Insert("OnConsoleMessage");
						p.Insert("Someone else logged into this account!");
						p.CreatePacket(peer);
					}
					{
						gamepacket_t p;
						p.Insert("OnConsoleMessage");
						p.Insert("Someone else was logged into this account! He was kicked out now.");
						p.CreatePacket(peer);
					}
					enet_peer_disconnect_later(currentPeer, 0);
				}
			}
			return 1;
		}
		else {
			return -1;
		}
	}
	else {
		return -2;
	}
}

int PlayerDB::guildRegister(ENetPeer* peer, string guildName, string guildStatement, string guildFlagfg, string guildFlagbg) {
	if (guildName.find(" ") != string::npos || guildName.find(".") != string::npos || guildName.find(",") != string::npos || guildName.find("@") != string::npos || guildName.find("[") != string::npos || guildName.find("]") != string::npos || guildName.find("#") != string::npos || guildName.find("<") != string::npos || guildName.find(">") != string::npos || guildName.find(":") != string::npos || guildName.find("{") != string::npos || guildName.find("}") != string::npos || guildName.find("|") != string::npos || guildName.find("+") != string::npos || guildName.find("_") != string::npos || guildName.find("~") != string::npos || guildName.find("-") != string::npos || guildName.find("!") != string::npos || guildName.find("$") != string::npos || guildName.find("%") != string::npos || guildName.find("^") != string::npos || guildName.find("&") != string::npos || guildName.find("`") != string::npos || guildName.find("*") != string::npos || guildName.find("(") != string::npos || guildName.find(")") != string::npos || guildName.find("=") != string::npos || guildName.find("'") != string::npos || guildName.find(";") != string::npos || guildName.find("/") != string::npos) {
		return -1;
	}

	if (guildName.length() < 3) {
		return -2;
	}
	if (guildName.length() > 15) {
		return -3;
	}
	int fg;
	int bg;

	try {
		fg = stoi(guildFlagfg);
	}
	catch (std::invalid_argument& e) {
		return -6;
	}
	try {
		bg = stoi(guildFlagbg);
	}
	catch (std::invalid_argument& e) {
		return -5;
	}
	if (guildFlagbg.length() > 4) {
		return -7;
	}
	if (guildFlagfg.length() > 4) {
		return -8;
	}

	string fixedguildName = PlayerDB::getProperName(guildName);

	std::ifstream ifs("guilds/" + fixedguildName + ".json");
	if (ifs.is_open()) {
		return -4;
	}


	/*std::ofstream o("guilds/" + fixedguildName + ".json");
	if (!o.is_open()) {
		cout << GetLastError() << endl;
		_getch();
	}

	json j;

	//  Guild Detail
	j["GuildName"] = guildName;
	j["GuildStatement"] = guildStatement;
	j["GuildWorld"] = ((PlayerInfo*)(peer->data))->currentWorld;

	//  Guild Level
	j["GuildLevel"] = 0;
	j["GuildExp"] = 0;

	// Guild Leader
	j["Leader"] = ((PlayerInfo*)(peer->data))->rawName;


	// Guild Flag
	j["foregroundflag"] = 0;
	j["backgroundflag"] = 0;


	// Role
	vector<string>guildmember;
	vector<string>guildelder;
	vector<string>guildco;

	j["CoLeader"] = guildelder;
	j["ElderLeader"] = guildco;
	j["Member"] = guildmem;

	o << j << std::endl; */
	return 1;
}

int PlayerDB::playerRegister(ENetPeer* peer, string username, string password, string passwordverify, string email, string discord) {
	string name = username;
	if (name == "CON" || name == "PRN" || name == "AUX" || name == "NUL" || name == "COM1" || name == "COM2" || name == "COM3" || name == "COM4" || name == "COM5" || name == "COM6" || name == "COM7" || name == "COM8" || name == "COM9" || name == "LPT1" || name == "LPT2" || name == "LPT3" || name == "LPT4" || name == "LPT5" || name == "LPT6" || name == "LPT7" || name == "LPT8" || name == "LPT9") return -1;
	username = PlayerDB::getProperName(username);
	if (discord.find("#") == std::string::npos && discord.length() != 0) return -5;
	if (email.find("@") == std::string::npos && email.length() != 0) return -4;
	if (passwordverify != password) return -3;
	if (username.length() < 3) return -2;
	std::ifstream ifs("players/" + username + ".json");
	if (ifs.is_open()) {
		return -1;
	}

	std::ofstream o("players/" + username + ".json");
	if (!o.is_open()) {
		cout << GetLastError() << endl;
		_getch();
	}
	json j;
	j["username"] = username;
	j["password"] = password;
	j["email"] = email;
	j["discord"] = discord;
	j["adminLevel"] = 0;
	j["level"] = 1;
	j["xp"] = 0;
	j["ClothBack"] = 0;
	j["ClothHand"] = 0;
	j["ClothFace"] = 0;
	j["ClothShirt"] = 0;
	j["ClothPants"] = 0;
	j["ClothNeck"] = 0;
	j["ClothHair"] = 0;
	j["ClothFeet"] = 0;
	j["ClothMask"] = 0;
	j["ClothAnces"] = 0;
	j["guild"] = "";
	j["joinguild"] = false;
	j["premwl"] = 0;
	j["effect"] = 8421376;
	j["skinColor"] = 0x8295C3FF;
	j["worldsowned"] = ((PlayerInfo*)(peer->data))->createworldsowned;
	o << j << std::endl;

	std::ofstream oo("inventory/" + username + ".json");
	if (!oo.is_open()) {
		cout << GetLastError() << endl;
		_getch();
	}

	json items;
	json jjall = json::array();


	json jj;
	jj["aposition"] = 1;
	jj["itemid"] = 18;
	jj["quantity"] = 1;
	jjall.push_back(jj);


	jj["aposition"] = 2;
	jj["itemid"] = 32;
	jj["quantity"] = 1;
	jjall.push_back(jj);

	for (int i = 2; i < 200; i++)
	{
		jj["aposition"] = i + 1;
		jj["itemid"] = 0;
		jj["quantity"] = 0;
		jjall.push_back(jj);
	}

	items["items"] = jjall;
	oo << items << std::endl;

	return 1;
}

struct AWorld {
	WorldInfo* ptr;
	WorldInfo info;
	int id;
};

class WorldDB {
public:
	WorldInfo get(string name);
	int getworldStatus(string name);
	AWorld get2(string name);
	void flush(WorldInfo info);
	void flush2(AWorld info);
	void save(AWorld info);
	void saveAll();
	void saveRedundant();
	vector<WorldInfo> getRandomWorlds();
	WorldDB();
private:
	vector<WorldInfo> worlds;
};

WorldDB::WorldDB() {
	// Constructor
}

namespace packet {
	void consolemessage(ENetPeer* peer, string message) {
		gamepacket_t p;
		p.Insert("OnConsoleMessage");
		p.Insert(message);
		p.CreatePacket(peer);
	}
	void OnAddNotification(ENetPeer* peer, string text, string audiosound, string interfaceimage) {
		auto p = packetEnd(appendInt(appendString(appendString(appendString(appendString(createPacket(), "OnAddNotification"),
			interfaceimage),
			text),
			audiosound),
			0));
		ENetPacket* packet = enet_packet_create(p.data,
			p.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
		delete p.data;
	}
	void OnNameChanged(ENetPeer* peer, int netID, string name) {
		gamepacket_t p;
		p.Insert("OnNameChanged");
		p.Insert(netID);
		p.Insert(name);
		p.CreatePacket(peer);
	}
	void dialog(ENetPeer* peer, string message) {
		gamepacket_t p;
		p.Insert("OnDialogRequest");
		p.Insert(message);
		p.CreatePacket(peer);
	}
	void onspawn(ENetPeer* peer, string message) {
		gamepacket_t p;
		p.Insert("OnSpawn");
		p.Insert(message);
		p.CreatePacket(peer);
	}
	void requestworldselectmenu(ENetPeer* peer, string message) {
		gamepacket_t p;
		p.Insert("OnRequestWorldSelectMenu");
		p.Insert(message);
		p.CreatePacket(peer);
	}
	void storerequest(ENetPeer* peer, string message) {
		gamepacket_t p;
		p.Insert("OnStoreRequest");
		p.Insert(message);
		p.CreatePacket(peer);
	}
	void storepurchaseresult(ENetPeer* peer, string message) {
		gamepacket_t p;
		p.Insert("OnStorePurchaseResult");
		p.Insert(message);
		p.CreatePacket(peer);
	}
	void SendTalkSelf(ENetPeer* peer, string text) {
		GamePacket p2 = packetEnd(appendIntx(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), text), 0), 1));
		ENetPacket* packet2 = enet_packet_create(p2.data,
			p2.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet2);
		delete p2.data;
	}
	void OnTalkBubble(ENetPeer* peer, int netID, string text, int chatColor, bool isOverlay) {
		if (isOverlay == true) {
			GamePacket p = packetEnd(appendIntx(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"),
				((PlayerInfo*)(peer->data))->netID), text), chatColor), 1));
			ENetPacket* packet = enet_packet_create(p.data,
				p.len,
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet);
			delete p.data;
		}
		else {
			GamePacket p = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"),
				((PlayerInfo*)(peer->data))->netID), text), chatColor));
			ENetPacket* packet = enet_packet_create(p.data,
				p.len,
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet);
			delete p.data;
		}
	}
	void OnTextOverlay(ENetPeer* peer, string text) {
		GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnTextOverlay"), text));
		ENetPacket* packet = enet_packet_create(p.data,
			p.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
		delete p.data;
	}
	void OnFailedToEnterWorld(ENetPeer* peer) {
		GamePacket p = packetEnd(appendIntx(appendString(createPacket(), "OnFailedToEnterWorld"), 1));
		ENetPacket* packet = enet_packet_create(p.data,
			p.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
		delete p.data;
	}
	void PlayAudio(ENetPeer* peer, string audioFile, int delayMS) {
		string text = "action|play_sfx\nfile|" + audioFile + "\ndelayMS|" + to_string(delayMS) + "\n";
		BYTE* data = new BYTE[5 + text.length()];
		BYTE zero = 0;
		int type = 3;
		memcpy(data, &type, 4);
		memcpy(data + 4, text.c_str(), text.length());
		memcpy(data + 4 + text.length(), &zero, 1);
		ENetPacket* packet = enet_packet_create(data,
			5 + text.length(),
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
		delete[] data;
	}
}

void SendConsole(const string text, const string type) {
	time_t currentTime; time(&currentTime); const auto localTime = localtime(&currentTime);
	const auto Hour = localTime->tm_hour; const auto Min = localTime->tm_min; const auto Sec = localTime->tm_sec;
	if (type != "CHAT") {
		cout << "[" + to_string(Hour) + ":" + to_string(Min) + ":" + to_string(Sec) + " " + type + """]: " << text << endl;
	}
}
void savegem(ENetPeer* peer) {
	if (((PlayerInfo*)(peer->data))->haveGrowId == true) {
		PlayerInfo* p5 = ((PlayerInfo*)(peer->data));
		string username = PlayerDB::getProperName(p5->rawName);
		ifstream fg("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
		json j;
		fg >> j;
		fg.close();
		ofstream fs("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
		fs << j;
		fs.close();
	}
}
void sendConsole(ENetPeer* x, string e) {
	GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), e));
	ENetPacket* packet = enet_packet_create(p.data,
		p.len,
		ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(x, 0, packet);
	delete p.data;
}
bool isWorldOwner(ENetPeer* peer, WorldInfo* world) {
	return ((PlayerInfo*)(peer->data))->rawName == world->owner;
}
bool isWorldAdmin(ENetPeer* peer, WorldInfo* world) {
	const auto uid = ((PlayerInfo*)(peer->data))->rawName;
	for (const auto i = 0; world->acclist.size();) {
		const auto x = world->acclist.at(i);
		if (uid == x.substr(0, x.find("|"))) {
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}
void savejson(ENetPeer* peer) {
	if (((PlayerInfo*)(peer->data))->haveGrowId == true) {
		PlayerInfo* p5 = ((PlayerInfo*)(peer->data));
		string username = PlayerDB::getProperName(p5->rawName);
		ifstream fg("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
		json j;
		fg >> j;
		fg.close();
		j["premwl"] = ((PlayerInfo*)(peer->data))->premwl;
		j["level"] = ((PlayerInfo*)(peer->data))->level;
		j["xp"] = ((PlayerInfo*)(peer->data))->xp;
		ofstream fs("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
		fs << j;
		fs.close();
	}
}

string getStrUpper(string txt) {
	string ret;
	for (char c : txt) ret += toupper(c);
	return ret;
}

AWorld WorldDB::get2(string name) {
	if (worlds.size() > 200) {
#ifdef TOTAL_LOG
		cout << "Saving redundant worlds!" << endl;
#endif
		saveRedundant();
#ifdef TOTAL_LOG
		cout << "Redundant worlds are saved!" << endl;
#endif
	}
	AWorld ret;
	name = getStrUpper(name);
	if (name.length() < 1) throw 1; // too short name
	for (char c : name) {
		if ((c < 'A' || c>'Z') && (c < '0' || c>'9'))
			throw 2; // wrong name
	}
	if (name == "EXIT") {
		throw 3;
	}
	if (name == "CON" || name == "PRN" || name == "AUX" || name == "NUL" || name == "COM1" || name == "COM2" || name == "COM3" || name == "COM4" || name == "COM5" || name == "COM6" || name == "COM7" || name == "COM8" || name == "COM9" || name == "LPT1" || name == "LPT2" || name == "LPT3" || name == "LPT4" || name == "LPT5" || name == "LPT6" || name == "LPT7" || name == "LPT8" || name == "LPT9") throw 3;
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds.at(i).name == name)
		{
			ret.id = i;
			ret.info = worlds.at(i);
			ret.ptr = &worlds.at(i);
			return ret;
		}

	}
	std::ifstream ifs("worlds/" + name + ".json");
	if (ifs.is_open()) {

		json j;
		ifs >> j;
		WorldInfo info;
		info.name = j["name"].get<string>();
		info.width = j["width"];
		info.height = j["height"];
		info.owner = j["owner"].get<string>();
		info.weather = j["weather"].get<int>();
		info.isPublic = j["isPublic"];
		for (int i = 0; i < j["access"].size(); i++) {
			info.acclist.push_back(j["access"][i]);
		}
		info.stuffID = j["stuffID"].get<int>();
		info.stuff_gravity = j["stuff_gravity"].get<int>();
		info.stuff_invert = j["stuff_invert"].get<bool>();
		info.stuff_spin = j["stuff_spin"].get<bool>();
		json tiles = j["tiles"];
		int square = info.width * info.height;
		info.items = new WorldItem[square];
		for (int i = 0; i < square; i++) {
			info.items[i].foreground = tiles[i]["fg"];
			info.items[i].background = tiles[i]["bg"];
		}
		worlds.push_back(info);
		ret.id = worlds.size() - 1;
		ret.info = info;
		ret.ptr = &worlds.at(worlds.size() - 1);
		return ret;
	}
	else {
		WorldInfo info = generateWorld(name, 100, 60);

		worlds.push_back(info);
		ret.id = worlds.size() - 1;
		ret.info = info;
		ret.ptr = &worlds.at(worlds.size() - 1);
		return ret;
	}
	throw 1;
}

WorldInfo WorldDB::get(string name) {

	return this->get2(name).info;
}

int WorldDB::getworldStatus(string name) {
	name = getStrUpper(name);
	//if (name == "CON" || name == "PRN" || name == "AUX" || name == "NUL" || name == "COM1" || name == "COM2" || name == "COM3" || name == "COM4" || name == "COM5" || name == "COM6" || name == "COM7" || name == "COM8" || name == "COM9" || name == "LPT1" || name == "LPT2" || name == "LPT3" || name == "LPT4" || name == "LPT5" || name == "LPT6" || name == "LPT7" || name == "LPT8" || name == "LPT9") return -1;

	//if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != string::npos) return -1;
	if (name.length() > 24) return -1;
	/*for (int i = 0; i < worlds.size(); i++) {
		if (worlds.at(i).name == name)
		{
			return 0;
		}
	}*/
	return 0;
}

void WorldDB::flush(WorldInfo info)
{
	std::ofstream o("worlds/" + info.name + ".json");
	if (!o.is_open()) {
		cout << GetLastError() << endl;
	}
	json j;
	j["name"] = info.name;
	j["width"] = info.width;
	j["height"] = info.height;
	j["owner"] = info.owner;
	j["isPublic"] = info.isPublic;
	j["weather"] = info.weather;
	j["stuffID"] = info.stuffID;
	j["stuff_gravity"] = info.stuff_gravity;
	j["stuff_invert"] = info.stuff_invert;
	j["stuff_spin"] = info.stuff_spin;
	j["access"] = info.acclist;
	json tiles = json::array();
	int square = info.width * info.height;

	for (int i = 0; i < square; i++)
	{
		json tile;
		tile["fg"] = info.items[i].foreground;
		tile["bg"] = info.items[i].background;
		tiles.push_back(tile);
	}
	j["tiles"] = tiles;
	o << j << std::endl;
}

void WorldDB::flush2(AWorld info)
{
	this->flush(info.info);
}

void WorldDB::save(AWorld info)
{
	flush2(info);
	delete info.info.items;
	worlds.erase(worlds.begin() + info.id);
}

void WorldDB::saveAll()
{
	for (int i = 0; i < worlds.size(); i++) {
		flush(worlds.at(i));
		delete[] worlds.at(i).items;
	}
	worlds.clear();
}

vector<WorldInfo> WorldDB::getRandomWorlds() {
	vector<WorldInfo> ret;
	for (int i = 0; i < ((worlds.size() < 10) ? worlds.size() : 10); i++)
	{ // load first four worlds, it is excepted that they are special
		ret.push_back(worlds.at(i));
	}
	// and lets get up to 6 random
	if (worlds.size() > 4) {
		for (int j = 0; j < 6; j++)
		{
			bool isPossible = true;
			WorldInfo world = worlds.at(rand() % (worlds.size() - 4));
			for (int i = 0; i < ret.size(); i++)
			{
				if (world.name == ret.at(i).name || world.name == "EXIT")
				{
					isPossible = false;
				}
			}
			if (isPossible)
				ret.push_back(world);
		}
	}
	return ret;
}

void WorldDB::saveRedundant()
{
	for (int i = 4; i < worlds.size(); i++) {
		bool canBeFree = true;
		ENetPeer * currentPeer;

		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (((PlayerInfo*)(currentPeer->data))->currentWorld == worlds.at(i).name)
				canBeFree = false;
		}
		if (canBeFree)
		{
			flush(worlds.at(i));
			delete worlds.at(i).items;
			worlds.erase(worlds.begin() + i);
			i--;
		}
	}
}

//WorldInfo world;
//vector<WorldInfo> worlds;
WorldDB worldDB;

bool CheckItemMaxed(ENetPeer* peer, int fItemId, int fQuantityAdd) {
	bool isMaxed = false;
	for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++) {
		if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemId && ((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount + fQuantityAdd > 200) {
			isMaxed = true;
			break;
		}
	}
	return isMaxed;
}
void saveAllWorlds() // atexit hack plz fix
{
	GamePacket p0 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"),
		"`4Global system message`o: Saving all worlds `oin `p5 `wseconds`o, you will be timed out for a short amount of time`w! `oDon't punch anything or you may get disconnected!``"));
	ENetPacket* packet0 = enet_packet_create(p0.data,
		p0.len,
		ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet0);
	cout << "[!] Saving worlds..." << endl;
	worldDB.saveAll();
	cout << "[!] Worlds saved!" << endl;
	Sleep(1000);

	Sleep(200);
	GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`5 `4Global system message`o: `2Saved `oall worlds``"));
	ENetPacket* packet = enet_packet_create(p.data,
		p.len,
		ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet);
	delete p0.data;
	delete p.data;
}
void updateGuild(ENetPeer* peer) {
	string guildname = PlayerDB::getProperName(((PlayerInfo*)(peer->data))->guild);
	if (guildname != "") {
		std::ifstream ifff("guilds/" + guildname + ".json");
		if (ifff.fail()) {
			ifff.close();
			cout << "Failed to loading guilds/" + guildname + ".json! From " + ((PlayerInfo*)(peer->data))->displayName + "." << endl;
			((PlayerInfo*)(peer->data))->guild = "";
			updateGuild;
		}
		json j;
		ifff >> j;

		int gfbg, gffg;

		string gstatement, gleader;

		vector<string> gmembers;

		gfbg = j["backgroundflag"];
		gffg = j["foregroundflag"];
		gstatement = j["GuildStatement"].get<string>();
		gleader = j["Leader"].get<string>();
		for (int i = 0; i < j["Member"].size(); i++) {
			gmembers.push_back(j["Member"][i]);
		}

		if (find(gmembers.begin(), gmembers.end(), ((PlayerInfo*)(peer->data))->rawName) == gmembers.end()) {
			((PlayerInfo*)(peer->data))->guild = "";
		}
		else {
			((PlayerInfo*)(peer->data))->guildBg = gfbg;
			((PlayerInfo*)(peer->data))->guildFg = gffg;
			((PlayerInfo*)(peer->data))->guildStatement = gstatement;
			((PlayerInfo*)(peer->data))->guildLeader = gleader;
			((PlayerInfo*)(peer->data))->guildMembers = gmembers;
		}

		ifff.close();
	}
}
void autosave()
{
	bool exist = std::experimental::filesystem::exists("save.txt");
	if (!exist)
	{
		ofstream save("save.txt");
		save << 0;
		save.close();
	}
	std::ifstream ok("save.txt");
	std::string limits((std::istreambuf_iterator<char>(ok)),
		(std::istreambuf_iterator<char>()));
	int a = atoi(limits.c_str());
	if (a == 0)
	{
		ofstream ok;
		ok.open("save.txt");
		ok << 50;
		ok.close();
		worldDB.saveAll();
		cout << "[!]Auto Saving Worlds" << endl;
	}
	else
	{
		int aa = a - 1;
		ofstream ss;
		ss.open("save.txt");
		ss << aa;
		ss.close();
		if (aa == 0)
		{
			ofstream ok;
			ok.open("save.txt");
			ok << 50;
			ok.close();
			worldDB.saveAll();
			cout << "[!]Auto Saving Worlds" << endl;
		}
	}
}
bool CheckItemExists(ENetPeer* peer, const int fItemId) {
	auto isExists = false;
	for (auto i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++) {
		if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemId) {
			isExists = true;
			break;
		}
	}
	return isExists;
}

WorldInfo* getPlyersWorld(ENetPeer* peer)
{
	try {
		return worldDB.get2(((PlayerInfo*)(peer->data))->currentWorld).ptr;
	} catch(int e) {
		return NULL;
	}
}

struct TileExtra {
	int packetType;
	int characterState;
	float objectSpeedX;
	int punchX;
	int punchY;
	int charStat;
	int blockid;
	int visual;
	int signs;
	int backgroundid;
	int displayblock;
	int time;
	int netID;
	int weatherspeed;
	int bpm;
	int unused1;
	int unused2;
	int unused3;
	//int bpm;
};
BYTE* packBlockVisual222(TileExtra* dataStruct)
{

	BYTE* data = new BYTE[104]; // 96
	for (int i = 0; i < 100; i++)
	{
		data[i] = 0;
	}
	memcpy(data, &dataStruct->packetType, 4);
	memcpy(data + 8, &dataStruct->netID, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 16, &dataStruct->objectSpeedX, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	memcpy(data + 52, &dataStruct->charStat, 4);
	memcpy(data + 56, &dataStruct->blockid, 2);
	memcpy(data + 58, &dataStruct->backgroundid, 2);
	memcpy(data + 60, &dataStruct->visual, 4);
	memcpy(data + 64, &dataStruct->displayblock, 4);


	return data;
}
BYTE* packStuffVisual(TileExtra* dataStruct, int options, int gravity)
{
	BYTE* data = new BYTE[102];
	for (int i = 0; i < 102; i++)
	{
		data[i] = 0;
	}
	memcpy(data, &dataStruct->packetType, 4);
	memcpy(data + 8, &dataStruct->netID, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	memcpy(data + 52, &dataStruct->charStat, 4);
	memcpy(data + 56, &dataStruct->blockid, 2);
	memcpy(data + 58, &dataStruct->backgroundid, 2);
	memcpy(data + 60, &dataStruct->visual, 4);
	memcpy(data + 64, &dataStruct->displayblock, 4);
	memcpy(data + 68, &gravity, 4);
	memcpy(data + 70, &options, 4);

	return data;
}

struct PlayerMoving {
	int packetType;
	int netID;
	float x;
	float y;
	int characterState;
	int plantingTree;
	float XSpeed;
	float YSpeed;
	int punchX;
	int punchY;

};


enum ClothTypes {
	HAIR,
	SHIRT,
	PANTS,
	FEET,
	FACE,
	HAND,
	BACK,
	MASK,
	NECKLACE,
	ANCES,
	NONE
};

enum BlockTypes {
	FOREGROUND,
	BACKGROUND,
	SEED,
	PAIN_BLOCK,
	BEDROCK,
	MAIN_DOOR,
	SIGN,
	DOOR,
	CLOTHING,
	FIST,
	CONSUMABLE,
	CHECKPOINT,
	GATEWAY,
	LOCK,
	WEATHER_MACHINE,
	JAMMER,
	GEM,
	BOARD,
	UNKNOWN
};


struct ItemDefinition {
	int id;

	unsigned char editableType = 0;
	unsigned char itemCategory = 0;
	unsigned char actionType = 0;
	unsigned char hitSoundType = 0;

	string name;

	string texture = "";
	int textureHash = 0;
	unsigned char itemKind = 0;
	int val1;
	unsigned char textureX = 0;
	unsigned char textureY = 0;
	unsigned char spreadType = 0;
	unsigned char isStripeyWallpaper = 0;
	unsigned char collisionType = 0;

	unsigned char breakHits = 0;

	int dropChance = 0;
	unsigned char clothingType = 0;
	BlockTypes blockType;
	int growTime;
	ClothTypes clothType;
	int rarity;
	string effect = "(Mod removed)";
	string effects = "(Mod added)";
	unsigned char maxAmount = 0;
	string extraFile = "";
	int extraFileHash = 0;
	int audioVolume = 0;
	string petName = "";
	string petPrefix = "";
	string petSuffix = "";
	string petAbility = "";
	unsigned	char seedBase = 0;
	unsigned	char seedOverlay = 0;
	unsigned	char treeBase = 0;
	unsigned	char treeLeaves = 0;
	int seedColor = 0;
	int seedOverlayColor = 0;
	bool isMultiFace = false;
	short val2;
	short isRayman = 0;
	string extraOptions = "";
	string texture2 = "";
	string extraOptions2 = "";
	string punchOptions = "";
	string description = "Nothing to see.";
};

vector<ItemDefinition> itemDefs;

ItemDefinition getItemDef(int id)
{
	if (id < itemDefs.size() && id > -1)
		return itemDefs.at(id);
	/*for (int i = 0; i < itemDefs.size(); i++)
	{
		if (id == itemDefs.at(i).id)
		{
			return itemDefs.at(i);
		}
	}*/
	throw 0;
	return itemDefs.at(0);
}

void craftItemDescriptions() {
	int current = -1;
	std::ifstream infile("Descriptions.txt");
	for (std::string line; getline(infile, line);)
	{
		if (line.length() > 3 && line[0] != '/' && line[1] != '/')
		{
			vector<string> ex = explode("|", line);
			ItemDefinition def;
			if (atoi(ex[0].c_str()) + 1 < itemDefs.size())
			{
				itemDefs.at(atoi(ex[0].c_str())).description = ex[1];
				if (!(atoi(ex[0].c_str()) % 2))
					itemDefs.at(atoi(ex[0].c_str()) + 1).description = "This is a tree.";
			}
		}
	}
}

std::ifstream::pos_type filesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

uint32_t HashString(unsigned char* str, int len)
{
	if (!str) return 0;

	unsigned char* n = (unsigned char*)str;
	uint32_t acc = 0x55555555;

	if (len == 0)
	{
		while (*n)
			acc = (acc >> 27) + (acc << 5) + *n++;
	}
	else
	{
		for (int i = 0; i < len; i++)
		{
			acc = (acc >> 27) + (acc << 5) + *n++;
		}
	}
	return acc;

}

unsigned char* getA(string fileName, int* pSizeOut, bool bAddBasePath, bool bAutoDecompress)
{
	unsigned char* pData = NULL;
	FILE* fp = fopen(fileName.c_str(), "rb");
	if (!fp)
	{
		cout << "File not found" << endl;
		if (!fp) return NULL;
	}

	fseek(fp, 0, SEEK_END);
	*pSizeOut = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pData = (unsigned char*)new unsigned char[((*pSizeOut) + 1)];
	if (!pData)
	{
		printf("Out of memory opening %s?", fileName.c_str());
		return 0;
	}
	pData[*pSizeOut] = 0;
	fread(pData, *pSizeOut, 1, fp);
	fclose(fp);

	return pData;
}

int itemdathash;
void buildItemsDatabase()
{
	string secret = "PBG892FXX982ABC*";
	std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
	int size = file.tellg();
	itemsDatSize = size;
	char* data = new char[size];
	file.seekg(0, std::ios::beg);

	if (file.read((char*)(data), size))
	{
		itemsDat = new BYTE[60 + size];
		int MessageType = 0x4;
		int PacketType = 0x10;
		int NetID = -1;
		int CharState = 0x8;

		memset(itemsDat, 0, 60);
		memcpy(itemsDat, &MessageType, 4);
		memcpy(itemsDat + 4, &PacketType, 4);
		memcpy(itemsDat + 8, &NetID, 4);
		memcpy(itemsDat + 16, &CharState, 4);
		memcpy(itemsDat + 56, &size, 4);
		file.seekg(0, std::ios::beg);
		if (file.read((char*)(itemsDat + 60), size))
		{
			uint8_t* pData;
			int size = 0;
			const char filename[] = "items.dat";
			size = filesize(filename);
			pData = getA((string)filename, &size, false, false);
			cout << "Updating items data success! Hash: " << HashString((unsigned char*)pData, size) << endl;
			itemdathash = HashString((unsigned char*)pData, size);
			file.close();
		}
	}
	else {
		cout << "Updating items data failed!" << endl;
		exit(0);
	}
	int itemCount;
	int memPos = 0;
	int16_t itemsdatVersion = 0;
	memcpy(&itemsdatVersion, data + memPos, 2);
	memPos += 2;
	memcpy(&itemCount, data + memPos, 4);
	memPos += 4; 
	for (int i = 0; i < itemCount; i++) { 
		ItemDefinition tile; 

		{
			memcpy(&tile.id, data + memPos, 4);
			memPos += 4;
		}
		{
			tile.editableType = data[memPos];
			memPos += 1;
		}
		{
			tile.itemCategory = data[memPos];
			memPos += 1;
		}
		{
			tile.actionType = data[memPos];
			memPos += 1;
		}
		{
			tile.hitSoundType = data[memPos];
			memPos += 1;
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.name += data[memPos] ^ (secret[(j + tile.id) % secret.length()]);

				memPos++;
			}
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.texture += data[memPos];
				memPos++;
			}
		}
		memcpy(&tile.textureHash, data + memPos, 4);
		memPos += 4;
		tile.itemKind = memPos[data];
		memPos += 1;
		memcpy(&tile.val1, data + memPos, 4);
		memPos += 4;
		tile.textureX = data[memPos];
		memPos += 1;
		tile.textureY = data[memPos];
		memPos += 1;
		tile.spreadType = data[memPos];
		memPos += 1;
		tile.isStripeyWallpaper = data[memPos];
		memPos += 1;
		tile.collisionType = data[memPos];
		memPos += 1;
		tile.breakHits = data[memPos] / 6;
		memPos += 1;
		memcpy(&tile.dropChance, data + memPos, 4);
		memPos += 4;
		tile.clothingType = data[memPos];
		memPos += 1;
		memcpy(&tile.rarity, data + memPos, 2);
		memPos += 2;
		tile.maxAmount = data[memPos];
		memPos += 1;
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.extraFile += data[memPos];
				memPos++;
			}
		}
		memcpy(&tile.extraFileHash, data + memPos, 4);
		memPos += 4;
		memcpy(&tile.audioVolume, data + memPos, 4);
		memPos += 4;
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.petName += data[memPos];
				memPos++;
			}
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.petPrefix += data[memPos];
				memPos++;
			}
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.petSuffix += data[memPos];
				memPos++;
			}
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.petAbility += data[memPos];
				memPos++;
			}
		}
		{
			tile.seedBase = data[memPos];
			memPos += 1;
		}
		{
			tile.seedOverlay = data[memPos];
			memPos += 1;
		}
		{
			tile.treeBase = data[memPos];
			memPos += 1;
		}
		{
			tile.treeLeaves = data[memPos];
			memPos += 1;
		}
		{
			memcpy(&tile.seedColor, data + memPos, 4);
			memPos += 4;
		}
		{
			memcpy(&tile.seedOverlayColor, data + memPos, 4);
			memPos += 4;
		}
		memPos += 4; // deleted ingredients
		{
			memcpy(&tile.growTime, data + memPos, 4);
			memPos += 4;
		}
		memcpy(&tile.val2, data + memPos, 2);
		memPos += 2;
		memcpy(&tile.isRayman, data + memPos, 2);
		memPos += 2;
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.extraOptions += data[memPos];
				memPos++;
			}
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.texture2 += data[memPos];
				memPos++;
			}
		}
		{
			int16_t strLen = *(int16_t*)&data[memPos];
			memPos += 2;
			for (int j = 0; j < strLen; j++) {
				tile.extraOptions2 += data[memPos];
				memPos++;
			}
		}
		memPos += 80;
		if (itemsdatVersion >= 11) {
			{
				int16_t strLen = *(int16_t*)&data[memPos];
				memPos += 2;
				for (int j = 0; j < strLen; j++) {
					tile.punchOptions += data[memPos];
					memPos++;
				}
			}
		}
		if (itemsdatVersion >= 12) memPos += 13;
		if (itemsdatVersion >= 13) memPos += 4;
		if (itemsdatVersion >= 14) memPos += 4;
		if (i != tile.id)
			cout << "Item are unordered!" << i << "/" << tile.id << endl;

		switch (tile.actionType) {
		case 0:
			tile.blockType = BlockTypes::FIST;
			break;
		case 1:
			// wrench tool
			break;
		case 2:
			tile.blockType = BlockTypes::DOOR;
			break;
		case 3:
			tile.blockType = BlockTypes::LOCK;
			break;
		case 4:
			tile.blockType = BlockTypes::GEM;
			break;
		case 8:
			tile.blockType = BlockTypes::CONSUMABLE;
			break;
		case 9:
			tile.blockType = BlockTypes::GATEWAY;
			break;
		case 10:
			tile.blockType = BlockTypes::SIGN;
			break;
		case 13:
			tile.blockType = BlockTypes::MAIN_DOOR;
			break;
		case 15:
			tile.blockType = BlockTypes::BEDROCK;
			break;
		case 17:
			tile.blockType = BlockTypes::FOREGROUND;
			break;
		case 18:
			tile.blockType = BlockTypes::BACKGROUND;
			break;
		case 19:
			tile.blockType = BlockTypes::SEED;
			break;
		case 20:
			tile.blockType = BlockTypes::CLOTHING; 
				switch(tile.clothingType){
					case 0: tile.clothType = ClothTypes::HAIR;
						break;
					case 1: tile.clothType = ClothTypes::SHIRT;
						break;
					case 2: tile.clothType = ClothTypes::PANTS;
						break;
					case 3: tile.clothType = ClothTypes::FEET;
						break; 
					case 4: tile.clothType = ClothTypes::FACE;
						break;
					case 5: tile.clothType = ClothTypes::HAND;
						break;
					case 6: tile.clothType = ClothTypes::BACK;
						break;
					case 7: tile.clothType = ClothTypes::MASK;
						break;
					case 8: tile.clothType = ClothTypes::NECKLACE;
						break;
						
				} 

			break;
		case 26: // portal
			tile.blockType = BlockTypes::DOOR;
			break;
		case 27:
			tile.blockType = BlockTypes::CHECKPOINT;
			break;
		case 28: // piano note
			tile.blockType = BlockTypes::BACKGROUND;
			break;
		case 41:
			tile.blockType = BlockTypes::WEATHER_MACHINE;
			break;
		case 34: // bulletin boardd
			tile.blockType = BlockTypes::BOARD;
			break;
		case 107: // ances
			tile.blockType = BlockTypes::CLOTHING;
			tile.clothType = ClothTypes::ANCES;
			break;
		default:
			 break;

		}
 

		// -----------------
		itemDefs.push_back(tile);
	} 
	craftItemDescriptions();
}

void showWrong(ENetPeer* peer, string listFull, string itemFind) {
	packet::dialog(peer, "add_label_with_icon|big|`wFind item: " + itemFind + "``|left|3802|\nadd_spacer|small|\n" + listFull + "add_textbox|Enter a word below to find the item|\nadd_text_input|item|Item Name||30|\nend_dialog|findid|Cancel|Find the item!|\n");
}
void PathFindingCore(ENetPeer* peer, int xs, int ys)
{

	try {
		int Square = xs + (ys * 100); // 100 = Width World lu
		string gayname = ((PlayerInfo*)(peer->data))->rawName;
		WorldInfo* world = getPlyersWorld(peer);
		if (world->items[Square].PosFind == Square)
		{
			packet::consolemessage(peer, "`4[CHEAT DETECTED] " + gayname + " STOP CHEATING!");
			enet_peer_disconnect_later(peer, 0);
		}

	}
	catch (const std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
}

void LoadCaptCha() {
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
		Sleep(300);
		int userMaxRand = 50, userLowRand = 0;
		resultnbr2 = rand() % userMaxRand + userLowRand + 1;
		srand(time(0));
		resultnbr1 = rand() % userMaxRand + userLowRand + 1;
		hasil = resultnbr1 + resultnbr2;
		string captcha = "set_default_color|`o\nadd_label_with_icon|big|`wAre you Human?``|left|206|\nadd_spacer|small|\nadd_textbox|What will be the sum of the following numbers|left|\nadd_textbox|" + to_string(resultnbr1) + " + " + to_string(resultnbr2) + "|left|\nadd_text_input|captcha_answer|Answer:||32|\nadd_button|Submit_|Submit|";
		packet::dialog(currentPeer, captcha);
	}
}
void Captcha() {
	while (1) {
		Sleep(300);
		LoadCaptCha();
	}
}
void UpdateBlockState(ENetPeer* peer, int x, int y, bool forEveryone, WorldInfo* worldInfo) {

	if (!worldInfo) return;

	int i = y * worldInfo->width + x;

	int blockStateFlags = 0;


	if (worldInfo->items[i].flipped)
		blockStateFlags |= 0x00200000;
	if (worldInfo->items[i].water)
		blockStateFlags |= 0x04000000;
	if (worldInfo->items[i].glue)
		blockStateFlags |= 0x08000000;
	if (worldInfo->items[i].fire)
		blockStateFlags |= 0x10000000;
	if (worldInfo->items[i].red)
		blockStateFlags |= 0x20000000;
	if (worldInfo->items[i].green)
		blockStateFlags |= 0x40000000;
	if (worldInfo->items[i].blue)
		blockStateFlags |= 0x80000000;
	if (worldInfo->items[i].active)
		blockStateFlags |= 0x00400000;
	if (worldInfo->items[i].silenced)
		blockStateFlags |= 0x02400000;
}
void quiz(ENetPeer* currentPeer)
{
	int a = rand() % 100;
	int b = rand() % 300;
	int c = rand() % 79;
	int d = rand() % 30;
	int h = a + b;
	int j = c + d;
	int all = h + j;
	int free = rand() % 100 + 100;
	string gems = std::to_string(free);
	string timer = std::to_string(all);
	ofstream myfile;
	myfile.open("math.txt");
	myfile << timer;
	myfile.close();
	ofstream gay;
	gay.open("gems.txt");
	gay << gems;
	gay.close();
	for (currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer)
	{
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
			continue;
		GamePacket p3 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`9** Growtopia Daily Math [Questions : `3'" + to_string(resultnbr1) + " + " + to_string(resultnbr2) + "'`9 = ?] Prize: `2" + to_string(prize) + "`9 (gems) ! `o(/c <answer>)."));
		ENetPacket* packet3 = enet_packet_create(p3.data,
			p3.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(currentPeer, 0, packet3);
		//enet_peer_reset(currentPeer);
		delete p3.data;
	}
}

void DailyMath() {
	while (DailyMath) {
		using namespace std::chrono;
		if (quest + 400000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
			quest = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			int userMaxRand = 100, userLowRand = 0;
			resultnbr2 = rand() % userMaxRand + userLowRand + 1;
			srand(time(0));
			resultnbr1 = rand() % userMaxRand + userLowRand + 1;
			hasil = resultnbr1 + resultnbr2;
			prize = rand() % 10000;
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
				sendConsole(currentPeer, "`9** Growtopia Daily Math [Questions : `3'" + to_string(resultnbr1) + " + " + to_string(resultnbr2) + "'`9 = ?] Prize: `2" + to_string(prize) + "`9 (gems) ! `o(/c <answer>).");
				SendConsole("(DailyMath) Growtopia Daily Math : " + to_string(resultnbr1) + " + " + to_string(resultnbr2) + " = ? Prize: " + to_string(prize) + "! `o(/c <answer>).", "DAILYMATH");
				packet::PlayAudio(currentPeer, "startopia_tool_droid.wav", 0);
				DailyMaths = true;
			}
		}
	}
}

void addAdmin(string username, string password, int level)
{
	Admin admin;
	admin.username = username;
	admin.password = password;
	admin.level = level;
	admins.push_back(admin);
}

int getAdminLevel(string username, string password) {
	for (int i = 0; i < admins.size(); i++) {
		Admin admin = admins[i];
		if (admin.username == username && admin.password == password) {
			return admin.level;
		}
	}
	return 0;
}

int adminlevel(string name) {
	std::ifstream ifff("players/" + PlayerDB::getProperName(name) + ".json");
	json j;
	ifff >> j;

	int adminlevel;
	adminlevel = j["adminLevel"];

	ifff.close();
	if (adminlevel == 0) {
		return 0;
	}
	else {
		return adminlevel;
	}

}
int level(string name) {
	std::ifstream ifff("players/" + PlayerDB::getProperName(name) + ".json");
	json j;
	ifff >> j;

	int level;
	level = j["level"];

	ifff.close();
	return level;

}

string getRankText(string name) {
	int lvl = 0;
	lvl = adminlevel(name);
	if (lvl == 0) {
		return "`wPlayer";
	}
	if (lvl == 444) {
		return "`w[`1VIP`w]";
	}
	else if (lvl == 666) {
		return "`#Moderator";
	}
	else if (lvl == 777) {
		return "`4Administrator";
	}
	else if (lvl == 999) {
		return "`4CO-Creator";
	}
	else if (lvl == 1337) {
		return "`cServer-Creator";
	}
}
string getRankId(string name) {
	int lvl = 0;
	lvl = adminlevel(name);
	if (lvl == 0) {
		return "18";
	}
	if (lvl == 444) {
		return "274";
	}
	else if (lvl == 666) {
		return "278";
	}
	else if (lvl == 777) {
		return "276";
	}
	else if (lvl == 999) {
		return "1956";
	}
	else if (lvl == 1337) {
		return "2376";
	}
}
string getRankTexts(string name) {
	int lvl = 0;
	lvl = level(name);
	if (lvl <= 10) {
		return "`2Newbie";
	}
	if (lvl >= 11) {
		return "`1Advance";
	}
	if (lvl >= 50) {
		return "`cPro";
	}
	if (lvl >= 100) {
		return "`eMaster";
	}
	if (lvl >= 150) {
		return "`9Expert";
	}
	if (lvl >= 200) {
		return "`5A`4C`qE";
	}
}
string getRankIds(string name) {
	int lvl = 0;
	lvl = level(name);
	if (lvl <= 10) {
		return "3900";
	}
	if (lvl >= 11) {
		return "3192";
	}
	if (lvl >= 50) {
		return "7832";
	}
	if (lvl >= 100) {
		return "7586";
	}
	if (lvl >= 150) {
		return "6312";
	}
	if (lvl >= 200) {
		return "1956";
	}
}

bool canSB(string username, string password) {
	for (int i = 0; i < admins.size(); i++) {
		Admin admin = admins[i];
		if (admin.username == username && admin.password == password && admin.level>1) {
			using namespace std::chrono;
			if (admin.lastSB + 900000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() || admin.level == 999)
			{
				admins[i].lastSB = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				return true;
			}
			else {
				return false;
			}
		}
	}
	return false;
}

bool canClear(string username, string password) {
	for (int i = 0; i < admins.size(); i++) {
		Admin admin = admins[i];
		if (admin.username == username && admin.password == password) {
			return admin.level > 0;
		}
	}
	return false;
}

bool isSuperAdmin(string username, string password) {
	for (int i = 0; i < admins.size(); i++) {
		Admin admin = admins[i];
		if (admin.username == username && admin.password == password && admin.level == 999) {
			return true;
		}
	}
	return false;
}

bool isHere(ENetPeer* peer, ENetPeer* peer2)
{
	return ((PlayerInfo*)(peer->data))->currentWorld == ((PlayerInfo*)(peer2->data))->currentWorld;
}

void sendInventory(ENetPeer* peer, PlayerInventory inventory)
{
	string asdf2 = "0400000009A7379237BB2509E8E0EC04F8720B050000000000000000FBBB0000010000007D920100FDFDFDFD04000000040000000000000000000000000000000000";
	int inventoryLen = inventory.items.size();
	int packetLen = (asdf2.length() / 2) + (inventoryLen * 4) + 4;
	BYTE* data2 = new BYTE[packetLen];
	for (int i = 0; i < asdf2.length(); i += 2)
	{
		char x = ch2n(asdf2[i]);
		x = x << 4;
		x += ch2n(asdf2[i + 1]);
		memcpy(data2 + (i / 2), &x, 1);
	}
	int endianInvVal = _byteswap_ulong(inventoryLen);
	memcpy(data2 + (asdf2.length() / 2) - 4, &endianInvVal, 4);
	endianInvVal = _byteswap_ulong(((PlayerInfo*)(peer->data))->currentInventorySize);
	memcpy(data2 + (asdf2.length() / 2) - 8, &endianInvVal, 4);
	int val = 0;
	for (int i = 0; i < inventoryLen; i++)
	{
		val = 0;
		val |= inventory.items.at(i).itemID;
		val |= inventory.items.at(i).itemCount << 16;
		val &= 0x00FFFFFF;
		val |= 0x00 << 24;
		memcpy(data2 + (i * 4) + (asdf2.length() / 2), &val, 4);
	}
	ENetPacket* packet3 = enet_packet_create(data2,
		packetLen,
		ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet3);
	delete data2;
	//enet_host_flush(server);
}
void PlayAudioWorld(ENetPeer* peer, string audioFile) {
	string text = "action|play_sfx\nfile|" + audioFile + "\ndelayMS|0\n";
	BYTE* data = new BYTE[5 + text.length()];
	BYTE zero = 0;
	int type = 3;
	memcpy(data, &type, 4);
	memcpy(data + 4, text.c_str(), text.length());
	memcpy(data + 4 + text.length(), &zero, 1);
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, currentPeer)) {
			ENetPacket* packet2 = enet_packet_create(data,
				5 + text.length(),
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(currentPeer, 0, packet2);
		}
	}
	delete[] data;
}

void OnSetCurrentWeather(ENetPeer* peer, int weather) {
	GamePacket p2 = packetEnd(appendInt(appendString(createPacket(), "OnSetCurrentWeather"), weather));
	ENetPacket* packet2 = enet_packet_create(p2.data,
		p2.len,
		ENET_PACKET_FLAG_RELIABLE);
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, currentPeer)) {
			enet_peer_send(currentPeer, 0, packet2);
		}
	}
	delete p2.data;
}
inline void autoSaveWorlds()
{
	while (true)
	{
		Sleep(300000);
		ENetPeer* currentPeer;
		serverIsFrozen = true;
		for (currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
		}
		GamePacket p0 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"),
			"`2Saving all worlds."));
		ENetPacket* packet0 = enet_packet_create(p0.data,
			p0.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_host_broadcast(server, 0, packet0);
		saveAllWorlds();

	}
	serverIsFrozen = false;

}

void sendGazette(ENetPeer* peer) {
	std::ifstream news("news.txt");
	std::stringstream buffer;
	buffer << news.rdbuf();
	std::string newsString(buffer.str());
	packet::dialog(peer, newsString);
}
bool SaveConvertedItem(int fItemid, int fQuantity, ENetPeer* peer)
{
	int invsizee = ((PlayerInfo*)(peer->data))->currentInventorySize;

	bool doesItemInInventryAlready = false;

	for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
	{
		if (((PlayerInfo*)(peer->data))->inventory.items[i].itemID == fItemid)
		{
			doesItemInInventryAlready = true;
			if (((PlayerInfo*)(peer->data))->inventory.items[i].itemCount >= 101) return false;
			break;
		}
	}

	std::ifstream iffff("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");

	json jj;

	if (iffff.fail()) {
		iffff.close();
		cout << "SaveConvertedItem funkcijoje (ifstream dalyje) error: itemid - " << fItemid << ", kiekis - " << fQuantity << endl;

	}
	if (iffff.is_open()) {


	}

	iffff >> jj; //load


	std::ofstream oo("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
	if (!oo.is_open()) {
		cout << GetLastError() << " SaveConvertedItem funkcijoje (ofstream dalyje) error: itemid - " << fItemid << ", kiekis - " << fQuantity << endl;
		_getch();
	}
	int howManyHasNow = 0;

	for (int i = 0; i < ((PlayerInfo*)(peer->data))->currentInventorySize; i++)
	{
		int itemidFromJson = jj["items"][i]["itemid"];
		int quantityFromJson = jj["items"][i]["quantity"];;
		if (doesItemInInventryAlready)
		{
			if (itemidFromJson == fItemid)
			{
				howManyHasNow = jj["items"][i]["quantity"];
				howManyHasNow += fQuantity;
				jj["items"][i]["quantity"] = howManyHasNow;

				for (int k = 0; k < ((PlayerInfo*)(peer->data))->inventory.items.size(); k++)
				{
					if (((PlayerInfo*)(peer->data))->inventory.items[k].itemID == fItemid)
					{
						((PlayerInfo*)(peer->data))->inventory.items[k].itemCount += (byte)fQuantity;
						break;
					}
				}
				sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);

				break;
			}
		}
		else if (itemidFromJson == 0 && quantityFromJson == 0)
		{
			jj["items"][i]["quantity"] = fQuantity;
			jj["items"][i]["itemid"] = fItemid;

			InventoryItem item;
			item.itemID = fItemid;
			item.itemCount = fQuantity;
			((PlayerInfo*)(peer->data))->inventory.items.push_back(item);

			sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);

			break;
		}
	}

	oo << jj << std::endl;
	return true;
}
void RemoveInventoryItem(int fItemid, int fQuantity, ENetPeer* peer)
{
	std::ifstream iffff("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");

	json jj;

	if (iffff.fail()) {
		iffff.close();

	}
	if (iffff.is_open()) {


	}

	iffff >> jj; //load


	std::ofstream oo("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
	if (!oo.is_open()) {
		cout << GetLastError() << " Gak bisa delete " << fItemid << " bruh" << fQuantity << endl;
		_getch();
	}

	//jj["items"][aposition]["aposition"] = aposition;


	for (int i = 0; i < ((PlayerInfo*)(peer->data))->currentInventorySize; i++)
	{
		int itemid = jj["items"][i]["itemid"];
		int quantity = jj["items"][i]["quantity"];
		if (itemid == fItemid)
		{
			if (quantity - fQuantity == 0)
			{
				jj["items"][i]["itemid"] = 0;
				jj["items"][i]["quantity"] = 0;
			}
			else
			{
				jj["items"][i]["itemid"] = itemid;
				jj["items"][i]["quantity"] = quantity - fQuantity;
			}

			break;
		}

	}
	oo << jj << std::endl;

	for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
	{
		if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemid)
		{
			if ((unsigned int)((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount > fQuantity && (unsigned int)((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount != fQuantity)
			{
				((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount -= fQuantity;
			}
			else
			{
				((PlayerInfo*)(peer->data))->inventory.items.erase(((PlayerInfo*)(peer->data))->inventory.items.begin() + i);
			}
			sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);
		}
	}


}
void SearchInventoryItem(ENetPeer* peer, int fItemid, int fQuantity, bool& iscontains)
{
	iscontains = false;
	for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
	{
		if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemid && ((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount >= fQuantity) {

			iscontains = true;
			break;
		}
	}
}
void SaveFindsItem(int fItemid, int fQuantity, ENetPeer* peer)
{

	std::ifstream iffff("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");

	json jj;

	if (iffff.fail()) {
		iffff.close();


	}
	if (iffff.is_open()) {


	}

	iffff >> jj; //load


	std::ofstream oo("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
	if (!oo.is_open()) {
		cout << GetLastError() << endl;
		_getch();
	}

	//jj["items"][aposition]["aposition"] = aposition;

	for (int i = 0; i < ((PlayerInfo*)(peer->data))->currentInventorySize; i++)
	{
		int itemid = jj["items"][i]["itemid"];
		int quantity = jj["items"][i]["quantity"];
		if (itemid == 0 && quantity == 0)
		{
			jj["items"][i]["itemid"] = fItemid;
			jj["items"][i]["quantity"] = fQuantity;
			break;
		}

	}
	oo << jj << std::endl;


	InventoryItem item;
	item.itemID = fItemid;
	item.itemCount = fQuantity;
	((PlayerInfo*)(peer->data))->inventory.items.push_back(item);

	sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);
}
void SaveShopsItemMoreTimes(int fItemid, int fQuantity, ENetPeer* peer, bool& success)
{
	size_t invsizee = ((PlayerInfo*)(peer->data))->currentInventorySize;
	bool invfull = false;
	bool alreadyhave = false;


	if (((PlayerInfo*)(peer->data))->inventory.items.size() == invsizee) {

		GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "add_label_with_icon|big|`4Whoops!|left|1432|\nadd_spacer|small|\nadd_textbox|`oSorry! Your inventory is full! You can purchase an inventory upgrade in the shop.|\nadd_spacer|small|\nadd_button|close|`5Close|0|0|"));
		ENetPacket* packet = enet_packet_create(ps.data,
			ps.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
		delete ps.data;


		alreadyhave = true;
	}

	bool isFullStock = false;
	bool isInInv = false;
	for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
	{

		if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemid && ((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount >= 200) {


			GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "add_label_with_icon|big|`4Whoops!|left|1432|\nadd_spacer|small|\nadd_textbox|`oSorry! You already have full stock of this item!|\nadd_spacer|small|\nadd_button|close|`5Close|0|0|"));
			ENetPacket* packet = enet_packet_create(ps.data,
				ps.len,
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet);
			delete ps.data;


			isFullStock = true;
		}

		if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemid && ((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount < 200)	isInInv = true;

	}

	if (isFullStock == true || alreadyhave == true)
	{
		success = false;
	}
	else
	{
		success = true;

		std::ifstream iffff("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");

		json jj;

		if (iffff.fail()) {
			iffff.close();


		}
		if (iffff.is_open()) {


		}

		iffff >> jj; //load


		std::ofstream oo("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
		if (!oo.is_open()) {
			cout << GetLastError() << endl;
			_getch();
		}

		//jj["items"][aposition]["aposition"] = aposition;

		if (isInInv == false)
		{

			for (int i = 0; i < ((PlayerInfo*)(peer->data))->currentInventorySize; i++)
			{
				int itemid = jj["items"][i]["itemid"];
				int quantity = jj["items"][i]["quantity"];

				if (itemid == 0 && quantity == 0)
				{
					jj["items"][i]["itemid"] = fItemid;
					jj["items"][i]["quantity"] = fQuantity;
					break;
				}

			}
			oo << jj << std::endl;


			InventoryItem item;
			item.itemID = fItemid;
			item.itemCount = fQuantity;
			((PlayerInfo*)(peer->data))->inventory.items.push_back(item);

			sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);
		}
		else
		{
			for (int i = 0; i < ((PlayerInfo*)(peer->data))->currentInventorySize; i++)
			{
				int itemid = jj["items"][i]["itemid"];
				int quantity = jj["items"][i]["quantity"];

				if (itemid == fItemid)
				{
					jj["items"][i]["quantity"] = quantity + fQuantity;
					break;
				}

			}
			oo << jj << std::endl;


			for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
			{
				if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == fItemid)
				{
					((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount += fQuantity;
					sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);
				}
			}

		}
	}
}
void lvl1growpass(ENetPeer* peer) {
	std::ifstream news("growpasses/lvl1.txt");
	std::stringstream buffer;
	buffer << news.rdbuf();
	std::string newsString(buffer.str());
	packet::dialog(peer, newsString);
}
void removeAcc(ENetPeer* peer, string name) {
	string resetacc;
	std::ifstream ifff("players/" + PlayerDB::getProperName(name) + ".json");
	json j;
	ifff >> j;
}
void sendGrowmojis(ENetPeer* peer) {
	std::ifstream emoji("growmoji.txt");
	std::stringstream buffer;
	buffer << emoji.rdbuf();
	std::string newsString(buffer.str());
	packet::dialog(peer, newsString);
}

BYTE* packPlayerMoving(PlayerMoving* dataStruct)
{
	BYTE* data = new BYTE[56];
	for (int i = 0; i < 56; i++)
	{
		data[i] = 0;
	}
	memcpy(data, &dataStruct->packetType, 4);
	memcpy(data + 4, &dataStruct->netID, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 20, &dataStruct->plantingTree, 4);
	memcpy(data + 24, &dataStruct->x, 4);
	memcpy(data + 28, &dataStruct->y, 4);
	memcpy(data + 32, &dataStruct->XSpeed, 4);
	memcpy(data + 36, &dataStruct->YSpeed, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	return data;
}

BYTE* packBlockVisual(BlockVisual* dataStruct)
{
	BYTE* data = new BYTE[72];
	for (int i = 0; i < 72; i++)
	{
		data[i] = 0;
	}
	memcpy(data, &dataStruct->packetType, 4);
	memcpy(data + 8, &dataStruct->netID, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	memcpy(data + 52, &dataStruct->charStat, 4);
	memcpy(data + 56, &dataStruct->blockid, 4);
	//memcpy(data + 58, &dataStruct->backgroundid, 4);
	memcpy(data + 60, &dataStruct->visual, 4);
	memcpy(data + 64, &dataStruct->displayblock, 4);


	return data;
}

PlayerMoving* unpackPlayerMoving(BYTE* data)
{
	PlayerMoving* dataStruct = new PlayerMoving;
	memcpy(&dataStruct->packetType, data, 4);
	memcpy(&dataStruct->netID, data + 4, 4);
	memcpy(&dataStruct->characterState, data + 12, 4);
	memcpy(&dataStruct->plantingTree, data + 20, 4);
	memcpy(&dataStruct->x, data + 24, 4);
	memcpy(&dataStruct->y, data + 28, 4);
	memcpy(&dataStruct->XSpeed, data + 32, 4);
	memcpy(&dataStruct->YSpeed, data + 36, 4);
	memcpy(&dataStruct->punchX, data + 44, 4);
	memcpy(&dataStruct->punchY, data + 48, 4);
	return dataStruct;
}

void SendPacket(int a1, string a2, ENetPeer* enetPeer)
{
	if (enetPeer)
	{
		ENetPacket* v3 = enet_packet_create(0, a2.length() + 5, 1);
		memcpy(v3->data, &a1, 4);
		//*(v3->data) = (DWORD)a1;
		memcpy((v3->data) + 4, a2.c_str(), a2.length());

		//cout << std::hex << (int)(char)v3->data[3] << endl;
		enet_peer_send(enetPeer, 0, v3);
	}
}

void SendPacketRaw(int a1, void *packetData, size_t packetDataSize, void *a4, ENetPeer* peer, int packetFlag)
{
	ENetPacket *p;

	if (peer) // check if we have it setup
	{
		if (a1 == 4 && *((BYTE *)packetData + 12) & 8)
		{
			p = enet_packet_create(0, packetDataSize + *((DWORD *)packetData + 13) + 5, packetFlag);
			int four = 4;
			memcpy(p->data, &four, 4);
			memcpy((char *)p->data + 4, packetData, packetDataSize);
			memcpy((char *)p->data + packetDataSize + 4, a4, *((DWORD *)packetData + 13));
			enet_peer_send(peer, 0, p);
		}
		else
		{
			p = enet_packet_create(0, packetDataSize + 5, packetFlag);
			memcpy(p->data, &a1, 4);
			memcpy((char *)p->data + 4, packetData, packetDataSize);
			enet_peer_send(peer, 0, p);
		}
	}
	delete (char*)packetData;
}
void sendState(ENetPeer* peer) {
	//return; // TODO
	PlayerInfo* info = ((PlayerInfo*)(peer->data));
	int netID = info->netID;
	ENetPeer* currentPeer;
	int state = getState(info);
	for (currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer)
	{
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
			continue;
		if (isHere(peer, currentPeer)) {
			PlayerMoving data;
			data.packetType = 0x14;
			data.characterState = 0; // animation
			data.x = 1000;
			data.y = 100;
			data.punchX = 0;
			data.punchY = 0;
			data.XSpeed = 300;
			data.YSpeed = 600;
			data.netID = netID;
			data.plantingTree = state;
			BYTE* raw = packPlayerMoving(&data);
			int var = info->peffect; // placing and breking
			memcpy(raw + 1, &var, 3);
			float waterspeed = 125.0f;
			memcpy(raw + 16, &waterspeed, 4);
			SendPacketRaw(4, raw, 56, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);
		}
	}
	// TODO
}
void SendDropSingle(ENetPeer* peer, int netID, int x, int y, int item, int count, BYTE specialEffect)
{
	if (item >= maxItems) return;
	if (item < 0) return;

	PlayerMoving data;
	data.packetType = 14;
	data.x = x;
	data.y = y;
	data.netID = netID;
	data.plantingTree = item;
	float val = count; // item count
	BYTE val2 = specialEffect;

	BYTE* raw = packPlayerMoving(&data);
	memcpy(raw + 16, &val, 4);
	memcpy(raw + 1, &val2, 1);

	SendPacketRaw(4, raw, 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
}
void updateDoor(ENetPeer* peer, int foreground, int x, int y, string text)
{
	PlayerMoving sign;
	sign.packetType = 0x3;
	sign.characterState = 0x0;
	sign.x = x;
	sign.y = y;
	sign.punchX = x;
	sign.punchY = y;
	sign.XSpeed = 0;
	sign.YSpeed = 0;
	sign.netID = -1;
	sign.plantingTree = foreground;
	SendPacketRaw(4, packPlayerMoving(&sign), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
	int hmm = 8;
	int text_len = text.length();
	int lol = 0;
	int wut = 5;
	int yeh = hmm + 3 + 1;
	int idk = 15 + text_len;
	int is_locked = 0;
	int bubble_type = 1;
	int ok = 52 + idk;
	int kek = ok + 4;
	int yup = ok - 8 - idk;
	int four = 4;
	int magic = 56;
	int wew = ok + 5 + 4;
	int wow = magic + 4 + 5;

	BYTE* data = new BYTE[kek];
	ENetPacket* p = enet_packet_create(0, wew, ENET_PACKET_FLAG_RELIABLE);
	for (int i = 0; i < kek; i++) data[i] = 0;
	memcpy(data, &wut, four); //4
	memcpy(data + yeh, &hmm, four); //8
	memcpy(data + yup, &x, 4); //12
	memcpy(data + yup + 4, &y, 4); //16
	memcpy(data + 4 + yup + 4, &idk, four); //20
	memcpy(data + magic, &foreground, 2); //22
	memcpy(data + four + magic, &lol, four); //26
	memcpy(data + magic + 4 + four, &bubble_type, 1); //27
	memcpy(data + wow, &text_len, 2); //data + wow = text_len, pos 29
	memcpy(data + 2 + wow, text.c_str(), text_len); //data + text_len_len + text_len_offs = text, pos 94
	memcpy(data + ok, &is_locked, four); //98
	memcpy(p->data, &four, four); //4
	memcpy((char*)p->data + four, data, kek); //kek = data_len
	ENetPeer* currentPeer;
	for (currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer)
	{
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
			continue;
		if (isHere(peer, currentPeer)) {
			enet_peer_send(currentPeer, 0, p);
		}
	}
	delete data;
}
void updateEntrance(ENetPeer* peer, int foreground, int x, int y, bool open, int bg) {
	BYTE* data = new BYTE[69];// memset(data, 0, 69);
	for (int i = 0; i < 69; i++) data[i] = 0;
	int four = 4; int five = 5; int eight = 8;
	int huhed = (65536 * bg) + foreground; int loled = 128;

	memcpy(data, &four, 4);
	memcpy(data + 4, &five, 4);
	memcpy(data + 16, &eight, 4);
	memcpy(data + 48, &x, 4);
	memcpy(data + 52, &y, 4);
	memcpy(data + 56, &eight, 4);
	memcpy(data + 60, &foreground, 4);
	memcpy(data + 62, &bg, 4);

	if (open) {
		int state = 0;
		memcpy(data + 66, &loled, 4);
		memcpy(data + 68, &state, 4);
	}
	else {
		int state = 100;
		int yeetus = 25600;
		memcpy(data + 67, &yeetus, 5);
		memcpy(data + 68, &state, 4);
	}
	ENetPacket* p = enet_packet_create(data, 69, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, p);
}
void updateSign(ENetPeer* peer, int foreground, int x, int y, string text, int background)
{
	PlayerMoving sign;
	sign.packetType = 0x3;
	sign.characterState = 0x0;
	sign.x = x;
	sign.y = y;
	sign.punchX = x;
	sign.punchY = y;
	sign.XSpeed = 0;
	sign.YSpeed = 0;
	sign.netID = -1;
	sign.plantingTree = foreground;
	SendPacketRaw(4, packPlayerMoving(&sign), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
	int hmm = 8, wot = text.length(), lol = 0, wut = 5;
	int yeh = hmm + 3 + 1, idk = 15 + wot, lmao = -1, yey = 2; //idk = text_len + 15, wut = type(?), wot = text_len, yey = len of text_len
	int ok = 52 + idk;
	int kek = ok + 4, yup = ok - 8 - idk;
	int thonk = 4, magic = 56, wew = ok + 5 + 4;
	int wow = magic + 4 + 5;
	BYTE* data = new BYTE[kek];
	ENetPacket* p = enet_packet_create(0, wew, ENET_PACKET_FLAG_RELIABLE);
	for (int i = 0; i < kek; i++) data[i] = 0;
	memcpy(data, &wut, thonk);
	memcpy(data + yeh, &hmm, thonk); //read discord
	memcpy(data + yup, &x, 4);
	memcpy(data + yup + 4, &y, 4);
	memcpy(data + 4 + yup + 4, &idk, thonk);
	memcpy(data + magic, &foreground, yey);
	memcpy(data + magic + 2, &background, yey); //p100 fix by the one and only lapada
	memcpy(data + thonk + magic, &lol, thonk);
	memcpy(data + magic + 4 + thonk, &yey, 1);
	memcpy(data + wow, &wot, yey); //data + wow = text_len
	memcpy(data + yey + wow, text.c_str(), wot); //data + text_len_len + text_len_offs = text
	memcpy(data + ok, &lmao, thonk); //end ?
	memcpy(p->data, &thonk, thonk);
	memcpy((char*)p->data + thonk, data, kek); //kek = data_len
	ENetPeer* currentPeer;
	for (currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer)
	{
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
			continue;
		if (isHere(peer, currentPeer)) {
			enet_peer_send(currentPeer, 0, p);
		}
	}
	delete data;
}


void updateSignSound(ENetPeer* peer, int foreground, int x, int y, string text, int background)
{
	int hmm = 8, wot = text.length(), lol = 0, wut = 5;
	int yeh = hmm + 3 + 1, idk = 15 + wot, lmao = -1, yey = 2; //idk = text_len + 15, wut = type(?), wot = text_len, yey = len of text_len
	int ok = 52 + idk;
	int kek = ok + 4, yup = ok - 8 - idk;
	int thonk = 4, magic = 56, wew = ok + 5 + 4;
	int wow = magic + 4 + 5;
	BYTE* data = new BYTE[kek];
	ENetPacket* p = enet_packet_create(0, wew, ENET_PACKET_FLAG_RELIABLE);
	for (int i = 0; i < kek; i++) data[i] = 0;
	memcpy(data, &wut, thonk);
	memcpy(data + yeh, &hmm, thonk); //read discord
	memcpy(data + yup, &x, 4);
	memcpy(data + yup + 4, &y, 4);
	memcpy(data + 4 + yup + 4, &idk, thonk);
	memcpy(data + magic, &foreground, yey);
	memcpy(data + magic + 2, &background, yey); //p100 fix by the one and only lapada
	memcpy(data + thonk + magic, &lol, thonk);
	memcpy(data + magic + 4 + thonk, &yey, 1);
	memcpy(data + wow, &wot, yey); //data + wow = text_len
	memcpy(data + yey + wow, text.c_str(), wot); //data + text_len_len + text_len_offs = text
	memcpy(data + ok, &lmao, thonk); //end ?
	memcpy(p->data, &thonk, thonk);
	memcpy((char*)p->data + thonk, data, kek); //kek = data_len
	ENetPeer* currentPeer;
	for (currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer)
	{
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
			continue;
		if (isHere(peer, currentPeer)) {
			enet_peer_send(currentPeer, 0, p);
		}
	}
	delete data;
}
void sendStuffweather(ENetPeer* peer, int x, int y, uint16_t itemid, uint16_t gravity, bool spin, bool invert) {
	PlayerMoving pmov;
	pmov.packetType = 5;
	pmov.characterState = 8;
	pmov.punchX = x;
	pmov.punchY = y;
	pmov.netID = -1;
	uint8_t* pmovpacked = packPlayerMoving(&pmov);
	*(uint32_t*)(pmovpacked + 52) = 10 + 8;
	uint8_t* packet = new uint8_t[4 + 56 + 10 + 8];
	memset(packet, 0, 4 + 56 + 10 + 8);
	packet[0] = 4;
	memcpy(packet + 4, pmovpacked, 56);
	*(uint16_t*)(packet + 4 + 56) = 3832; // WEATHER MACHINE - STUFF
	*(uint16_t*)(packet + 4 + 56 + 6) = 1;
	*(uint8_t*)(packet + 4 + 56 + 8) = 0x31;
	*(uint32_t*)(packet + 4 + 56 + 9) = itemid;
	*(uint32_t*)(packet + 4 + 56 + 13) = gravity;
	*(uint8_t*)(packet + 4 + 56 + 17) = spin | (invert << 1);
	ENetPacket* epacket = enet_packet_create(packet, 4 + 56 + 8 + 10, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, epacket);
	delete pmovpacked;
	delete[] packet;
}
void sendBackground(ENetPeer* peer, int x, int y, uint16_t itemid) {
	PlayerMoving pmov;
	pmov.packetType = 5;
	pmov.characterState = 8;
	pmov.punchX = x;
	pmov.punchY = y;
	pmov.netID = -1;
	uint8_t* pmovpacked = packPlayerMoving(&pmov);
	*(uint32_t*)(pmovpacked + 52) = 5 + 8;
	uint8_t* packet = new uint8_t[4 + 56 + 5 + 8];
	memset(packet, 0, 4 + 56 + 5 + 8);
	packet[0] = 4;
	memcpy(packet + 4, pmovpacked, 56);
	*(uint16_t*)(packet + 4 + 56) = 5000; // WEATHER MACHINE - BACKGROUND
	*(uint16_t*)(packet + 4 + 56 + 6) = 1;
	*(uint8_t*)(packet + 4 + 56 + 8) = 0x28;
	*(uint16_t*)(packet + 4 + 56 + 9) = itemid;
	ENetPacket* epacket = enet_packet_create(packet, 4 + 56 + 8 + 5, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, epacket);
	delete pmovpacked;
	delete[] packet;
}


	void onPeerConnect(ENetPeer* peer)
	{
		ENetPeer * currentPeer;

		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (peer != currentPeer)
			{
				if (isHere(peer, currentPeer))
				{
					string netIdS = std::to_string(((PlayerInfo*)(currentPeer->data))->netID);
					packet::onspawn(peer, "spawn|avatar\nnetID|" + netIdS + "\nuserID|" + netIdS + "\ncolrect|0|0|20|30\nposXY|" + std::to_string(((PlayerInfo*)(currentPeer->data))->x) + "|" + std::to_string(((PlayerInfo*)(currentPeer->data))->y) + "\nname|``" + ((PlayerInfo*)(currentPeer->data))->displayName + "``\ncountry|" + ((PlayerInfo*)(currentPeer->data))->country + "\ninvis|0\nmstate|0\nsmstate|0\n"); // ((PlayerInfo*)(server->peers[i].data))->tankIDName
					string netIdS2 = std::to_string(((PlayerInfo*)(peer->data))->netID);
					packet::onspawn(currentPeer, "spawn|avatar\nnetID|" + netIdS2 + "\nuserID|" + netIdS2 + "\ncolrect|0|0|20|30\nposXY|" + std::to_string(((PlayerInfo*)(peer->data))->x) + "|" + std::to_string(((PlayerInfo*)(peer->data))->y) + "\nname|``" + ((PlayerInfo*)(peer->data))->displayName + "``\ncountry|" + ((PlayerInfo*)(peer->data))->country + "\ninvis|0\nmstate|0\nsmstate|0\n"); // ((PlayerInfo*)(server->peers[i].data))->tankIDName
				}
			}
		}
		
	}

	void updateAllClothes(ENetPeer* peer)
	{
		ENetPeer* currentPeer;
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (isHere(peer, currentPeer))
			{
				GamePacket p3 = packetEnd(appendFloat(appendIntx(appendFloat(appendFloat(appendFloat(appendString(createPacket(), "OnSetClothing"), ((PlayerInfo*)(peer->data))->cloth_hair, ((PlayerInfo*)(peer->data))->cloth_shirt, ((PlayerInfo*)(peer->data))->cloth_pants), ((PlayerInfo*)(peer->data))->cloth_feet, ((PlayerInfo*)(peer->data))->cloth_face, ((PlayerInfo*)(peer->data))->cloth_hand), ((PlayerInfo*)(peer->data))->cloth_back, ((PlayerInfo*)(peer->data))->cloth_mask, ((PlayerInfo*)(peer->data))->cloth_necklace), ((PlayerInfo*)(peer->data))->skinColor), ((PlayerInfo*)(peer->data))->cloth_ances, 0.0f, 0.0f));
				memcpy(p3.data + 8, &(((PlayerInfo*)(peer->data))->netID), 4); // ffloor
				ENetPacket* packet3 = enet_packet_create(p3.data,
					p3.len,
					ENET_PACKET_FLAG_RELIABLE);

				enet_peer_send(currentPeer, 0, packet3);
				delete p3.data;
				//enet_host_flush(server);
				GamePacket p4 = packetEnd(appendFloat(appendIntx(appendFloat(appendFloat(appendFloat(appendString(createPacket(), "OnSetClothing"), ((PlayerInfo*)(currentPeer->data))->cloth_hair, ((PlayerInfo*)(currentPeer->data))->cloth_shirt, ((PlayerInfo*)(currentPeer->data))->cloth_pants), ((PlayerInfo*)(currentPeer->data))->cloth_feet, ((PlayerInfo*)(currentPeer->data))->cloth_face, ((PlayerInfo*)(currentPeer->data))->cloth_hand), ((PlayerInfo*)(currentPeer->data))->cloth_back, ((PlayerInfo*)(currentPeer->data))->cloth_mask, ((PlayerInfo*)(currentPeer->data))->cloth_necklace), ((PlayerInfo*)(currentPeer->data))->skinColor), ((PlayerInfo*)(currentPeer->data))->cloth_ances, 0.0f, 0.0f));
				memcpy(p4.data + 8, &(((PlayerInfo*)(currentPeer->data))->netID), 4); // ffloor
				ENetPacket* packet4 = enet_packet_create(p4.data,
					p4.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet4);
				delete p4.data;
				//enet_host_flush(server);
			}
		}
	}

	void sendClothes(ENetPeer* peer) {
		GamePacket p3 = packetEnd(appendFloat(appendIntx(appendFloat(appendFloat(appendFloat(appendString(createPacket(), "OnSetClothing"), ((PlayerInfo*)(peer->data))->cloth_hair, ((PlayerInfo*)(peer->data))->cloth_shirt, ((PlayerInfo*)(peer->data))->cloth_pants), ((PlayerInfo*)(peer->data))->cloth_feet, ((PlayerInfo*)(peer->data))->cloth_face, ((PlayerInfo*)(peer->data))->cloth_hand), ((PlayerInfo*)(peer->data))->cloth_back, ((PlayerInfo*)(peer->data))->cloth_mask, ((PlayerInfo*)(peer->data))->cloth_necklace), ((PlayerInfo*)(peer->data))->skinColor), ((PlayerInfo*)(peer->data))->cloth_ances, 0.0f, 0.0f));
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
			if (isHere(peer, currentPeer)) {
				string text = "action|play_sfx\nfile|audio/change_clothes.wav\ndelayMS|0\n";
				BYTE* data = new BYTE[5 + text.length()];
				BYTE zero = 0;
				int type = 3;
				memcpy(data, &type, 4);
				memcpy(data + 4, text.c_str(), text.length());
				memcpy(data + 4 + text.length(), &zero, 1);
				ENetPacket* packet2 = enet_packet_create(data,
					5 + text.length(),
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(currentPeer, 0, packet2);
				delete[] data;
				memcpy(p3.data + 8, &(((PlayerInfo*)(peer->data))->netID), 4); // ffloor
				ENetPacket* packet3 = enet_packet_create(p3.data,
					p3.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(currentPeer, 0, packet3);
			}
		}
		ifstream fg("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
		json j;
		fg >> j;
		fg.close();
		PlayerInfo* p = ((PlayerInfo*)(peer->data));
		string username = PlayerDB::getProperName(p->rawName);
		int wls = p->premwl;
		int clothback = p->cloth_back;
		int clothhand = p->cloth_hand;
		int clothface = p->cloth_face;
		int clothhair = p->cloth_hair;
		int clothfeet = p->cloth_feet;
		int clothpants = p->cloth_pants;
		int clothneck = p->cloth_necklace;
		int clothshirt = p->cloth_shirt;
		int clothmask = p->cloth_mask;
		int clothances = p->cloth_ances;
		j["ClothBack"] = clothback;
		j["ClothHand"] = clothhand;
		j["ClothFace"] = clothface;
		j["ClothShirt"] = clothshirt;
		j["ClothPants"] = clothpants;
		j["ClothNeck"] = clothneck;
		j["ClothHair"] = clothhair;
		j["ClothFeet"] = clothfeet;
		j["ClothMask"] = clothmask;
		j["ClothAnces"] = clothances;
		j["effect"] = p->peffect;
		j["premwl"] = p->premwl;
		j["worldsowned"] = ((PlayerInfo*)(peer->data))->worldsowned;
		ofstream fs("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
		fs << j;
		fs.close();
	}
	void sendEmoji(ENetPeer* peer, string emoji)
	{
		GamePacket p2ssw = packetEnd(appendString(appendInt(appendString(createPacket(), "OnEmoticonDataChanged"), 0), u8"(wl)|ā|1&(yes)|Ă|1&(no)|ă|1&(love)|Ą|1&(oops)|ą|1&(shy)|Ć|1&(wink)|ć|1&(tongue)|Ĉ|1&(agree)|ĉ|1&(sleep)|Ċ|1&(punch)|ċ|1&(music)|Č|1&(build)|č|1&(megaphone)|Ď|1&(sigh)|ď|1&(mad)|Đ|1&(wow)|đ|1&(dance)|Ē|1&(see-no-evil)|ē|1&(bheart)|Ĕ|1&(heart)|ĕ|1&(grow)|Ė|1&(gems)|ė|1&(kiss)|Ę|1&(gtoken)|ę|1&(lol)|Ě|1&(smile)|Ā|1&(cool)|Ĝ|1&(cry)|ĝ|1&(vend)|Ğ|1&(bunny)|ě|1&(cactus)|ğ|1&(pine)|Ĥ|1&(peace)|ģ|1&(terror)|ġ|1&(troll)|Ġ|1&(evil)|Ģ|1&(fireworks)|Ħ|1&(football)|ĥ|1&(alien)|ħ|1&(party)|Ĩ|1&(pizza)|ĩ|1&(clap)|Ī|1&(song)|ī|1&(ghost)|Ĭ|1&(nuke)|ĭ|1&(halo)|Į|1&(turkey)|į|1&(gift)|İ|1&(cake)|ı|1&(heartarrow)|Ĳ|1&(lucky)|ĳ|1&(shamrock)|Ĵ|1&(grin)|ĵ|1&(ill)|Ķ|1&"));
		ENetPacket* packet2ssw = enet_packet_create(p2ssw.data, p2ssw.len, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet2ssw);
		delete p2ssw.data;
	}
	void craftItemText() {
		int current = -1;
		std::ifstream infile("effect.txt");
		for (std::string line; getline(infile, line);) {
			if (line.length() > 5 && line[0] != '/' && line[1] != '/') {
				vector<string> ex = explode("|", line);
				ItemDefinition def;
				itemDefs.at(atoi(ex[0].c_str())).effect = ex[3] + " `$(`o" + ex[1] + " `omod removed)";
				itemDefs.at(atoi(ex[0].c_str())).effects = ex[2] + " `$(`o" + ex[1] + " `omod added)";
			}
		}
	}
	void getAutoEffect(ENetPeer* peer) {
		PlayerInfo* info = ((PlayerInfo*)(peer->data));
		if (info->cloth_hand == 5480) {
			info->peffect = 8421456;
			craftItemText();
		}
		else if (info->cloth_necklace == 11232) {
			info->peffect = 8421376 + 224;
			craftItemText();
		}
		else if (info->cloth_hand == 11440) {
			info->peffect = 8421376 + 111;
			craftItemText();
		}
		else if (info->cloth_hand == 5276) {
			info->peffect = 8421376 + 47;
			craftItemText();
		}
		else if (info->cloth_hair == 4746) {
			info->peffect = 8421376 + 75;
			craftItemText();
		}
		else if (info->cloth_hair == 4748) {
			info->peffect = 8421376 + 75;
			craftItemText();
		}
		else if (info->cloth_hair == 4750) {
			info->peffect = 8421376 + 75;
			craftItemText();
		}
		else if (info->cloth_hand == 10652) {
			info->peffect = 8421376 + 188;
			craftItemText();
		}
		else if (info->cloth_hand == 9716) {
			info->peffect = 8421529;
			craftItemText();
		}
		else if (info->cloth_shirt == 1780) {
			info->peffect = 8421396;
			craftItemText();
		}
		else if (info->cloth_shirt == 10426) {
			info->peffect = 8421559;
			craftItemText();
		}
		else if (info->cloth_face == 1204) {
			info->peffect = 8421386;
			craftItemText();
		}
		else if (info->cloth_face == 10128) {
			info->peffect = 8421376 + 683;
			craftItemText();
		}
		else if (info->cloth_feet == 10618) {
			info->peffect = 8421376 + 699;
			craftItemText();
		}
		else if (info->cloth_face == 138) {
			info->peffect = 8421377;
			craftItemText();
		}
		else if (info->cloth_face == 2476) {
			info->peffect = 8421415;
			craftItemText();
		}
		else if (info->cloth_hand == 366 || info->cloth_hand == 1464) {
			info->peffect = 8421378;
			craftItemText();
		}
		else if (info->cloth_hand == 472) {
			info->peffect = 8421379;
			craftItemText();
		}
		else if (info->cloth_hand == 7912) {
			info->peffect = 8421487;
			craftItemText();
		}
		else if (info->cloth_hand == 594) {
			info->peffect = 8421380;
			craftItemText();
		}
		else if (info->cloth_hand == 768) {
			info->peffect = 8421381;
			craftItemText();
		}
		else if (info->cloth_hand == 900) {
			info->peffect = 8421382;
			craftItemText();
		}
		else if (info->cloth_hand == 910) {
			info->peffect = 8421383;
			craftItemText();
		}
		else if (info->cloth_hand == 930) {
			info->peffect = 8421384;
			craftItemText();
		}
		else if (info->cloth_hand == 1016) {
			info->peffect = 8421385;
			craftItemText();
		}
		else if (info->cloth_hand == 1378) {
			info->peffect = 8421387;
			craftItemText();
		}
		else if (info->cloth_hand == 1484) {
			info->peffect = 8421389;
			craftItemText();
		}
		else if (info->cloth_hand == 1512) {
			info->peffect = 8421390;
			craftItemText();
		}
		else if (info->cloth_hand == 1542) {
			info->peffect = 8421391;
			craftItemText();
		}
		else if (info->cloth_hand == 1576) {
			info->peffect = 8421392;
			craftItemText();
		}
		else if (info->cloth_hand == 1676) {
			info->peffect = 8421393;
			craftItemText();
		}
		else if (info->cloth_hand == 1710) {
			info->peffect = 8421394;
			craftItemText();
		}
		else if (info->cloth_hand == 1748) {
			info->peffect = 8421395;
			craftItemText();
		}
		else if (info->cloth_hand == 1782) {
			info->peffect = 8421397;
			craftItemText();
		}
		else if (info->cloth_hand == 1804) {
			info->peffect = 8421398;
			craftItemText();
		}
		else if (info->cloth_hand == 1868) {
			info->peffect = 8421399;
			craftItemText();
		}
		else if (info->cloth_hand == 1874) {
			info->peffect = 8421400;
			craftItemText();
		}
		else if (info->cloth_hand == 1946) {
			info->peffect = 8421401;
			craftItemText();
		}
		else if (info->cloth_hand == 1948) {
			info->peffect = 8421402;
			craftItemText();
		}
		else if (info->cloth_hand == 1956) {
			info->peffect = 8421403;
			craftItemText();
		}
		else if (info->cloth_hand == 2908) {
			info->peffect = 8421405;
			craftItemText();
		}
		else if (info->cloth_hand == 2952) {
			info->peffect = 8421405;
			craftItemText();
		}
		else if (info->cloth_hand == 6312) {
			info->peffect = 8421405;
			craftItemText();
		}
		else if (info->cloth_hand == 1980) {
			info->peffect = 8421406;
			craftItemText();
		}
		else if (info->cloth_hand == 2066) {
			info->peffect = 8421407;
			craftItemText();
		}
		else if (info->cloth_hand == 2212) {
			info->peffect = 8421408;
			craftItemText();
		}
		else if (info->cloth_hand == 2218) {
			info->peffect = 8421409;
			craftItemText();
		}
		else if (info->cloth_feet == 2220) {
			info->peffect = 8421410;
			craftItemText();
		}
		else if (info->cloth_hand == 2266) {
			info->peffect = 8421411;
			craftItemText();
		}
		else if (info->cloth_hand == 2386) {
			info->peffect = 8421412;
			craftItemText();
		}
		else if (info->cloth_hand == 2388) {
			info->peffect = 8421413;
			craftItemText();
		}
		else if (info->cloth_hand == 2450) {
			info->peffect = 8421414;
			craftItemText();
		}
		else if (info->cloth_hand == 2512) {
			info->peffect = 8421417;
			craftItemText();
		}
		else if (info->cloth_hand == 2572) {
			info->peffect = 8421418;
			craftItemText();
		}
		else if (info->cloth_hand == 2592) {
			info->peffect = 8421419;
			craftItemText();
		}
		else if (info->cloth_hand == 2720) {
			info->peffect = 8421420;
			craftItemText();
		}
		else if (info->cloth_hand == 2752) {
			info->peffect = 8421421;
			craftItemText();
		}
		else if (info->cloth_hand == 2754) {
			info->peffect = 8421422;
			craftItemText();
		}
		else if (info->cloth_hand == 2756) {
			info->peffect = 8421423;
			craftItemText();
		}
		else if (info->cloth_hand == 2802) {
			info->peffect = 8421425;
			craftItemText();
		}
		else if (info->cloth_hand == 2866) {
			info->peffect = 8421426;
			craftItemText();
		}
		else if (info->cloth_hand == 2876) {
			info->peffect = 8421427;
			craftItemText();
		}
		else if (info->cloth_hand == 2886) {
			info->peffect = 8421430;
			craftItemText();
		}
		else if (info->cloth_hand == 2890) {
			info->peffect = 8421431;
			craftItemText();
		}
		else if (info->cloth_hand == 3066) {
			info->peffect = 8421433;
			craftItemText();
		}
		else if (info->cloth_hand == 3124) {
			info->peffect = 8421434;
			craftItemText();
		}
		else if (info->cloth_hand == 3168) {
			info->peffect = 8421435;
			craftItemText();
		}
		else if (info->cloth_hand == 3214) {
			info->peffect = 8421436;
			craftItemText();
		}
		else if (info->cloth_hand == 3300) {
			info->peffect = 8421440;
			craftItemText();
		}
		else if (info->cloth_hand == 3418) {
			info->peffect = 8421441;
			craftItemText();
		}
		else if (info->cloth_hand == 3476) {
			info->peffect = 8421442;
			craftItemText();
		}
		else if (info->cloth_hand == 3686) {
			info->peffect = 8421444;
			craftItemText();
		}
		else if (info->cloth_hand == 3716) {
			info->peffect = 8421445;
			craftItemText();
		}
		else if (info->cloth_hand == 4290) {
			info->peffect = 8421447;
			craftItemText();
		}
		else if (info->cloth_hand == 4474) {
			info->peffect = 8421448;
			craftItemText();
		}
		else if (info->cloth_hand == 4464) {
			info->peffect = 8421449;
			craftItemText();
		}
		else if (info->cloth_hand == 1576) {
			info->peffect = 8421450;
			craftItemText();
		}
		else if (info->cloth_hand == 4778 || info->cloth_hand == 6026) {
			info->peffect = 8421452;
			craftItemText();
		}
		else if (info->cloth_hand == 4996) {
			info->peffect = 8421453;
			craftItemText();
		}
		else if (info->cloth_hand == 4840) {
			info->peffect = 8421454;
			craftItemText();
		}
		else if (info->cloth_hand == 5480) {
			info->peffect = 8421456;
			craftItemText();
		}
		else if (info->cloth_hand == 6110) {
			info->peffect = 8421457;
			craftItemText();
		}
		else if (info->cloth_hand == 6308) {
			info->peffect = 8421458;
			craftItemText();
		}
		else if (info->cloth_hand == 6310) {
			info->peffect = 8421459;
			craftItemText();
		}
		else if (info->cloth_hand == 6298) {
			info->peffect = 8421460;
			craftItemText();
		}
		else if (info->cloth_hand == 6756) {
			info->peffect = 8421461;
			craftItemText();
		}
		else if (info->cloth_hand == 7044) {
			info->peffect = 8421462;
			craftItemText();
		}
		else if (info->cloth_shirt == 6892) {
			info->peffect = 8421463;
			craftItemText();
		}
		else if (info->cloth_hand == 7088) {
			info->peffect = 8421465;
			craftItemText();
		}
		else if (info->cloth_hand == 7098) {
			info->peffect = 8421466;
			craftItemText();
		}
		else if (info->cloth_shirt == 7192) {
			info->peffect = 8421467;
			craftItemText();
		}
		else if (info->cloth_shirt == 7136) {
			info->peffect = 8421468;
			craftItemText();
		}
		else if (info->cloth_mask == 7216) {
			info->peffect = 8421470;
			craftItemText();
		}
		else if (info->cloth_back == 7196) {
			info->peffect = 8421471;
			craftItemText();
		}
		else if (info->cloth_back == 7392) {
			info->peffect = 8421472;
			craftItemText();
		}
		else if (info->cloth_hand == 7488) {
			info->peffect = 8421479;
			craftItemText();
		}
		else if (info->cloth_hand == 7586) {
			info->peffect = 8421480;
			craftItemText();
		}
		else if (info->cloth_hand == 7650) {
			info->peffect = 8421481;
			craftItemText();
		}
		else if (info->cloth_feet == 7950) {
			info->peffect = 8421489;
			craftItemText();
		}
		else if (info->cloth_hand == 8036) {
			info->peffect = 8421494;
			craftItemText();
		}
		else if (info->cloth_hand == 8910) {
			info->peffect = 8421505;
			craftItemText();
		}
		else if (info->cloth_hand == 8942) {
			info->peffect = 8421506;
			craftItemText();
		}
		else if (info->cloth_hand == 8948) {
			info->peffect = 8421507;
			craftItemText();
		}
		else if (info->cloth_hand == 8946) {
			info->peffect = 8421509;
			craftItemText();
		}
		else if (info->cloth_back == 9006) {
			info->peffect = 8421511;
			craftItemText();
		}
		else if (info->cloth_hand == 9116 || info->cloth_hand == 9118 || info->cloth_hand == 9120 || info->cloth_hand == 9122) {
			info->peffect = 8421376 + 111;
			craftItemText();
		}
		else {
			info->peffect = 8421376;
			craftItemText();
		}
	}

	void sendPData(ENetPeer* peer, PlayerMoving* data)
	{
		ENetPeer * currentPeer;

		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (peer != currentPeer)
			{
				if (isHere(peer, currentPeer))
				{
					data->netID = ((PlayerInfo*)(peer->data))->netID;

					SendPacketRaw(4, packPlayerMoving(data), 56, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);
				}
			}
		}
	}

	int getPlayersCountInWorld(string name)
	{
		int count = 0;
		ENetPeer * currentPeer;
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (((PlayerInfo*)(currentPeer->data))->currentWorld == name)
				count++;
		}
		return count;
	}

	void sendRoulete(ENetPeer* peer, int x, int y)
	{
		ENetPeer* currentPeer;
		int val = rand() % 37;
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (isHere(peer, currentPeer))
			{
					gamepacket_t p(2500);
					p.Insert("OnTalkBubble");
					p.Insert(((PlayerInfo*)(peer->data))->netID);
					p.Insert("`w[" + ((PlayerInfo*)(peer->data))->displayName + " `wspun the wheel and got `4" + std::to_string(val) + "`w!]");
					p.Insert(0);
					p.CreatePacket(currentPeer);
				}
			}
		}

	void sendPuncheffect(ENetPeer* peer) {
		PlayerInfo* info = ((PlayerInfo*)(peer->data));
		int netID = info->netID;
		int state = getState(info);
		int pro = getState(info);
		int statey = 0;
		if (info->cloth_hand == 6028) statey = 1024;
		if (info->cloth_hand == 6262) statey = 8192;
		if (info->haveGrowId == false) statey = 50000;
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
			if (isHere(peer, currentPeer)) {
				PlayerMoving data;
				data.packetType = 0x14;
				data.characterState = statey;
				data.x = 1000;
				data.y = 100;
				data.punchX = 0;
				data.punchY = 0;
				data.XSpeed = 300;
				data.YSpeed = 600;
				data.netID = netID;
				data.plantingTree = state;
				BYTE* raw = packPlayerMoving(&data);
				int var = info->peffect;
				memcpy(raw + 1, &var, 3);
				SendPacketRaw(4, raw, 56, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);
			}
		}
	}
	void sendPuncheffectpeer(ENetPeer* peer, int punch) {
		PlayerInfo* info = ((PlayerInfo*)(peer->data));
		int netID = info->netID;
		int state = getState(info);
		PlayerMoving data;
		float water = 125.0f;
		data.packetType = 0x14;
		data.characterState = ((PlayerInfo*)(peer->data))->characterState; // animation
		data.x = 1000;
		if (((PlayerInfo*)(peer->data))->cloth_hand == 366) {
			data.y = -400;
		}
		else {
			data.y = 400;
		}
		data.punchX = -1;
		data.punchY = -1;
		data.XSpeed = 300;
		if (((PlayerInfo*)(peer->data))->cloth_back == 9472) {
			data.YSpeed = 600;
		}
		else {
			data.YSpeed = 1150;
		}
		data.netID = netID;
		data.plantingTree = state;
		BYTE* raw = packPlayerMoving(&data);
		int var = punch;
		memcpy(raw + 1, &var, 3);
		memcpy(raw + 16, &water, 4);
		SendPacketRaw(4, raw, 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
	}

	void sendNotification(ENetPeer* peer, string song, string flag, string message) {
		GamePacket p = packetEnd(appendInt(appendString(appendString(appendString(appendString(createPacket(), "OnAddNotification"), song), message), flag), 0));
		ENetPacket* packet = enet_packet_create(p.data,
			p.len,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
		delete p.data;
	}

	void sendNothingHappened(ENetPeer* peer, int x, int y) {
		PlayerMoving data;
		data.netID = ((PlayerInfo*)(peer->data))->netID;
		data.packetType = 0x8;
		data.plantingTree = 0;
		data.netID = -1;
		data.x = x;
		data.y = y;
		data.punchX = x;
		data.punchY = y;
		SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
	}

void loadnews() {
	std::ifstream ifs("news.txt");
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	string target = "\r";
	string news = "";
	int found = -1;
	do {
		found = content.find(target, found + 1);
		if (found != -1) {
			news = content.substr(0, found) + content.substr(found + target.length());
		}
		else {
			news = content;
		}
	} while (found != -1);
	if(news != "") {
		newslist = news;
	}
}
string packPlayerMoving2(PlayerMoving* dataStruct)
{
	string data;
	data.resize(56);
	STRINT(data, 0) = dataStruct->packetType;
	STRINT(data, 4) = dataStruct->netID;
	STRINT(data, 12) = dataStruct->characterState;
	STRINT(data, 20) = dataStruct->plantingTree;
	STRINT(data, 24) = *(int*)&dataStruct->x;
	STRINT(data, 28) = *(int*)&dataStruct->y;
	STRINT(data, 32) = *(int*)&dataStruct->XSpeed;
	STRINT(data, 36) = *(int*)&dataStruct->YSpeed;
	STRINT(data, 44) = dataStruct->punchX;
	STRINT(data, 48) = dataStruct->punchY;
	return data;
}
void sendTileData(ENetPeer* peer, int x, int y, int visual, uint16_t fgblock, uint16_t bgblock, string tiledata) {
	PlayerMoving pmov;
	pmov.packetType = 5;
	pmov.characterState = 0;
	pmov.x = 0;
	pmov.y = 0;
	pmov.XSpeed = 0;
	pmov.YSpeed = 0;
	pmov.plantingTree = 0;
	pmov.punchX = x;
	pmov.punchY = y;
	pmov.netID = 0;

	string packetstr;
	packetstr.resize(4);
	packetstr[0] = 4;
	packetstr += packPlayerMoving2(&pmov);
	packetstr[16] = 8;
	packetstr.resize(packetstr.size() + 4);
	STRINT(packetstr, 52 + 4) = tiledata.size() + 4;
	STR16(packetstr, 56 + 4) = fgblock;
	STR16(packetstr, 58 + 4) = bgblock;
	packetstr += tiledata;

	ENetPacket* packet = enet_packet_create(&packetstr[0],
		packetstr.length(),
		ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, packet);
}
string lockTileDatas(int visual, uint32_t owner, uint32_t adminLength, uint32_t* admins, bool isPublic = false, uint8_t bpm = 0) {
	string data;
	data.resize(4 + 2 + 4 + 4 + adminLength * 4 + 8);
	if (bpm) data.resize(data.length() + 4);
	data[2] = 0x01;
	if (isPublic) data[2] |= 0x80;
	data[4] = 3;
	data[5] = visual; // or 0x02
	STRINT(data, 6) = owner;
	//data[14] = 1;
	STRINT(data, 10) = adminLength;
	for (uint32_t i = 0; i < adminLength; i++) {
		STRINT(data, 14 + i * 4) = admins[i];
	}

	if (bpm) {
		STRINT(data, 10)++;
		STRINT(data, 14 + adminLength * 4) = -bpm;
	}
	return data;
}

uint8_t* lockTileData(uint32_t owner, uint32_t adminLength, uint32_t* admins) {
	uint8_t* data = new uint8_t[4 + 2 + 4 + 4 + adminLength * 4 + 8];
	memset(data, 0, 4 + 2 + 4 + 4 + adminLength * 4 + 8);
	data[2] = 0x1;
	data[4] = 3;
	*(uint32_t*)(data + 6) = owner;

	*(uint32_t*)(data + 10) = adminLength;
	for (uint32_t i = 0; i < adminLength; i++) {
		*(uint32_t*)(data + 14 + i * 4) = admins[i];
	}
	return data;
}

void sendTileUpdate(int x, int y, int tile, int causedBy, ENetPeer* peer)
{
	if (tile > itemDefs.size()) {
		return;
	}
	bool isLock = false;
	PlayerMoving data;
	//data.packetType = 0x14;
	data.packetType = 0x3;

	//data.characterState = 0x924; // animation
	data.characterState = 0x0; // animation
	data.x = x;
	data.y = y;
	data.punchX = x;
	data.punchY = y;
	data.XSpeed = 0;
	data.YSpeed = 0;
	data.netID = causedBy;
	data.plantingTree = tile;

	WorldInfo* world = getPlyersWorld(peer);

	string gay = world->items[x + (y * world->width)].text;
	int gay2 = world->items[x + (y * world->width)].foreground;
	if (getItemDef(world->items[x + (y * world->width)].foreground).blockType == BlockTypes::SIGN || world->items[x + (y * world->width)].foreground == 1420 || world->items[x + (y * world->width)].foreground == 6214)
	{
		if (world->owner != "") {
			if (((PlayerInfo*)(peer->data))->rawName == world->owner) {
				if (tile == 32) {
					((PlayerInfo*)(peer->data))->wrenchx = x;
					((PlayerInfo*)(peer->data))->wrenchy = y;
					GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wEdit " + getItemDef(world->items[x + (y * world->width)].foreground).name + "``|left|" + to_string(gay2) + "|\n\nadd_textbox|`oWhat would you like to write on this sign?|\nadd_text_input|ch3||" + gay + "|100|\nembed_data|tilex|" + std::to_string(((PlayerInfo*)(peer->data))->wrenchx) + "\nembed_data|tiley|" + std::to_string(((PlayerInfo*)(peer->data))->wrenchy) + "\nend_dialog|sign_edit|Cancel|OK|"));
					ENetPacket* packet = enet_packet_create(p.data,
						p.len,
						ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet);
					delete p.data;
				}
			}
		}
	}
	if (getItemDef(world->items[x + (y * world->width)].foreground).blockType == BlockTypes::GATEWAY) {
		if (tile == 32) {
			if (((PlayerInfo*)(peer->data))->rawName == world->owner) {
				((PlayerInfo*)(peer->data))->wrenchx = x;
				((PlayerInfo*)(peer->data))->wrenchy = y;
				int id = getItemDef(world->items[x + (y * world->width)].foreground).id;
				if (world->items[x + (y * world->width)].isOpened == false) {
					packet::dialog(peer, "add_label_with_icon|big|Edit " + getItemDef(world->items[x + (y * world->width)].foreground).name + "|left|" + to_string(id) + "|\nadd_checkbox|opentopublic|`ois Open to Public?|0|\nend_dialog|entrance|Cancel|OK|");
				}
				else {
					packet::dialog(peer, "add_label_with_icon|big|Edit " + getItemDef(world->items[x + (y * world->width)].foreground).name + "|left|" + to_string(id) + "|\nadd_checkbox|opentopublic|`ois Open to Public?|1|\nend_dialog|entrance|Cancel|OK|");
				}
			}
		}
	}
	if (getItemDef(tile).blockType == BlockTypes::CONSUMABLE) {
		if (tile == 11398) {
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
				if (isHere(peer, currentPeer)) {
					if (x == ((PlayerInfo*)(peer->data))->x / 32 && y == ((PlayerInfo*)(peer->data))->y / 32) {
						RemoveInventoryItem(11398, 1, peer);
						vector<int> alien_pod{ 10990, 11000, 11410, 11426, 10952, 10956, 10954, 10958, 10960, 10996, 11408, 11412, 11414, 10998, 11422, 10994 };
						int rand_item = alien_pod[rand() % alien_pod.size()];
						int count = 1;
						if (rand_item == 10990) count = 1;
						if (rand_item == 11000) count = 1;
						if (rand_item == 11410) count = 1;
						if (rand_item == 11426) count = 1;
						if (rand_item == 10952) count = 1;
						if (rand_item == 10956) count = 1;
						if (rand_item == 10954) count = 1;
						if (rand_item == 10958) count = 1;
						if (rand_item == 10960) count = 1;
						if (rand_item == 10958) count = 1;
						if (rand_item == 10996) count = 1;
						if (rand_item == 11408) count = 1;
						if (rand_item == 11412) count = 1;
						if (rand_item == 11414) count = 1;
						if (rand_item == 10998) count = 1;
						if (rand_item == 11422) count = 1;
						if (rand_item == 10994) count = 1;
						if (rand_item == 10960 || rand_item == 10956 || rand_item == 10958 || rand_item == 10954) {
							int target = 5;
							if (tile == 9286) target = 10;
							if ((rand() % 1000) <= target) {}
							else rand_item = 11422;
						}
						packet::SendTalkSelf(peer, "You received `2" + to_string(count) + " " + getItemDef(rand_item).name + "`` from the Alien Landing Pod.");
						packet::consolemessage(peer, "You received `2" + to_string(count) + " " + getItemDef(rand_item).name + "`` from the Alien Landing Pod.");
						bool success = true;
						SaveShopsItemMoreTimes(rand_item, count, peer, success);
						return;
					}
					else if (x == ((PlayerInfo*)(currentPeer->data))->x / 32 && y == ((PlayerInfo*)(currentPeer->data))->y / 32) {
						packet::SendTalkSelf(peer, "You can only use that on yourself.");
						return;
					}
					else {
						packet::SendTalkSelf(peer, "Must be used on a person.");
						return;
					}
				}
			}
			if (tile == 10456) {
				if (x == ((PlayerInfo*)(peer->data))->x / 32 && y == ((PlayerInfo*)(peer->data))->y / 32) {
					if (((PlayerInfo*)(peer->data))->cloth_back != 10456) {
						((PlayerInfo*)(peer->data))->cloth_back = 10456;
						((PlayerInfo*)(peer->data))->peffect = 8421559;
						sendPuncheffectpeer(peer, ((PlayerInfo*)(peer->data))->peffect);
						sendClothes(peer);
					}
					else {
						((PlayerInfo*)(peer->data))->cloth_back = 10426;
						((PlayerInfo*)(peer->data))->peffect = 8421559;
						sendPuncheffectpeer(peer, ((PlayerInfo*)(peer->data))->peffect);
						sendClothes(peer);
					}
				}
				else {
					packet::SendTalkSelf(peer, "Must be used on a person.");
				}
			}
		}
		return;
	}

	if (world == NULL) return;
	if (x<0 || y<0 || x>world->width - 1 || y>world->height - 1 || tile > itemDefs.size()) return; // needs - 1
	sendNothingHappened(peer, x, y);
	if (((PlayerInfo*)(peer->data))->adminLevel < 665)
	{
		if (world->items[x + (y * world->width)].foreground == 6 || world->items[x + (y * world->width)].foreground == 8 || world->items[x + (y * world->width)].foreground == 3760)
			return;
		if (tile == 6 || tile == 8 || tile == 3760 || tile == 6864)
			return;
	}
	if (world->name == "ADMIN" && !getAdminLevel(((PlayerInfo*)(peer->data))->rawName, ((PlayerInfo*)(peer->data))->tankIDPass))
	{
		if (world->items[x + (y * world->width)].foreground == 758)
			sendRoulete(peer, x, y);
		return;
	}
	if (world->name != "ADMIN") {
		if (world->owner != "") {

			string name = ((PlayerInfo*)(peer->data))->rawName;
			if (((PlayerInfo*)(peer->data))->rawName == world->owner || (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end() || ((PlayerInfo*)(peer->data))->adminLevel == 1337)) {
				if (((PlayerInfo*)(peer->data))->rawName == "") return;
				// WE ARE GOOD TO GO

				if (world->items[x + (y * world->width)].foreground == 242 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 1796 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 4428 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 2408 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 7188 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 5980 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 2950 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()) || world->items[x + (y * world->width)].foreground == 5638 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end()))
				{
					return;
				}

				if (tile == 32 && (find(world->acclist.begin(), world->acclist.end(), name) != world->acclist.end())) {
					return;
				}
				string offlinelist = "";
				string offname = "";
				int ischecked;
				int ischecked1;
				int ischecked2;
				for (std::vector<string>::const_iterator i = world->acclist.begin(); i != world->acclist.end(); ++i) {
					offname = *i;
					offlinelist += "\nadd_checkbox|isAccessed|" + offname + "|0|\n";

				}


				if (world->isPublic == true) {
					ischecked = 1;
				}
				else {
					ischecked = 0;
				}
				int noclap = 0;
				bool casin = world->isCasino;
				if (casin == true) {
					noclap = 1;
				}
				else {
					noclap = 0;
				}
				if (tile == 32) {
					if (world->items[x + (y * world->width)].foreground == 242 || world->items[x + (y * world->width)].foreground == 5814 || world->items[x + (y * world->width)].foreground == 2408 || world->items[x + (y * world->width)].foreground == 1796 || world->items[x + (y * world->width)].foreground == 4428 || world->items[x + (y * world->width)].foreground == 7188) {

						GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wEdit " + getItemDef(world->items[x + (y * world->width)].foreground).name + "``|left|" + to_string(world->items[x + (y * world->width)].foreground) + "|\nadd_textbox|`wAccess list:|left|\nadd_spacer|small|" + offlinelist + "add_spacer|small|\nadd_player_picker|netid|`wAdd|\nadd_spacer|small|\nadd_checkbox|isWorldPublic|Allow anyone to build|" + std::to_string(ischecked) + "|\nadd_checkbox|allowNoclip|Disable noclip|" + std::to_string(noclap) + "|\nend_dialog|wlmenu|Cancel|OK|"));
						ENetPacket* packet = enet_packet_create(p.data,
							p.len,
							ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packet);
						delete p.data;
					}
				}
			}

			else if (find(world->acclist.begin(), world->acclist.end(), ((PlayerInfo*)(peer->data))->rawName) != world->acclist.end())
			{
				if (world->items[x + (y * world->width)].foreground == 242 || world->items[x + (y * world->width)].foreground == 5814 || world->items[x + (y * world->width)].foreground == 2408 || world->items[x + (y * world->width)].foreground == 1796 || world->items[x + (y * world->width)].foreground == 4428 || world->items[x + (y * world->width)].foreground == 7188 || world->items[x + (y * world->width)].foreground == 4802 || world->items[x + (y * world->width)].foreground == 10410)
				{


					string ownername = world->owner;
					GamePacket p2 = packetEnd(appendIntx(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`0" + ownername + "'s `$World Lock`0. (`2Access Granted`w)"), 0), 1));


					ENetPacket* packet2 = enet_packet_create(p2.data,
						p2.len,
						ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet2);
					delete p2.data;
					string text = "action|play_sfx\nfile|audio/punch_locked.wav\ndelayMS|0\n";
					BYTE* data = new BYTE[5 + text.length()];
					BYTE zero = 0;
					int type = 3;
					memcpy(data, &type, 4);
					memcpy(data + 4, text.c_str(), text.length()); // change memcpy here
					memcpy(data + 4 + text.length(), &zero, 1); // change memcpy here, revert to 4

					ENetPacket* packetsou = enet_packet_create(data,
						5 + text.length(),
						ENET_PACKET_FLAG_RELIABLE);

					enet_peer_send(peer, 0, packetsou);


					return;
				}

			}
			else if (world->isPublic)
			{
				if (world->items[x + (y * world->width)].foreground == 242 || world->items[x + (y * world->width)].foreground == 5814 || world->items[x + (y * world->width)].foreground == 2408 || world->items[x + (y * world->width)].foreground == 1796 || world->items[x + (y * world->width)].foreground == 4428 || world->items[x + (y * world->width)].foreground == 7188)
				{
					string ownername = world->owner;
					GamePacket p2 = packetEnd(appendIntx(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`0" + ownername + "'s `$World Lock`0. (`2Access Granted`w)"), 0), 1));


					ENetPacket* packet2 = enet_packet_create(p2.data,
						p2.len,
						ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet2);
					delete p2.data;
					string text = "action|play_sfx\nfile|audio/punch_locked.wav\ndelayMS|0\n";
					BYTE* data = new BYTE[5 + text.length()];
					BYTE zero = 0;
					int type = 3;
					memcpy(data, &type, 4);
					memcpy(data + 4, text.c_str(), text.length()); // change memcpy here
					memcpy(data + 4 + text.length(), &zero, 1); // change memcpy here, revert to 4

					ENetPacket* packetsou = enet_packet_create(data,
						5 + text.length(),
						ENET_PACKET_FLAG_RELIABLE);

					enet_peer_send(peer, 0, packetsou);


					return;
				}

			}
			else {
				ItemDefinition pro;
				pro = getItemDef(world->items[x + (y * world->width)].foreground);
				if (world->items[x + (y * world->width)].foreground == 242 || world->items[x + (y * world->width)].foreground == 5814 || world->items[x + (y * world->width)].foreground == 2408 || world->items[x + (y * world->width)].foreground == 1796 || world->items[x + (y * world->width)].foreground == 4428 || world->items[x + (y * world->width)].foreground == 7188) {
					string ownername = world->owner;
					GamePacket p2 = packetEnd(appendIntx(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`0" + ownername + "'s `$World Lock`0. (`4No access`w)"), 0), 1));


					ENetPacket* packet2 = enet_packet_create(p2.data,
						p2.len,
						ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet2);
					delete p2.data;
					string text = "action|play_sfx\nfile|audio/punch_locked.wav\ndelayMS|0\n";
					BYTE* data = new BYTE[5 + text.length()];
					BYTE zero = 0;
					int type = 3;
					memcpy(data, &type, 4);
					memcpy(data + 4, text.c_str(), text.length()); // change memcpy here
					memcpy(data + 4 + text.length(), &zero, 1); // change memcpy here, revert to 4

					ENetPacket* packetsou = enet_packet_create(data,
						5 + text.length(),
						ENET_PACKET_FLAG_RELIABLE);

					enet_peer_send(peer, 0, packetsou);


					return;
				}
				else
				{
					string text = "action|play_sfx\nfile|audio/punch_locked.wav\ndelayMS|0\n";
					BYTE* data = new BYTE[5 + text.length()];
					BYTE zero = 0;
					int type = 3;
					memcpy(data, &type, 4);
					memcpy(data + 4, text.c_str(), text.length()); // change memcpy here
					memcpy(data + 4 + text.length(), &zero, 1); // change memcpy here, revert to 4

					ENetPacket* packetsou = enet_packet_create(data,
						5 + text.length(),
						ENET_PACKET_FLAG_RELIABLE);

					enet_peer_send(peer, 0, packetsou);


					return;
				}

			} /*lockeds*/
			if (tile == 242 || tile == 2408 || tile == 1796 || tile == 4428 || tile == 7188) {



				GamePacket p3 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`0Only one `$World Lock`0 can be placed in a world!"), 0));


				ENetPacket* packet3 = enet_packet_create(p3.data,
					p3.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet3);
				delete p3.data;
				return;
			}
		}
	}

	if (world->items[x + (y * world->width)].foreground == 5958) {
		if (tile == 32) {
			packet::dialog(peer, "add_label_with_icon|big|`wEpoch Machine|left|5958|\nadd_spacer|small|\nadd_textbox|`oSelect Your Doom:|\nadd_spacer|small|\nadd_checkbox|epochice|Ice Age|" + to_string(world->ice) + "|\nadd_checkbox|epochvol|Volcano|" + to_string(world->volcano) + "|\nadd_checkbox|epochland|Floating Islands|" + to_string(world->land) + "|\nadd_text_input|timedilation|Cycle time (minutes): |0|3|\nend_dialog|epochweather|Cancel|Okay|");
		}
	}
	if (world->items[x + (y * world->width)].foreground == 5818) {
		if (tile == 32) {
			string leader = ((PlayerInfo*)(peer->data))->guildLeader;
			string rawname = ((PlayerInfo*)(peer->data))->rawName;
			((PlayerInfo*)(peer->data))->wrenchx = x;
			((PlayerInfo*)(peer->data))->wrenchy = y;
			int id = getItemDef(world->items[x + (y * world->width)].foreground).id;
			if (rawname == leader || isWorldOwner(peer, world) || isWorldAdmin(peer, world)) {
				if (world->items[x + (y * world->width)].isOpened == false) {
					packet::dialog(peer, "add_label_with_icon|big|Edit " + getItemDef(world->items[x + (y * world->width)].foreground).name + "|left|" + to_string(id) + "|\nadd_checkbox|gopentopublic|`ois Open to Public?|0|\nend_dialog|gentrance|Cancel|OK|");
				}
				else {
					packet::dialog(peer, "add_label_with_icon|big|Edit " + getItemDef(world->items[x + (y * world->width)].foreground).name + "|left|" + to_string(id) + "|\nadd_checkbox|gopentopublic|`ois Open to Public?|1|\nend_dialog|gentrance|Cancel|OK|");
				}
			}
		}
	}
	if (world->items[x + (y * world->width)].foreground == 5000) {
		if (tile == 32) {
			if (world->owner == "" || isWorldOwner(peer, world) || isWorldAdmin(peer, world) || getPlyersWorld(peer)->isPublic) {
				string fyg = "set_default_color|`o\nadd_label_with_icon|big|`wWeather Machine - Background``|left|5000|\nadd_spacer|small|\nadd_textbox|`oYou can scan any Background Block to set it up in your weather machine.|\nadd_item_picker|bg_pick|Item: `2" + getItemDef(world->bgID).name + "|Select any Background Block|\nadd_quick_exit|\nend_dialog|bg_weather|Cancel|Okay|";
				packet::dialog(peer, fyg);
				((PlayerInfo*)(peer->data))->wrenchedBlockLocation = x + (y * world->width);
			}
			else {
				packet::SendTalkSelf(peer, "That area is owned by " + world->owner + "");
				return;
			}
		}
	}
	if (tile == 32) {
		if (world->items[x + (y * world->width)].foreground == 3832) {
			if (tile == 32) {
				if (world->owner == "" || isWorldOwner(peer, world) || isWorldAdmin(peer, world) || getPlyersWorld(peer)->isPublic) {
					string gaekid = "set_default_color|`o\nadd_label_with_icon|big|`wWeather Machine - Stuff``|left|3832|\nadd_spacer|small|\nadd_item_picker|stuff_pick|Item: `2" + getItemDef(world->stuffID).name + "|Select any item to rain down|\nadd_text_input|stuff_gravity|Gravity:|" + to_string(world->stuff_gravity) + "|3|\nadd_checkbox|stuff_spin|Spin Items|" + to_string(world->stuff_spin) + "|\nadd_checkbox|stuff_invert|Invert Sky Colors|" + to_string(world->stuff_invert) + "|\nadd_textbox|`oSelect item and click okay to continue!|\nend_dialog|stuff_weather|Cancel|Okay|";
					packet::dialog(peer, gaekid);
					((PlayerInfo*)(peer->data))->wrenchedBlockLocation = x + (y * world->width);
				}
				else {
					packet::SendTalkSelf(peer, "That area is owned by " + world->owner + "");
					return;
				}
			}
		}
	}

	if (tile == 18) {
		if (world->owner == "" || isWorldOwner(peer, world) || isWorldAdmin(peer, world) || getPlyersWorld(peer)->isPublic) {
			if (world->items[x + (y * world->width)].foreground == 340) {
				world->items[x + (y * world->width)].foreground = 0;
				int valgems = 0;
				valgems = rand() % 1000;
				int valgem = rand() % 1000;
				valgem = valgems + 1;
				std::ifstream ifs("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
				std::string content((std::istreambuf_iterator<char>(ifs)),
					(std::istreambuf_iterator<char>()));


				int gembux = atoi(content.c_str());
				int fingembux = gembux + valgem;
				ofstream myfile;
				myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
				myfile << fingembux;
				myfile.close();

				int gemcalc = gembux + valgem;
				GamePacket pp = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), gemcalc));
				ENetPacket* packetpp = enet_packet_create(pp.data,
					pp.len,
					ENET_PACKET_FLAG_RELIABLE);

				enet_peer_send(peer, 0, packetpp);
				delete pp.data;
			}
			if (world->items[x + (y * world->width)].foreground == 1210) {
				if (world->weather == 8) {
					world->weather = 0;
				}
				else {
					world->weather = 8;
				}
				OnSetCurrentWeather(peer, world->weather);
			}
			if (world->items[x + (y * world->width)].foreground == 5958) {
				if (world->weather == 38) {
					world->weather = 0;
				}
				else {
					world->weather = 38;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 3832) {
				if (world->weather == 29) {
					world->weather = 0;
				}
				else {
					world->weather = 29;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 984) {
				if (world->weather == 5) {
					world->weather = 0;
				}
				else {
					world->weather = 5;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 946) {
				if (world->weather == 3) {
					world->weather = 0;
				}
				else {
					world->weather = 3;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 934) {
				if (world->weather == 2) {
					world->weather = 0;
				}
				else {
					world->weather = 2;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 3252) {
				if (world->weather == 20) {
					world->weather = 0;
				}
				else {
					world->weather = 20;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 7644) {
				if (world->weather == 44) {
					world->weather = 0;
				}
				else {
					world->weather = 44;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 6854) {
				if (world->weather == 42) {
					world->weather = 0;
				}
				else {
					world->weather = 42;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 5716) {
				if (world->weather == 37) {
					world->weather = 0;
				}
				else {
					world->weather = 37;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 5654) {
				if (world->weather == 36) {
					world->weather = 0;
				}
				else {
					world->weather = 36;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 8896) {
				if (world->weather == 47) {
					world->weather = 0;
				}
				else {
					world->weather = 47;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 8836) {
				if (world->weather == 48) {
					world->weather = 0;
				}
				else {
					world->weather = 48;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 5112) {
				if (world->weather == 35) {
					world->weather = 0;
				}
				else {
					world->weather = 35;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 5000) {
				if (world->weather == 34) {
					world->weather = 0;
				}
				else {
					world->weather = 34;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 4892) {
				if (world->weather == 33) {
					world->weather = 0;
				}
				else {
					world->weather = 33;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 4776) {
				if (world->weather == 32) {
					world->weather = 0;
				}
				else {
					world->weather = 32;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 4486) {
				if (world->weather == 31) {
					world->weather = 0;
				}
				else {
					world->weather = 31;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 4242) {
				if (world->weather == 30) {
					world->weather = 0;
				}
				else {
					world->weather = 30;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 1490) {
				if (world->weather == 10) {
					world->weather = 0;
				}
				else {
					world->weather = 10;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 3694) {
				if (world->weather == 28) {
					world->weather = 0;
				}
				else {
					world->weather = 28;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 3534) {
				if (world->weather == 22) {
					world->weather = 0;
				}
				else {
					world->weather = 22;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 3446) {
				if (world->weather == 21) {
					world->weather = 0;
				}
				else {
					world->weather = 21;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 2284) {
				if (world->weather == 18) {
					world->weather = 0;
				}
				else {
					world->weather = 18;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 2046) {
				if (world->weather == 17) {
					world->weather = 0;
				}
				else {
					world->weather = 17;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 1750) {
				if (world->weather == 15) {
					world->weather = 0;
				}
				else {
					world->weather = 15;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 1364) {
				if (world->weather == 11) {
					world->weather = 0;
				}
				else {
					world->weather = 11;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 932) {
				if (world->weather == 4) {
					world->weather = 0;
				}
				else {
					world->weather = 4;
				}
				OnSetCurrentWeather(peer, world->weather);
			}

			if (world->items[x + (y * world->width)].foreground == 10286) {
				if (world->weather == 51) {
					world->weather = 0;
				}
				else {
					world->weather = 51;
				}
				OnSetCurrentWeather(peer, world->weather);
			}
		}
	}

	if (tile == 822) {
		world->items[x + (y * world->width)].water = !world->items[x + (y * world->width)].water;
		return;
	}
	if (tile == 1136) {
		packet::dialog(peer, "set_default_color|`w\n\nadd_label_with_icon|big|`wMars Blast|left|1136|\nadd_textbox|`oThis item creates a new world! Enter a unique name for it.|left|\nadd_text_input|marstext|`oNew World Name||30|\nend_dialog|sendmb|Cancel|`5Create!|\n");
		return;
	}
	if (tile == 3062)
	{
		world->items[x + (y * world->width)].fire = !world->items[x + (y * world->width)].fire;
		return;
	}
	if (tile == 1866)
	{
		world->items[x + (y * world->width)].glue = !world->items[x + (y * world->width)].glue;
		return;
	}
	ItemDefinition def;
	try {
		def = getItemDef(tile);
		if (def.blockType == BlockTypes::CLOTHING) return;
	}
	catch (int e) {
		def.breakHits = 4;
		def.blockType = BlockTypes::UNKNOWN;
#ifdef TOTAL_LOG
		cout << "Ugh, unsupported item " << tile << endl;
#endif
	}

	if (tile == 18) {
		if (world->items[x + (y * world->width)].background == 6864 && world->items[x + (y * world->width)].foreground == 0) return;
		if (world->items[x + (y * world->width)].background == 0 && world->items[x + (y * world->width)].foreground == 0) return;
		//data.netID = -1;
		int tool = ((PlayerInfo*)(peer->data))->cloth_hand;
		data.packetType = 0x8;
		data.plantingTree = (tool == 98 || tool == 1438 || tool == 4956) ? 8 : 6;
		int block = world->items[x + (y * world->width)].foreground > 0 ? world->items[x + (y * world->width)].foreground : world->items[x + (y * world->width)].background;
		//if (world->items[x + (y*world->width)].foreground == 0) return;
		using namespace std::chrono;
		if ((duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() - world->items[x + (y * world->width)].breakTime >= 4000)
		{
			world->items[x + (y * world->width)].breakTime = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			world->items[x + (y * world->width)].breakLevel = 0; // TODO
			if (world->items[x + (y * world->width)].foreground == 758)
				sendRoulete(peer, x, y);
		}
		if (y < world->height)
		{
			world->items[x + (y * world->width)].breakTime = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			world->items[x + (y * world->width)].breakLevel += (int)((tool == 98 || tool == 1438 || tool == 4956) ? 8 : 6); // TODO
		}


		if (y < world->height && world->items[x + (y * world->width)].breakLevel >= getItemDef(block).breakHits * 6) { // TODO
			data.packetType = 0x3;// 0xC; // 0xF // World::HandlePacketTileChangeRequest
			data.plantingTree = 18;
			world->items[x + (y * world->width)].breakLevel = 0;
			if (world->items[x + (y * world->width)].foreground != 0)
			{
				if (world->items[x + (y * world->width)].foreground == 242)
				{
					world->owner = "";
					world->isPublic = false;
				}
				world->items[x + (y * world->width)].foreground = 0;
			}
			else {
				world->items[x + (y * world->width)].background = 6864;
			}

		}


	}
	else {
		for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
		{
			if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == tile)
			{
				if ((unsigned int)((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount > 1)
				{
					((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount--;
				}
				else {
					((PlayerInfo*)(peer->data))->inventory.items.erase(((PlayerInfo*)(peer->data))->inventory.items.begin() + i);

				}
			}
		}
		if (def.blockType == BlockTypes::BACKGROUND)
		{
			world->items[x + (y * world->width)].background = tile;
		}
		else {
			if (world->items[x + (y * world->width)].foreground != 0)return;
			world->items[x + (y * world->width)].foreground = tile;
			ItemDefinition pro;
			pro = getItemDef(tile);
			if (tile == 242 || tile == 2408 || tile == 1796 || tile == 4428 || tile == 7188 || tile == 10410) {
				if (((PlayerInfo*)(peer->data))->rawName == world->owner) return;
				world->owner = ((PlayerInfo*)(peer->data))->rawName;
				world->ownerID = ((PlayerInfo*)(peer->data))->netID;
				world->isPublic = false;
				isLock = true;
				ENetPeer* currentPeer;

				for (currentPeer = server->peers;
					currentPeer < &server->peers[server->peerCount];
					++currentPeer)
				{
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
						continue;
					if (isHere(peer, currentPeer)) {
						packet::consolemessage(peer, "`5[`w" + world->name + " `ohas been World Locked by " + ((PlayerInfo*)(peer->data))->displayName + "`5]");
						packet::SendTalkSelf(peer, "`5[`w" + world->name + " `ohas been World Locked by " + ((PlayerInfo*)(peer->data))->displayName + "`5]");
						packet::PlayAudio(peer, "audio/use_lock.wav", 0);
					}
				}
			}

		}

		world->items[x + (y * world->width)].breakLevel = 0;
	}

	ENetPeer* currentPeer;

	for (currentPeer = server->peers;
		currentPeer < &server->peers[server->peerCount];
		++currentPeer)
	{
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
			continue;
		if (isHere(peer, currentPeer))
			SendPacketRaw(4, packPlayerMoving(&data), 56, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);

		//cout << "Tile update at: " << data2->punchX << "x" << data2->punchY << endl;
	}
	string name = ((PlayerInfo*)(peer->data))->rawName;
	if (isLock) {
		ENetPeer* currentPeer;

		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (isHere(peer, currentPeer)) {
				sendTileData(currentPeer, x, y, 0x10, tile, world->items[x + (y * world->width)].background, lockTileDatas(0x20, ((PlayerInfo*)(peer->data))->userID, 0, 0, false, 100));
			}
		}
	}
}

		
void resetacc(ENetPeer* peer) {
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		string name2 = PlayerDB::getProperName(((PlayerInfo*)(currentPeer->data))->rawName);
		remove(("players/" + name2 + ".json").c_str());
		enet_peer_disconnect_later(currentPeer, 0);
	}
}
	void sendPlayerLeave(ENetPeer* peer, PlayerInfo* player)
	{
		ENetPeer * currentPeer;
		gamepacket_t p;
		p.Insert("OnRemove");
		p.Insert("netID|" + std::to_string(player->netID) + "\n");
		gamepacket_t p2;
		p2.Insert("OnConsoleMessage");
		p2.Insert("`5<`w" + player->displayName + "`` `5left, `w" + std::to_string(getPlayersCountInWorld(player->currentWorld) - 1) + "`` `5others here>``");
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (isHere(peer, currentPeer)) {
				{
					p.CreatePacket(peer);
					
					{
						p.CreatePacket(currentPeer);
					}
					
				}
				{
					p2.CreatePacket(currentPeer);
				}
			}
		}
	}

	static inline void ltrim(string &s)
	{
		s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
			return !isspace(ch);
		}));
	}

	static inline void rtrim(string &s)
	{
		s.erase(find_if(s.rbegin(), s.rend(), [](int ch) {
			return !isspace(ch);
		}).base(), s.end());
	}

	static inline void trim(string &s)
	{
		ltrim(s);
		rtrim(s);
	}

	static inline string trimString(string s)
	{
    	trim(s);
    	return s;
	}

	int countSpaces(string& str)
	{ 
		int count = 0; 
		int length = str.length(); 
		for (int i = 0; i < length; i++)
		{ 
			int c = str[i]; 
			if (isspace(c)) 
				count++; 
		}
		return count; 
	} 

	inline void sendWrongCmdLog(ENetPeer* peer)
	{
		packet::consolemessage(peer, "`4Unknown command. `oEnter `$/help `ofor a list of valid commands.");
	}
  
	void removeExtraSpaces(string &str) 
	{
		int n = str.length(); 
		int i = 0, j = -1; 
		bool spaceFound = false; 
		while (++j < n && str[j] == ' '); 
	
		while (j < n) 
		{ 
			if (str[j] != ' ') 
			{ 
				if ((str[j] == '.' || str[j] == ',' || 
					str[j] == '?') && i - 1 >= 0 && 
					str[i - 1] == ' ') 
					str[i - 1] = str[j++]; 
				else
					str[i++] = str[j++]; 

				spaceFound = false; 
			} 
 
			else if (str[j++] == ' ') 
			{
				if (!spaceFound) 
				{ 
					str[i++] = ' '; 
					spaceFound = true; 
				} 
			} 
		}
		if (i <= 1) 
        	str.erase(str.begin() + i, str.end()); 
    	else
        	str.erase(str.begin() + i, str.end()); 
	} 

	void sendChatMessage(ENetPeer* peer, int netID, string message)
	{
		if (message.length() == 0) return;
		for (char c : message)
			if (c < 0x18 || std::all_of(message.begin(), message.end(), isspace)) {
				return;
			}
		string name = "";
		ENetPeer* currentPeer;
		for (currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
			if (((PlayerInfo*)(currentPeer->data))->netID == netID)
				name = ((PlayerInfo*)(currentPeer->data))->displayName;
		}
		GamePacket p;
		GamePacket p2;
		if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> `5" + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), "`5" + message), 0));
		}
		else if (((PlayerInfo*)(peer->data))->adminLevel == 999) {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> `c" + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), "`c" + message), 0));
		}
		else if (((PlayerInfo*)(peer->data))->adminLevel == 777) {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> `2" + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), "`2" + message), 0));
		}
		else if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> `^" + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), "`^" + message), 0));
		}
		else if (((PlayerInfo*)(peer->data))->adminLevel == 444) {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> `1" + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), "`1" + message), 0));
		}
		else if (((PlayerInfo*)(peer->data))->adminLevel == 111) {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> `9" + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), "`9" + message), 0));
		}
		else {
			p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "CP:_PL:0_OID:_CT:[W]_ `o<`w" + name + "`o> " + message));
			p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), netID), message), 0));
		}
		for (currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
			if (isHere(peer, currentPeer)) {
				ENetPacket* packet = enet_packet_create(p.data,
					p.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(currentPeer, 0, packet);
				ENetPacket* packet2 = enet_packet_create(p2.data,
					p2.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(currentPeer, 0, packet2);
			}
		}
		delete p.data;
		delete p2.data;
	}

	void sendWho(ENetPeer* peer)
	{
		ENetPeer * currentPeer;
		string name = "";
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (isHere(peer, currentPeer))
			{
				if(((PlayerInfo*)(currentPeer->data))->isGhost)
					continue;

				gamepacket_t p;
				p.Insert("OnTalkBubble");
				p.Insert(((PlayerInfo*)(currentPeer->data))->netID);
				p.Insert(((PlayerInfo*)(currentPeer->data))->displayName);
				p.Insert(1);
				p.CreatePacket(peer);
			}
		}
	}

	// droping items WorldObjectMap::HandlePacket
	void sendDrop(ENetPeer* peer, int netID, int x, int y, int item, int count, BYTE specialEffect, bool onlyForPeer)
	{
		if (item >= 7068) return;
		if (item < 0) return;
		if (onlyForPeer) {
			PlayerMoving data;
			data.packetType = 14;
			data.x = x;
			data.y = y;
			data.netID = netID;
			data.plantingTree = item;
			float val = count; // item count
			BYTE val2 = specialEffect;

			BYTE* raw = packPlayerMoving(&data);
			memcpy(raw + 16, &val, 4);
			memcpy(raw + 1, &val2, 1);

			SendPacketRaw(4, raw, 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
		}
		else {
			DroppedItem dropItem;
			dropItem.x = x;
			dropItem.y = y;
			dropItem.count = count;
			dropItem.id = item;
			dropItem.uid = worldDB.get2(((PlayerInfo *)(peer->data))->currentWorld).ptr->currentItemUID++;
			worldDB.get2(((PlayerInfo *)(peer->data))->currentWorld).ptr->droppedItems.push_back(dropItem);
			ENetPeer * currentPeer;
			for (currentPeer = server->peers;
				currentPeer < &server->peers[server->peerCount];
				++currentPeer)
			{
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
					continue;
				if (isHere(peer, currentPeer)) {

					ItemSharedUID m_uid;
					m_uid.actual_uid = dropItem.uid;
					m_uid.shared_uid = (((PlayerInfo*)(currentPeer->data)))->last_uid++;
					(((PlayerInfo*)(currentPeer->data)))->item_uids.push_back(m_uid);
					PlayerMoving data;
					data.packetType = 14;
					data.x = x;
					data.y = y;
					data.netID = netID;
					data.plantingTree = item;
					float val = count; // item count
					BYTE val2 = specialEffect;

					BYTE* raw = packPlayerMoving(&data);
					memcpy(raw + 16, &val, 4);
					memcpy(raw + 1, &val2, 1);

					SendPacketRaw(4, raw, 56, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);
				}
			}
		}
	}

	//This is only on server. The inventory is automatically updated on the client.
	void addItemToInventory(ENetPeer * peer, int id) {
		PlayerInventory inventory = ((PlayerInfo *)(peer->data))->inventory;
		for (int i = 0; i < inventory.items.size(); i++) {
			if (inventory.items[i].itemID == id && inventory.items[i].itemCount < 200) {
				inventory.items[i].itemCount++;
				return;
			}
		}
		if (inventory.items.size() >= inventory.inventorySize)
			return;
		InventoryItem item;
		item.itemCount = 1;
		item.itemID = id;
		inventory.items.push_back(item);
	}

	int getSharedUID(ENetPeer* peer, int itemNetID) {
		auto v = ((PlayerInfo*)(peer->data))->item_uids;
		for (auto t = v.begin(); t != v.end(); ++t) {
			if (t->actual_uid == itemNetID) {
				return t->shared_uid;
			}
		}
		return 0;
	}

	int checkForUIDMatch(ENetPeer * peer, int itemNetID) {
		auto v = ((PlayerInfo*)(peer->data))->item_uids;
		for (auto t = v.begin(); t != v.end(); ++t) {
			if (t->shared_uid == itemNetID) {
				return t->actual_uid;
			}
		}
		return 0;
	}

	void sendCollect(ENetPeer* peer, int netID, int itemNetID) {
		ENetPeer * currentPeer;
		PlayerMoving data;
		data.packetType = 14;
		data.netID = netID;
		data.plantingTree = itemNetID;
		data.characterState = 0;
		cout << "Request collect: " << std::to_string(itemNetID) << endl;
		WorldInfo *world = getPlyersWorld(peer);
		for (auto m_item = world->droppedItems.begin(); m_item != world->droppedItems.end(); ++m_item) {
			if ((checkForUIDMatch(peer, itemNetID)) == m_item->uid) {
				cout << "Success!" << endl;
				for (currentPeer = server->peers;
					currentPeer < &server->peers[server->peerCount];
					++currentPeer)
				{
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
						continue;
					if (isHere(peer, currentPeer)) {
						data.plantingTree = getSharedUID(currentPeer, m_item->uid);
						BYTE* raw = packPlayerMoving(&data);
						SendPacketRaw(4, raw, 56, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);
					}
				}
				world->droppedItems.erase(m_item);
				m_item--;
				return;
			}
		}
	}

	void sendWorld(ENetPeer* peer, WorldInfo* worldInfo)
	{

		// LOADING DROPPED ITEMS
		/*DroppedItem itemDropped;
		itemDropped.id = 0;
		itemDropped.count = 0;
		itemDropped.x = 0;
		itemDropped.y = 0;
		itemDropped.uid = 0;*/
		// TODO DROPPING ITEMS!!!!!!!!!!!!!!!!
		/*if (worldInfo->dropSized == false) {
			worldInfo->droppedItems.resize(1024000);
			for (int i = 0; i < 65536; i++) worldInfo->droppedItems.push_back(itemDropped);
			worldInfo->dropSized = true;
		}*/


		int zero = 0;
		((PlayerInfo*)(peer->data))->droppeditemcount = 0;
#ifdef TOTAL_LOG
		cout << "[!] Entering a world..." << endl;
#endif
		((PlayerInfo*)(peer->data))->joinClothesUpdated = false;
		string asdf = "0400000004A7379237BB2509E8E0EC04F8720B050000000000000000FBBB0000010000007D920100FDFDFDFD04000000040000000000000000000000070000000000"; // 0400000004A7379237BB2509E8E0EC04F8720B050000000000000000FBBB0000010000007D920100FDFDFDFD04000000040000000000000000000000080000000000000000000000000000000000000000000000000000000000000048133A0500000000BEBB0000070000000000
		string worldName = worldInfo->name;
		int xSize = worldInfo->width;
		int ySize = worldInfo->height;
		int square = xSize * ySize;
		__int16 nameLen = (__int16)worldName.length();
		int payloadLen = asdf.length() / 2;
		int dataLen = payloadLen + 2 + nameLen + 12 + (square * 8) + 4 + 100;
		int offsetData = dataLen - 100;
		int allocMem = payloadLen + 2 + nameLen + 12 + (square * 8) + 4 + 16000 + 100 + (worldInfo->droppedCount * 20);
		BYTE* data = new BYTE[allocMem];
		memset(data, 0, allocMem);
		for (int i = 0; i < asdf.length(); i += 2)
		{
			char x = ch2n(asdf[i]);
			x = x << 4;
			x += ch2n(asdf[i + 1]);
			memcpy(data + (i / 2), &x, 1);
		}

		__int16 item = 0;
		int smth = 0;
		for (int i = 0; i < square * 8; i += 4) memcpy(data + payloadLen + i + 14 + nameLen, &zero, 4);
		for (int i = 0; i < square * 8; i += 8) memcpy(data + payloadLen + i + 14 + nameLen, &item, 2);
		memcpy(data + payloadLen, &nameLen, 2);
		memcpy(data + payloadLen + 2, worldName.c_str(), nameLen);
		memcpy(data + payloadLen + 2 + nameLen, &xSize, 4);
		memcpy(data + payloadLen + 6 + nameLen, &ySize, 4);
		memcpy(data + payloadLen + 10 + nameLen, &square, 4);
		BYTE* blockPtr = data + payloadLen + 14 + nameLen;

		int sizeofblockstruct = 8;


		for (int i = 0; i < square; i++) {

			int tile = worldInfo->items[i].foreground;
			sizeofblockstruct = 8;


			//if (world->items[x + (y*world->width)].foreground == 242 or world->items[x + (y*world->width)].foreground == 2408 or world->items[x + (y*world->width)].foreground == 5980 or world->items[x + (y*world->width)].foreground == 2950 or world->items[x + (y*world->width)].foreground == 5814 or world->items[x + (y*world->width)].foreground == 4428 or world->items[x + (y*world->width)].foreground == 1796 or world->items[x + (y*world->width)].foreground == 4802 or world->items[x + (y*world->width)].foreground == 4994 or world->items[x + (y*world->width)].foreground == 5260 or world->items[x + (y*world->width)].foreground == 7188)
			if (tile == 6) {
				int type = 0x00010000;
				memcpy(blockPtr, &tile, 2);
				memcpy(blockPtr + 4, &type, 4);
				BYTE btype = 1;
				memcpy(blockPtr + 8, &btype, 1);

				string doorText = "EXIT";
				const char* doorTextChars = doorText.c_str();
				short length = (short)doorText.size();
				memcpy(blockPtr + 9, &length, 2);
				memcpy(blockPtr + 11, doorTextChars, length);
				sizeofblockstruct += 4 + length;
				dataLen += 4 + length; // it's already 8.

			}
			else if (getItemDef(tile).blockType == BlockTypes::SIGN || tile == 1420 || tile == 6124) {
				int type = 0x00010000;
				memcpy(blockPtr, &worldInfo->items[i].foreground, 2);
				memcpy(blockPtr + 4, &type, 4);
				BYTE btype = 2;
				memcpy(blockPtr + 8, &btype, 1);
				string signText = worldInfo->items[i].text;
				const char* signTextChars = signText.c_str();
				short length = (short)signText.size();
				memcpy(blockPtr + 9, &length, 2);
				memcpy(blockPtr + 11, signTextChars, length);
				int minus1 = -1;
				memcpy(blockPtr + 11 + length, &minus1, 4);
				sizeofblockstruct += 3 + length + 4;
				dataLen += 3 + length + 4; // it's already 8.
			}
			else if (tile == 3832) {
				int type = 0x00010000;
				memcpy(blockPtr, &worldInfo->items[i].foreground, 2);
				memcpy(blockPtr + 4, &type, 4);
				BYTE btype = 0x31;
				memcpy(blockPtr + 8, &btype, 1);


				short flags = 0;
				int item = worldInfo->items[i].displayBlock;
				int gravity = worldInfo->items[i].gravity;
				flags = 3;

				memcpy(blockPtr + 9, &item, 4);
				memcpy(blockPtr + 13, &gravity, 4);
				memcpy(blockPtr + 17, &flags, 2);
				sizeofblockstruct += 10;
				dataLen += 10;
			}
			else if ((worldInfo->items[i].foreground == 0) || (worldInfo->items[i].foreground == 2) || (worldInfo->items[i].foreground == 8) || (worldInfo->items[i].foreground == 100) || (worldInfo->items[i].foreground == 4))
			{

				memcpy(blockPtr, &worldInfo->items[i].foreground, 2);
				int type = 0x00000000;

				// type 1 = locked
				if (worldInfo->items[i].activated)
					type |= 0x00200000;
				if (worldInfo->items[i].water)
					type |= 0x04000000;
				if (worldInfo->items[i].glue)
					type |= 0x08000000;
				if (worldInfo->items[i].fire)
					type |= 0x10000000;
				if (worldInfo->items[i].red)
					type |= 0x20000000;
				if (worldInfo->items[i].green)
					type |= 0x40000000;
				if (worldInfo->items[i].blue)
					type |= 0x80000000;

				// int type = 0x04000000; = water
				// int type = 0x08000000 = glue
				// int type = 0x10000000; = fire
				// int type = 0x20000000; = red color
				// int type = 0x40000000; = green color
				// int type = 0x80000000; = blue color


				memcpy(blockPtr + 4, &type, 4);
				/*if (worldInfo->items[i].foreground % 2)
				{
					blockPtr += 6;
				}*/
			}
			else
			{
				memcpy(blockPtr, &zero, 2);
			}
			memcpy(blockPtr + 2, &worldInfo->items[i].background, 2);
			blockPtr += sizeofblockstruct;


		}

		/*int increase = 20;
	//TODO

		int inc = 20;
		memcpy(blockPtr, &worldInfo->droppedCount, 4);
		memcpy(blockPtr + 4, &worldInfo->droppedCount, 4);

		for (int i = 0; i < worldInfo->droppedCount; i++) {

			memcpy(blockPtr + inc - 12, &worldInfo->droppedItems.at(i).id, 2);
			memcpy(blockPtr + inc - 10, &worldInfo->droppedItems.at(i).x, 4);
			memcpy(blockPtr + inc - 6, &worldInfo->droppedItems.at(i).y, 4);
			memcpy(blockPtr + inc - 2, &worldInfo->droppedItems.at(i).count, 2);
			memcpy(blockPtr + inc, &i, 4);
			inc += 16;

		}
		blockPtr += inc;
		dataLen += inc;*/

		//((PlayerInfo*)(peer->data))->droppeditemcount = worldInfo->droppedCount;
		offsetData = dataLen - 100;

		//              0       1       2       3       4       5       6       7       8       9      10     11      12      13      14
		string asdf2 = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
		BYTE* data2 = new BYTE[101];
		memcpy(data2 + 0, &zero, 4);
		for (int i = 0; i < asdf2.length(); i += 2)
		{
			char x = ch2n(asdf2[i]);
			x = x << 4;
			x += ch2n(asdf2[i + 1]);
			memcpy(data2 + (i / 2), &x, 1);
		}
		int weather = worldInfo->weather;
		memcpy(data2 + 4, &weather, 4);

		memcpy(data + offsetData, data2, 100);


		//cout << dataLen << " <- dataLen allocMem -> " << allocMem << endl;
		memcpy(data + dataLen - 4, &smth, 4);
		ENetPacket* packet2 = enet_packet_create(data,
			dataLen,
			ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet2);
		//enet_host_flush(server);
		for (int i = 0; i < square; i++) {
			ItemDefinition pro;
			pro = getItemDef(worldInfo->items[i].foreground);
			if ((worldInfo->items[i].foreground == 0) || (getItemDef(worldInfo->items[i].foreground).blockType) == BlockTypes::SIGN || worldInfo->items[i].foreground == 1420 || worldInfo->items[i].foreground == 6214 || (worldInfo->items[i].foreground == 3832) || (worldInfo->items[i].foreground == 2946) || (worldInfo->items[i].foreground == 6) || (worldInfo->items[i].foreground == 4) || (worldInfo->items[i].foreground == 2) || (worldInfo->items[i].foreground == 8) || (worldInfo->items[i].foreground == 100))
				; // nothing
			else if (worldInfo->items[i].foreground == 242 || worldInfo->items[i].foreground == 2408 || worldInfo->items[i].foreground == 1796 || worldInfo->items[i].foreground == 4428 || worldInfo->items[i].foreground == 7188)
			{
				ENetPeer* currentPeer;

				for (currentPeer = server->peers;
					currentPeer < &server->peers[server->peerCount];
					++currentPeer)
				{
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
						continue;
					if (isHere(peer, currentPeer)) {
						int x = i % xSize, y = i / xSize;
						sendTileData(currentPeer, x, y, 0x10, worldInfo->items[x + (y * worldInfo->width)].foreground, worldInfo->items[x + (y * worldInfo->width)].background, lockTileDatas(0x20, worldInfo->ownerID, 0, 0, false, 100));
					}
				}
			}
			else
			{
				PlayerMoving data;
				//data.packetType = 0x14;
				data.packetType = 0x3;

				//data.characterState = 0x924; // animation
				data.characterState = 0x0; // animation
				data.x = i % worldInfo->width;
				data.y = i / worldInfo->height;
				data.punchX = i % worldInfo->width;
				data.punchY = i / worldInfo->width;
				data.XSpeed = 0;
				data.YSpeed = 0;
				data.netID = -1;
				data.plantingTree = worldInfo->items[i].foreground;
				SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
				int x = i % xSize, y = i / xSize;
				UpdateBlockState(peer, x, y, true, worldInfo);
			}
		}
		int idx = 0;
		for (int i = 0; i < worldInfo->droppedItemUid; i++)
		{
			bool found = false;
			for (int j = 0; j < worldInfo->droppedItems.size(); j++)
			{
				if (worldInfo->droppedItems.at(j).uid == i)
				{
					SendDropSingle(peer, -1, worldInfo->droppedItems.at(j).x, worldInfo->droppedItems.at(j).y, worldInfo->droppedItems.at(j).id, worldInfo->droppedItems.at(j).count, 0);
					found = true;
					break;
				}
			}
			// temporary fix
			if (!found) SendDropSingle(peer, -1, -1000, -1000, 0, 1, 0);
		}
		((PlayerInfo*)(peer->data))->currentWorld = worldInfo->name;
		for (int i = 0; i < xSize; i++) {
			for (int j = 0; j < ySize; j++) {
				int squaresign = i + (j * 100);

				bool displaysss = std::experimental::filesystem::exists("display/" + worldInfo->name + "X" + std::to_string(squaresign) + ".txt");

				if (displaysss) {
					if (worldInfo->items[squaresign].foreground == 2946)
					{

						int x = squaresign % worldInfo->width;
						int y = squaresign / worldInfo->width;
						//cout << "[!] foundzzzzzzzzzzzzzz!";
						WorldInfo* world = getPlyersWorld(peer);
						ENetPeer* currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							if (isHere(peer, currentPeer)) {
								BlockVisual data;
								data.packetType = 0x5;
								data.characterState = 8;
								data.punchX = x;
								data.punchY = y;
								data.charStat = 13; // 13y
								data.blockid = 2946; // 2946 3794 = display shelf
													 //data.netID = ((PlayerInfo*)(peer->data))->netID;
								data.backgroundid = 6864;
								data.visual = 0x00010000; //0x00210000

								std::ifstream ifs("display/" + worldInfo->name + "X" + std::to_string(squaresign) + ".txt");
								std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
								int id = atoi(content.c_str());

								world->items[x + (y * world->width)].displayblock = id;

								int n = id;
								string hex = "";
								{
									std::stringstream ss;
									ss << std::hex << n; // int decimal_value
									std::string res(ss.str());

									hex = res + "17";
								}

								if (hex == "2017") {
									continue;
								}


								int xx;
								std::stringstream ss;
								ss << std::hex << hex;
								ss >> xx;
								data.displayblock = xx;


								SendPacketRaw(192, packBlockVisual(&data), 69, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);

							}
						}
					}
				}
			}
		}

		int otherpeople = 0;
		int count = 0;
		ENetPeer* currentPeer;
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			count++;
			if (isHere(peer, currentPeer))
				otherpeople++;
		}
		string gstatement, gleader;
		((PlayerInfo*)(peer->data))->guildLeader = gleader;
		if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
			((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName + "`w(`eGL`w)";
		}
		int otherpeoples = otherpeople - 1;
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 0) {
					((PlayerInfo*)(peer->data))->displayName = "`2" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
				else if (((PlayerInfo*)(peer->data))->legend == true) {
					((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName + " of Legend";
				}
				else if (((PlayerInfo*)(peer->data))->level >= 125) {
					((PlayerInfo*)(peer->data))->displayName >= "`4Dr. " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 0) {
					((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName;
				}
				else if (((PlayerInfo*)(peer->data))->legend == true) {
					((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName + " of Legend";
				}
				else if (((PlayerInfo*)(peer->data))->level >= 125) {
					((PlayerInfo*)(peer->data))->displayName >= "`4Dr. " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 0) {
					((PlayerInfo*)(peer->data))->displayName = "`2" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
				else if (((PlayerInfo*)(peer->data))->legend == true) {
					((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName + " of Legend";
				}
				else if (((PlayerInfo*)(peer->data))->level >= 125) {
					((PlayerInfo*)(peer->data))->displayName >= "`4Dr. " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 0) {
					((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName;
				}
				else if (((PlayerInfo*)(peer->data))->legend == true) {
					((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName + " of Legend";
				}
				else if (((PlayerInfo*)(peer->data))->level >= 125) {
					((PlayerInfo*)(peer->data))->displayName >= "`4Dr. " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 111) {
					((PlayerInfo*)(peer->data))->displayName = "`w[`2rowPass`w] " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 111) {
					((PlayerInfo*)(peer->data))->displayName = "`w[`2GrowPass`w] " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 444) {
					((PlayerInfo*)(peer->data))->displayName = "`w[`2GrowPass`w] " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 444) {
					((PlayerInfo*)(peer->data))->displayName = "`w[`2GrowPass`w] " + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`#@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`#@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`#@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`#@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 777) {
					((PlayerInfo*)(peer->data))->displayName = "`c@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 777) {
					((PlayerInfo*)(peer->data))->displayName = "`c@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 777) {
					((PlayerInfo*)(peer->data))->displayName = "`c@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 777) {
					((PlayerInfo*)(peer->data))->displayName = "`cc@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 999) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 999) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 999) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 999) {
					((PlayerInfo*)(peer->data))->displayName = "`4@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		if (((PlayerInfo*)(peer->data))->haveGrowId) {
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
					((PlayerInfo*)(peer->data))->displayName = "`6@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
					((PlayerInfo*)(peer->data))->displayName = "`6@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}
		else
		{
			if (((PlayerInfo*)(peer->data))->rawName == worldInfo->owner)
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
					((PlayerInfo*)(peer->data))->displayName = "`6@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}

			else
				string gstatement, gleader;
			((PlayerInfo*)(peer->data))->guildLeader = gleader;
			if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
				((PlayerInfo*)(peer->data))->displayName = ((PlayerInfo*)(peer->data))->tankIDName + "`w(`eGL`w)";
			}
			else
			{
				if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
					((PlayerInfo*)(peer->data))->displayName = "`6@" + ((PlayerInfo*)(peer->data))->tankIDName;
				}
			}
		}

		string act = ((PlayerInfo*)(peer->data))->currentWorld;
		sendState(peer);
		if (worldInfo->weather == 29) {
			WorldInfo* world = getPlyersWorld(peer);
			int x = 0;
			int y = 0;
			int stuffGra = world->stuff_gravity;
			bool stuff_spin = world->stuff_spin;
			int stuffID = world->stuffID;
			bool stuff_invert = world->stuff_invert;
			world->stuff_invert = stuff_invert;
			world->stuff_spin = stuff_spin;
			world->stuff_gravity = stuffGra;
			world->stuffID = stuffID;
			sendStuffweather(currentPeer, x, y, world->stuffID, stuffGra, stuff_spin, stuff_invert);
		}
		else {
			GamePacket p7 = packetEnd(appendInt(appendString(createPacket(), "OnSetCurrentWeather"), worldInfo->weather));
			ENetPacket* packet7 = enet_packet_create(p7.data,
				p7.len,
				ENET_PACKET_FLAG_RELIABLE);

			enet_peer_send(peer, 0, packet7);
			delete p7.data;
		}
		string nameworld = worldInfo->name;
		string ownerworld = worldInfo->owner;
		string accessname = "";
		for (std::vector<string>::const_iterator i = worldInfo->acclist.begin(); i != worldInfo->acclist.end(); ++i) {
			accessname = *i;
		}
		if (worldInfo->owner == ((PlayerInfo*)(peer->data))->rawName)
		{

			GamePacket p3 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`5 `5[`0" + nameworld + " `$World Locked `oby " + ownerworld + " ``(`2ACCESS GRANTED``)`5]"));
			ENetPacket* packet3 = enet_packet_create(p3.data,
				p3.len,
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet3);
			delete p3.data;





		}
		else if (((PlayerInfo*)(peer->data))->rawName == accessname)
		{

			GamePacket p3 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`5 `5[`0" + nameworld + " `$World Locked `oby " + ownerworld + " ``(`2ACCESS GRANTED``)`5]"));
			ENetPacket* packet3 = enet_packet_create(p3.data,
				p3.len,
				ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet3);
			delete p3.data;





		}
		else if (((PlayerInfo*)(peer->data))->adminLevel == 1337)
		{
			if (ownerworld != "") {
				GamePacket p3 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`5 `5[`0" + nameworld + " `$World Locked `oby " + ownerworld + " ``(`2ACCESS GRANTED``)`5]"));
				ENetPacket* packet3 = enet_packet_create(p3.data,
					p3.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet3);
				delete p3.data;

			}





		}

		else
		{

			if (ownerworld != "") {
				GamePacket p3 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`5 `5[`0" + nameworld + " `$World Locked `oby " + ownerworld + "`5]"));
				ENetPacket* packet3 = enet_packet_create(p3.data,
					p3.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet3);
				delete p3.data;
			}
		}
		delete data;
	}
	void sendAction(ENetPeer* peer, int netID, string action)
	{
		ENetPeer * currentPeer;
		string name = "";
		gamepacket_t p(0, netID);
		p.Insert("OnAction");
		p.Insert(action); 
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			if (isHere(peer, currentPeer)) {
				p.CreatePacket(currentPeer);
			}
		}
	}

	

	void sendWorldOffers(ENetPeer* peer)
	{
		if (!((PlayerInfo*)(peer->data))->isIn) return;
		vector<WorldInfo> worlds = worldDB.getRandomWorlds();
		string worldOffers = "default|";
		if (worlds.size() > 0) {
			worldOffers += worlds[0].name;
		}
		
		worldOffers += "\nadd_button|Showing: `wWorlds``|_catselect_|0.6|3529161471|\n";
		for (int i = 0; i < worlds.size(); i++) {
			worldOffers += "add_floater|"+worlds[i].name+"|"+std::to_string(getPlayersCountInWorld(worlds[i].name))+"|0.55|3529161471\n";
		}
		//GamePacket p3 = packetEnd(appendString(appendString(createPacket(), "OnRequestWorldSelectMenu"), "default|GO FOR IT\nadd_button|Showing: `wFake Worlds``|_catselect_|0.6|3529161471|\nadd_floater|Subscribe|5|0.55|3529161471\nadd_floater|Growtopia|4|0.52|4278190335\nadd_floater|Noobs|150|0.49|3529161471\nadd_floater|...|3|0.49|3529161471\nadd_floater|`6:O :O :O``|2|0.46|3529161471\nadd_floater|SEEMS TO WORK|2|0.46|3529161471\nadd_floater|?????|1|0.43|3529161471\nadd_floater|KEKEKEKEK|13|0.7|3417414143\n"));
		//for (int i = 0; i < p.len; i++) cout << (int)*(p.data + i) << " ";
		packet::requestworldselectmenu(peer, worldOffers);
	}




	//replaced X-to-close with a Ctrl+C exit
	void exitHandler(int s) {
		saveAllWorlds();
		exit(0);

	}

void loadConfig() {
	/*inside config.json:
	{
	"port": 17091,
	"cdn": "0098/CDNContent37/
	"
	}
	*/
	
	
			std::ifstream ifs("config.json");
	if (ifs.is_open()) {
		json j;
		ifs >> j;
		ifs.close();
		try {
			configPort = j["port"].get<int>();
			configCDN = j["cdn"].get<string>();
			
			cout << "Config loaded." << endl;
		} catch (...) {
			cout << "Invalid Config, Fixing..." << endl;
			string config_contents = "{ \"port\": 17091, \"cdn\": \"0098/CDNContent77/cache/\" }";

			ofstream myfile1;
			myfile1.open("config.json");
			myfile1 << config_contents;
			myfile1.close();
			cout << "Config Has Been Fixed! Reloading..." << endl;
			std::ifstream ifs("config.json");
			json j;
			ifs >> j;
			ifs.close();
				configPort = j["port"].get<int>();
				configCDN = j["cdn"].get<string>();

				cout << "Config loaded." << endl;
		}
	} else {
		cout << "Config not found, Creating..." << endl;
		string config_contents = "{ \"port\": 17091, \"cdn\": \"0098/CDNContent77/cache/\" }";

		ofstream myfile1;
		myfile1.open("config.json");
		myfile1 << config_contents;
		myfile1.close();
		cout << "Config Has Been Created! Reloading..." << endl;
		std::ifstream ifs("config.json");
		json j;
		ifs >> j;
		ifs.close();
			configPort = j["port"].get<int>();
			configCDN = j["cdn"].get<string>();

			cout << "Config loaded." << endl;
	}
}

string randomDuctTapeMessage (size_t length) {
	auto randchar = []() -> char
    {
        const char charset[] =
        "f"
        "m";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar );
    return str;
}
	/*
	action|log
msg|`4UPDATE REQUIRED!`` : The `$V2.981`` update is now available for your device.  Go get it!  You'll need to install it before you can play online.
[DBG] Some text is here: action|set_url
url|http://ubistatic-a.akamaihd.net/0098/20180909/GrowtopiaInstaller.exe
label|Download Latest Version
	*/
//Linux should not have any arguments in main function.
#ifdef _WIN32
	int _tmain(int argc, _TCHAR* argv[])
#else
	int main()
#endif
{
	cout << "Growtopia private server (c) Ibord, Credit : GrowtopiaNoobs" << endl;
	system("Color 0A");
		
	cout << "Loading config from config.json" << endl;
	loadConfig();
		
	enet_initialize();
	//Unnecessary save at exit. Commented out to make the program exit slightly quicker.
	/*if (atexit(saveAllWorlds)) {
		cout << "Worlds won't be saved for this session..." << endl;
	}*/
	/*if (RegisterApplicationRestart(L" -restarted", 0) == S_OK)
	{
		cout << "Autorestart is ready" << endl;
	}
	else {
		cout << "Binding autorestart failed!" << endl;
	}
	Sleep(65000);
	int* p = NULL;
	*p = 5;*/
	ENetAddress address;
	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	enet_address_set_host (&address, "0.0.0.0");
	//address.host = ENET_HOST_ANY;
	/* Bind the server to port 1234. */
	address.port = configPort;
	server = enet_host_create(&address /* the address to bind the server host to */,
		1024      /* allow up to 32 clients and/or outgoing connections */,
		10      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);
	if (server == NULL)
	{
		fprintf(stderr,
			"An error occurred while trying to create an ENet server host.\n");
		while (1);
		exit(EXIT_FAILURE);
	}
	server->checksum = enet_crc32;
	enet_host_compress_with_range_coder(server);
	cout << "Building items database..." << endl;
	ifstream myFile("items.dat");
	if (myFile.fail()) {
		std::cout << "Items.dat not found!" << endl;
		std::cout << "Please put items.dat in this folder:" << endl;
                system("cd");
		std::cout << "If you dont have items.dat, you can get it from Growtopia cache folder. Please exit." << endl;
		//Sleep(10000);
                //exit(-1);
                while (true); // cross platform solution (Linux pls!)
	}
	buildItemsDatabase();
	cout << "Database is built!" << endl;
	loadnews();

	thread AutoSaveWorlds(autoSaveWorlds);
	if (AutoSaveWorlds.joinable()) AutoSaveWorlds.detach();

	worldDB.get("START");
	ENetEvent event;
	/* Wait up to 1000 milliseconds for an event. */
	while (true)
	while (enet_host_service(server, &event, 1000) > 0)
	{
		ENetPeer* peer = event.peer;
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
#ifdef TOTAL_LOG
			printf("A new client connected.\n");
#endif
			
			/* Store any relevant client information here. */
			//event.peer->data = "Client information";
			ENetPeer * currentPeer;
			int count = 0;
			for (currentPeer = server->peers;
				currentPeer < &server->peers[server->peerCount];
				++currentPeer)
			{
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
					continue;
				if (currentPeer->address.host == peer->address.host)
					count++;
			}

			event.peer->data = new PlayerInfo;
			/* Get the string ip from peer */
			char clientConnection[16];
			enet_address_get_host_ip(&peer->address, clientConnection, 16);
			((PlayerInfo*)(peer->data))->charIP = clientConnection;
			if (count > 3)
			{
				packet::consolemessage(peer, "`rToo many accounts are logged on from this IP. Log off one account before playing please.``");
				enet_peer_disconnect_later(peer, 0);
			}
			else {
				sendData(peer, 1, 0, 0);
			}


			continue;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			if (((PlayerInfo*)(peer->data))->isUpdating)
			{
				cout << "packet drop" << endl;
				continue;
			}

			int messageType = GetMessageTypeFromPacket(event.packet);

			WorldInfo* world = getPlyersWorld(peer);
			switch (messageType) {
			case 2:
			{
				//cout << GetTextPointerFromPacket(event.packet) << endl;
				string cch = GetTextPointerFromPacket(event.packet);
				if (cch.find("©") != std::string::npos) enet_peer_reset(peer);
				string str = cch.substr(cch.find("text|") + 5, cch.length() - cch.find("text|") - 1);
				if (cch.find("action|wrench") == 0) {
					vector<string> ex = explode("|", cch);


					stringstream ss;


					ss << ex[3];


					string temp;
					int found;
					while (!ss.eof()) {


						ss >> temp;


						if (stringstream(temp) >> found)
							//cout << found;
							((PlayerInfo*)(peer->data))->wrenchsession = found;


						temp = "";
					}
					string worldsowned;
					string rolex;
					string rolexx;
					if (((PlayerInfo*)(peer->data))->haveGrowId == false) {
						continue;
					}
					ENetPeer* currentPeer;
					for (currentPeer = server->peers;
						currentPeer < &server->peers[server->peerCount];
						++currentPeer)
					{
						if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
							continue;


						if (isHere(peer, currentPeer)) {
							if (((PlayerInfo*)(currentPeer->data))->haveGrowId == false) {
								continue;
							}
							if (((PlayerInfo*)(currentPeer->data))->netID == ((PlayerInfo*)(peer->data))->wrenchsession) {
								std::ifstream ifs("gemdb/" + ((PlayerInfo*)(currentPeer->data))->rawName + ".txt");
								std::string content2x((std::istreambuf_iterator<char>(ifs)),
									(std::istreambuf_iterator<char>()));

								((PlayerInfo*)(peer->data))->lastInfo = ((PlayerInfo*)(currentPeer->data))->rawName;
								((PlayerInfo*)(peer->data))->lastInfoname = ((PlayerInfo*)(currentPeer->data))->tankIDName;

								string name = ((PlayerInfo*)(currentPeer->data))->displayName;
								string rawnam = ((PlayerInfo*)(peer->data))->rawName;
								string rawnamofwrench = ((PlayerInfo*)(currentPeer->data))->rawName;
								string guildleader = ((PlayerInfo*)(peer->data))->guildLeader;
								if (rawnamofwrench != rawnam)
								{

									if (rawnamofwrench != "")
									{
										if (find(((PlayerInfo*)(peer->data))->guildMembers.begin(), ((PlayerInfo*)(peer->data))->guildMembers.end(), name) != ((PlayerInfo*)(peer->data))->guildMembers.end()) {
											if (world->owner == ((PlayerInfo*)(peer->data))->rawName && ((PlayerInfo*)(peer->data))->haveGrowId || ((PlayerInfo*)(peer->data))->adminLevel >= 777)
											{
												if (((PlayerInfo*)(peer->data))->adminLevel >= 999 || ((PlayerInfo*)(peer->data))->rawName == "saku" || ((PlayerInfo*)(peer->data))->rawName == "nabz")
												{
													GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`w" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small||\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_button|infobutton|`!Punish/View|0|0|\nadd_button|pull|`5Pull|0|0|\nadd_button|kick|`4Kick|0|0|\nadd_button|wban|`4World Ban|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
													ENetPacket* packet = enet_packet_create(p.data,
														p.len,
														ENET_PACKET_FLAG_RELIABLE);
													enet_peer_send(peer, 0, packet);
													delete p.data;
												}
												else
												{
													GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|18|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_button|pull|`5Pull|0|0|\nadd_button|kick|`4Kick|0|0|\nadd_button|wban|`4World Ban|0|0|\nadd_button|giveownership|`wGive ownership to this player!|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
													ENetPacket* packet = enet_packet_create(p.data,
														p.len,
														ENET_PACKET_FLAG_RELIABLE);
													enet_peer_send(peer, 0, packet);
													delete p.data;
												}
											}
											else
											{
												GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
												ENetPacket* packet = enet_packet_create(p.data,
													p.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet);
												delete p.data;
											}
										}
										else if (((PlayerInfo*)(peer->data))->rawName == guildleader) {
											if (world->owner == ((PlayerInfo*)(peer->data))->rawName && ((PlayerInfo*)(peer->data))->haveGrowId || ((PlayerInfo*)(peer->data))->adminLevel >= 777)
											{
												if (((PlayerInfo*)(peer->data))->adminLevel >= 999 || ((PlayerInfo*)(peer->data))->rawName == "mindpin")
												{
													GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`w" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_label|small|\nadd_button|trades|`9Trade|0|0|\nadd_button|infobutton|`!Punish/View|0|0|\nadd_button|pull|`5Pull|0|0|\nadd_button|kick|`4Kick|0|0|\nadd_button|wban|`4World Ban|0|0|\nadd_button|inviteguildbutton|`2Invite to guild``|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
													ENetPacket* packet = enet_packet_create(p.data,
														p.len,
														ENET_PACKET_FLAG_RELIABLE);
													enet_peer_send(peer, 0, packet);
													delete p.data;
												}
												else
												{
													GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_button|pull|`5Pull|0|0|\nadd_button|kick|`4Kick|0|0|\nadd_button|wban|`4World Ban|0|0|\nadd_button|inviteguildbutton|`2Invite to guild``|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
													ENetPacket* packet = enet_packet_create(p.data,
														p.len,
														ENET_PACKET_FLAG_RELIABLE);
													enet_peer_send(peer, 0, packet);
													delete p.data;
												}
											}
											else
											{
												GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_button|inviteguildbutton|`2Invite to guild``|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
												ENetPacket* packet = enet_packet_create(p.data,
													p.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet);
												delete p.data;
											}
										}
										else {
											if (world->owner == ((PlayerInfo*)(peer->data))->rawName && ((PlayerInfo*)(peer->data))->haveGrowId || ((PlayerInfo*)(peer->data))->adminLevel >= 777)
											{
												if (((PlayerInfo*)(peer->data))->adminLevel >= 999 || ((PlayerInfo*)(peer->data))->rawName == "mindpin")
												{
													GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`w" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`9Trade|0|0|\nadd_button|infobutton|`!Punish/View|0|0|\nadd_button|pull|`5Pull|0|0|\nadd_button|kick|`4Kick|0|0|\nadd_button|wban|`4World Ban|0|0|\nadd_button|giveownership|`wGive ownership to this player!|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
													ENetPacket* packet = enet_packet_create(p.data,
														p.len,
														ENET_PACKET_FLAG_RELIABLE);
													enet_peer_send(peer, 0, packet);
													delete p.data;
												}
												else
												{
													GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_button|pull|`5Pull|0|0|\nadd_button|kick|`4Kick|0|0|\nadd_button|wban|`4World Ban|0|0|\nadd_button|giveownership|`wGive ownership to this player!|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
													ENetPacket* packet = enet_packet_create(p.data,
														p.len,
														ENET_PACKET_FLAG_RELIABLE);
													enet_peer_send(peer, 0, packet);
													delete p.data;
												}
											}
											else
											{
												GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_label|small|\nadd_label|small|`oGems : `6" + content2x + "$|left|4|\nadd_button|trades|`wTrade|0|0|\nadd_spacer|small|\nadd_button||Continue|0|0|\nadd_quick_exit"));
												ENetPacket* packet = enet_packet_create(p.data,
													p.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet);
												delete p.data;
											}
										}
									}
									else
									{
										GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\nadd_label_with_icon|big|`0" + name + " `w(`2" + to_string(((PlayerInfo*)(currentPeer->data))->level) + "`w)``|left|" + getRankId(((PlayerInfo*)(peer->data))->lastInfoname) + "|\nadd_spacer|small|\nadd_button|chc0|Close|noflags|0|0|\n\nadd_quick_exit|\nnend_dialog|gazette||OK|"));
										ENetPacket* packet = enet_packet_create(p.data,
											p.len,
											ENET_PACKET_FLAG_RELIABLE);
										enet_peer_send(peer, 0, packet);
										delete p.data;
									}
								}
								else
								{
									if (((PlayerInfo*)(peer->data))->haveGrowId == true)
									{
										std::ostringstream oss;
										if (!((PlayerInfo*)(peer->data))->worldsowned.empty())
										{
											std::copy(((PlayerInfo*)(peer->data))->worldsowned.begin(), ((PlayerInfo*)(peer->data))->worldsowned.end() - 1,
												std::ostream_iterator<string>(oss, " "));

											// Now add the last element with no delimiter
											oss << ((PlayerInfo*)(peer->data))->worldsowned.back();
										}
										else {
											string oss = "You dont have any worlds!";
										}
										int levels = ((PlayerInfo*)(peer->data))->level;
										int xp = ((PlayerInfo*)(peer->data))->xp;
										string currentworld = ((PlayerInfo*)(peer->data))->currentWorld;
										int yy = ((PlayerInfo*)(peer->data))->posX / 32;
										int xx = ((PlayerInfo*)(peer->data))->posY / 32;

										if (((PlayerInfo*)(peer->data))->isinvited == true)
										{
											GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_player_info|" + name + " | " + std::to_string(levels) + " | " + std::to_string(xp) + " | " + to_string(levels * 1500) + "|\nadd_spacer|small|\nadd_button|\nadd_spacer|small|\nadd_button|growmojis|`$Growmojis|\nadd_spacer|small|\nadd_button|gpasslabel|`9Royal GrowPass||\nadd_spacer|small|\nadd_button|ingamerole|`6Purchase Rank||\nadd_spacer|small|\nadd_button|chc0|Close|noflags|0|0|\n\nadd_quick_exit|\nnend_dialog|gazette||OK|"));
											ENetPacket* packet = enet_packet_create(p.data,
												p.len,
												ENET_PACKET_FLAG_RELIABLE);
											enet_peer_send(peer, 0, packet);
											delete p.data;
										}
										else
										{
											GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_player_info|" + name + " | " + std::to_string(levels) + " | " + std::to_string(xp) + " | " + to_string(levels * 1500) + "|\nadd_spacer|small|\nadd_button|\nadd_spacer|small|\nadd_button|growmojis|`$Growmojis|\nadd_spacer|small|\nadd_button|gpasslabel|`9Royal GrowPass||\nadd_spacer|small|\nadd_button|ingamerole|`6Purchase Rank||\nadd_spacer|small|\nadd_button|chc0|Close|noflags|0|0|\n\nadd_quick_exit|\nnend_dialog|gazette||OK|"));
											ENetPacket* packet = enet_packet_create(p.data,
												p.len,
												ENET_PACKET_FLAG_RELIABLE);
											enet_peer_send(peer, 0, packet);
											delete p.data;
										}
									}
									else
									{
									}
								}

							}


						}


					}
				}

				if (cch.find("action|friends\n") == 0) {
					if (((PlayerInfo*)(peer->data))->haveGrowId == false) {
						packet::dialog(peer, "set_default_color|`o\n\nadd_label_with_icon|big|`wGet a GrowID``|left|206|\n\nadd_spacer|small|\nadd_textbox|A `wGrowID `wmeans `oyou can use a name and password to logon from any device.|\nadd_spacer|small|\nadd_textbox|This `wname `owill be reserved for you and `wshown to other players`o, so choose carefully!|\nadd_text_input|username|GrowID||30|\nadd_text_input|password|Password||100|\nadd_text_input|passwordverify|Password Verify||100|\nadd_textbox|Your `wemail address `owill only be used for account verification purposes and won't be spammed or shared. If you use a fake email, you'll never be able to recover or change your password.|\nadd_text_input|email|Email||100|\nadd_textbox|Your `wDiscord ID `owill be used for secondary verification if you lost access to your `wemail address`o! Please enter in such format: `wdiscordname#tag`o. Your `wDiscord Tag `ocan be found in your `wDiscord account settings`o.|\nadd_text_input|discord|Discord||100|\nend_dialog|register|Cancel|Get My GrowID!|\n");
						enet_host_flush(server);
					}
					if (((PlayerInfo*)(peer->data))->joinguild == true) {
						string properson = "set_default_color|`w\n\nadd_label_with_icon|big|Social Portal``|left|1366|\n\nadd_spacer|small|\nadd_button|backonlinelist|Show Friends``|0|0|\nadd_button|showguild|Show Guild Members``|0|0|\nend_dialog||OK||\nadd_quick_exit|";
						packet::dialog(peer, properson);
					}
					else {
						string gayfriend = "set_default_color|`w\n\nadd_label_with_icon|big|Social Portal``|left|1366|\n\nadd_spacer|small|\nadd_button|backonlinelist|Show Friends``|0|0|\nadd_button|createguildinfo|Create Guild``|0|0|\nend_dialog||OK||\nadd_quick_exit|";
						packet::dialog(peer, gayfriend);
					}
				}

				if (cch.find("action|setSkin") == 0) {
					if (!world) continue;
					std::stringstream ss(cch);
					std::string to;
					int id = -1;
					string color;
					while (std::getline(ss, to, '\n')) {
						vector<string> infoDat = explode("|", to);
						if (infoDat[0] == "color") color = infoDat[1];
						if (has_only_digits(color) == false) continue;
						id = atoi(color.c_str());
						if (color == "2190853119") {
							id = -2104114177;
						}
						else if (color == "2527912447") {
							id = -1767054849;
						}
						else if (color == "2864971775") {
							id = -1429995521;
						}
						else if (color == "3033464831") {
							id = -1261502465;
						}
						else if (color == "3370516479") {
							id = -924450817;
						}

					}
					((PlayerInfo*)(peer->data))->skinColor = id;
					sendClothes(peer);
				}

				if (cch.find("action|buy\nitem|store") == 0)
				{
					string text1 = "set_description_text|Welcome to the `2Growtopia Store``! Select the item you'd like more info on.`o `wWant to get `5Supporter`` status? Any Gem purchase (or `57,000`` Gems earned with free `5Tapjoy`` offers) will make you one. You'll get new skin colors, the `5Recycle`` tool to convert unwanted items into Gems, and more bonuses!";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||1|0|0|0||||-1|-1||||";;
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1|||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||0|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|1|";
					string text10 = "|\nadd_button|itemomonth|`oItem Of The Month``|interface/large/store_buttons/store_buttons16.rttex|September 2021:`` `9Ouroboros Charm``!<CR><CR>`2You Get:`` 1 `9Ouroboros Charm``.<CR><CR>`5Description: ``The endless loop of life and death, personified and celebrated. Is it a charm or is it a curse?|0|3|350000|0||interface/large/gui_store_button_overlays1.rttex|0|0||-1|-1||1|||||||";
					string text11 = "|\nadd_button|ads_tv|`oGrowShow TV``|interface/large/store_buttons/store_buttons30.rttex|`2You Get:`` 1 GrowShow TV.<CR><CR>`5Description:`` Growtopia's most loved gameshow now brings you its very own TV to watch up to 3 ads per day for AMAZING prizes.|0|4|50|0|||-1|-1||-1|-1||1||||||";
					string text12 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|2|";
					string text13 = "|\nadd_button|gems_glory|Road To Glory|interface/large/store_buttons/store_buttons30.rttex|rt_grope_loyalty_bundle01|0|0|5|0||interface/large/gui_store_button_overlays1.rttex|0|0|/interface/large/gui_shop_buybanner.rttex|1|0|`2You Get:`` Road To Glory and 100k Gems Instantly.<CR>`5Description:`` Earn Gem rewards when you level up. Every 10 levels you will get additional Gem rewards up to Level 50! Claim all rewards instantly if you are over level 50!! 1.6M Gems in total!! |1|||||||";
					string text14 = "|\nadd_button|gems_rain|It's Rainin' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_rain|1|5|0|0|||-1|-1||-1|-1|`2You Get:`` 200,000 Gems, 2 Growtoken, and 1 Megaphone.<CR><CR>`5Description:`` All the gems you could ever want and more plus 2 Growtokens and a Megaphone to tell the whole world about it.|1|||||||";
					string text15 = "|\nadd_button|gems_fountain|Gem Fountain|interface/large/store_buttons/store_buttons2.rttex|rt_grope_gem_fountain|0|2|0|0|||-1|-1||-1|-1|`2You Get:`` 90,000 Gems and 1 Growtoken.<CR><CR>`5Description:`` Get a pile of gems to shop to your hearts desire and 1 Growtoken.|1|||||||";
					string text16 = "|\nadd_button|gems_chest|Chest o' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_chest|0|5|0|0|||-1|-1||-1|-1|`2You Get:`` 30,000 Gems.<CR><CR>`5Description:`` Get a chest containing gems.|1|||||||";
					string text17 = "|\nadd_button|gems_bag|Bag o' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_bag|1|0|0|0|||-1|-1||-1|-1|`2You Get:`` 14,000 Gems.<CR><CR>`5Description:`` Get a small bag of gems.|1|||||||";
					string text18 = "|\nadd_button|tapjoy|Earn Free Gems|interface/large/store_buttons/store_buttons.rttex||1|2|0|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|3|";
					string text20 = "|\nadd_button|365d|`o1-Year Subscription Token``|interface/large/store_buttons/store_buttons22.rttex|rt_grope_subs_bundle02|0|5|0|0|||-1|-1||-1|-1|`2You Get:`` 1x 1-Year Subscription Token and 25 Growtokens.<CR><CR>`5Description:`` One full year of special treatment AND 25 Growtokens upfront! You'll get 70 season tokens (as long as there's a seasonal clash running), and 2500 gems every day and a chance of doubling any XP earned, growtime reduction on all seeds planted and Exclusive Skins!|1||||||";
					string text21 = "|\nadd_button|30d|`o30-Day Subscription Token``|interface/large/store_buttons/store_buttons22.rttex|rt_grope_subs_bundle01|0|4|0|0|||-1|-1||-1|-1|`2You Get:`` 1x 30-Day Free Subscription Token and 2 Growtokens.<CR><CR>`5Description:`` 30 full days of special treatment AND 2 Growtokens upfront! You'll get 70 season tokens (as long as there's a seasonal clash running), and 2500 gems every day and a chance of doubling any XP earned, growtime reduction on all seeds planted and Exclusive Skins!|1||||||";
					string text22 = "|\nadd_button|video_tapjoy|Watch Videos For Gems|interface/large/store_buttons/store_buttons29.rttex||0|1|0|0|1/5 VIDEOS WATCHED||-1|-1||-1|-1||1||||||";

					/*
					string text1 = "set_description_text|Welcome to the `2Growtopia Store``!  Tap the item you'd like more info on.`o  `wWant to get `5Supporter`` status? Any Gem purchase (or `57,000`` Gems earned with free `5Tapjoy`` offers) will make you one. You'll get new skin colors, the `5Recycle`` tool to convert unwanted items into Gems, and more bonuses!";
					string text2 = "|\nadd_button|iapp_menu|Buy Gems|interface/large/store_buttons5.rttex||0|2|0|0|";
					string text3 = "|\nadd_button|subs_menu|Subscriptions|interface/large/store_buttons22.rttex||0|1|0|0|";
					string text4 = "|\nadd_button|itemomonth|`oItem Of The Month``|interface/large/store_buttons16.rttex|`2September 2018:`` `9Sorcerer's Tunic of Mystery!`` Capable of reflecting the true colors of the world around it, this rare tunic is made of captured starlight and aether. If you think knitting with thread is hard, just try doing it with moonbeams and magic! The result is worth it though, as these clothes won't just make you look amazing - you'll be able to channel their inherent power into blasts of cosmic energy!``|0|3|200000|0|";
					string text5 = "|\nadd_button|locks_menu|Locks And Stuff|interface/large/store_buttons3.rttex||0|4|0|0|";
					string text6 = "|\nadd_button|itempack_menu|Item Packs|interface/large/store_buttons3.rttex||0|3|0|0|";
					string text7 = "|\nadd_button|bigitems_menu|Awesome Items|interface/large/store_buttons4.rttex||0|6|0|0|";
					string text8 = "|\nadd_button|weather_menu|Weather Machines|interface/large/store_buttons5.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|4|0|0||";
					string text9 = "|\nadd_button|token_menu|Growtoken Items|interface/large/store_buttons9.rttex||0|0|0|0|";

					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22);
				}*/
					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22);
				}
				if (cch.find("action|buy\nitem|locks") == 0) {
					string text1 = "set_description_text|`2Locks And Stuff!``  Select the item you'd like more info on, or BACK to go back.";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|store_menu|Home|interface/large/btn_shop2.rttex||0|0|0|0||||-1|-1||||";
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||1|1|0|0||||-1|-1||||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||0|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_button|upgrade_backpack|`0Upgrade Backpack`` (`w10 Slots``)|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 10 Additional Backpack Slots.<CR><CR>`5Description:`` Sewing an extra pocket onto your backpack will allow you to store `$10`` additional item types.  How else are you going to fit all those toilets and doors?|0|1|3700|0|||-1|-1||-1|-1||1|||||||";
					string text10 = "|\nadd_button|rename|`oBirth Certificate``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Birth Certificate.<CR><CR>`5Description:`` Tired of being who you are? By forging a new birth certificate, you can change your GrowID! The Birth Certificate will be consumed when used. This item only works if you have a GrowID, and you can only use one every 60 days, so you're not confusing everybody.|0|6|20000|0|||-1|-1||-1|-1||1|||||||";
					string text11 = "|\nadd_button|clothes|`oClothes Pack``|interface/large/store_buttons/store_buttons2.rttex|`2You Get:`` 3 Randomly Wearble Items.<CR><CR>`5Description:`` Why not look the part? Some may even have special powers...|0|0|50|0|||-1|-1||-1|-1||1|||||||";
					string text12 = "|\nadd_button|rare_clothes|`oRare Clothes Pack``|interface/large/store_buttons/store_buttons2.rttex|`2You Get:`` 3 Randomly Chosen Wearbale Items.<CR><CR>`5Description:`` Enjoy the garb of kings! Some may even have special powers...|0|1|500|0|||-1|-1||-1|-1||1|||||||";
					string text13 = "|\nadd_button|transmutation_device|`oTransmutabooth``|interface/large/store_buttons/store_buttons27.rttex|`2You Get:`` 1 Transmutabooth.<CR><CR>`5Description:`` Behold! A wondrous technological achievement from the innovative minds at GrowTech, the Transmutabooth allows you to merge clothing items, transferring the visual appearance of one onto another in the same slot! If you've ever wanted your Cyclopean Visor to look like Shades (while keeping its mod), now you can!|0|7|25000|0|||-1|-1||-1|-1||1|||||||";
					string text14 = "|\nadd_button|contact_lenses|`oContact Lens Pack``|interface/large/store_buttons/store_buttons22.rttex|`2You Get:`` 10 Random Contact Lens Colors.<CR><CR>`5Description:`` Need a colorful new look? This pack includes 10 random Contact Lens colors (and may include Contact Lens Cleaning Solution, to return to your natural eye color)!|0|7|15000|0|||-1|-1||-1|-1||1|||||||";
					string text15 = "|\nadd_button|eye_drops|`oEye Drop Pack``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 `#Rare Bathroom Mirror`` and 10 random Eye Drop Colors.<CR><CR>`5Description:`` Need a fresh new look?  This pack includes a 10 random Eye Drop Colors (may include Eye Cleaning Solution, to leave your eyes sparkly clean)!|0|6|30000|0|||-1|-1||-1|-1||1|||||||";
					string text16 = "|\nadd_button|nyan_hat|`oTurtle Hat``|interface/large/store_buttons/store_buttons3.rttex|`2You Get:`` 1 Turtle Hat.<CR><CR>`5Description:`` It's the greatest hat ever. It bloops out bubbles as you run! `4Not available any other way!``|0|2|25000|0|||-1|-1||-1|-1||1|||||||";
					string text17 = "|\nadd_button|tiny_horsie|`oTiny Horsie``|interface/large/store_buttons/store_buttons3.rttex|`2You Get:`` 1 Tiny Horsie.<CR><CR>`5Description:`` Tired of wearing shoes? Wear a Tiny Horsie instead! Or possibly a large dachshund, we're not sure. Regardless, it lets you run around faster than normal, plus you're on a horse! `4Not available any other way!``|0|5|25000|0|||-1|-1||-1|-1||1|||||||";
					string text18 = "|\nadd_button|star_ship|`oPleiadian Star Ship``|interface/large/store_buttons/store_buttons4.rttex|`2You Get:`` 1 Pleiadian Star Ship.<CR><CR>`5Description:`` Float on, my brother. It's all groovy. This star ship can't fly, but you can still zoom around in it, leaving a trail of energy rings and moving at enhanced speed. Sponsored by Pleiadian. `4Not available any other way!``|0|3|25000|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_button|dragon_hand|`oDragon Hand``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Dragon Hand.<CR><CR>`5Description:`` Call forth the dragons of legend!  With the Dragon Hand, you will command your own pet dragon. Instead of punching blocks or players, you can order your dragon to incinerate them! In addition to just being awesome, this also does increased damage, and pushes other players farther. `4Not available any other way!``|0|1|50000|0|||-1|-1||-1|-1||1|||||||";
					string text20 = "|\nadd_button|corvette|`oLittle Red Corvette``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Little Red Corvette.<CR><CR>`5Description:`` Cruise around the neighborhood in style with this sweet convertible. It moves at enhanced speed and leaves other Growtopians in your dust. `4Not available any other way!``|0|1|25000|0|||-1|-1||-1|-1||1|||||||";
					string text21 = "|\nadd_button|stick_horse|`oStick Horse``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Stick Horse.<CR><CR>`5Description:`` Nobody looks cooler than a person bouncing along on a stick with a fake horse head attached. NOBODY. `4Not available any other way!``|0|3|25000|0|||-1|-1||-1|-1||1|||||||";
					string text22 = "|\nadd_button|ambulance|`oAmbulance``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Ambulance.<CR><CR>`5Description:`` Rush to the scene of an accident while lawyers chase you in this speedy rescue vehicle. `4Not available any other way!``|0|3|25000|0|||-1|-1||-1|-1||1|||||||";
					string text23 = "|\nadd_button|raptor|`oRiding Raptor``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Riding Raptor.<CR><CR>`5Description:`` Long thought to be extinct, it turns out that these dinosaurs are actually alive and easily tamed. And riding one lets you run around faster than normal! `4Not available any other way!``|0|7|25000|0|||-1|-1||-1|-1||1|||||||";
					string text24 = "|\nadd_button|owl|`oMid-Pacific Owl``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 Mid-Pacific Owl.<CR><CR>`5Description:`` This owl is a bit lazy - if you stop moving around, he'll land on your head and fall asleep. Dedicated to the students of the Mid-Pacific Institute. `4Not available any other way!``|0|1|30000|0|||-1|-1||-1|-1||1|||||||";
					string text25 = "|\nadd_button|unicorn|`oUnicorn Garland``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 Unicorn Garland.<CR><CR>`5Description:`` Prance about in the fields with your very own pet unicorn! It shoots `1R`2A`3I`4N`5B`6O`7W`8S``. `4Not available any other way!``|0|4|50000|0|||-1|-1||-1|-1||1|||||||";
					string text26 = "|\nadd_button|starboard|`oStarBoard``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 StarBoard.<CR><CR>`5Description:`` Hoverboards are here at last! Zoom around Growtopia on this brand new model, which is powered by fusion energy (that means stars spit out of the bottom). Moves faster than walking. Sponsored by Miwsky, Chudy, and Dawid. `4Not available any other way!``|0|1|30000|0|||-1|-1||-1|-1||1|||||||";
					string text27 = "|\nadd_button|motorcycle|`oGrowley Motorcycle``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 Growley Motorcycle.<CR><CR>`5Description:`` The coolest motorcycles available are Growley Dennisons. Get a sporty blue one today! It even moves faster than walking, which is pretty good for a motorcycle. `4Not available any other way!``|0|6|50000|0|||-1|-1||-1|-1||1|||||||";
					string text28 = "|\nadd_button|monkey_on_back|`oMonkey On Your Back``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 Monkey On Your Back.<CR><CR>`5Description:`` Most people work really hard to get rid of these, but hey, if you want one, it's available! `4But not available any other way!`` Sponsored by SweGamerHD's subscribers, Kizashi, and Inforced. `#Note: This is a neck item, not a back item. He's grabbing your neck!``|0|2|50000|0|||-1|-1||-1|-1||1|||||||";
					string text29 = "|\nadd_button|carrot_sword|`oCarrot Sword``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 Carrot Sword.<CR><CR>`5Description:`` Razor sharp, yet oddly tasty. This can carve bunny symbols into your foes! `4Not available any other way!`` Sponsored by MrMehMeh.|0|3|15000|0|||-1|-1||-1|-1||1|||||||";
					string text30 = "|\nadd_button|red_bicycle|`oRed Bicycle``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 Red Bicycle.<CR><CR>`5Description:`` It's the environmentally friendly way to get around! Ride this bicycle at high speed hither and zither throughout Growtopia. `4Not available any other way!``|0|5|30000|0|||-1|-1||-1|-1||1|||||||";
					string text31 = "|\nadd_button|fire_truck|`oFire Truck``|interface/large/store_buttons/store_buttons14.rttex|`2You Get:`` 1 Fire Truck.<CR><CR>`5Description:`` Race to the scene of the fire in this speedy vehicle! `4Not available any other way!``|0|2|50000|0|||-1|-1||-1|-1||1|||||||";
					string text32 = "|\nadd_button|pet_slime|`oPet Slime``|interface/large/store_buttons/store_buttons14.rttex|`2You Get:`` 1 Pet Slime.<CR><CR>`5Description:`` What could be better than a blob of greasy slime that follows you around? How about a blob of greasy slime that follows you around and spits corrosive acid, melting blocks more quickly than a normal punch? `4Not available any other way!``|0|4|100000|0|||-1|-1||-1|-1||1|||||||";
					string text33 = "|\nadd_button|dabstep_shoes|`oDabstep Low Top Sneakers``|interface/large/store_buttons/store_buttons21.rttex|`2You Get:`` 1 Dabstep Low Top Sneakers.<CR><CR>`5Description:`` Light up every footfall and move to a better beat with these dabulous shoes! When you're wearing these, the world is your dance floor! `4Not available any other way!``|0|2|30000|0|||-1|-1||-1|-1||1|||||||";


					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22 + text23 + text24 + text25 + text26 + text27 + text28 + text29 + text30 + text31 + text32 + text33);

				}
				if (cch.find("action|buy\nitem|itempack") == 0) {
					string text1 = "set_description_text|`2Item Packs!``  Select the item you'd like more info on, or BACK to go back.";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||0|0|0|0||||-1|-1||||";
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1||||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||1|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_button|world_lock|`oWorld Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 World Lock.<CR><CR>`5Description:`` Become the undisputed ruler of your domain with one of these babies.  It works like a normal lock except it locks the `$entire world``!  Won't work on worlds that other people already have locks on. You can even add additional normal locks to give access to certain areas to friends. `5It's a perma-item, is never lost when destroyed.``  `wRecycles for 200 Gems.``|0|7|2000|0|||-1|-1||-1|-1||1|||||||";
					string text10 = "|\nadd_button|10_wl|`oWorld Lock Pack``|interface/large/store_buttons/store_buttons18.rttex|`2You Get:`` 10 World Locks.<CR><CR>`5Description:`` 10-pack of World Locks. Become the undisputed ruler of up to TEN kingdoms with these babies. Each works like a normal lock except it locks the `$entire world``!  Won't work on worlds that other people already have locks on. You can even add additional normal locks to give access to certain areas to friends. `5It's a perma-item, is never lost when destroyed.`` `wEach recycles for 200 Gems.``|0|3|20000|0|||-1|-1||-1|-1||1|||||||";
					string text11 = "|\nadd_button|ads_tv|`oGrowShow TV``|interface/large/store_buttons/store_buttons30.rttex|`2You Get:`` 1 GrowShow TV.<CR><CR>`5Description:`` Growtopia's most loved gameshow now brings you its very own TV to watch up to 3 ads per day for AMAZING prizes.|0|4|50|0|||-1|-1||-1|-1||1|||||||";
					string text12 = "|\nadd_button|small_lock|`oSmall Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Small Lock.<CR><CR>`5Description:`` Protect up to `$10`` tiles.  Can add friends to the lock so others can edit that area as well. `5It's a perma-item, is never lost when destroyed.``|1|3|50|0|||-1|-1||-1|-1||1|||||||";
					string text13 = "|\nadd_button|big_lock|`oBig Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Big Lock.<CR><CR>`5Description:`` Protect up to `$48`` tiles.  Can add friends to the lock so others can edit that area as well. `5It's a perma-item, is never lost when destroyed.``|1|1|200|0|||-1|-1||-1|-1||1|||||||";
					string text14 = "|\nadd_button|huge_lock|`oHuge Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Huge Lock.<CR><CR>`5Description:`` Protect up to `$200`` tiles.  Can add friends to the lock so others can edit that area as well. `5It's a perma-item, is never lost when destroyed.``|0|4|500|0|||-1|-1||-1|-1||1|||||||";
					string text15 = "|\nadd_button|door_pack|`oDoor And Sign Hello Pack``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Door and 1 Sign.<CR><CR>`5Description:`` Own your very own door and sign! This pack comes with one of each. Leave cryptic messages and create a door that can open to, well, anywhere.|0|3|15|0|||-1|-1||-1|-1||1|||||||";
					string text16 = "|\nadd_button|door_mover|`oDoor Mover``|interface/large/store_buttons/store_buttons8.rttex|`2You Get:`` 1 Door Mover.<CR><CR>`5Description:`` Unsatisfied with your world's layout?  This one-use device can be used to move the White Door to any new location in your world, provided there are 2 empty spaces for it to fit in. Disappears when used. `2Only usable on a world you have World Locked.``|0|6|5000|0|||-1|-1||-1|-1||1|||||||";
					string text17 = "|\nadd_button|vending_machine|`oVending Machine``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 Vending Machine.<CR><CR>`5Description:`` Tired of interacting with human beings? Try a Vending Machine! You can put a stack of items inside it, set a price in World Locks, and people can buy from the machine while you sit back and rake in the profits! `5It's a perma-item, is never lost when destroyed, and it is not available any other way.``|0|6|8000|0|||-1|-1||-1|-1||1|||||||";
					string text18 = "|\nadd_button|digi_vend|`oDigiVend Machine``|interface/large/store_buttons/store_buttons29.rttex|`2You Get:`` 1 DigiVend Machine.<CR><CR>`5Description:`` Get with the times and go digital! This wired vending machine can connect its contents to Vending Hubs AND the multiversal economy, providing a unified shopping experience along with price checks to help you sell your goods! All that, and still no human-related hassle! Use your wrench on this to stock it with an item and set a price in World Locks. Other players will be able to buy from it! Only works in World-Locked worlds.|0|2|12000|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_button|checkout_counter|`oVending Hub - Checkout Counter``|interface/large/store_buttons/store_buttons29.rttex|`2You Get:`` 1 Vending Hub.<CR><CR>`5Description:`` Your one-stop shop! This vending hub will collect and display (and let shoppers buy) the contents of ALL DigiVends in its row or column (wrench it to set which the direction)! Wow! Now that's a shopping experience we can all enjoy! Note: Only works in World-Locked worlds.|0|3|50000|0|||-1|-1||-1|-1||1|||||||";
					string text20 = "|\nadd_button|change_addr|`oChange of Address``|interface/large/store_buttons/store_buttons12.rttex|`2You Get:`` 1 Change of Address.<CR><CR>`5Description:`` Don't like the name of your world? You can use up one of these to trade your world's name with the name of any other world that you own. You must have a `2World Lock`` in both worlds. Go lock up that empty world with the new name you want and swap away!|0|6|20000|0|||-1|-1||-1|-1||1|||||||";
					string text21 = "|\nadd_button|signal_jammer|`oSignal Jammer``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Signal Jammer.<CR><CR>`5Description:`` Get off the grid! Install a `$Signal Jammer``! A single punch will cause it to whir to life, tireless hiding your world and its population from pesky snoopers - only those who know the world name will be able to enter. `5It's a perma-item, is never lost when destroyed.``|1|6|2000|0|||-1|-1||-1|-1||1|||||||";
					string text22 = "|\nadd_button|punch_jammer|`oPunch Jammer``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Punch Jammer.<CR><CR>`5Description:`` Tired of getting bashed around? Set up a Punch Jammer in your world, and people won't be able to punch each other! Can be turned on and off as needed. `5It's a perma-item, is never lost when destroyed.``|0|4|15000|0|||-1|-1||-1|-1||1|||||||";
					string text23 = "|\nadd_button|zombie_jammer|`oZombie Jammer``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Zombie Jammer.<CR><CR>`5Description:`` Got a parkour or race that you don't want slowed down? Turn this on and nobody can be infected by zombie bites in your world. It does not prevent direct infection by the g-Virus itself though. `5It's a perma-item, is never lost when destroyed.``|0|5|15000|0|||-1|-1||-1|-1||1|||||||";
					string text24 = "|\nadd_button|starship_blast|`oImperial Starship Blast``|interface/large/store_buttons/store_buttons21.rttex|`2You Get:`` 1 Imperial Starship Blast.<CR><CR>`5Description:`` Command your very own Starship and explore the cosmos! This blast contains one of 3 possible Imperial ship types - which will you get? Note: Each Starship comes with a full tank of gas, an Imperial Helm - Mk. I, Imperial Reactor - Mk. I and an Imperial Viewscreen - Mk. I, so you'll be all set for your adventure among the stars! Note: A Starship also comes with an assortment of space-age blocks!|0|1|20000|0|||-1|-1||-1|-1||1|||||||";
					string text25 = "|\nadd_button|surg_blast|`oSurgWorld Blast``|interface/large/store_buttons/store_buttons27.rttex|`2You Get:`` 1 SurgWorld Blast and 1 Caduceaxe.<CR><CR>`5Description:`` Your gateway to a world of medical wonders! SurgWorld is a place of care and healing, with all kinds of interesting blocks, top tips on how to treat people with surgery, and an increased chance of getting maladies while you work! Also comes with 1 Caduceaxe to extract Vaccine Drops from blocks. `6Warning:`` May break when extracting vaccine.|0|2|10000|0|||-1|-1||-1|-1||1|||||||";
					string text26 = "|\nadd_button|bountiful_blast|`oBountiful Blast``|interface/large/store_buttons/store_buttons27.rttex|`2You Get:`` 1 Bountiful Blast.<CR><CR>`5Description:`` Enter a world of fertile soil, cheerful sunshine and lush green hills, and bountiful new trees! This blast is your ticket to a different kind of farming!|0|3|5000|0|||-1|-1||-1|-1||1|||||||";
					string text27 = "|\nadd_button|thermo_blast|`oThermonuclear Blast``|interface/large/store_buttons/store_buttons8.rttex|`2You Get:`` 1 Thermonuclear Blast.<CR><CR>`5Description:`` This supervillainous device will blast you to a new world that has been scoured completely empty - it contains nothing but Bedrock and a White Door. Remember: When using this, you are creating a NEW world by typing in a new name. It would be irresponsible to let you blow up an entire existing world.|0|5|15000|0|||-1|-1||-1|-1||1|||||||";
					string text28 = "|\nadd_button|antigravity_generator|`oAntigravity Generator``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Antigravity Generator.<CR><CR>`5Description:`` Disables gravity in your world when activated! Well, it reduces gravity, and lets everybody jump as much as they want! `5It's a perma-item - never lost when destroyed! `4Not available any other way!````|0|3|450000|0|||-1|-1||-1|-1||1|||||||";
					string text29 = "|\nadd_button|building_blocks_machine|`oBuilding Blocks Machine``|interface/large/store_buttons/store_buttons26.rttex|`2You Get:`` 1 Building Blocks Machine.<CR><CR>`5Description:`` Eager to add some new building materials to your construction stockpile? Tired of collecting them from random worlds and weirdos? Well, pop this beauty in your world and it'll start cranking out awesome blocks in no time! Contains the `5RARE Creepy Baby Block and Digital Dirt`` amongst a heap of other new blocks! Careful, though - blocks don't just come from nothing, and this machine will eventually run out of power once it makes a bunch!|0|3|8000|0|||-1|-1||-1|-1||1|||||||";
					string text30 = "|\nadd_button|builders_lock|`oBuilder's Lock``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Builders Lock.<CR><CR>`5Description:`` Protect up to `$200`` tiles. Wrench the lock to limit it - it can either only allow building, or only allow breaking! `5It's a perma-item, is never lost when destroyed.``|0|2|50000|0|||-1|-1||-1|-1||1|||||||";
					string text31 = "|\nadd_button|weather_sunny|`oWeather Machine - Sunny``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Weather Machine - Sunny.<CR><CR>`5Description:`` You probably don't need this one... but if you ever have a desire to turn a sunset or desert world back to normal, grab a Sunny Weather Machine to restore the default Growtopia sky! `5It's a perma-item, is never lost when destroyed.``|0|5|1000|0|||-1|-1||-1|-1||1|||||||";
					string text32 = "|\nadd_button|weather_night|`oWeather Machine - Night``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Weather Machine - Night.<CR><CR>`5Description:`` You might not call it weather, but we do! This will turn the background of your world into a lovely night scene with stars and moon. `5It's a perma-item, is never lost when destroyed.``|0|6|10000|0|||-1|-1||-1|-1||1|||||||";
					string text33 = "|\nadd_button|weather_arid|`oWeather Machine - Arid``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Weather Machine - Arid.<CR><CR>`5Description:`` Want your world to look like a cartoon desert? This will turn the background of your world into a desert scene with all the trimmings. `5It's a perma-item, is never lost when destroyed.``|0|7|10000|0|||-1|-1||-1|-1||1|||||||";
					string text34 = "|\nadd_button|weather_rainy|`oWeather Machine - Rainy City``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Weather Machine - Rainy City.<CR><CR>`5Description:`` This will turn the background of your world into a dark, rainy city scene complete with sound effects. `5It's a perma-item, is never lost when destroyed.``|0|5|10000|0|||-1|-1||-1|-1||1|||||||";
					string text35 = "|\nadd_button|weather_warp|`oWeather Machine - Warp Speed``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 Weather Machine - Warp Speed.<CR><CR>`5Description:`` This Weather Machine will launch your world through space at relativistic speeds, which will cause you to age more slowly, as well as see stars flying by rapidly in the background. `5It's a perma-item, is never lost when destroyed.``|0|3|10000|0|||-1|-1||-1|-1||1|||||||";
					string text36 = "|\nadd_button|mars_blast|`oMars Blast``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Mars Blast.<CR><CR>`5Description:`` Blast off to Mars!  This powerful rocket ship will launch you to a new world set up like the surface of Mars, with a special martian sky background, and unique terrain not found elsewhere in the solar system. Mars even has lower gravity than Growtopia normally does! Remember: When using this, you are creating a NEW world by typing in a new name. You can't convert an existing world to Mars, that would be dangerous.|0|7|15000|0|||-1|-1||-1|-1||1|||||||";
					string text37 = "|\nadd_button|undersea_blast|`oUndersea Blast``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Undersea Blast.<CR><CR>`5Description:`` Explore the ocean!  This advanced device will terraform a new world set up like the bottom of the ocean, with a special ocean background, and special blocks like Seaweed, Coral, Jellyfish, Sharks, and maybe a special surprise... Remember, by using this you are creating a NEW world by typing in a new name. You can't convert an existing world to an ocean, that would be dangerous.|0|7|15000|0|||-1|-1||-1|-1||1|||||||";
					string text38 = "|\nadd_button|cave_blast|`oCave Blast``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Cave Blast.<CR><CR>`5Description:`` This explosive device will punch a hole in the ground, giving you a dark cavern to explore. There are even rumors of treasure and the entrance to ancient mines, hidden deep in the caves... but make sure you bring a World Lock. The blasted world is not locked when it's created, so lock it before somebody shows up! Remember: When using this, you are creating a NEW world by typing in a new name. You can't convert an existing world to a cave, that would be dangerous.|0|2|30000|0|||-1|-1||-1|-1||1|||||||";
					string text39 = "|\nadd_button|weather_stuff|`oWeather Machine - Stuff``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Weather Machine - Stuff.<CR><CR>`5Description:`` This is the most fun weather imaginable - Choose any item from your inventory, adjust some settings, and watch it rain down from the sky! Or up, if you prefer reversing the gravity. `5It's a perma-item, is never lost when destroyed.``|0|6|50000|0|||-1|-1||-1|-1||1|||||||";
					string text40 = "|\nadd_button|weather_jungle|`oWeather Machine - Jungle``|interface/large/store_buttons/store_buttons16.rttex|`2You Get:`` 1 Weather Machine - Jungle.<CR><CR>`5Description:`` This weather machine will turn the background of your world into a steamy jungle. `5It's a perma-item, is never lost when destroyed.``|0|5|20000|0|||-1|-1||-1|-1||1|||||||";
					string text41 = "|\nadd_button|weather_backgd|`oWeather Machine - Background``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Weather Machine - Background.<CR><CR>`5Description:`` This amazing device can scan any Background Block, and will make your entire world look like it's been filled with that block. Also handy for hiding music notes! `5It's a perma-item, is never lost when destroyed.``|0|1|150000|0|||-1|-1||-1|-1||1|||||||";
					string text42 = "|\nadd_button|digital_rain_weather|`oWeather Machine - Digital Rain``|interface/large/store_buttons/store_buttons22.rttex|`2You Get:`` 1 Weather Machine - Digital Rain.<CR><CR>`5Description:`` Take the grow pill, and we'll show you how deep the rabbit hole goes! Splash the scrolling code of creation across the skies of your worlds. They say you learn to understand it after a while... Note: You can only have one of these per world. `5It's a perma-item, is never lost when destroyed.``|0|6|30000|0|||-1|-1||-1|-1||1|||||||";
					string text43 = "|\nadd_button|treasure_blast|`oTreasure Blast``|interface/large/store_buttons/store_buttons26.rttex|`2You Get:`` 1 Treasure Blast.<CR><CR>`5Description:`` Enter a world of snow-capped peaks and long-forgotten mysteries! Riddles and secrets - and a ton of treasure - await those who brave this blast's blocks! Remember, when you use this, it'll create a new world by typing in a new name! No sense in searching for clues to great treasures in well-trod worlds, is there?|0|6|10000|0|||-1|-1||-1|-1||1|||||||";


					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22 + text23 + text24 + text25 + text26 + text27 + text28 + text29 + text30 + text31 + text32 + text33 + text34 + text35 + text36 + text37 + text38 + text39 + text40 + text41 + text42 + text43);

				}
				if (cch.find("action|buy\nitem|bigitems") == 0) {
					string text1 = "set_description_text|`2Item Packs!``  Select the item you'd like more info on, or BACK to go back.";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||0|0|0|0||||-1|-1||||";
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1||||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||1|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_button|world_lock|`oWorld Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 World Lock.<CR><CR>`5Description:`` Become the undisputed ruler of your domain with one of these babies.  It works like a normal lock except it locks the `$entire world``!  Won't work on worlds that other people already have locks on. You can even add additional normal locks to give access to certain areas to friends. `5It's a perma-item, is never lost when destroyed.``  `wRecycles for 200 Gems.``|0|7|2000|0|||-1|-1||-1|-1||1|||||||";
					string text10 = "|\nadd_button|10_wl|`oWorld Lock Pack``|interface/large/store_buttons/store_buttons18.rttex|`2You Get:`` 10 World Locks.<CR><CR>`5Description:`` 10-pack of World Locks. Become the undisputed ruler of up to TEN kingdoms with these babies. Each works like a normal lock except it locks the `$entire world``!  Won't work on worlds that other people already have locks on. You can even add additional normal locks to give access to certain areas to friends. `5It's a perma-item, is never lost when destroyed.`` `wEach recycles for 200 Gems.``|0|3|20000|0|||-1|-1||-1|-1||1|||||||";
					string text11 = "|\nadd_button|ads_tv|`oGrowShow TV``|interface/large/store_buttons/store_buttons30.rttex|`2You Get:`` 1 GrowShow TV.<CR><CR>`5Description:`` Growtopia's most loved gameshow now brings you its very own TV to watch up to 3 ads per day for AMAZING prizes.|0|4|50|0|||-1|-1||-1|-1||1|||||||";
					string text12 = "|\nadd_button|small_lock|`oSmall Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Small Lock.<CR><CR>`5Description:`` Protect up to `$10`` tiles.  Can add friends to the lock so others can edit that area as well. `5It's a perma-item, is never lost when destroyed.``|1|3|50|0|||-1|-1||-1|-1||1|||||||";
					string text13 = "|\nadd_button|big_lock|`oBig Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Big Lock.<CR><CR>`5Description:`` Protect up to `$48`` tiles.  Can add friends to the lock so others can edit that area as well. `5It's a perma-item, is never lost when destroyed.``|1|1|200|0|||-1|-1||-1|-1||1|||||||";
					string text14 = "|\nadd_button|huge_lock|`oHuge Lock``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Huge Lock.<CR><CR>`5Description:`` Protect up to `$200`` tiles.  Can add friends to the lock so others can edit that area as well. `5It's a perma-item, is never lost when destroyed.``|0|4|500|0|||-1|-1||-1|-1||1|||||||";
					string text15 = "|\nadd_button|door_pack|`oDoor And Sign Hello Pack``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Door and 1 Sign.<CR><CR>`5Description:`` Own your very own door and sign! This pack comes with one of each. Leave cryptic messages and create a door that can open to, well, anywhere.|0|3|15|0|||-1|-1||-1|-1||1|||||||";
					string text16 = "|\nadd_button|door_mover|`oDoor Mover``|interface/large/store_buttons/store_buttons8.rttex|`2You Get:`` 1 Door Mover.<CR><CR>`5Description:`` Unsatisfied with your world's layout?  This one-use device can be used to move the White Door to any new location in your world, provided there are 2 empty spaces for it to fit in. Disappears when used. `2Only usable on a world you have World Locked.``|0|6|5000|0|||-1|-1||-1|-1||1|||||||";
					string text17 = "|\nadd_button|vending_machine|`oVending Machine``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 Vending Machine.<CR><CR>`5Description:`` Tired of interacting with human beings? Try a Vending Machine! You can put a stack of items inside it, set a price in World Locks, and people can buy from the machine while you sit back and rake in the profits! `5It's a perma-item, is never lost when destroyed, and it is not available any other way.``|0|6|8000|0|||-1|-1||-1|-1||1|||||||";
					string text18 = "|\nadd_button|digi_vend|`oDigiVend Machine``|interface/large/store_buttons/store_buttons29.rttex|`2You Get:`` 1 DigiVend Machine.<CR><CR>`5Description:`` Get with the times and go digital! This wired vending machine can connect its contents to Vending Hubs AND the multiversal economy, providing a unified shopping experience along with price checks to help you sell your goods! All that, and still no human-related hassle! Use your wrench on this to stock it with an item and set a price in World Locks. Other players will be able to buy from it! Only works in World-Locked worlds.|0|2|12000|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_button|checkout_counter|`oVending Hub - Checkout Counter``|interface/large/store_buttons/store_buttons29.rttex|`2You Get:`` 1 Vending Hub.<CR><CR>`5Description:`` Your one-stop shop! This vending hub will collect and display (and let shoppers buy) the contents of ALL DigiVends in its row or column (wrench it to set which the direction)! Wow! Now that's a shopping experience we can all enjoy! Note: Only works in World-Locked worlds.|0|3|50000|0|||-1|-1||-1|-1||1|||||||";
					string text20 = "|\nadd_button|change_addr|`oChange of Address``|interface/large/store_buttons/store_buttons12.rttex|`2You Get:`` 1 Change of Address.<CR><CR>`5Description:`` Don't like the name of your world? You can use up one of these to trade your world's name with the name of any other world that you own. You must have a `2World Lock`` in both worlds. Go lock up that empty world with the new name you want and swap away!|0|6|20000|0|||-1|-1||-1|-1||1|||||||";
					string text21 = "|\nadd_button|signal_jammer|`oSignal Jammer``|interface/large/store_buttons/store_buttons.rttex|`2You Get:`` 1 Signal Jammer.<CR><CR>`5Description:`` Get off the grid! Install a `$Signal Jammer``! A single punch will cause it to whir to life, tireless hiding your world and its population from pesky snoopers - only those who know the world name will be able to enter. `5It's a perma-item, is never lost when destroyed.``|1|6|2000|0|||-1|-1||-1|-1||1|||||||";
					string text22 = "|\nadd_button|punch_jammer|`oPunch Jammer``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Punch Jammer.<CR><CR>`5Description:`` Tired of getting bashed around? Set up a Punch Jammer in your world, and people won't be able to punch each other! Can be turned on and off as needed. `5It's a perma-item, is never lost when destroyed.``|0|4|15000|0|||-1|-1||-1|-1||1|||||||";
					string text23 = "|\nadd_button|zombie_jammer|`oZombie Jammer``|interface/large/store_buttons/store_buttons7.rttex|`2You Get:`` 1 Zombie Jammer.<CR><CR>`5Description:`` Got a parkour or race that you don't want slowed down? Turn this on and nobody can be infected by zombie bites in your world. It does not prevent direct infection by the g-Virus itself though. `5It's a perma-item, is never lost when destroyed.``|0|5|15000|0|||-1|-1||-1|-1||1|||||||";
					string text24 = "|\nadd_button|starship_blast|`oImperial Starship Blast``|interface/large/store_buttons/store_buttons21.rttex|`2You Get:`` 1 Imperial Starship Blast.<CR><CR>`5Description:`` Command your very own Starship and explore the cosmos! This blast contains one of 3 possible Imperial ship types - which will you get? Note: Each Starship comes with a full tank of gas, an Imperial Helm - Mk. I, Imperial Reactor - Mk. I and an Imperial Viewscreen - Mk. I, so you'll be all set for your adventure among the stars! Note: A Starship also comes with an assortment of space-age blocks!|0|1|20000|0|||-1|-1||-1|-1||1|||||||";
					string text25 = "|\nadd_button|surg_blast|`oSurgWorld Blast``|interface/large/store_buttons/store_buttons27.rttex|`2You Get:`` 1 SurgWorld Blast and 1 Caduceaxe.<CR><CR>`5Description:`` Your gateway to a world of medical wonders! SurgWorld is a place of care and healing, with all kinds of interesting blocks, top tips on how to treat people with surgery, and an increased chance of getting maladies while you work! Also comes with 1 Caduceaxe to extract Vaccine Drops from blocks. `6Warning:`` May break when extracting vaccine.|0|2|10000|0|||-1|-1||-1|-1||1|||||||";
					string text26 = "|\nadd_button|bountiful_blast|`oBountiful Blast``|interface/large/store_buttons/store_buttons27.rttex|`2You Get:`` 1 Bountiful Blast.<CR><CR>`5Description:`` Enter a world of fertile soil, cheerful sunshine and lush green hills, and bountiful new trees! This blast is your ticket to a different kind of farming!|0|3|5000|0|||-1|-1||-1|-1||1|||||||";
					string text27 = "|\nadd_button|thermo_blast|`oThermonuclear Blast``|interface/large/store_buttons/store_buttons8.rttex|`2You Get:`` 1 Thermonuclear Blast.<CR><CR>`5Description:`` This supervillainous device will blast you to a new world that has been scoured completely empty - it contains nothing but Bedrock and a White Door. Remember: When using this, you are creating a NEW world by typing in a new name. It would be irresponsible to let you blow up an entire existing world.|0|5|15000|0|||-1|-1||-1|-1||1|||||||";
					string text28 = "|\nadd_button|antigravity_generator|`oAntigravity Generator``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Antigravity Generator.<CR><CR>`5Description:`` Disables gravity in your world when activated! Well, it reduces gravity, and lets everybody jump as much as they want! `5It's a perma-item - never lost when destroyed! `4Not available any other way!````|0|3|450000|0|||-1|-1||-1|-1||1|||||||";
					string text29 = "|\nadd_button|building_blocks_machine|`oBuilding Blocks Machine``|interface/large/store_buttons/store_buttons26.rttex|`2You Get:`` 1 Building Blocks Machine.<CR><CR>`5Description:`` Eager to add some new building materials to your construction stockpile? Tired of collecting them from random worlds and weirdos? Well, pop this beauty in your world and it'll start cranking out awesome blocks in no time! Contains the `5RARE Creepy Baby Block and Digital Dirt`` amongst a heap of other new blocks! Careful, though - blocks don't just come from nothing, and this machine will eventually run out of power once it makes a bunch!|0|3|8000|0|||-1|-1||-1|-1||1|||||||";
					string text30 = "|\nadd_button|builders_lock|`oBuilder's Lock``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Builders Lock.<CR><CR>`5Description:`` Protect up to `$200`` tiles. Wrench the lock to limit it - it can either only allow building, or only allow breaking! `5It's a perma-item, is never lost when destroyed.``|0|2|50000|0|||-1|-1||-1|-1||1|||||||";
					string text31 = "|\nadd_button|weather_sunny|`oWeather Machine - Sunny``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Weather Machine - Sunny.<CR><CR>`5Description:`` You probably don't need this one... but if you ever have a desire to turn a sunset or desert world back to normal, grab a Sunny Weather Machine to restore the default Growtopia sky! `5It's a perma-item, is never lost when destroyed.``|0|5|1000|0|||-1|-1||-1|-1||1|||||||";
					string text32 = "|\nadd_button|weather_night|`oWeather Machine - Night``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Weather Machine - Night.<CR><CR>`5Description:`` You might not call it weather, but we do! This will turn the background of your world into a lovely night scene with stars and moon. `5It's a perma-item, is never lost when destroyed.``|0|6|10000|0|||-1|-1||-1|-1||1|||||||";
					string text33 = "|\nadd_button|weather_arid|`oWeather Machine - Arid``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Weather Machine - Arid.<CR><CR>`5Description:`` Want your world to look like a cartoon desert? This will turn the background of your world into a desert scene with all the trimmings. `5It's a perma-item, is never lost when destroyed.``|0|7|10000|0|||-1|-1||-1|-1||1|||||||";
					string text34 = "|\nadd_button|weather_rainy|`oWeather Machine - Rainy City``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Weather Machine - Rainy City.<CR><CR>`5Description:`` This will turn the background of your world into a dark, rainy city scene complete with sound effects. `5It's a perma-item, is never lost when destroyed.``|0|5|10000|0|||-1|-1||-1|-1||1|||||||";
					string text35 = "|\nadd_button|weather_warp|`oWeather Machine - Warp Speed``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 Weather Machine - Warp Speed.<CR><CR>`5Description:`` This Weather Machine will launch your world through space at relativistic speeds, which will cause you to age more slowly, as well as see stars flying by rapidly in the background. `5It's a perma-item, is never lost when destroyed.``|0|3|10000|0|||-1|-1||-1|-1||1|||||||";
					string text36 = "|\nadd_button|mars_blast|`oMars Blast``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Mars Blast.<CR><CR>`5Description:`` Blast off to Mars!  This powerful rocket ship will launch you to a new world set up like the surface of Mars, with a special martian sky background, and unique terrain not found elsewhere in the solar system. Mars even has lower gravity than Growtopia normally does! Remember: When using this, you are creating a NEW world by typing in a new name. You can't convert an existing world to Mars, that would be dangerous.|0|7|15000|0|||-1|-1||-1|-1||1|||||||";
					string text37 = "|\nadd_button|undersea_blast|`oUndersea Blast``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Undersea Blast.<CR><CR>`5Description:`` Explore the ocean!  This advanced device will terraform a new world set up like the bottom of the ocean, with a special ocean background, and special blocks like Seaweed, Coral, Jellyfish, Sharks, and maybe a special surprise... Remember, by using this you are creating a NEW world by typing in a new name. You can't convert an existing world to an ocean, that would be dangerous.|0|7|15000|0|||-1|-1||-1|-1||1|||||||";
					string text38 = "|\nadd_button|cave_blast|`oCave Blast``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Cave Blast.<CR><CR>`5Description:`` This explosive device will punch a hole in the ground, giving you a dark cavern to explore. There are even rumors of treasure and the entrance to ancient mines, hidden deep in the caves... but make sure you bring a World Lock. The blasted world is not locked when it's created, so lock it before somebody shows up! Remember: When using this, you are creating a NEW world by typing in a new name. You can't convert an existing world to a cave, that would be dangerous.|0|2|30000|0|||-1|-1||-1|-1||1|||||||";
					string text39 = "|\nadd_button|weather_stuff|`oWeather Machine - Stuff``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Weather Machine - Stuff.<CR><CR>`5Description:`` This is the most fun weather imaginable - Choose any item from your inventory, adjust some settings, and watch it rain down from the sky! Or up, if you prefer reversing the gravity. `5It's a perma-item, is never lost when destroyed.``|0|6|50000|0|||-1|-1||-1|-1||1|||||||";
					string text40 = "|\nadd_button|weather_jungle|`oWeather Machine - Jungle``|interface/large/store_buttons/store_buttons16.rttex|`2You Get:`` 1 Weather Machine - Jungle.<CR><CR>`5Description:`` This weather machine will turn the background of your world into a steamy jungle. `5It's a perma-item, is never lost when destroyed.``|0|5|20000|0|||-1|-1||-1|-1||1|||||||";
					string text41 = "|\nadd_button|weather_backgd|`oWeather Machine - Background``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Weather Machine - Background.<CR><CR>`5Description:`` This amazing device can scan any Background Block, and will make your entire world look like it's been filled with that block. Also handy for hiding music notes! `5It's a perma-item, is never lost when destroyed.``|0|1|150000|0|||-1|-1||-1|-1||1|||||||";
					string text42 = "|\nadd_button|digital_rain_weather|`oWeather Machine - Digital Rain``|interface/large/store_buttons/store_buttons22.rttex|`2You Get:`` 1 Weather Machine - Digital Rain.<CR><CR>`5Description:`` Take the grow pill, and we'll show you how deep the rabbit hole goes! Splash the scrolling code of creation across the skies of your worlds. They say you learn to understand it after a while... Note: You can only have one of these per world. `5It's a perma-item, is never lost when destroyed.``|0|6|30000|0|||-1|-1||-1|-1||1|||||||";
					string text43 = "|\nadd_button|treasure_blast|`oTreasure Blast``|interface/large/store_buttons/store_buttons26.rttex|`2You Get:`` 1 Treasure Blast.<CR><CR>`5Description:`` Enter a world of snow-capped peaks and long-forgotten mysteries! Riddles and secrets - and a ton of treasure - await those who brave this blast's blocks! Remember, when you use this, it'll create a new world by typing in a new name! No sense in searching for clues to great treasures in well-trod worlds, is there?|0|6|10000|0|||-1|-1||-1|-1||1|||||||";


					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22 + text23 + text24 + text25 + text26 + text27 + text28 + text29 + text30 + text31 + text32 + text33 + text34 + text35 + text36 + text37 + text38 + text39 + text40 + text41 + text42 + text43);

				}
				if (cch.find("action|buy\nitem|weather") == 0) {
					string text1 = "set_description_text|`2Weather Machines!``  Select the item you'd like more info on, or BACK to go back.";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||0|0|0|0||||-1|-1||||";
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1||||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||0|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|1|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_button|race_pack|`oRacing Action Pack``|interface/large/store_buttons/store_buttons2.rttex|`2You Get:`` 1 Racing Start Flag, 1 Racing End Flag, 2 Checkpoints, 2 Big Old Sideways Arrows, 1 Big Old Up Arrow, 1 Big Old Down Arrow, 1 WristBand, 1 HeadBand, 1 Sports Ball Jersey and 1 Air Robinsons.<CR><CR>`5Description:`` Get all you need to host races in your worlds! You'll win the races too, with new Air Robinsons that make you run faster!|0|7|3500|0|||-1|-1||-1|-1||1|||||||";
					string text10 = "|\nadd_button|music_pack|`oComposer's Pack``|interface/large/store_buttons/store_buttons3.rttex|`2You Get:`` 20 Sheet Music: Blank, 20 Sheet Music: Piano Note, 20 Sheet Music: Bass Note, 20 Sheet Music Drums, 5 Sheet Music: Sharp Piano, 5 Sheet Music: Flat Piano, 5 Sheet Music: Flat Bass and 5 Sheet Music: Sharp Bass .<CR><CR>`5Description:`` With these handy blocks, you'll be able to compose your own music, using your World-Locked world as a sheet of music. Requires a World Lock (sold separately!).|0|0|5000|0|||-1|-1||-1|-1||1|||||||";
					string text11 = "|\nadd_button|school_pack|`oEducation Pack``|interface/large/store_buttons/store_buttons4.rttex|`2You Get:`` 10 ChalkBoards, 3 School Desks, 20 Red Bricks, 1 Bulletin Board, 10 Pencils, 1 Growtopia Lunchbox, 1 Grey Hair Bun, 1 Apple and 1 Random School Uniform Item.<CR><CR>`5Description:`` If you want to build a school in Growtopia, here's what you need!|0|0|5000|0|||-1|-1||-1|-1||1|||||||";
					string text12 = "|\nadd_button|dungeon_pack|`oDungeon Pack``|interface/large/store_buttons/store_buttons4.rttex|`2You Get:`` 20 Grimstone, 20 Blackrock Wall, 20 Iron Bars, 3 Jail Doors, 3 Skeletons, 1 Headsman's Axe, 1 Worthless Rags. 5 Torches and a `#Rare Iron Mask!``.<CR><CR>`5Description:`` Lock up your enemies in a dank dungeon! Of course they can still leave whenever they want. But they won't want to, because it looks so cool! Iron Mask muffles your speech!|0|1|10000|0|||-1|-1||-1|-1||1|||||||";
					string text13 = "|\nadd_button|fantasy_pack|`oFantasy Pack``|interface/large/store_buttons/store_buttons3.rttex|`2You Get:`` 1 Mystical Wizard Hat Seed, 1 Wizards Robe, 1 Golden Sword, 1 Elvish Longbow, 10 Barrels, 3 Tavern Signs, 3 Treasure Chests and 3 Dragon Gates.<CR><CR>`5Description:`` Hear ye, hear ye! It's a pack of magical wonders!|0|6|5000|0|||-1|-1||-1|-1||1|||||||";
					string text14 = "|\nadd_button|vegas_pack|`oVegas Pack``|interface/large/store_buttons/store_buttons4.rttex|`2You Get:`` 10 Neon Lights, 1 Card Block Seed, 1 `#Rare Pink Cadillac`` 4 Flipping Coins, 1 Dice Block, 1 Gamblers Visor, 1 Slot Machine, 1 Roulette Wheel and 1 Showgirl Hat, 1 Showgirl top and 1 Showgirl Leggins.<CR><CR>`5Description:`` What happens in Growtopia stays in Growtopia!|0|5|20000|0|||-1|-1||-1|-1||1|||||||";
					string text15 = "|\nadd_button|farm_pack|`oFarm Pack``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Cow, 1 Chicken, 10 Wheat, 10 Barn Block, 10 Red Wood Walls, 1 Barn Door, 1 Straw Hat, 1 Overalls, 1 Pitchfork, 1 Farmgirl Hair, 1 `#Rare`` `2Dear John Tractor``.<CR><CR>`5Description:`` Put the `2Grow`` in Growtopia with this pack, including a Cow you can milk, a Chicken that lays eggs and a farmer's outfit. Best of all? You get a `#Rare`` `2Dear John Tractor`` you can ride that will mow down trees!|0|0|15000|0|||-1|-1||-1|-1||1|||||||";
					string text16 = "|\nadd_button|science_pack|`oMad Science Kit``|interface/large/store_buttons/store_buttons5.rttex|`2You Get:`` 1 Science Station, 1 Laboratory, 1 LabCoat, 1 Combover Hair, 1 Goggles, 5 Chemical 5, 10 Chemical G, 5 Chemical Y, 5 Chemical B, 5 Chemical P and 1 `#Rare`` `2Death Ray``.<CR><CR>`5Description:`` It's SCIENCE! Defy the natural order with a Science Station that produces chemicals, a Laboratory in which to mix them and a full outfit to do so safely! You'll also get a starter pack of assorted chemicals. Mix them up! Special bonus: A `#Rare`` `2Death Ray`` to make your science truly mad!|0|3|5000|0|||-1|-1||-1|-1||1|||||||";
					string text17 = "|\nadd_button|city_pack|`oCity Pack``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 10 Sidewalks, 3 Street Signs, 3 Streetlamps, 10 Gothic Building tiles, 10 Tenement Building tiles, 10 Fire Escapes, 3 Gargoyles, 10 Hedges, 1 Blue Mailbox, 1 Fire Hydrant and A `#Rare`` `2ATM Machine``.<CR><CR>`5Description:`` Life in the big city is rough but a `#Rare`` `2ATM Machine`` that dishes out gems once a day is very nice!|0|0|8000|0|||-1|-1||-1|-1||1|||||||";
					string text18 = "|\nadd_button|west_pack|`oWild West Pack``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Cowboy Hat, 1 Cowboy Boots, 1 War Paint, 1 Face Bandana, 1 Sheriff Vest, 1 Layer Cake Dress,  1 Corset, 1 Kansas Curls, 10 Western Building 1 Saloon Doors, 5 Western Banners, 1 Buffalo, 10 Rustic Fences, 1 Campfire and 1 Parasol.<CR><CR>`5Description:`` Yippee-kai-yay! This pack includes everything you need to have wild time in the wild west! The Campfire plays cowboy music, and the `#Parasol`` lets you drift down slowly. Special bonus: A `#Rare`` `2Six Shooter`` to blast criminals with!|0|2|8000|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_button|astro_pack|`oAstro Pack``|interface/large/store_buttons/store_buttons6.rttex|`2You Get:`` 1 Astronaut Helmet, 1 Space Suit, 1 Space Pants, 1 Moon Boots, 1 Rocket Thruster, 1 Solar Panel, 6 Space Connectors, 1 Porthole, 1 Compu Panel, 1 Forcefield and 1 `#Rare`` `2Zorbnik DNA``.<CR><CR>`5Description:`` Boldly go where no Growtopian has gone before with an entire Astronaut outfit. As a special bonus, you can have this `#Rare`` `2Zorbnik DNA`` we found on a distant planet. It doesn't do anything by itself, but by trading with your friends, you can collect 10 of them, and then... well, who knows?|0|6|5000|0|||-1|-1||-1|-1||1|||||||";
					string text20 = "|\nadd_button|prehistoric_pack|`oPrehistoric Pack``|interface/large/store_buttons/store_buttons8.rttex|`2You Get:`` 1 Caveman Club, 1 Cave Woman Hair, 1 Caveman Hair, 1 Sabertooth Toga, 1 Fuzzy Bikini Top, 1 Fuzzy Bikni Bottom, 1 Cavewoman Outfit, 10 Cliffside, 5 Rock Platforms, 1 Cave Entrance, 3 Prehistoric Palms and 1 `#Rare Sabertooth Growtopian``.<CR><CR>`5Description:`` Travel way back in time with this pack, including full Caveman and Cavewoman outfits and `#Rare Sabertooth Growtopian`` (that's a mask of sorts). Unleash your inner monster!|0|0|5000|0|||-1|-1||-1|-1||1|||||||";
					string text21 = "|\nadd_button|shop_pack|`oShop Pack``|interface/large/store_buttons/store_buttons8.rttex|`2You Get:`` 4 Display Boxes, 1 For Sale Sign, 1 Gem Sign, 1 Exclamation Sign, 1 Shop Sign, 1 Open Sign, 1 Cash Register, 1 Mannequin and 1 Security Camera.<CR><CR>`5Description:`` Run a fancy shop with these new items! Advertise your wares with an Open/Closed Sign you can switch with a punch, a Cash Register, a Mannequin you can dress up to show off clothing, and a `#Rare`` Security Camera, which reports when people enter and take items!|0|7|10000|0|||-1|-1||-1|-1||1|||||||";
					string text22 = "|\nadd_button|home_pack|`oHome Pack``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Television, 4 Couches, 2 Curtains, 1 Wall Clock, 1 Microwave, 1 Meaty Apron, 1 Ducky Pants, 1 Ducky top and 1 Eggs Benedict.<CR><CR>`5Description:`` Welcome home to Growtopia! Decorate with a Television, Window Curtains, Couches, a `#Rare`` Wall Clock that actually tells time, and a Microwave to cook in. Then dress up in a Meaty Apron and Ducky Pajamas to sit down and eat Eggs Benedict, which increases the amount of XP you earn!|0|6|5000|0|||-1|-1||-1|-1||1|||||||";
					string text23 = "|\nadd_button|cinema_pack|`oCinema Pack``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 ClapBoard, 1 Black Beret, 1 3D Glasses, 6 Theater Curtains, 6 Marquess Blocks, 1 Directors Chair, 4 Theater Seats, 6 Movie Screens, 1 Movie Camera and 1 `#Rare GHX Speaker``.<CR><CR>`5Description:`` It's movie time! Everything you need for the big screen experience including a `#Rare GHX Speaker`` that plays the score from Growtopia: The Movie.|0|2|6000|0|||-1|-1||-1|-1||1|||||||";
					string text24 = "|\nadd_button|adventure_pack|`oAdventure Pack``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 4 Gateways to Adventure, 4 Path Markers, 1 Lazy Cobra, 1 Adventure Brazier, 4 Adventure Barriers, 1 Rope, 1 Torch, 1 Key, 1 Golden Idol, 1 `#Rare Adventuring Mustache``, 1 Explorer's Ponytail and 1 Sling Bag .<CR><CR>`5Description:`` Join Dr. Exploro and her father (also technically Dr. Exploro) as they seek out adventure! You can make your own adventure maps with the tools in this pack.|0|7|20000|0|||-1|-1||-1|-1||1|||||||";
					string text25 = "|\nadd_button|rockin_pack|`oRockin' Pack``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 3 `#Rare Musical Instruments`` Including A Keytar, a Bass Guitar and Tambourine, 1 Starchild Make Up, 1 Rockin' Headband, 1 Leopard Leggings, 1 Shredded Ts-Shirt, 1 Drumkit, 6 Stage Supports, 6 Mega Rock Speakers and 6 Rock n' Roll Wallpaper.<CR><CR>`5Description:`` ROCK N' ROLL!!! Play live music in-game! We Formed a Band! Growtopia makes me want to rock out.|0|0|9999|0|||-1|-1||-1|-1||1|||||||";
					string text26 = "|\nadd_button|game_pack|`oGame Pack``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 `#Rare Game Generator``,  4 Game Blocks, 4 Game Flags, 4 Game Graves and 4 Game Goals.<CR><CR>`5Description:`` Growtopia's not all trading and socializing! Create games for your friends with the Game Pack (and a lot of elbow grease).|0|6|50000|0|||-1|-1||-1|-1||1|||||||";
					string text27 = "|\nadd_button|superhero|`oSuperhero Pack``|interface/large/store_buttons/store_buttons12.rttex|`2You Get:`` 1 Mask, 1 Shirt, 1 Boots, 1 Tights, 1 Cape, `#Rare Super Logos`` or `#Rare Utility Belt`` and 1 `2Phone Booth``.<CR><CR>`5Description:`` Battle the criminal element in Growtopia with a complete random superhero outfit including a cape that lets you double jump. Each of these items comes in one of six random colors. You also get one of 5 `#Rare`` Super Logos, which automatically match the color of any shirt you wear or a `#Rare`` Utility Belt... of course use the `2Phone Booth`` to change into your secret identity!|0|0|10000|0|||-1|-1||-1|-1||1|||||||";
					string text28 = "|\nadd_button|fashion_pack|`oFashion Pack``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 3 Random Clothing Items, 3 Jade Blocks and 1 `#Rare Spotlight``.<CR><CR>`5Description:`` The hottest new looks for the season are here now with 3 random Fashion Clothing (dress, shoes, or purse), Jade Blocks to pose on, and a `#Rare`` Spotlight to shine on your fabulousness.|0|0|6000|0|||-1|-1||-1|-1||1|||||||";
					string text29 = "|\nadd_button|sportsball_pack|`oSportsball Pack``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 2 Basketball Hoops, 2 Sporty Goals, 5 Stadiums, 5 Crowded Stadiums, 10 Field Grass, 1 Football Helmet, 1 Growies Cap, 1 Ref's Jersey, 1 World Cup Jersey, 1 `#Rare Sports Item`` or `#Rare Growmoji!``.<CR><CR>`5Description:`` We like sports and we don't care who knows! This pack includes everything you need to get sporty! Use the Sports Items to launch Sportsballs at each other.|0|1|20000|0|||-1|-1||-1|-1||1|||||||";
					string text30 = "|\nadd_button|firefighter|`oFirefighter Pack``|interface/large/store_buttons/store_buttons14.rttex|`2You Get:`` 1 Yellow Helmet, 1 Yellow Jacket, 1 Yellow Pants, 1 Firemans Boots, 1 Fire Hose, and 1 `#Rare Firehouse`` .<CR><CR>`5Description:`` Rescue Growtopians from the fire! Includes a full Yellow Firefighter Outfit, Fire Hose and a `#Rare Firehouse``, which will protect your own world from fires.|0|1|10000|0|||-1|-1||-1|-1||1|||||||";
					string text31 = "|\nadd_button|steampack|`oSteampack``|interface/large/store_buttons/store_buttons14.rttex|`2You Get:`` 10 Steam Tubes, 2 Steam Stompers, 2 Steam Organs, 2 Steam Vents, 2 Steam Valves and 1 `#Rare Steampunk Top Hat``.<CR><CR>`5Description:`` Steam! It's a wondrous new technology that lets you create paths of Steam Blocks, then jump on a Steam Stomper to launch a jet of steam through the path, triggering steam-powered devices. Build puzzles, songs, parkour challenges, and more!|0|6|20000|0|||-1|-1||-1|-1||1|||||||";
					string text32 = "|\nadd_button|paintbrush|`oPainter's Pack``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 `#Rare Paintbrush`` and 20 Random Colored Paint Buckets.<CR><CR>`5Description:`` Want to paint your world? This pack includes 20 buckets of random paint colors (may include Varnish, to clean up your messes)! You can paint any block in your world different colors to personalize it.|0|1|30000|0|||-1|-1||-1|-1||1|||||||";
					string text33 = "|\nadd_button|paleo_kit|`oPaleontologist's Kit``|interface/large/store_buttons/store_buttons16.rttex|`2You Get:`` 5 Fossil Brushes, 1 Rock Hammer, 1 Rock Chisel, 1 Blue Hardhat and 1 `#Rare Fossil Prep Station``.<CR><CR>`5Description:`` If you want to dig up fossils, this is the kit for you! Includes everything you need! Use the prepstation to get your fossils ready for display.|0|0|20000|0|||-1|-1||-1|-1||1|||||||";
					string text34 = "|\nadd_button|robot_starter_pack|`oCyBlocks Starter Pack``|interface/large/store_buttons/store_buttons18.rttex|`2You Get:`` 1 `5Rare ShockBot`` and 10 random movement commands.<CR><CR>`5Description:`` CyBlocks Starter Pack includes one `5Rare`` ShockBot and 10 random movement commands to use with it. `5ShockBot`` is a perma-item, is never lost when destroyed.|0|6|5000|0|||-1|-1||-1|-1||1|||||||";
					string text35 = "|\nadd_button|robot_command_pack|`oCyBlocks Command Pack``|interface/large/store_buttons/store_buttons19.rttex|`2You Get:`` 10 Random CyBlock Commands.<CR><CR>`5Description:`` Grants 10 random CyBlock Commands to help control your CyBots!|0|2|2000|0|||-1|-1||-1|-1||1|||||||";
					string text36 = "|\nadd_button|robot_pack|`oCyBot Pack``|interface/large/store_buttons/store_buttons19.rttex|`2You Get:`` 1 `5Rare CyBot``!<CR><CR>`5Description:`` Grants one random `5Rare`` CyBot! Use CyBlock Commands to send these mechanical monsters into action! `5Note: Each CyBot is a perma-item, and will never be lost when destroyed.``|0|3|15000|0|||-1|-1||-1|-1||1|||||||";
					string text37 = "|\nadd_button|gang_pack|`oGangland Style``|interface/large/store_buttons/store_buttons2.rttex|`2You Get:`` 1 Fedora, 1 Dames Fedora, 1 Pinstripe Suit with Pants, 1 Flapper Headband with Dress, 1 Cigar, 1 Tommy Gun, 1 Victola and 10 Art Deco Blocks .<CR><CR>`5Description:`` Step into the 1920's with a Complete Outfit, a Tommygun, a Victrola that plays jazz music, and 10 Art Deco Blocks. It's the whole package!|0|6|5000|0|||-1|-1||-1|-1||1|||||||";

					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22 + text23 + text24 + text25 + text26 + text27 + text28 + text29 + text30 + text31 + text32 + text33 + text34 + text35 + text36 + text37);

				}
				if (cch.find("action|buy\nitem|token") == 0) {
					string text1 = "set_description_text|`2Spend your Growtokens!`` (You have `52``) You earn Growtokens from Crazy Jim and Sales-Man. Select the item you'd like more info on, or BACK to go back.";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||0|0|0|0||||-1|-1||||";
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1||||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||0|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||1|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_button|challenge_timer|`oChallenge Timer``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Challenge Timer.<CR><CR>`5Description:`` Get more people playing your parkours with this secure prize system. You'll need a `#Challenge Start Flag`` and `#Challenge End Flag`` as well (not included). Stock prizes into the Challenge Timer, set a time limit, and watch as players race from start to end. If they make it in time, they win a prize!|0|0|-5|0|||-1|-1||-1|-1||1|||||||";
					string text10 = "|\nadd_button|xp_potion|`oExperience Potion``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Experience Potion.<CR><CR>`5Description:`` This `#Untradeable`` delicious fizzy drink will make you smarter! 10,000 XP smarter instantly, to be exact.|0|2|-10|0|||-1|-1||-1|-1||1|||||||";
					string text11 = "|\nadd_button|megaphone|`oMegaphone``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Megaphone.<CR><CR>`5Description:`` You like broadcasting messages, but you're not so big on spending gems? Buy a Megaphone with Growtokens! Each Megaphone can be used once to send a super broadcast to all players in the game.|0|7|-10|0|||-1|-1||-1|-1||1|||||||";
					string text13 = "|\nadd_button|mini_mod|`oMini-Mod``|interface/large/store_buttons/store_buttons17.rttex|`2You Get:`` 1 Mini-Mod.<CR><CR>`5Description:`` Oh no, it's a Mini-Mod! Punch him to activate (you'll want to punch him!). When activated, he won't allow anyone to drop items in your world.|0|0|-20|0|||-1|-1||-1|-1||1|||||||";
					string text14 = "|\nadd_button|derpy_star|`oDerpy Star Block``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 Derpy Star Block.<CR><CR>`5Description:`` DER IM A SUPERSTAR. This is a fairly ordinary block, except for the derpy star on it. Note: it is not permanent, and it doesn't drop seeds. So use it wisely!|0|3|-30|0|||-1|-1||-1|-1||1|||||||";
					string text15 = "|\nadd_button|dirt_gun|`oBLYoshi's Free Dirt``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 BLYoshi's Free Dirt.<CR><CR>`5Description:`` Free might be stretching it, but hey, once you buy this deadly rifle, you can spew out all the dirt you want for free! Note: the dirt is launched at high velocity and explodes on impact. Sponsored by BLYoshi.|0|4|-40|0|||-1|-1||-1|-1||1|||||||";
					string text16 = "|\nadd_button|nothingness|`oWeather Machine - Nothingness``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Weather Machine - Nothingness.<CR><CR>`5Description:`` Tired of all that fancy weather?  This machine will turn your world completely black. Yup, that's it. Not a single pixel in the background except pure blackness.|0|3|-50|0|||-1|-1||-1|-1||1|||||||";
					string text17 = "|\nadd_button|spike_juice|`oSpike Juice``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 Spike Juice.<CR><CR>`5Description:`` It's fresh squeezed, with little bits of spikes still in it! Drinking this `#Untradeable`` one-use potion will make you immune to Death Spikes and Lava for 5 seconds.|0|5|-60|0|||-1|-1||-1|-1||1|||||||";
					string text18 = "|\nadd_button|doodad|`oDoodad``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Doodad.<CR><CR>`5Description:`` I have no idea what this thing does. It's something electronic? Maybe?|0|5|-75|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_button|crystal_cape|`oCrystal Cape``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 Crystal Cape.<CR><CR>`5Description:`` This cape is woven of pure crystal, which makes it pretty uncomfortable. But it also makes it magical! It lets you double-jump off of an imaginary Crystal Block in mid-air. Sponsored by Edvoid20, HemeTems, and Aboge.|0|5|-90|0|||-1|-1||-1|-1||1|||||||";
					string text20 = "|\nadd_button|focused_eyes|`oFocused Eyes``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Focused Eyes.<CR><CR>`5Description:`` This `#Untradeable`` item lets you shoot electricity from your eyes! Wear them with pride, and creepiness.|0|4|-100|0|||-1|-1||-1|-1||1|||||||";
					string text21 = "|\nadd_button|grip_tape|`oGrip Tape``|interface/large/store_buttons/store_buttons14.rttex|`2You Get:`` 1 Grip Tape.<CR><CR>`5Description:`` This is handy for wrapping around the handle of a weapon or tool. It can improve your grip, as well as protect you from cold metal handles. If you aren't planning to craft a weapon that requires Grip Tape, this does you no good at all!|0|5|-100|0|||-1|-1||-1|-1||1|||||||";
					string text22 = "|\nadd_button|cat_eyes|`oCat Eyes``|interface/large/store_buttons/store_buttons23.rttex|`2You Get:`` 1 Cat Eyes.<CR><CR>`5Description:`` Wow, pawesome! These new eyes are the cat's meow, and the purrfect addition to any style.|0|5|-100|0|||-1|-1||-1|-1||1|||||||";
					string text23 = "|\nadd_button|night_vision|`oNight Vision Goggles``|interface/large/store_buttons/store_buttons15.rttex|`2You Get:`` 1 Night Vision Goggles.<CR><CR>`5Description:`` Scared of the dark? We have a solution. You can wear these goggles just to look cool, but if you also happen to have a D Battery (`4batteries not included``) on you, you will be able to see through darkness like it's not even there! Each D Battery can power your goggles for 1 minute. `2If you are in a world you own, the goggles will not require batteries!`` Note: you can't turn the goggles off without removing them, so you'll be wasting your battery if you wear them in daylight while carrying D Batteries.|0|3|-110|0|||-1|-1||-1|-1||1|||||||";
					string text24 = "|\nadd_button|muddy_pants|`oMuddy Pants``|interface/large/store_buttons/store_buttons12.rttex|`2You Get:`` 1 Muddy Pants.<CR><CR>`5Description:`` Well, this is just a pair of muddy pants. But it does come with a super secret bonus surprise that is sure to blow your mind!|0|7|-125|0|||-1|-1||-1|-1||1|||||||";
					string text25 = "|\nadd_button|piranha|`oCuddly Piranha``|interface/large/store_buttons/store_buttons10.rttex|`2You Get:`` 1 Cuddly Piranha.<CR><CR>`5Description:`` This friendly pet piranha won't stay in its bowl!  It just wants to snuggle with your face!|0|0|-150|0|||-1|-1||-1|-1||1|||||||";
					string text26 = "|\nadd_button|puddy_leash|`oPuddy Leash``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 Puddy Leash.<CR><CR>`5Description:`` Puddy is a friendly little kitten who will follow you around forever.|0|7|-180|0|||-1|-1||-1|-1||1|||||||";
					string text27 = "|\nadd_button|golden_axe|`oGolden Pickaxe``|interface/large/store_buttons/store_buttons9.rttex|`2You Get:`` 1 Golden Pickaxe.<CR><CR>`5Description:`` Get your own sparkly pickaxe! This `#Untradeable`` item is a status symbol! Oh sure, it isn't any more effective than a normal pickaxe, but it sparkles!|0|1|-200|0|||-1|-1||-1|-1||1|||||||";
					string text28 = "|\nadd_button|puppy_leash|`oPuppy Leash``|interface/large/store_buttons/store_buttons11.rttex|`2You Get:`` 1 Puppy Leash.<CR><CR>`5Description:`` Get your own pet puppy! This little dog will follow you around forever, never wavering in her loyalty, thus making her `#Untradeable``.|0|4|-200|0|||-1|-1||-1|-1||1|||||||";
					string text29 = "|\nadd_button|diggers_spade|`oDigger's Spade``|interface/large/store_buttons/store_buttons13.rttex|`2You Get:`` 1 Digger's Spade.<CR><CR>`5Description:`` This may appear to be a humble shovel, but in fact it is enchanted with the greatest magic in Growtopia. It can smash Dirt or Cave Background in a single hit! Unfortunately, it's worthless at digging through anything else. Note: The spade is `#UNTRADEABLE``.|0|7|-200|0|||-1|-1||-1|-1||1|||||||";
					string text30 = "|\nadd_button|meow_ears|`oMeow Ears``|interface/large/store_buttons/store_buttons22.rttex|`2You Get:`` 1 Meow Ears.<CR><CR>`5Description:`` Meow's super special ears that everyone can now get! Note: These ears are `#UNTRADEABLE``.|0|0|-200|0|||-1|-1||-1|-1||1|||||||";
					string text31 = "|\nadd_button|frosty_hair|`oFrosty Hair``|interface/large/store_buttons/store_buttons23.rttex|`2You Get:`` 1 Frosty Hair.<CR><CR>`5Description:`` Coldplay is cold, but you can be freezing! Note: The frosty hair is `#UNTRADEABLE``.|0|0|-200|0|||-1|-1||-1|-1||1|||||||";
					string text32 = "|\nadd_button|zerkon_helmet|`oEvil Space Helmet``|interface/large/store_buttons/store_buttons21.rttex|`2You Get:`` 1 Evil Space Helmet.<CR><CR>`5Description:`` Zerkon commands a starship too small to actually board - pah, time to rule the galaxy properly! Note: The evil space helmet is `#UNTRADEABLE``.|0|6|-200|0|||-1|-1||-1|-1||1|||||||";
					string text33 = "|\nadd_button|seils_magic_orb|`oSeil's Magic Orbs``|interface/large/store_buttons/store_buttons21.rttex|`2You Get:`` 1 Seil's Magic Orbs.<CR><CR>`5Description:`` Seil is some kind of evil wizard, now you can be too! Note: These magic orbs are `#UNTRADEABLE``.|0|7|-200|0|||-1|-1||-1|-1||1|||||||";
					string text34 = "|\nadd_button|atomic_shadow_scythe|`oAtomic Shadow Scythe``|interface/large/store_buttons/store_buttons21.rttex|`2You Get:`` 1 Atomic Shadow Scythe.<CR><CR>`5Description:`` AtomicShadow might actually be evil, now you can try it out! Note: The shadow scythe is `#UNTRADEABLE``.|0|5|-200|0|||-1|-1||-1|-1||1|||||||";
					string text35 = "|\nadd_button|poseidon_diggers_trident|`oPoseidon's Digger's Trident``|interface/large/store_buttons/store_buttons25.rttex|`2You Get:`` 1 Poseidon's Digger's Trident.<CR><CR>`5Description:`` A gift from the gods. This may appear to be a humble trident, but in fact it has the power of Poseidon himself. It can smash `8Deep Sand`` or `8Ocean Rock`` in a single hit. Unfortunately, you don't get to wield the full might of Poseidon... the trident is worthless at smashing anything else. Note: The trident is `#UNTRADEABLE``.|0|6|-200|0|||-1|-1||-1|-1||1|||||||";

					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22 + text23 + text24 + text25 + text26 + text27 + text28 + text29 + text30 + text31 + text32 + text33 + text34 + text35);

				}
				if (cch.find("action|storenavigate\nitem|main\nselection|gems_rain") == 0) {
					string text1 = "set_description_text|Welcome to the `2Growtopia Store``! Select the item you'd like more info on.`o `wWant to get `5Supporter`` status? Any Gem purchase (or `57,000`` Gems earned with free `5Tapjoy`` offers) will make you one. You'll get new skin colors, the `5Recycle`` tool to convert unwanted items into Gems, and more bonuses!";
					string text2 = "|enable_tabs|1";
					string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||1|0|0|0||||-1|-1||||";
					string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1||||";
					string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||0|3|0|0||||-1|-1||||";
					string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
					string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
					string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
					string text9 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|1|";
					string text10 = "|\nadd_button|itemomonth|`oItem Of The Month``|interface/large/store_buttons/store_buttons16.rttex|September 2021:`` `9Ouroboros Charm``!<CR><CR>`2You Get:`` 1 `9Ouroboros Charm``.<CR><CR>`5Description: ``The endless loop of life and death, personified and celebrated. Is it a charm or is it a curse?|0|3|350000|0||interface/large/gui_store_button_overlays1.rttex|0|0||-1|-1||1|||||||";
					string text11 = "|\nadd_button|ads_tv|`oGrowShow TV``|interface/large/store_buttons/store_buttons30.rttex|`2You Get:`` 1 GrowShow TV.<CR><CR>`5Description:`` Growtopia's most loved gameshow now brings you its very own TV to watch up to 3 ads per day for AMAZING prizes.|0|4|50|0|||-1|-1||-1|-1||1|||||||";
					string text12 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|2|";
					string text13 = "|\nadd_button|gems_glory|Road To Glory|interface/large/store_buttons/store_buttons30.rttex|rt_grope_loyalty_bundle01|0|0|0|0||interface/large/gui_store_button_overlays1.rttex|0|0|/interface/large/gui_shop_buybanner.rttex|1|0|`2You Get:`` Road To Glory and 100k Gems Instantly.<CR>`5Description:`` Earn Gem rewards when you level up. Every 10 levels you will get additional Gem rewards up to Level 50! Claim all rewards instantly if you are over level 50!! 1.6M Gems in total!! |1|||||||";
					string text14 = "|\nadd_button|gems_rain|It's Rainin' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_rain|1|5|0|0|||-1|-1||-1|-1|`2You Get:`` 200,000 Gems, 2 Growtoken, and 1 Megaphone.<CR><CR>`5Description:`` All the gems you could ever want and more plus 2 Growtokens and a Megaphone to tell the whole world about it.|1|||||||";
					string text15 = "|\nadd_button|gems_fountain|Gem Fountain|interface/large/store_buttons/store_buttons2.rttex|rt_grope_gem_fountain|0|2|0|0|||-1|-1||-1|-1|`2You Get:`` 90,000 Gems and 1 Growtoken.<CR><CR>`5Description:`` Get a pile of gems to shop to your hearts desire and 1 Growtoken.|1|||||||";
					string text16 = "|\nadd_button|gems_chest|Chest o' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_chest|0|5|0|0|||-1|-1||-1|-1|`2You Get:`` 30,000 Gems.<CR><CR>`5Description:`` Get a chest containing gems.|1|||||||";
					string text17 = "|\nadd_button|gems_bag|Bag o' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_bag|1|0|0|0|||-1|-1||-1|-1|`2You Get:`` 14,000 Gems.<CR><CR>`5Description:`` Get a small bag of gems.|1|||||||";
					string text18 = "|\nadd_button|tapjoy|Earn Free Gems|interface/large/store_buttons/store_buttons.rttex||1|2|0|0|||-1|-1||-1|-1||1|||||||";
					string text19 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|3|";
					string text20 = "|\nadd_button|365d|`o1-Year Subscription Token``|interface/large/store_buttons/store_buttons22.rttex|rt_grope_subs_bundle02|0|5|0|0|||-1|-1||-1|-1|`2You Get:`` 1x 1-Year Subscription Token and 25 Growtokens.<CR><CR>`5Description:`` One full year of special treatment AND 25 Growtokens upfront! You'll get 70 season tokens (as long as there's a seasonal clash running), and 2500 gems every day and a chance of doubling any XP earned, growtime reduction on all seeds planted and Exclusive Skins!|1|||||||";
					string text21 = "|\nadd_button|30d|`o30-Day Subscription Token``|interface/large/store_buttons/store_buttons22.rttex|rt_grope_subs_bundle01|0|4|0|0|||-1|-1||-1|-1|`2You Get:`` 1x 30-Day Free Subscription Token and 2 Growtokens.<CR><CR>`5Description:`` 30 full days of special treatment AND 2 Growtokens upfront! You'll get 70 season tokens (as long as there's a seasonal clash running), and 2500 gems every day and a chance of doubling any XP earned, growtime reduction on all seeds planted and Exclusive Skins!|1|||||||";
					string text22 = "|\nadd_button|video_tapjoy|Watch Videos For Gems|interface/large/store_buttons/store_buttons29.rttex||0|1|0|0|1/5 VIDEOS WATCHED||-1|-1||-1|-1||1|||||||";
					string text23 = "|\nselect_item|gems_rain";


					packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22 + text23);

				}
				if (cch.find("action|buy\nitem|itemomonth") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygems1 = ((PlayerInfo*)(peer->data))->buygems;
					int ad = atoi(acontent.c_str());
					int buygemsz1 = buygems1 - 350000;
					int as = ad + buygemsz1;
					bool success = true;
					if (ad > 349999) {
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << as;
						myfile.close();
						GamePacket psa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), as));
						ENetPacket* packetsa = enet_packet_create(psa.data, psa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetsa);
						delete psa.data;

						packet::consolemessage(peer, "`5Got 1 `9Ouroboros Charm");
						packet::storepurchaseresult(peer, "`5You just bought an Ouroboros Charm and\n`oReceived: `o1 `9Ouroboros Charm.");

						SaveShopsItemMoreTimes(11232, 1, peer, success);
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");

					}
				}
				if (cch.find("action|buy\nitem|nyan_hat") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemsss1 = ((PlayerInfo*)(peer->data))->buygems;
					int asd = atoi(acontent.c_str());
					int buygemsssz1 = buygemsss1 - 25000;
					int aws = asd + buygemsssz1;
					bool success = true;
					if (asd > 24999) {
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << aws;
						myfile.close();
						GamePacket psa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), aws));
						ENetPacket* packetsa = enet_packet_create(psa.data, psa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetsa);
						delete psa.data;

						packet::consolemessage(peer, "`5Got 1 `#Turtle Hat");
						packet::storepurchaseresult(peer, "`5You just bought a Turtle Hat and\n`oReceived: `o1 Turtle Hat.");

						SaveShopsItemMoreTimes(574, 1, peer, success);
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");

					}
				}
				if (cch.find("action|buy\nitem|tiny_horsie") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemsss1 = ((PlayerInfo*)(peer->data))->buygems;
					int asd = atoi(acontent.c_str());
					int buygemsssz1 = buygemsss1 - 25000;
					int aws = asd + buygemsssz1;
					bool success = true;
					if (asd > 24999) {
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << aws;
						myfile.close();
						GamePacket psa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), aws));
						ENetPacket* packetsa = enet_packet_create(psa.data, psa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetsa);
						delete psa.data;

						packet::consolemessage(peer, "`5Got 1 `#Tiny Horsie");
						packet::storepurchaseresult(peer, "`5You just bought a Tiny Horsie and\n`oReceived: `o1 Tiny Horsie.");

						SaveShopsItemMoreTimes(592, 1, peer, success);
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");

					}
				}
				if (cch.find("action|buy\nitem|diggers_spade") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
						packet::consolemessage(peer, "`5Got 1 `oDigger´s Spade Digger´s Spade ");
						packet::storepurchaseresult(peer, "`5You just bought a Digger´s Spade and\n`oReceived: `o1 `2Digger´s Spade.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(2952, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|xp_potion") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 10, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oExperience Potion ");
						packet::storepurchaseresult(peer, "`5You just bought a Experience Potion and\n`oReceived: `o1 `2Experience Potion.");
						RemoveInventoryItem(1486, 10, peer);
						SaveShopsItemMoreTimes(1488, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|puddy_leash") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 180, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oPuddy Leash ");
						packet::storepurchaseresult(peer, "`5You just bought a Puddy Leash and\n`oReceived: `o1 `2Puddy Leash.");
						RemoveInventoryItem(1486, 180, peer);
						SaveShopsItemMoreTimes(2032, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|golden_axe") == 0) {
					auto iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						RemoveInventoryItem(1486, 200, peer);
						bool success = true;
						packet::PlayAudio(peer, "audio/piano_nice.wav", 0);
						SaveShopsItemMoreTimes(1438, 1, peer, success);
						packet::consolemessage(peer, "`5Got 1 `oGolden Pickaxe ");
						packet::storepurchaseresult(peer, "`5You just bought a Golden Pickaxe and\n`oReceived: `o1 `2Golden Pickaxe.");
					}
					else {
						packet::storepurchaseresult(peer, "You don't have enough Growtoken to buy this item.");
					}
					}
				if (cch.find("action|buy\nitem|puppy_leash") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 180, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oPuppy Leash ");
						packet::storepurchaseresult(peer, "`5You just bought a Puppy Leash and\n`oReceived: `o1 `2Puppy Leash.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(1742, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|meow_ears") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oMeow Ears ");
						packet::storepurchaseresult(peer, "`5You just bought Meow Ears and\n`oReceived: `o1 `2Meow Ears.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(698, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|frosty_hair") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oFrosty Hair ");
						packet::storepurchaseresult(peer, "`5You just bought Frosty Hair and\n`oReceived: `o1 `2Frosty Hair.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(1444, 1, peer, success);
					}
				}

				if (cch.find("action|buy\nitem|seils_magic_orb") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oSeils Magic Orbs ");
						packet::storepurchaseresult(peer, "`5You just bought Seils Magic Orbs and\n`oReceived: `o1 `2Seils Magic Orbs.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(820, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|zerkon_helmet") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oSeils Magic Orbs ");
						packet::storepurchaseresult(peer, "`5You just bought Seils Magic Orbs and\n`oReceived: `o1 `2Seils Magic Orbs.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(1440, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|atomic_shadow_scythe") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oAtomic Shadow Scythe ");
						packet::storepurchaseresult(peer, "`5You just bought a Atomic Shadow Scythe and\n`oReceived: `o1 `2Atomic Shadow Scythe.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(1484, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|poseidon_diggers_trident") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 200, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oPoseidon's Digger's Trident ");
						packet::storepurchaseresult(peer, "`5You just bought a Poseidon's Digger's Trident and\n`oReceived: `o1 `2Poseidon's Digger's Trident.");
						RemoveInventoryItem(1486, 200, peer);
						SaveShopsItemMoreTimes(7434, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|megaphone") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 10, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oMegaphone ");
						packet::storepurchaseresult(peer, "`5You just bought a Megaphone and\n`oReceived: `o1 `2Megaphone.");
						RemoveInventoryItem(1486, 10, peer);
						SaveShopsItemMoreTimes(2480, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|mini_mod") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 20, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oMini-Mod ");
						packet::storepurchaseresult(peer, "`5You just bought a Mini-Mod and\n`oReceived: `o1 `2Mini-Mod.");
						RemoveInventoryItem(1486, 20, peer);
						SaveShopsItemMoreTimes(4758, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|derpy_star") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 30, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oDerpy Star Block ");
						packet::storepurchaseresult(peer, "`5You just bought a Derpy Star Block and\n`oReceived: `o1 `2Derpy Star Block.");
						RemoveInventoryItem(1486, 30, peer);
						SaveShopsItemMoreTimes(1628, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|dirt_gun") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 40, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oBlYoshi's Dirtgun ");
						packet::storepurchaseresult(peer, "`5You just bought a BLYoshi's Dirtgun and\n`oReceived: `o1 `2BlYoshi's Dirtgun.");
						RemoveInventoryItem(1486, 40, peer);
						SaveShopsItemMoreTimes(2876, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|nothingness") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 50, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oWeather Machine - Nothingness. ");
						packet::storepurchaseresult(peer, "`5You just bought a Nothingness and\n`oReceived: `o1 `2Weather Machine - Nothingness.");
						RemoveInventoryItem(1486, 50, peer);
						SaveShopsItemMoreTimes(1490, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|spike_juice") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 60, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oSpike Juice ");
						packet::storepurchaseresult(peer, "`5You just bought a Spike Juice and\n`oReceived: `o1 `2Spike Juice.");
						RemoveInventoryItem(1486, 60, peer);
						SaveShopsItemMoreTimes(1662, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|doodad") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 75, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oDoodad ");
						packet::storepurchaseresult(peer, "`5You just bought a Doodad and\n`oReceived: `o1 `2Doodad.");
						RemoveInventoryItem(1486, 75, peer);
						SaveShopsItemMoreTimes(1492, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|crystal_cape") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 90, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oCrystal Cape ");
						packet::storepurchaseresult(peer, "`5You just bought a Doodad and\n`oReceived: `o1 `2Crystal Cape.");
						RemoveInventoryItem(1486, 90, peer);
						SaveShopsItemMoreTimes(1738, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|focused_eyes") == 0) {
					auto Price = 100;
					PlayerInfo* pData = ((PlayerInfo*)(peer->data));
					auto ItemID = 1204;
					auto count = 1;
					auto contains = false;
					auto KiekTuri = 0;
					try {
						for (auto i = 0; i < pData->inventory.items.size(); i++) {
							if (pData->inventory.items.at(i).itemID == 1486 && pData->inventory.items.at(i).itemCount >= 1) {
								KiekTuri = pData->inventory.items.at(i).itemCount;
								break;
							}
						}
					}
					catch (const std::out_of_range& e) {
						std::cout << e.what() << std::endl;
					}
					SearchInventoryItem(peer, 1486, Price, contains);
					if (contains) {
						if (CheckItemMaxed(peer, ItemID, count) || pData->inventory.items.size() + 1 >= pData->currentInventorySize && CheckItemExists(peer, ItemID) && CheckItemMaxed(peer, ItemID, 1) || pData->inventory.items.size() + 1 >= pData->currentInventorySize && !CheckItemExists(peer, ItemID)) {
							packet::PlayAudio(peer, "audio/bleep_fail.wav", 0);
							packet::storepurchaseresult(peer, "You don't have enough space in your inventory that. You may be carrying to many of one of the items you are trying to purchase or you don't have enough free spaces to fit them all in your backpack!");
							break;
						}
						RemoveInventoryItem(1486, Price, peer);
						bool success = false;
						SaveShopsItemMoreTimes(ItemID, count, peer, success);
						packet::PlayAudio(peer, "audio/piano_nice.wav", 0);
						packet::storepurchaseresult(peer, "You've purchased " + to_string(count) + " `o" + getItemDef(ItemID).name + " `wfor `$" + to_string(Price) + " `wGrowtokens.\nYou have `$" + to_string(KiekTuri - Price) + " `wGrowtokens left.\n\n`5Received: ``" + to_string(count) + " " + getItemDef(ItemID).name + "");
					}
					else {
						packet::PlayAudio(peer, "audio/bleep_fail.wav", 0);
						packet::storepurchaseresult(peer, "You can't afford `o" + getItemDef(ItemID).name + "``!  You're `$" + to_string(Price - KiekTuri) + "`` Growtokens short.");
					}
				}
				if (cch.find("action|buy\nitem|grip_tape") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 100, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
							packet::consolemessage(peer, "`5Got 1 `oGrip Tape ");
						packet::storepurchaseresult(peer, "`5You just bought a Grip Tape and\n`oReceived: `o1 `2Grip Tape.");
						RemoveInventoryItem(1486, 100, peer);
						SaveShopsItemMoreTimes(3248, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|cat_eyes") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 100, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
						packet::consolemessage(peer, "`5Got 1 `oCat Eyes ");
						packet::storepurchaseresult(peer, "`5You just bought a Cat Eyes and\n`oReceived: `o1 `2Cat Eyes.");
						RemoveInventoryItem(1486, 100, peer);
						SaveShopsItemMoreTimes(7106, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|night_vision") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 110, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
						packet::consolemessage(peer, "`5Got 1 `oNight Vision Goggles ");
						packet::storepurchaseresult(peer, "`5You just bought a Night Vision Goggles and\n`oReceived: `o1 `2Night Vision Goggles.");
						RemoveInventoryItem(1486, 110, peer);
						SaveShopsItemMoreTimes(3576, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|piranha") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 150, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
						packet::consolemessage(peer, "`5Got 1 `oCuddly Piranha ");
						packet::storepurchaseresult(peer, "`5You just bought a Cuddly Piranha and\n`oReceived: `o1 `2Cuddly Piranha.");
						RemoveInventoryItem(1486, 150, peer);
						SaveShopsItemMoreTimes(1534, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|muddy_pants") == 0) {
					bool iscontains = false;
					SearchInventoryItem(peer, 1486, 125, iscontains);
					if (!iscontains)
					{
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Tokens To Buy This Items. `5Try again later.");

					}
					else {
						bool success = true;
						if (success)
						packet::consolemessage(peer, "`5Got 1 `oMuddy Pants ");
						packet::storepurchaseresult(peer, "`5You just bought a Muddy Pants and\n`oReceived: `o1 `2Muddy Pants.");
						RemoveInventoryItem(1486, 125, peer);
						SaveShopsItemMoreTimes(2584, 1, peer, success);

					}
				}
				if (cch.find("action|buy\nitem|door_mover") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemsss1 = ((PlayerInfo*)(peer->data))->buygems;
					int asd = atoi(acontent.c_str());
					int buygemsssz1 = buygemsss1 - 5000;
					int aws = asd + buygemsssz1;
					bool success = true;
					if (asd > 4999) {
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << aws;
						myfile.close();
						GamePacket psa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), aws));
						ENetPacket* packetsa = enet_packet_create(psa.data, psa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetsa);
						delete psa.data;

						packet::consolemessage(peer, "`5Got 1 `#Door Mover");
						packet::storepurchaseresult(peer, "`5You just bought a Door Mover and\n`oReceived: `o1 Door Mover.");

						SaveShopsItemMoreTimes(1404, 1, peer, success);
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");

					}
				}
				if (cch.find("action|buy\nitem|rockin_pack") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 10000;
					int asss = adss + buygemssz12;
					if (adss > 9999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `oKeytar");
						packet::consolemessage(peer, "`5Got 1 `oBass Guitar");
						packet::consolemessage(peer, "`5Got 1 `oTambourine");
						packet::consolemessage(peer, "`5Got 1 `oStarchild Makeup");
						packet::consolemessage(peer, "`5Got 1 `oRockin' Headband");
						packet::consolemessage(peer, "`5Got 1 `oLeopard Leggings");
						packet::consolemessage(peer, "`5Got 1 `oShredded T-Shirt");
						packet::consolemessage(peer, "`5Got 1 `oDrumkit");
						packet::consolemessage(peer, "`5Got 6 Stage Support");
						packet::consolemessage(peer, "`5Got 6 Mega Rock Speaker");
						packet::consolemessage(peer, "`5Got 6 Rock N' Roll Wallpaper");
						packet::storepurchaseresult(peer, "`5You just bought 1 Rockin'Pack and\n`oReceived: `o1 Rockin'Pack.");

						SaveShopsItemMoreTimes(1714, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1710, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1712, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1718, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1732, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1722, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1720, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1724, 1, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1728, 6, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1730, 6, peer, success); // aposition, itemid, quantity, peer, success
						SaveShopsItemMoreTimes(1726, 6, peer, success); // aposition, itemid, quantity, peer, success
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|upgrade_backpack") == 0) {
					packet::storepurchaseresult(peer, "`4You already have max inventory");
				}
				if (cch.find("action|buy\nitem|unicorn") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 50000;
					int asss = adss + buygemssz12;
					if (adss > 49999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Unicorn");
						packet::storepurchaseresult(peer, "`5You just bought an Unicorn and\n`oReceived: `o1 Unicorn.");

						SaveShopsItemMoreTimes(1648, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|owl") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 25000;
					int asss = adss + buygemssz12;
					if (adss > 24999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Mid Pacific Owl");
						packet::storepurchaseresult(peer, "`5You just bought a Mid Pacific Owl and\n`oReceived: `o1 Mid Pacific Owl.");

						SaveShopsItemMoreTimes(1540, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|raptor") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 25000;
					int asss = adss + buygemssz12;
					if (adss > 24999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Riding Raptor");
						packet::storepurchaseresult(peer, "`5You just bought a Riding Raptor and\n`oReceived: `o1 Riding Raptor.");

						SaveShopsItemMoreTimes(1320, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|ambulance") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 25000;
					int asss = adss + buygemssz12;
					if (adss > 24999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Ambulance");
						packet::storepurchaseresult(peer, "`5You just bought an Ambulance and\n`oReceived: `o1 Ambulance.");

						SaveShopsItemMoreTimes(1272, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|corvette") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 25000;
					int asss = adss + buygemssz12;
					if (adss > 24999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Little Red Corvette");
						packet::storepurchaseresult(peer, "`5You just bought a Little Red Corvette and\n`oReceived: `o1 Little Red Corvette.");

						SaveShopsItemMoreTimes(766, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|dragon_hand") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 50000;
					int asss = adss + buygemssz12;
					if (adss > 49999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `4Dragon Hand");
						packet::storepurchaseresult(peer, "`5You just bought a Dragon Hand and\n`oReceived: `o1 Dragon Hand.");

						SaveShopsItemMoreTimes(900, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|star_ship") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 25000;
					int asss = adss + buygemssz12;
					if (adss > 24999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Pleiadian Star Ship");
						packet::storepurchaseresult(peer, "`5You just bought a Pleiadian Star Ship and\n`oReceived: `o1 Pleiadian Star Ship.");

						SaveShopsItemMoreTimes(760, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|small_lock") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 50;
					int asss = adss + buygemssz12;
					if (adss > 49) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Small Lock");
						packet::storepurchaseresult(peer, "`5You just bought 1 Small Lock and\n`oReceived: `o1 Small Lock.");

						SaveShopsItemMoreTimes(202, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|big_lock") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 200;
					int asss = adss + buygemssz12;
					if (adss > 199) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Big Lock");
						packet::storepurchaseresult(peer, "`5You just bought 1 Big Lock and\n`oReceived: `o1 Big Lock.");

						SaveShopsItemMoreTimes(204, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|big_lock") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 500;
					int asss = adss + buygemssz12;
					if (adss > 499) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Huge Lock");
						packet::storepurchaseresult(peer, "`5You just bought 1 Huge Lock and\n`oReceived: `o1 Huge Lock.");

						SaveShopsItemMoreTimes(206, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|door_pack") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 15;
					int asss = adss + buygemssz12;
					if (adss > 14) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Door");
						packet::consolemessage(peer, "`5Got 1 `#Sign");
						packet::storepurchaseresult(peer, "`5You just bought a Door and Sign and\n`oReceived: `o1 Door and Sign.");

						SaveShopsItemMoreTimes(12, 1, peer, success);
						SaveShopsItemMoreTimes(20, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|10_wl") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 20000;
					int asss = adss + buygemssz12;
					if (adss > 19999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 10 `#World Locks");
						packet::storepurchaseresult(peer, "`5You just bought 10 World Locks and\n`oReceived: `o10 World Locks.");

						SaveShopsItemMoreTimes(242, 10, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|vending_machine") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygemss12 = ((PlayerInfo*)(peer->data))->buygems;
					int adss = atoi(acontent.c_str());
					int buygemssz12 = buygemss12 - 8000;
					int asss = adss + buygemssz12;
					if (adss > 7999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#Vending Machine");
						packet::storepurchaseresult(peer, "`5You just bought 1 Vending Machine and\n`oReceived: `o1 Vending Machine.");

						SaveShopsItemMoreTimes(242, 10, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|world_lock") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygems12 = ((PlayerInfo*)(peer->data))->buygems;
					int ads = atoi(acontent.c_str());
					int buygemsz12 = buygems12 - 2000;
					int ass = ads + buygemsz12;
					if (ads > 1999) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << ass;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#World Lock");
						packet::storepurchaseresult(peer, "`5You just bought a World Lock and\n`oReceived: `o1 World Lock.");

						SaveShopsItemMoreTimes(242, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), ass));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|buy\nitem|ads_tv") == 0) {
					std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
					std::string acontent((std::istreambuf_iterator<char>(ifsz)),
						(std::istreambuf_iterator<char>()));
					int buygems123 = ((PlayerInfo*)(peer->data))->buygems;
					int ads1 = atoi(acontent.c_str());
					int buygemsz123 = buygems123 - 50;
					int asss = ads1 + buygemsz123;
					if (ads1 > 49) {
						bool success = true;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << asss;
						myfile.close();

						packet::consolemessage(peer, "`5Got 1 `#GrowShow TV");
						packet::storepurchaseresult(peer, "`5You just bought a GrowShow TV and\n`oReceived: `o1 GrowShow TV.");

						SaveShopsItemMoreTimes(9466, 1, peer, success);
						GamePacket pssa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), asss));
						ENetPacket* packetssa = enet_packet_create(pssa.data, pssa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetssa);
						delete pssa.data;
						packet::PlayAudio(peer, "audio/cash_register.wav", 0);
					}
					else {
						packet::storepurchaseresult(peer, "`4Purchase Failed: You Don't Have Enough Gems To Buy This Items. `5Try again later.");
					}
				}
				if (cch.find("action|respawn") == 0)
				{
					int x = 3040;
					int y = 736;

					if (!world) continue;

					for (int i = 0; i < world->width * world->height; i++)
					{
						if (world->items[i].foreground == 6) {
							x = (i % world->width) * 32;
							y = (i / world->width) * 32;
						}
					}
					{
						PlayerMoving data;
						data.packetType = 0x0;
						data.characterState = 0x924; // animation
						data.x = x;
						data.y = y;
						data.punchX = -1;
						data.punchY = -1;
						data.XSpeed = 0;
						data.YSpeed = 0;
						data.netID = ((PlayerInfo*)(peer->data))->netID;
						data.plantingTree = 0x0;
						SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
					}

					{
						int x = 3040;
						int y = 736;

						for (int i = 0; i < world->width * world->height; i++)
						{
							if (world->items[i].foreground == 6) {
								x = (i % world->width) * 32;
								y = (i / world->width) * 32;
							}
						}

						gamepacket_t p(0, ((PlayerInfo*)(peer->data))->netID);
						p.Insert("OnSetPos");
						p.Insert(x, y);
						p.CreatePacket(peer);
					}
					{
						int x = 3040;
						int y = 736;

						for (int i = 0; i < world->width * world->height; i++)
						{
							if (world->items[i].foreground == 6) {
								x = (i % world->width) * 32;
								y = (i / world->width) * 32;
							}
						}

						gamepacket_t p(0, ((PlayerInfo*)(peer->data))->netID);
						p.Insert("OnSetFreezeState");
						p.Insert(0);
						p.CreatePacket(peer);
					}
#ifdef TOTAL_LOG
					cout << "Respawning... " << endl;
#endif
				}
				if (cch.find("action|growid") == 0)
				{
					string ip = std::to_string(peer->address.host);
					if (ip.length() == 3) {
						packet::SendTalkSelf(peer, "`4Oops! `wYou've reached the max `5GrowID `waccounts you can make for this device or IP address!");
						continue;
					}
					Sleep(1000);
#ifndef REGISTRATION
					{
						GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "Registration is not supported yet!"));
						ENetPacket* packet = enet_packet_create(p.data,
							p.len,
							ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packet);
						delete p.data;
						//enet_host_flush(server);
					}
#endif
#ifdef REGISTRATION
					//GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`w" + itemDefs.at(id).name + "``|left|" + std::to_string(id) + "|\n\nadd_spacer|small|\nadd_textbox|" + itemDefs.at(id).description + "|left|\nadd_spacer|small|\nadd_quick_exit|\nadd_button|chc0|Close|noflags|0|0|\nnend_dialog|gazette||OK|"));
					packet::dialog(peer, "set_default_color|`o\n\nadd_label_with_icon|big|`wGet a GrowID``|left|206|\n\nadd_spacer|small|\nadd_textbox|A `wGrowID `wmeans `oyou can use a name and password to logon from any device.|\nadd_spacer|small|\nadd_textbox|This `wname `owill be reserved for you and `wshown to other players`o, so choose carefully!|\nadd_text_input|username|GrowID||30|\nadd_text_input|password|Password||100|\nadd_text_input|passwordverify|Password Verify||100|\nadd_textbox|Your `wemail address `owill only be used for account verification purposes and won't be spammed or shared. If you use a fake email, you'll never be able to recover or change your password.|\nadd_text_input|email|Email||100|\nadd_textbox|Your `wDiscord ID `owill be used for secondary verification if you lost access to your `wemail address`o! Please enter in such format: `wdiscordname#tag`o. Your `wDiscord Tag `ocan be found in your `wDiscord account settings`o.|\nadd_text_input|discord|Discord||100|\nend_dialog|register|Cancel|Get My GrowID!|\n");
#endif
				}
				if (cch.find("action|store\nlocation|gem") == 0)
				{
				string text1 = "set_description_text|Welcome to the `2Growtopia Store``! Select the item you'd like more info on.`o `wWant to get `5Supporter`` status? Any Gem purchase (or `57,000`` Gems earned with free `5Tapjoy`` offers) will make you one. You'll get new skin colors, the `5Recycle`` tool to convert unwanted items into Gems, and more bonuses!";
				string text2 = "|enable_tabs|1";
				string text3 = "|\nadd_tab_button|main_menu|Home|interface/large/btn_shop2.rttex||1|0|0|0||||-1|-1||||";;
				string text4 = "|\nadd_tab_button|locks_menu|Locks And Stuff|interface/large/btn_shop2.rttex||0|1|0|0||||-1|-1|||";
				string text5 = "|\nadd_tab_button|itempack_menu|Item Packs|interface/large/btn_shop2.rttex||0|3|0|0||||-1|-1||||";
				string text6 = "|\nadd_tab_button|bigitems_menu|Awesome Items|interface/large/btn_shop2.rttex||0|4|0|0||||-1|-1||||";
				string text7 = "|\nadd_tab_button|weather_menu|Weather Machines|interface/large/btn_shop2.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|5|0|0||||-1|-1||||";
				string text8 = "|\nadd_tab_button|token_menu|Growtoken Items|interface/large/btn_shop2.rttex||0|2|0|0||||-1|-1||||";
				string text9 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|1|";
				string text10 = "|\nadd_button|itemomonth|`oItem Of The Month``|interface/large/store_buttons/store_buttons16.rttex|September 2021:`` `9Ouroboros Charm``!<CR><CR>`2You Get:`` 1 `9Ouroboros Charm``.<CR><CR>`5Description: ``The endless loop of life and death, personified and celebrated. Is it a charm or is it a curse?|0|3|350000|0||interface/large/gui_store_button_overlays1.rttex|0|0||-1|-1||1|||||||";
				string text11 = "|\nadd_button|ads_tv|`oGrowShow TV``|interface/large/store_buttons/store_buttons30.rttex|`2You Get:`` 1 GrowShow TV.<CR><CR>`5Description:`` Growtopia's most loved gameshow now brings you its very own TV to watch up to 3 ads per day for AMAZING prizes.|0|4|50|0|||-1|-1||-1|-1||1||||||";
				string text12 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|2|";
				string text13 = "|\nadd_button|gems_glory|Road To Glory|interface/large/store_buttons/store_buttons30.rttex|rt_grope_loyalty_bundle01|0|0|0|0||interface/large/gui_store_button_overlays1.rttex|0|0|/interface/large/gui_shop_buybanner.rttex|1|0|`2You Get:`` Road To Glory and 100k Gems Instantly.<CR>`5Description:`` Earn Gem rewards when you level up. Every 10 levels you will get additional Gem rewards up to Level 50! Claim all rewards instantly if you are over level 50!! 1.6M Gems in total!! |1|||||||";
				string text14 = "|\nadd_button|gems_rain|It's Rainin' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_rain|1|5|0|0|||-1|-1||-1|-1|`2You Get:`` 200,000 Gems, 2 Growtoken, and 1 Megaphone.<CR><CR>`5Description:`` All the gems you could ever want and more plus 2 Growtokens and a Megaphone to tell the whole world about it.|1|||||||";
				string text15 = "|\nadd_button|gems_fountain|Gem Fountain|interface/large/store_buttons/store_buttons2.rttex|rt_grope_gem_fountain|0|2|0|0|||-1|-1||-1|-1|`2You Get:`` 90,000 Gems and 1 Growtoken.<CR><CR>`5Description:`` Get a pile of gems to shop to your hearts desire and 1 Growtoken.|1|||||||";
				string text16 = "|\nadd_button|gems_chest|Chest o' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_chest|0|5|0|0|||-1|-1||-1|-1|`2You Get:`` 30,000 Gems.<CR><CR>`5Description:`` Get a chest containing gems.|1|||||||";
				string text17 = "|\nadd_button|gems_bag|Bag o' Gems|interface/large/store_buttons/store_buttons.rttex|rt_grope_gem_bag|1|0|0|0|||-1|-1||-1|-1|`2You Get:`` 14,000 Gems.<CR><CR>`5Description:`` Get a small bag of gems.|1|||||||";
				string text18 = "|\nadd_button|tapjoy|Earn Free Gems|interface/large/store_buttons/store_buttons.rttex||1|2|0|0|||-1|-1||-1|-1||1|||||||";
				string text19 = "|\nadd_banner|interface/large/gui_shop_featured_header.rttex|0|3|";
				string text20 = "|\nadd_button|365d|`o1-Year Subscription Token``|interface/large/store_buttons/store_buttons22.rttex|rt_grope_subs_bundle02|0|5|0|0|||-1|-1||-1|-1|`2You Get:`` 1x 1-Year Subscription Token and 25 Growtokens.<CR><CR>`5Description:`` One full year of special treatment AND 25 Growtokens upfront! You'll get 70 season tokens (as long as there's a seasonal clash running), and 2500 gems every day and a chance of doubling any XP earned, growtime reduction on all seeds planted and Exclusive Skins!|1||||||";
				string text21 = "|\nadd_button|30d|`o30-Day Subscription Token``|interface/large/store_buttons/store_buttons22.rttex|rt_grope_subs_bundle01|0|4|0|0|||-1|-1||-1|-1|`2You Get:`` 1x 30-Day Free Subscription Token and 2 Growtokens.<CR><CR>`5Description:`` 30 full days of special treatment AND 2 Growtokens upfront! You'll get 70 season tokens (as long as there's a seasonal clash running), and 2500 gems every day and a chance of doubling any XP earned, growtime reduction on all seeds planted and Exclusive Skins!|1||||||";
				string text22 = "|\nadd_button|video_tapjoy|Watch Videos For Gems|interface/large/store_buttons/store_buttons29.rttex||0|1|0|0|1/5 VIDEOS WATCHED||-1|-1||-1|-1||1||||||";
				/*
							string text1 = "set_description_text|Welcome to the `2Growtopia Store``!  Tap the item you'd like more info on.`o  `wWant to get `5Supporter`` status? Any Gem purchase (or `57,000`` Gems earned with free `5Tapjoy`` offers) will make you one. You'll get new skin colors, the `5Recycle`` tool to convert unwanted items into Gems, and more bonuses!";
							string text2 = "|\nadd_button|iapp_menu|Buy Gems|interface/large/store_buttons5.rttex||0|2|0|0|";
							string text3 = "|\nadd_button|subs_menu|Subscriptions|interface/large/store_buttons22.rttex||0|1|0|0|";
							string text4 = "|\nadd_button|itemomonth|`oItem Of The Month``|interface/large/store_buttons16.rttex|`2September 2021:`` `9The endless loop of life and death, personified and celebrated. Is it a charm or is it a curse?``|0|3|350000|0|";
							string text5 = "|\nadd_button|locks_menu|Locks And Stuff|interface/large/store_buttons3.rttex||0|4|5|0|";
							string text6 = "|\nadd_button|itempack_menu|Item Packs|interface/large/store_buttons3.rttex||0|3|0|10|";
							string text7 = "|\nadd_button|bigitems_menu|Awesome Items|interface/large/store_buttons4.rttex||0|6|12|0|";
							string text8 = "|\nadd_button|weather_menu|Weather Machines|interface/large/store_buttons5.rttex|Tired of the same sunny sky?  We offer alternatives within...|0|4|15|0||";
							string text9 = "|\nadd_button|token_menu|Growtoken Items|interface/large/store_buttons9.rttex||0|1|15|0|";

							packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22);
						}*/
				packet::storerequest(peer, text1 + text2 + text3 + text4 + text5 + text6 + text7 + text8 + text9 + text10 + text11 + text12 + text13 + text14 + text15 + text16 + text17 + text18 + text19 + text20 + text21 + text22);
                }
				if (cch.find("action|info") == 0)
				{
					std::stringstream ss(cch);
					std::string to;
					int id = -1;
					int count = -1;
					while (std::getline(ss, to, '\n')) {
						vector<string> infoDat = explode("|", to);
						if (infoDat.size() == 3) {
							if (infoDat[1] == "itemID") id = atoi(infoDat[2].c_str());
							if (infoDat[1] == "count") count = atoi(infoDat[2].c_str());
						}
					}
					if (id == -1 || count == -1) continue;
					if (itemDefs.size() < id || id < 0) continue;
					packet::dialog(peer, "set_default_color|`o\n\nadd_label_with_icon|big|`w"+ itemDefs.at(id).name +"``|left|"+std::to_string(id)+"|\n\nadd_spacer|small|\nadd_textbox|"+ itemDefs.at(id).description +"|left|\nadd_spacer|small|\nadd_quick_exit|\nend_dialog|item_info|OK||");
				}
				if (cch.find("action|dialog_return\ndialog_name|sign_edit") == 0) {
					if (world != NULL) {
						if (((PlayerInfo*)(peer->data))->rawName == PlayerDB::getProperName(world->owner)) {
							std::stringstream ss(GetTextPointerFromPacket(event.packet));
							std::string to;
							int x = 0;
							int y = 0;
							bool created = false;
							string text = "";
							string world = ((PlayerInfo*)(peer->data))->currentWorld;
							while (std::getline(ss, to, '\n')) {
								string id = to.substr(0, to.find("|"));
								string act = to.substr(to.find("|") + 1, to.length() - to.find("|") - 1);
								if (id == "tilex")
								{
									x = atoi(act.c_str());

								}
								else if (id == "tiley")
								{
									y = atoi(act.c_str());
								}
								else if (id == "ch3")
								{
									text = act;
									created = true;
								}
								if (created == true) {
									if (text == "__%&P&%__") {
										continue;
									}
								}
								if (text.length() < 255) {
									WorldInfo* worldInfo = getPlyersWorld(peer);
									int squaresign = ((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * 100);
									updateSignSound(peer, worldInfo->items[squaresign].foreground, squaresign % worldInfo->width, squaresign / worldInfo->width, text, worldInfo->items[squaresign].background);
									worldInfo->items[squaresign].text = text;
								}
							}
						}
					}
				}
				if (cch.find("action|dialog_return") == 0)
				{
					std::stringstream ss(cch);
					std::string to;
					string btn = "";
					bool captcha = false;
					bool isRegisterDialog = false;
					bool SurgDialog = false;
					bool isGuildDialog = false;
					string guildName = "";
					string guildStatement = "";
					string guildFlagBg = "";
					string guildFlagFg = "";
					string item = "";
					bool marsdialog = false;
					string marstext = "";
					bool isFindDialog = false;
					bool isEpoch = false;
					bool ice = false;
					bool volcano = false;
					bool land = false;
					string itemFind = "";
					string username = "";
					string password = "";
					string passwordverify = "";
					string email = "";
					string discord = "";
					string captchaanswer, number1, number2 = "";
					int level = 1;
					int xp = 0;
					int hasil = 0;
					int resultnbr1 = 0;
					int resultnbr2 = 0;
					string textlevel = "";
					bool isTrashDialog = false;
					string trashitemcount = "";
					bool isEntranceDialog = false;
					string entranceresult = "";
					bool isGEntrance = false;
					string gentranceresult = "";
					bool isBG = false;
					string bgitem = "";
					bool levels = false;
					bool gems = false;
					bool cbox1 = false;
					int cboxEpoch = 0;
					bool isStuff = false;
					string stuff_gravity = "";
					bool stuff_invert = false;
					bool stuff_spin = false;
					string stuffITEM = "";
					bool gacha = false;
					bool isBuyItemDialog = false;
					string textgems = "";
					string textgacha = "";
					while (std::getline(ss, to, '\n')) {
						vector<string> infoDat = explode("|", to);
						if (infoDat.size() == 2) {
							if (infoDat[0] == "dialog_name" && infoDat[1] == "findid") {
								isFindDialog = true;
							}
							if (isFindDialog) {
								if (infoDat[0] == "item") itemFind = infoDat[1];
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "sendmb") marsdialog = true;
							if (infoDat[0] == "dialog_name" && infoDat[1] == "buyitembyrarity") isBuyItemDialog = true;
							if (infoDat[0] == "dialog_name" && infoDat[1] == "surge") SurgDialog = true;
							if (infoDat[0] == "dialog_name" && infoDat[1] == "leveldialog")
							{
								levels = true;
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "entrance") {
								isEntranceDialog = true;
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "gentrance") {
								isGEntrance = true;
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "trashdialog") {
								isTrashDialog = true;
							}
							if (isTrashDialog) {
								if (infoDat[0] == "trashitemcount") trashitemcount = infoDat[1];
							}
							if (levels) {
								if (infoDat[0] == "textlevel")
								{
									textlevel = infoDat[1];
									bool contains_non_int = !std::regex_match(textlevel, std::regex("^[0-9]+$"));

									if (contains_non_int == true) {
										GamePacket pfi = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`9Requesting offer failed... You may only use positive numbers!"));
										ENetPacket* packetfi = enet_packet_create(pfi.data,
											pfi.len,
											ENET_PACKET_FLAG_RELIABLE);
										enet_peer_send(peer, 0, packetfi);

										delete pfi.data;
									}
									else
									{
										int a = atoi(textlevel.c_str());
										if (a > 0) {
											int level = ((PlayerInfo*)(peer->data))->level;
											int a = atoi(textlevel.c_str());
											int blevel = a * 10;
											int wls = ((PlayerInfo*)(peer->data))->premwl;
											((PlayerInfo*)(peer->data))->buygems = blevel;
											if (wls >= a)
											{
												GamePacket p2 = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`w\n\nadd_label_with_icon|big|`oPurchase Confirmation``|left|1366|\nadd_spacer|small|\nadd_label|small|`4You'll give:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`o" + std::to_string(a) + "`w) Premium WLS``|left|242|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_label|small|`2You'll get:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`o" + std::to_string(blevel) + "`w) Level``|left|1488|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_button|confirmlevel|`wDo The Purchase!|noflags|0|0|\nadd_button|cancel|`oCancel|noflags|0|0|"));
												ENetPacket* packet2 = enet_packet_create(p2.data,
													p2.len,
													ENET_PACKET_FLAG_RELIABLE);

												enet_peer_send(peer, 0, packet2);
												delete p2.data;
											}
											else
											{
												GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "`4Not enough WL's"));
												ENetPacket* packet = enet_packet_create(p.data,
													p.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet);
												delete p.data;
											}
										}
									}
								}
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "gemsdialog")
							{
								gems = true;
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "gachadialog")
							{
								gacha = true;
							}
							if (infoDat[0] == "dialog_name" && infoDat[1] == "bg_weather") isBG = true;
							if (infoDat[0] == "bg_pick") isBG = true;
							if (infoDat[0] == "dialog_name" && infoDat[1] == "stuff_weather") isStuff = true;
							if (infoDat[0] == "stuff_pick") isStuff = true;
							if (isStuff) {
								if (infoDat[0] == "stuff_pick") stuffITEM = infoDat[1];
								if (infoDat[0] == "stuff_gravity") stuff_gravity = infoDat[1];
								if (infoDat[0] == "stuff_spin") stuff_spin = atoi(infoDat[1].c_str());
								if (infoDat[0] == "stuff_invert") stuff_invert = atoi(infoDat[1].c_str());
							}
							if (isEntranceDialog) {
								if (infoDat[0] == "opentopublic") entranceresult = infoDat[1];
								if (entranceresult == "1") {
									world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].isOpened = true;
									updateEntrance(peer, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].foreground, ((PlayerInfo*)(peer->data))->wrenchx, ((PlayerInfo*)(peer->data))->wrenchy, true, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].background);
								}
								else {
									world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].isOpened = false;
									updateEntrance(peer, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].foreground, ((PlayerInfo*)(peer->data))->wrenchx, ((PlayerInfo*)(peer->data))->wrenchy, false, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].background);
								}
							}
							if (isGEntrance) {
								if (infoDat[0] == "gopentopublic") entranceresult = infoDat[1];
								if (gentranceresult == "1") {
									world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].isOpened = true;
									updateEntrance(peer, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].foreground, ((PlayerInfo*)(peer->data))->wrenchx, ((PlayerInfo*)(peer->data))->wrenchy, true, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].background);
								}
								else {
									world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].isOpened = false;
									updateEntrance(peer, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].foreground, ((PlayerInfo*)(peer->data))->wrenchx, ((PlayerInfo*)(peer->data))->wrenchy, false, world->items[((PlayerInfo*)(peer->data))->wrenchx + (((PlayerInfo*)(peer->data))->wrenchy * world->width)].background);
								}
							}
							if (isStuff) {
								if (world != NULL) {
									if (stuffITEM != "") {
										int x = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation % world->width;
										int y = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation / world->width;
										int id = atoi(stuffITEM.c_str());
										int stuffGra = atoi(stuff_gravity.c_str());
										world->stuff_invert = stuff_invert;
										world->stuff_spin = stuff_spin;
										world->stuff_gravity = stuffGra;
										world->stuffID = atoi(stuffITEM.c_str());
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
											if (isHere(peer, currentPeer)) {
												sendStuffweather(currentPeer, x, y, world->stuffID, stuffGra, stuff_spin, stuff_invert);
												getPlyersWorld(currentPeer)->weather = 29;
											}
										}
									}
									else {
										int x = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation % world->width;
										int y = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation / world->width;
										int stuffGra = atoi(stuff_gravity.c_str());
										world->stuff_invert = stuff_invert;
										world->stuff_spin = stuff_spin;
										world->stuff_gravity = stuffGra;
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
											if (isHere(peer, currentPeer)) {
												sendStuffweather(currentPeer, x, y, world->stuffID, stuffGra, stuff_spin, stuff_invert);
												getPlyersWorld(currentPeer)->weather = 29;
											}
										}
									}
									int x = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation % world->width;
									int y = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation / world->width;
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
										if (isHere(peer, currentPeer)) {
											OnSetCurrentWeather(currentPeer, 29);
											continue;
										}
									}
								}
							}
							if (isBG) {
							if (world != NULL) {
								if (infoDat[0] == "bg_pick") bgitem = infoDat[1];
								int id = atoi(bgitem.c_str());
								int x = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation % world->width;
								int y = ((PlayerInfo*)(peer->data))->wrenchedBlockLocation / world->width;
								if (world->bgID == id || infoDat.at(1) == "") {
									break;
								}
								if (getItemDef(id).blockType == BlockTypes::BACKGROUND) {
									world->bgID = atoi(bgitem.c_str());
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
										if (isHere(peer, currentPeer)) {
											OnSetCurrentWeather(currentPeer, 34);
											sendBackground(currentPeer, x, y, atoi(bgitem.c_str()));
											getPlyersWorld(currentPeer)->weather = 34;
										}
								else {
									packet::SendTalkSelf(peer, "You can only set background blocks to Weather Machine - Background!");
									}
								}
									break;
								}
							}
								}
							if (gems) {
								if (infoDat[0] == "textgems")
								{
									textgems = infoDat[1];
									bool contains_non_int
										= !std::regex_match(textgems, std::regex("^[0-9]+$"));

									if (contains_non_int == true) {
										packet::consolemessage(peer, "You can only use positive numbers.");
									}
									else
									{
										int a = atoi(textgems.c_str());
										if (a > 0) {
											std::ifstream ifsz("/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
											std::string acontent((std::istreambuf_iterator<char>(ifsz)),
												(std::istreambuf_iterator<char>()));
											int cgems = atoi(acontent.c_str());
											int bgems = a * 20000;
											int wls = ((PlayerInfo*)(peer->data))->premwl;
											((PlayerInfo*)(peer->data))->buygems = a;
											if (wls >= a)
											{
												string sukses = "set_default_color|`w\n\nadd_label_with_icon|big|`oPurchase Confirmation``|left|1366|\nadd_spacer|small|\nadd_label|small|`4You'll give:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`o" + std::to_string(a) + "`w) Premium WLS``|left|242|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_label|small|`2You'll get:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`o" + std::to_string(bgems) + "`w) Gems``|left|112|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_button|confirmgems|`wDo The Purchase!|noflags|0|0|\nadd_button|cancel|`oCancel|noflags|0|0|";
												packet::dialog(peer, sukses);
											}
											else
											{
												packet::consolemessage(peer, "`4You don't have enough wl");
											}
										}
									}
								}
							}
							if (infoDat[0] == "buttonClicked") btn = infoDat[1];
							if (infoDat[0] == "dialog_name" && infoDat[1] == "guildconfirm") 
								isGuildDialog = true;
							if (infoDat[0] == "dialog_name" && infoDat[1] == "register")
							{
								isRegisterDialog = true;
							}
							if (isRegisterDialog) {
								if (infoDat[0] == "username") username = infoDat[1];
								if (infoDat[0] == "password") password = infoDat[1];
								if (infoDat[0] == "passwordverify") passwordverify = infoDat[1];
								if (infoDat[0] == "email") email = infoDat[1];
								if (infoDat[0] == "discord") discord = infoDat[1];
							}
							if (isGuildDialog) {
								if (infoDat[0] == "gname") guildName = infoDat[1];
								if (infoDat[0] == "gstatement") guildStatement = infoDat[1];
								if (infoDat[0] == "ggcflagbg") guildFlagBg = infoDat[1];
								if (infoDat[0] == "ggcflagfg") guildFlagFg = infoDat[1];
							}

							if (infoDat[0] == "dialog_name" && infoDat[1] == "epochweather")
								isEpoch = true;
							if (isEpoch) {
								if (infoDat[0] == "epochice") ice = atoi(infoDat[1].c_str());
								if (infoDat[0] == "epochvol") volcano = atoi(infoDat[1].c_str());
								if (infoDat[0] == "epochland") land = atoi(infoDat[1].c_str());
							}

							if (infoDat[0] == "buttonClicked" && infoDat[1] == "Submit_") captcha = true;
							if (captcha) {
								if (infoDat[0] == "captcha_answer") captchaanswer = infoDat[1];
							}
							if (infoDat[0] == "timedilation") {
								string settimedilation = infoDat[1];
								if (settimedilation.size() > 2 || settimedilation.size() <= 0 || settimedilation.size() < 1)
								{
									packet::SendTalkSelf(peer, "`wYou can't set Cycle time more than 2 digits");
									break;
								}
								if (settimedilation.find_first_not_of("0123456789") != string::npos)
								{
									packet::SendTalkSelf(peer, "`wYou can't set Cycle time with non digits");
									break;
								}
								if (stoi(settimedilation) < 0)
								{
									packet::SendTalkSelf(peer, "`wYou can't set Cycle time below 0 minute");
									break;
								}
								if (settimedilation == "")
								{
									packet::SendTalkSelf(peer, "`wYou can't set Cycle time below 0 minute");
									break;
								}
								((PlayerInfo*)(peer->data))->timedilation = stoi(infoDat[1]);
							}
						}
					}
					
					if (isTrashDialog) {
						int itemid = ((PlayerInfo*)(peer->data))->lasttrashitem;
						int x;
						try {
							x = stoi(trashitemcount);
						}
						catch (std::invalid_argument& e) {
							continue;
						}
						short int currentItemCount = 0;
						for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++) {
							if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == itemid) {
								currentItemCount = (unsigned int)((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount;
							}
						}
						if (x <= 0 || x > currentItemCount) continue;
						if (itemid < 1 || itemid > maxItems) continue;
						bool iscontainseas = false;
						SearchInventoryItem(peer, itemid, 1, iscontainseas);
						if (!iscontainseas) break;
						((PlayerInfo*)(peer->data))->lastunt = itemid;
						((PlayerInfo*)(peer->data))->lastuntc = x;
						int id = ((PlayerInfo*)(peer->data))->lasttrashitem;
						packet::consolemessage(peer, "`o" + std::to_string(x) + " `w" + getItemDef(itemid).name + " `otrashed.");
						RemoveInventoryItem(((PlayerInfo*)(peer->data))->lasttrashitem, x, peer);
						packet::PlayAudio(peer, "trash.wav", 0);
						((PlayerInfo*)(peer->data))->lasttrashitem = 0;
					}
					
					
					if (isFindDialog && btn.substr(0, 4) == "tool") {
						if (has_only_digits(btn.substr(4, btn.length() - 4)) == false) break;
						int id = atoi(btn.substr(4, btn.length() - 4).c_str());
						int intid = atoi(btn.substr(4, btn.length() - 4).c_str());
						bool iscontains = false;
						string ide = btn.substr(4, btn.length() - 4).c_str();
						ItemDefinition gaydef;
						gaydef = getItemDef(id);

						if (id == 9350 || id == 11232 || id == 242 || id == 1796 || id == 7188 || id == 6802 || id == 1486) {
							if (((PlayerInfo*)(peer->data))->adminLevel < 1337) {
								packet::consolemessage(peer, "`4You must purchase this item");
								break;
							}
							else {}
						}
						if (id == 10456 || id == 7480 || id == 112 || id == 1790 || id == 1900 || id == 6336) {
							if (((PlayerInfo*)(peer->data))->adminLevel < 1337) {
								packet::consolemessage(peer, "This item is too heavy for you!");
								break;
							}
							else {
							}
						}
						for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++) {
							if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == intid) {
								packet::consolemessage(peer, "`oSorry! Your inventory already contains this item!");
								iscontains = true;
							}
						}
						if (iscontains) {
							iscontains = false;
							continue;
						}
						if (CheckItemMaxed(peer, id, 200)) {
							packet::consolemessage(peer, "You already reached the max count of the item!");
							continue;
						}
						bool success = true;
						if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
							packet::consolemessage(peer, "`oItem `w" + getItemDef(intid).name + " `o(`w" + ide + "`o) With rarity : `o(`w" + std::to_string(getItemDef(intid).rarity) + "`o) `ohas been `2added `oto your inventory");
							packet::dialog(peer, "add_label_with_icon|big|`wItem Finder``|left|6016|\nadd_spacer|small|\nadd_label|small|`$Items Found :|left|\nadd_label_with_icon|small|`c" + getItemDef(intid).name + "``|left|" + ide + "|\nadd_textbox|`$Items ID `o[`c" + ide + "`o]``|\nadd_spacer|small|\nadd_textbox|`2Find Again :|\nadd_textbox|`$Enter a word below to find the item|\nadd_text_input|item|Item Name||30|\nend_dialog|findid|Cancel|`2Find the item!|\nadd_quick_exit|");
						}
						else {
							packet::consolemessage(peer, "`oItem `w" + getItemDef(intid).name + " `oWith rarity : `o(`w" + std::to_string(getItemDef(intid).rarity) + "`o) `ohas been `2added `oto your inventory");
							packet::dialog(peer, "add_label_with_icon|big|`wItem Finder``|left|6016|\nadd_spacer|small|\nadd_label|small|`$Items Found :|left|\nadd_label_with_icon|small|`c" + getItemDef(intid).name + "``|left|" + ide + "|\nadd_spacer|small|\nadd_textbox|`2Find Again :|\nadd_textbox|`$Enter a word below to find the item|\nadd_text_input|item|Item Name||30|\nend_dialog|findid|Cancel|`2Find the item!|\nadd_quick_exit|");
						}
						SaveShopsItemMoreTimes(intid, 200, peer, success);
					}

					else if (isFindDialog) {
						string itemLower2;
						vector<ItemDefinition> itemDefsfind;
						for (char c : itemFind) if (c < 0x20 || c>0x7A) goto SKIPFind;
						if (itemFind.length() < 3) goto SKIPFind3;
						for (const ItemDefinition& item : itemDefs)
						{
							string itemLower;
							for (char c : item.name) if (c < 0x20 || c>0x7A) goto SKIPFind2;
							if (!(item.id % 2 == 0)) goto SKIPFind2;
							itemLower2 = item.name;
							std::transform(itemLower2.begin(), itemLower2.end(), itemLower2.begin(), ::tolower);
							if (itemLower2.find(itemLower) != std::string::npos) {
								itemDefsfind.push_back(item);
							}
						SKIPFind2:;
						}
					SKIPFind3:;
						string listMiddle = "";
						string listFull = "";

						for (const ItemDefinition& item : itemDefsfind)
						{
							string kys = item.name;
							std::transform(kys.begin(), kys.end(), kys.begin(), ::tolower);
							string kms = itemFind;
							std::transform(kms.begin(), kms.end(), kms.begin(), ::tolower);
							if (kys.find(kms) != std::string::npos)
								listMiddle += "add_button_with_icon|tool" + to_string(item.id) + "|`$" + item.name + "``|left|" + to_string(item.id) + "||\n";
						}
						if (itemFind.length() < 3) {
							listFull = "add_textbox|`4Word is less then 3 letters!``|\nadd_spacer|small|\n";
							showWrong(peer, listFull, itemFind);
						}
						else if (itemDefsfind.size() == 0) {
							//listFull = "add_textbox|`4Found no item match!``|\nadd_spacer|small|\n";
							showWrong(peer, listFull, itemFind);

						}
						else {
							string finddialogs = "add_label_with_icon|big|`wFound item : " + itemFind + "``|left|6016|\nadd_spacer|small|\nadd_textbox|Enter a word below to find the item|\nadd_text_input|item|Item Name||20|\nend_dialog|findid|Cancel|Find the item!|\nadd_spacer|big|\n" + listMiddle + "add_quick_exit|\n";
							packet::dialog(peer, finddialogs);
						}
					}
				SKIPFind:;

					if (captcha) {
						int x;
						try {
							x = stoi(captchaanswer);
						}
						catch (std::invalid_argument& e) {
							continue;
						}
						string youareanalien = captchaanswer;
						if (x > hasil || x != hasil || captchaanswer.length() < 0 || captchaanswer == "") {
							packet::dialog(peer, "set_default_color|`o\nadd_label_with_icon|big|`wAre you Human?``|left|206|\nadd_spacer|small|\nadd_textbox|What will be the sum of the following numbers|left|\nadd_textbox|" + to_string(resultnbr1) + " + " + to_string(resultnbr2) + "|left|\nadd_text_input|captcha_answer|Answer:||32|\nadd_button|Captcha_|Submit|");
							break;
						}
						else {
							continue;
						}
					}

					if (isGuildDialog) {


						int GCState = PlayerDB::guildRegister(peer, guildName, guildStatement, guildFlagFg, guildFlagBg);
						if (GCState == -1) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oSpecial characters are not allowed in Guild name.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -2) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild name you've entered is too short.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -3) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild name you've entered is too long.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -4) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild name you've entered is already taken.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -5) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild Flag Background ID you've entered must be a number.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -6) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild Flag Foreground ID you've entered must be a number.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -7) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild Flag Background ID you've entered is too long or too short.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (GCState == -8) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oThe Guild Flag Foreground ID you've entered is too long or too short.``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						if (world->owner != ((PlayerInfo*)(peer->data))->rawName) {
							GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation|left|5814|\nadd_textbox|`4Oops! `oYou must make guild in world you owned!``|\nadd_text_input|gname|`oGuild Name:``|" + guildName + "|15|\nadd_text_input|gstatement|`oGuild Statement:``|" + guildStatement + "|40|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``|" + guildFlagBg + "|5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``|" + guildFlagFg + "|5|\n\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\n\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\n\nadd_spacer|small|\nend_dialog|guildconfirm|`wCancel``|`oCreate Guild``|\n"));
							ENetPacket* packet = enet_packet_create(ps.data,
								ps.len,
								ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packet);

							delete ps.data;
						}
						else {
							if (GCState == 1) {

								((PlayerInfo*)(peer->data))->createGuildName = guildName;
								((PlayerInfo*)(peer->data))->createGuildStatement = guildStatement;


								((PlayerInfo*)(peer->data))->createGuildFlagBg = guildFlagBg;
								((PlayerInfo*)(peer->data))->createGuildFlagFg = guildFlagFg;

								GamePacket ps = packetEnd(appendString(appendString(createPacket(), "OnDialogRequest"), "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild|left|5814|\nadd_textbox|`1Guild Name: `o" + guildName + "``|\nadd_textbox|`1Guild Statement: `o" + guildStatement + "``|\nadd_label_with_icon|small|`1<-Guild Flag Background``|left|" + guildFlagBg + "|\nadd_label_with_icon|small|`1<-Guild Flag Foreground``|left|" + guildFlagFg + "|\n\nadd_spacer|small|\nadd_textbox|`oCost: `4250,000 Gems``|\n\nadd_spacer|small|\nadd_button|confirmcreateguild|`oCreate Guild``|\nend_dialog||`wCancel``||\n"));
								ENetPacket* packet = enet_packet_create(ps.data,
									ps.len,
									ENET_PACKET_FLAG_RELIABLE);
								enet_peer_send(peer, 0, packet);

								delete ps.data;

							}
						}
					}

					if (btn == "worldPublic") if (((PlayerInfo*)(peer->data))->rawName == getPlyersWorld(peer)->owner) getPlyersWorld(peer)->isPublic = true;
					if (btn == "worldPrivate") if (((PlayerInfo*)(peer->data))->rawName == getPlyersWorld(peer)->owner) getPlyersWorld(peer)->isPublic = false;
					if (btn == "backsocialportal") {
						string kamugay = "set_default_color|`w\n\nadd_label_with_icon|big|Social Portal``|left|1366|\n\nadd_spacer|small|\nadd_button|backonlinelist|Show Friends``|0|0|\nend_dialog||OK||\nadd_quick_exit|";
						packet::dialog(peer, kamugay);
					}
					if (btn == "frnoption") {
						string checkboxshit = "add_checkbox|checkbox_public|Show location to friends|1";
						string checkboxshits = "add_checkbox|checkbox_notifications|Show friend notifications|1";;
						string req = "set_default_color|`o\n\nadd_label_with_icon|big|`wFriend Options``|left|1366|\n\nadd_spacer|small|\n" + checkboxshit + "\n" + checkboxshits + "\nadd_button||`oClose``|0|0|\nadd_quick_exit|";
						packet::dialog(peer, req);
					}
					if (btn == "open2") {
						WorldInfo* world = getPlyersWorld(peer);
						int x = 0;
						int y = 0;
						if (world->items[x + (y * world->width)].foreground == 9350) {

							int premwl = ((PlayerInfo*)(peer->data))->premwl;

							int valwls = 0;
							int valwl;
							valwls = rand() % 10;
							valwl = valwls + 1;
							((PlayerInfo*)(peer->data))->premwl = premwl + valwl;

							string message1 = "`9You found `2" + to_string(valwl) + " `9Premium WLS";
							packet::SendTalkSelf(peer, message1);
							std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


							if (ifff.fail()) {
								ifff.close();


							}
							if (ifff.is_open()) {
							}
							json j;
							ifff >> j; //load


							j["premwl"] = ((PlayerInfo*)(peer->data))->premwl;
							std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
							if (!o.is_open()) {
								cout << GetLastError() << endl;
								_getch();
							}

							o << j << std::endl;
						}
					}
					if (btn == "backonlinelist") {

						string onlinefrnlist = "";
						int onlinecount = 0;
						int totalcount = ((PlayerInfo*)(peer->data))->friendinfo.size();
						ENetPeer* currentPeer;

						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;

							string name = ((PlayerInfo*)(currentPeer->data))->rawName;
							if (find(((PlayerInfo*)(peer->data))->friendinfo.begin(), ((PlayerInfo*)(peer->data))->friendinfo.end(), name) != ((PlayerInfo*)(peer->data))->friendinfo.end()) {
								onlinefrnlist += "\nadd_button|onlinefrns_" + ((PlayerInfo*)(currentPeer->data))->rawName + "|`2ONLINE: `o" + ((PlayerInfo*)(currentPeer->data))->displayName + "``|0|0|";
								onlinecount++;

							}

						}
						if (totalcount == 0) {
							string singsing = "set_default_color|`o\n\nadd_label_with_icon|big|`o" + std::to_string(onlinecount) + " of " + std::to_string(totalcount) + " `wFriends Online``|left|1366|\n\nadd_spacer|small|\nadd_label|small|`oYou currently have no friends.  That's just sad.  To make some, click a person's wrench icon, then choose `5Add as friend`o.``|left|4|\n\nadd_spacer|small|\nadd_button|frnoption|`oFriend Options``|0|0|\nadd_button|backsocialportal|Back|0|0|\nadd_button||`oClose``|0|0|\nadd_quick_exit|";
							packet::dialog(peer, singsing);
						}
						else if (onlinecount == 0) {
							string kamusimp = "set_default_color|`o\n\nadd_label_with_icon|big|`o" + std::to_string(onlinecount) + " of " + std::to_string(totalcount) + " `wFriends Online``|left|1366|\n\nadd_spacer|small|\nadd_button|chc0|`wClose``|0|0|\nadd_label|small|`oNone of your friends are currently online.``|left|4|\n\nadd_spacer|small|\nadd_button|showoffline|`oShow offline``|0|0|\nadd_button|frnoption|`oFriend Options``|0|0|\nadd_button|backsocialportal|Back|0|0|\nadd_button||`oClose``|0|0|\nadd_quick_exit|";
							packet::dialog(peer, kamusimp);
						}

						else {
							string ngl = "set_default_color|`o\n\nadd_label_with_icon|big|`o" + std::to_string(onlinecount) + " of " + std::to_string(totalcount) + " `wFriends Online``|left|1366|\n\nadd_spacer|small|\nadd_button|chc0|`wClose``|0|0|" + onlinefrnlist + "\n\nadd_spacer|small|\nadd_button|showoffline|`oShow offline``|0|0|\nadd_button|frnoption|`oFriend Options``|0|0|\nadd_button|backsocialportal|Back|0|0|\nadd_button||`oClose``|0|0|\nadd_quick_exit|";
							packet::dialog(peer, ngl);
						}
					}
					if (btn == "showoffline") {
						string onlinelist = "";
						string offlinelist = "";
						string offname = "";
						int onlinecount = 0;
						int totalcount = ((PlayerInfo*)(peer->data))->friendinfo.size();
						vector<string>offliness = ((PlayerInfo*)(peer->data))->friendinfo;

						ENetPeer* currentPeer;

						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							string name = ((PlayerInfo*)(currentPeer->data))->rawName;

							if (find(((PlayerInfo*)(peer->data))->friendinfo.begin(), ((PlayerInfo*)(peer->data))->friendinfo.end(), name) != ((PlayerInfo*)(peer->data))->friendinfo.end()) {
								onlinelist += "\nadd_button|onlinefrns_" + ((PlayerInfo*)(currentPeer->data))->rawName + "|`2ONLINE: `o" + ((PlayerInfo*)(currentPeer->data))->displayName + "``|0|0|";
								onlinecount++;

								offliness.erase(std::remove(offliness.begin(), offliness.end(), name), offliness.end());
							}
						}
						for (std::vector<string>::const_iterator i = offliness.begin(); i != offliness.end(); ++i) {
							offname = *i;
							offlinelist += "\nadd_button|offlinefrns_" + offname + "|`4OFFLINE: `o" + offname + "``|0|0|";

						}

						/*if (onlinecount > 0) {
							string ureallygay = "set_default_color|`o\n\nadd_label_with_icon|big|`o" + std::to_string(onlinecount) + " of " + std::to_string(totalcount) + " `wFriends Online|left|1366|\n\nadd_spacer|small|\nadd_button|chc0|`wClose``|0|0|\n\nadd_spacer|small|\nadd_textbox|All of your friend are online!|\n\nadd_spacer|small| \n\nadd_spacer|small| \nadd_button|frnoption|`oFriend Options``|0|0|\nadd_button|backonlinelist|Back``|0|0|\nadd_button||`oClose``|0|0|\nadd_quick_exit|";
							packet::dialog(peer, ureallygay);
						}
						else {*/
						string gaying = "set_default_color|`o\n\nadd_label_with_icon|big|`o" + std::to_string(onlinecount) + " of " + std::to_string(totalcount) + " `wFriends Online|left|1366|\n\nadd_spacer|small|\nadd_button|chc0|`wClose``|0|0|\nadd_spacer|small|" + offlinelist + "\nadd_spacer|small|\n\nadd_button|frnoption|`oFriend Options``|0|0|\nadd_button|backonlinelist|Back``|0|0|\nadd_button||`oClose``|0|0|\nadd_quick_exit|";
						packet::dialog(peer, gaying);
					}
					if (btn == "createguild") {
						string guildisigay = "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild Creation``|left|5814|\nadd_spacer|small|\nadd_text_input|gname|Guild Name: ||20|\nadd_text_input|gstatement|Guild Statement: ||100|\nadd_text_input|ggcflagbg|`oGuild Flag Background ID:``||5|\nadd_text_input|ggcflagfg|`oGuild Flag Foreground ID:``||5|\nadd_spacer|small|\nadd_textbox|`oConfirm your guild settings by selecting `2Create Guild `obelow to create your guild.|\nadd_spacer|small|\nadd_textbox|`8Remember`o: A guild can only be created in a world owned by you and locked with a `5World Lock`o!|\nadd_spacer|small|\nadd_textbox|`4Warning! `oThe guild name cannot be changed once you have confirmed the guild settings!|\nadd_spacer|small|\nend_dialog|guildconfirm|Cancel|Create Guild|\nadd_quick_exit|";
						packet::dialog(peer, guildisigay);
					}
					if (btn == "createguildinfo") {
						string gayguild = "set_default_color|`o\n\nadd_label_with_icon|big|`wGrow Guild|left|5814|\nadd_label|small|`oWelcome to Grow Guilds where you can create a Guild! With a Guild you can level up the Guild to add more members.``|left|4|\n\nadd_spacer|small|\nadd_textbox|`oYou will be charged `60 `oGems.``|\nadd_spacer|small|\nadd_button|createguild|`oCreate a Guild``|0|0|\nadd_button|backsocialportal|Back|0|0|\nend_dialog||Close||\nadd_quick_exit|";
						packet::dialog(peer, gayguild);
						/*packet::consolemessage(peer, "`w[`2+`w] `wThis option will be added soon!");*/
					}
					if (btn == "confirmcreateguild") {
						packet::consolemessage(peer, "You created guild");
						string guildName = ((PlayerInfo*)(peer->data))->createGuildName;
						string guildStatement = ((PlayerInfo*)(peer->data))->createGuildStatement;
						string fixedguildName = PlayerDB::getProperName(guildName);
						string guildFlagbg = ((PlayerInfo*)(peer->data))->createGuildFlagBg;
						string guildFlagfg = ((PlayerInfo*)(peer->data))->createGuildFlagFg;

						//guildmem.push_back(((PlayerInfo*)(peer->data))->rawName);

						std::ofstream o("guilds/" + fixedguildName + ".json");
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}
						json j;
						vector<string> test1s;
						vector<string>test2s;

						((PlayerInfo*)(peer->data))->guildMembers.push_back(((PlayerInfo*)(peer->data))->rawName);
						j["GuildName"] = ((PlayerInfo*)(peer->data))->createGuildName;
						j["GuildRawName"] = fixedguildName;
						j["GuildStatement"] = ((PlayerInfo*)(peer->data))->createGuildStatement;
						j["Leader"] = ((PlayerInfo*)(peer->data))->rawName;
						j["Co-Leader"] = test1s;
						j["Elder-Leader"] = test2s;
						j["Member"] = ((PlayerInfo*)(peer->data))->guildMembers;
						j["GuildLevel"] = 0;
						j["GuildExp"] = 0;
						j["GuildWorld"] = ((PlayerInfo*)(peer->data))->currentWorld;
						j["backgroundflag"] = stoi(((PlayerInfo*)(peer->data))->createGuildFlagBg);
						j["foregroundflag"] = stoi(((PlayerInfo*)(peer->data))->createGuildFlagFg);
						o << j << std::endl;

						((PlayerInfo*)(peer->data))->guild = guildName;
						((PlayerInfo*)(peer->data))->joinguild = true;
						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json x;
						ifff >> x; //load

						x["guild"] = ((PlayerInfo*)(peer->data))->guild;
						x["joinguild"] = ((PlayerInfo*)(peer->data))->joinguild; //edit

						std::ofstream y("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!y.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						y << x << std::endl;
					}
					if (btn == "gpasslabel") {
						string ezgpass = "set_default_color|`o\nadd_label_with_icon|big|`9GrowPasses``|left|10410|\nadd_spacer|small|\nadd_button|gpasslvl1|`9Level 1 GrowPass|\nadd_button|growpasslvl2|`9Level 2 GrowPass|\nadd_button|growpasslvl3|`9Level 3 GrowPass``|0|0|\nend_dialog|gazette|Close|";
					    packet::dialog(peer, ezgpass);
					}

					if (btn == "showguild") {
						string onlinegmlist = "";
						string grole = "";
						int onlinecount = 0;
						string guildname = PlayerDB::getProperName(((PlayerInfo*)(peer->data))->guild);
						if (guildname != "") {
							try {
								std::ifstream ifff("guilds/" + guildname + ".json");
								if (ifff.fail()) {
									ifff.close();
									((PlayerInfo*)(peer->data))->guild = "";
									continue;
								}
								json j;
								ifff >> j;
								int gfbg, gffg, guildlvl, guildxp;
								string gstatement, gleader;
								vector<string> gmembers; vector<string> GE; vector<string> GC;
								gfbg = j["backgroundflag"];
								gffg = j["foregroundflag"];
								gstatement = j["GuildStatement"].get<std::string>();
								gleader = j["Leader"].get<std::string>();
								guildlvl = j["GuildLevel"];
								guildxp = j["GuildExp"];
								for (int i = 0; i < j["Member"].size(); i++) {
									gmembers.push_back(j["Member"][i]);
								}
								for (int i = 0; i < j["Elder-Leader"].size(); i++) {
									GE.push_back(j["Elder-Leader"][i]);
								}
								for (int i = 0; i < j["Co-Leader"].size(); i++) {
									GC.push_back(j["Co-Leader"][i]);
								}
								((PlayerInfo*)(peer->data))->guildlevel = guildlvl;
								((PlayerInfo*)(peer->data))->guildexp = guildxp;
								((PlayerInfo*)(peer->data))->guildBg = gfbg;
								((PlayerInfo*)(peer->data))->guildFg = gffg;
								((PlayerInfo*)(peer->data))->guildStatement = gstatement;
								((PlayerInfo*)(peer->data))->guildLeader = gleader;
								((PlayerInfo*)(peer->data))->guildMembers = gmembers;
								((PlayerInfo*)(peer->data))->guildGE = GE;
								((PlayerInfo*)(peer->data))->guildGC = GC;
								ifff.close();
							}
							catch (std::exception&) {
								SendConsole("showguild Critical error details: rawName(" + ((PlayerInfo*)(peer->data))->rawName + ")", "ERROR");
								enet_peer_disconnect_now(peer, 0);
							}
							catch (std::runtime_error&) {
								SendConsole("showguild Critical error details: name(" + ((PlayerInfo*)(peer->data))->rawName + ")", "ERROR");
								enet_peer_disconnect_now(peer, 0);
							}
							catch (...) {
								SendConsole("showguild Critical error details: name(" + ((PlayerInfo*)(peer->data))->rawName + ")", "ERROR");
								enet_peer_disconnect_now(peer, 0);
							}
						}
						for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
							string name = ((PlayerInfo*)(currentPeer->data))->rawName;
							if (find(((PlayerInfo*)(peer->data))->guildMembers.begin(), ((PlayerInfo*)(peer->data))->guildMembers.end(), name) != ((PlayerInfo*)(peer->data))->guildMembers.end()) {
								if (((PlayerInfo*)(currentPeer->data))->rawName == ((PlayerInfo*)(peer->data))->guildLeader) {
									onlinegmlist += "\nadd_button|onlinegm_" + ((PlayerInfo*)(currentPeer->data))->rawName + "|`2ONLINE: `o" + ((PlayerInfo*)(currentPeer->data))->tankIDName + " `e(GL)``|0|0|";
									onlinecount++;
								}
								else {
									onlinegmlist += "\nadd_button|onlinegm_" + ((PlayerInfo*)(currentPeer->data))->rawName + "|`2ONLINE: `o" + ((PlayerInfo*)(currentPeer->data))->tankIDName + " " + grole + "``|0|0|";
									onlinecount++;
								}
							}
						}
						int guildsize = 15;
						if (((PlayerInfo*)(peer->data))->guildlevel == 2) guildsize = 20;
						else if (((PlayerInfo*)(peer->data))->guildlevel == 3) guildsize = 25;
						else if (((PlayerInfo*)(peer->data))->guildlevel == 4) guildsize = 30;
						else if (((PlayerInfo*)(peer->data))->guildlevel == 5) guildsize = 35;
						else if (((PlayerInfo*)(peer->data))->guildlevel == 6) guildsize = 40;
						else if (((PlayerInfo*)(peer->data))->guildlevel == 7) guildsize = 45;
						else if (((PlayerInfo*)(peer->data))->guildlevel == 8) guildsize = 50;
						if (((PlayerInfo*)(peer->data))->rawName == ((PlayerInfo*)(peer->data))->guildLeader) {
							string guilds = "set_default_color|`o\n\nadd_dual_layer_icon_label|big|`wGuild Home|left|" + std::to_string(((PlayerInfo*)(peer->data))->guildBg) + "|" + std::to_string(((PlayerInfo*)(peer->data))->guildFg) + "|1.0|0|\n\nadd_spacer|small|\nadd_textbox|`oGuild Name: " + guildname + "|left|\nadd_textbox|Guild Statement: " + ((PlayerInfo*)(peer->data))->guildStatement + "``|\nadd_textbox|`oGuild size: " + to_string(guildsize) + " members|\nadd_textbox|`oGuild Level: " + std::to_string(((PlayerInfo*)(peer->data))->guildlevel) + "|\n\nadd_spacer|small|\nadd_button|guildoffline|`wShow offline too``|0|0|\nadd_button|goguildhome|`wGo to Guild Home``|0|0|\nadd_button|guildleveluper|`wUpgrade Guild``|0|0|\nadd_button|EditStatement|`wEdit Guild Statement|0|0|\nadd_button|leavefromguild|`4Abandon Guild``|0|0|\n\nadd_spacer|small|\nadd_textbox|`5" + std::to_string(onlinecount) + " of " + std::to_string(((PlayerInfo*)(peer->data))->guildMembers.size()) + " `wGuild Members Online|" + onlinegmlist + "\n\nadd_spacer|small|\nadd_button||`wClose``|0|0|\nadd_quick_exit|";
							packet::dialog(peer, guilds);
						}
						else {
							string guild = "set_default_color|`o\n\nadd_dual_layer_icon_label|big|`wGuild Home|left|" + std::to_string(((PlayerInfo*)(peer->data))->guildBg) + "|" + std::to_string(((PlayerInfo*)(peer->data))->guildFg) + "|1.0|0|\n\nadd_spacer|small|\nadd_textbox|`oGuild Name: " + guildname + "|left|\nadd_textbox|Guild Statement: " + ((PlayerInfo*)(peer->data))->guildStatement + "``|\nadd_textbox|`oGuild size: " + to_string(guildsize) + " members|\nadd_textbox|`oGuild Level: " + std::to_string(((PlayerInfo*)(peer->data))->guildlevel) + "|\n\nadd_spacer|small|\nadd_button|guildoffline|`wShow offline too``|0|0|\nadd_button|goguildhome|`wGo to Guild Home``|0|0|\nadd_button|leavefromguild|`4Leave from guild``|0|0|\n\nadd_spacer|small|\nadd_textbox|`5" + std::to_string(onlinecount) + " of " + std::to_string(((PlayerInfo*)(peer->data))->guildMembers.size()) + " `wGuild Members Online|" + onlinegmlist + "\n\nadd_spacer|small|\nadd_button||`wClose``|0|0|\nadd_quick_exit|";
							packet::dialog(peer, guild);
						}
					}
					if (btn == "buygems") {
						string gayinggems = "set_default_color|\nadd_label_with_icon|big|`wPurchase Gems``|left|112|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\n\nadd_spacer|small|\nadd_textbox|Price: `320000 / 1 Growtopia World Lock.|left|\nadd_textbox|Duration: `w[`4~`w]|left|\nadd_textbox|Stock: `w[`4~`w]|\nadd_spacer|small|\nadd_textbox|`9Rules:|left|\nadd_smalltext|`91. `2Do not sell it to other people.|left|\nadd_smalltext|`92. `2Trying To Sell Your Gems To Other People Will Result Ban/Ipban.|left|\nadd_spacer|left|\nadd_textbox|`eHow To Buy:|\nadd_smalltext|`rIf u want buy `9Gems`r, Message `4@OWNER `ron Discord Server.``|left|\nadd_spacer|small|\nadd_textbox|`eWhen will i received my purchase:|\nadd_smalltext|`rYou will receive within `424`r hours after you have made your payment.|left|\n\nadd_textbox|`oHow much you want to buy??|\nadd_text_input|textgems|||100|\nend_dialog|gemsdialog|Cancel|OK|\n";
						packet::dialog(peer, gayinggems);
					}
					if (btn == "growmojis") {
						sendGrowmojis(peer);
					}
					if (btn == "buyminimod")
					{
						if (((PlayerInfo*)(peer->data))->adminLevel > 665) {
							packet::consolemessage(peer, "`4You can't longer purchase Mod Rank.");
							continue;
						}
						string gaymod = "set_default_color|\nadd_label_with_icon|big|`wPurchase Moderator``|left|278|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\n\nadd_spacer|small|\nadd_textbox|Price: `3600 `9Premium Wls`w.|left|\nadd_textbox|Duration: `w[`4~`w]|left|\nadd_textbox|Stock: `w[`4~`w]|\nadd_spacer|small|\nadd_textbox|`9Rules:|left|\nadd_smalltext|`91. `2Do Not Abuse Your Role|left|\nadd_smalltext|`92. `2if you are going to ban people, make sure to have screenshots/video proof.|left|\nadd_smalltext|`93. `2Sharing Acoount will result in account loss.|left|\nadd_smalltext|`94. `2Trying to sell account will result in ip-banned.|left|\nadd_spacer|small|\nadd_textbox|`9Commands:|small|\nadd_smalltext|`eAll commands are displayed in /mhelp (moderator help).|small|\nadd_spacer|left|\nadd_textbox|`eHow To Buy:|\nadd_smalltext|`rIf u want buy `#@Moderator `rRank, Message `6Server Creator `ron Discord Server.``|left|\nadd_spacer|small|\nadd_textbox|`eWhen will i receive my purchase:|\nadd_smalltext|`rYou will receive it within `424`r hours after you have made your payment.|left|\nadd_button|buymodpremium|`wPurchase `^Moderator `wWith premium wls!|noflags|0|0|\nadd_spacer|small|\nend_dialog|gazette|Close||";
						packet::dialog(peer, gaymod);
					}
					if (btn == "buygpass1")
					{
						if (((PlayerInfo*)(peer->data))->adminLevel > 110) {
							packet::consolemessage(peer, "`4You can't longer purchase GrowPass.");
							continue;
						}
						string gaypass = "set_default_color|\nadd_label_with_icon|big|`wPurchase GrowPass``|left|278|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\n\nadd_spacer|small|\nadd_textbox|Price: `350 `9Premium Wls`w.|left|\nadd_textbox|Duration: `w[`4~`w]|left|\nadd_textbox|Stock: `w[`4~`w]|\nadd_spacer|small|\nadd_textbox|`9Rules:|left|\nadd_smalltext|`91. `2Do not sell your account.|left|\nadd_smalltext|`92. `2No WL Refund after buy.|left|\nadd_spacer|small|\nadd_textbox|`9Benefits:|small|\nadd_smalltext|`eYou can get Lvl 1 GrowPass Rewards.|small|\nadd_spacer|left|\nadd_textbox|`eHow To Buy:|\nadd_smalltext|`rIf u want buy `9GrowPass`w, Message `6Server Creator `ron Discord Server.``|left|\nadd_spacer|small|\nadd_textbox|`eWhen will i receive my purchase:|\nadd_smalltext|`rYou will receive it within `424`r hours after you have made your payment.|left|\nadd_button|buygrowpass1|`wPurchase `9GrowPass `wWith premium wls!|noflags|0|0|\nadd_spacer|small|\nend_dialog|gazette|Close||";
						packet::dialog(peer, gaypass);
					}
					if (btn == "gpasslvl1") {
						if (((PlayerInfo*)(peer->data))->adminLevel < 111) {
							packet::consolemessage(peer, "`4You must purchase GrowPass first.");
							continue;
						}
						if (((PlayerInfo*)(peer->data))->adminLevel > 111) {
							packet::consolemessage(peer, "`4You must purchase GrowPass first.");
							continue;
						}
						lvl1growpass(peer);
					}
					if (btn == "gpass1") {
						if (((PlayerInfo*)(peer->data))->adminLevel = 111) {
							bool claimed = false;
							bool success = true;
							SaveShopsItemMoreTimes(8552, 1, peer, success);
							if (!success) break;
							packet::SendTalkSelf(peer, "`2Successfully claimed Angel of Mercy's Wing.");
							if (!claimed)
							{
								packet::consolemessage(peer, "`2You have claimed this reward before.");
								break;
							}
						}
					}
					if (btn == "buyvip")
					{
						if (((PlayerInfo*)(peer->data))->adminLevel > 443) {
							packet::consolemessage(peer, "`4You can't longer purchase VIP Rank.");
							continue;
						}
						string gayvip = "set_default_color|\nadd_label_with_icon|big|`wPurchase VIP``|left|6802|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\n\nadd_spacer|small|\nadd_textbox|Price: `2200 `9Premium Wls`w.|left|\nadd_textbox|Duration: `w[`4~`w]|left|\nadd_textbox|Stock: `w[`4~`w]|\nadd_spacer|small|\nadd_textbox|`9Rules:|left|\nadd_smalltext|`91. `2Do Not Abuse Your Role|left|\nadd_smalltext|`94. `2Trying to sell account will result in ip-banned.|left|\nadd_spacer|small|\nadd_textbox|`9Commands:|small|\nadd_smalltext|`eAll commands are displayed in /vhelp (vip help).|small|\nadd_spacer|left|\nadd_textbox|`eHow To Buy:|\nadd_smalltext|``rIf u want buy `9VIP `rRank, Message `6SERVER Creator `ron Discord Server.``|left|\nadd_spacer|small|\nadd_textbox|`eWhen will i receive my purchase:|\nadd_smalltext|`rYou will receive it within `424`r hours after you have made your payment.|left|\nadd_button|confirmvip1|`wPurchase `1VIP `wWith Premium WL|noflags|0|0|\nadd_spacer|small|\nend_dialog|gazette|Close||";
						packet::dialog(peer, gayvip);
					}
					if (btn == "buygacha")
					{
						string gaycha = "set_default_color|\nadd_label_with_icon|big|`wPurchase Gacha``|left|6802|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\n\nadd_spacer|small|\nadd_textbox|Price: `21 `9Premium Wls per chest`w.|left|\nadd_textbox|Benefit : Earn 0-10 WLS with this chest|left|\nadd_textbox|Stock: `w[`4~`w]|small|left|\nadd_spacer|small|\nadd_textbox|`eWhen will i receive my purchase:|\nadd_smalltext|`rYou will receive it within `424`r hours after you have made your payment.|left|\nadd_button|confirmgacha|`wPurchase `1Gacha `wWith Premium WL|noflags|0|0|\nadd_spacer|small|\nend_dialog|gazette|Close||";
						packet::dialog(peer, gaycha);
					}
					if (btn == "buymodpremium")
					{
						int premwl = ((PlayerInfo*)(peer->data))->premwl;
						if (premwl >= 600)
						{
							string buygaymod = "set_default_color|`w\n\nadd_label_with_icon|big|`oPurchase Confirmation``|left|1366|\nadd_spacer|small|\nadd_label|small|`4You'll give:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`o600`w) Premium WLS``|left|242|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_label|small|`2You'll get:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`oPermanent`w) Moderator Role``|left|32|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_button|confirmmod1|`wDo The Purchase!|noflags|0|0|\nadd_button|cancel|`oCancel|noflags|0|0|";
							packet::dialog(peer, buygaymod);
						}
						else
						{
							packet::consolemessage(peer, "`4You don't have enough Premium Wls!");
						}
					}
					if (btn == "buygrowpass1")
					{
						int premwl = ((PlayerInfo*)(peer->data))->premwl;
						if (premwl >= 100)
						{
							string buygaypass1 = "set_default_color|`w\n\nadd_label_with_icon|big|`oPurchase Confirmation``|left|1366|\nadd_spacer|small|\nadd_label|small|`4You'll give:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`o100`w) Premium WLS``|left|242|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_label|small|`2You'll get:|left|23|\nadd_spacer|small|\nadd_label_with_icon|small|`w(`oPermanent`w) GrowPass Role``|left|32|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_spacer|small|\nadd_button|confirmgpass1|`wDo The Purchase!|noflags|0|0|\nadd_button|cancel|`oCancel|noflags|0|0|";
							packet::dialog(peer, buygaypass1);
						}
						else
						{
							packet::consolemessage(peer, "`4You don't have enough Premium Wls!");
						}
					}
					if (btn == "confirmgems") {
						std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						std::string acontent((std::istreambuf_iterator<char>(ifsz)),
							(std::istreambuf_iterator<char>()));
						int buygems = ((PlayerInfo*)(peer->data))->buygems;
						int buygemsz = buygems * 20000;
						int a = atoi(acontent.c_str());
						int aa = a + buygemsz;
						int cwl = ((PlayerInfo*)(peer->data))->premwl;
						int rwl = cwl - buygems;
						((PlayerInfo*)(peer->data))->premwl = rwl;
						ofstream myfile;
						myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						myfile << aa;
						myfile.close();
						GamePacket psa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), aa));
						ENetPacket* packetsa = enet_packet_create(psa.data, psa.len, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packetsa);
						delete psa.data;

						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json j;
						ifff >> j; //load


						j["premwl"] = ((PlayerInfo*)(peer->data))->premwl; //edit



						std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						o << j << std::endl;
					}
					if (btn == "confirmlevel") {
						int level = ((PlayerInfo*)(peer->data))->level;
						int blevel = ((PlayerInfo*)(peer->data))->buygems;
						int tlevel = level + blevel;
						int wls = ((PlayerInfo*)(peer->data))->premwl;
						int minuswl = blevel / 10;
						int rwl = wls - minuswl;
						((PlayerInfo*)(peer->data))->premwl = rwl;
						((PlayerInfo*)(peer->data))->level = tlevel;
						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json j;
						ifff >> j; //load


						j["level"] = tlevel; //edit
						j["premwl"] = ((PlayerInfo*)(peer->data))->premwl = rwl;; //edit



						std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						o << j << std::endl;
					}
					if (btn == "confirmmod1") {
						int premwl = ((PlayerInfo*)(peer->data))->premwl;

						((PlayerInfo*)(peer->data))->premwl = premwl - 600;
						((PlayerInfo*)(peer->data))->adminLevel = 666;

						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json j;
						ifff >> j; //load


						j["premwl"] = ((PlayerInfo*)(peer->data))->premwl; //edit
						j["adminLevel"] = ((PlayerInfo*)(peer->data))->adminLevel; //edit



						std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						o << j << std::endl;
						string imie = ((PlayerInfo*)(peer->data))->rawName;
						string message2 = "`w** Player `3" + imie + " `wis the newest member of `bMODERATOR!`w **";
						packet::consolemessage(peer, "`2You bought moderator.");
						string text = "action|play_sfx\nfile|audio/double_chance.wav\ndelayMS|0\n";
						BYTE* data = new BYTE[5 + text.length()];
						BYTE zero = 0;
						int type = 3;
						memcpy(data, &type, 4);
						memcpy(data + 4, text.c_str(), text.length());
						memcpy(data + 4 + text.length(), &zero, 1);
						ENetPeer* currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							packet::consolemessage(currentPeer, message2);
						}
						enet_peer_disconnect_later(peer, 0);
					}
					if (btn == "confirmgpass1") {
						int premwl = ((PlayerInfo*)(peer->data))->premwl;

						((PlayerInfo*)(peer->data))->premwl = premwl - 100;
						((PlayerInfo*)(peer->data))->adminLevel = 111;

						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json j;
						ifff >> j; //load


						j["premwl"] = ((PlayerInfo*)(peer->data))->premwl; //edit
						j["adminLevel"] = ((PlayerInfo*)(peer->data))->adminLevel; //edit



						std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						o << j << std::endl;
						string imie = ((PlayerInfo*)(peer->data))->rawName;
						string message2 = "`w** Player `3" + imie + " `wis the newest member of `2GROWPASS!`w **";
						packet::consolemessage(peer, "`2You bought GrowPass.");
						string text = "action|play_sfx\nfile|audio/double_chance.wav\ndelayMS|0\n";
						BYTE* data = new BYTE[5 + text.length()];
						BYTE zero = 0;
						int type = 3;
						memcpy(data, &type, 4);
						memcpy(data + 4, text.c_str(), text.length());
						memcpy(data + 4 + text.length(), &zero, 1);
						ENetPeer* currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							packet::consolemessage(currentPeer, message2);
						}
						enet_peer_disconnect_later(peer, 0);
					}
					if (btn == "confirmvip1") {
						int premwl = ((PlayerInfo*)(peer->data))->premwl;

						((PlayerInfo*)(peer->data))->premwl = premwl - 200;
						((PlayerInfo*)(peer->data))->adminLevel = 444;

						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json j;
						ifff >> j; //load


						j["premwl"] = ((PlayerInfo*)(peer->data))->premwl; //edit
						j["adminLevel"] = ((PlayerInfo*)(peer->data))->adminLevel; //edit



						std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						o << j << std::endl;
						string imie = ((PlayerInfo*)(peer->data))->rawName;
						string message2 = "`w** Player `3" + imie + " `wis the newest member of `1VIP!`w **";

						string text = "action|play_sfx\nfile|audio/double_chance.wav\ndelayMS|0\n";
						BYTE* data = new BYTE[5 + text.length()];
						BYTE zero = 0;
						int type = 3;
						memcpy(data, &type, 4);
						memcpy(data + 4, text.c_str(), text.length());
						memcpy(data + 4 + text.length(), &zero, 1);
						ENetPeer* currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							packet::consolemessage(peer, message2);
						}
						enet_peer_disconnect_later(peer, 0);
					}
					if (btn == "confirmgacha") {
						int premwl = ((PlayerInfo*)(peer->data))->premwl;

						((PlayerInfo*)(peer->data))->premwl = premwl - 1;

						std::ifstream ifff("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json");


						if (ifff.fail()) {
							ifff.close();


						}
						if (ifff.is_open()) {
						}
						json j;
						ifff >> j; //load


						j["premwl"] = ((PlayerInfo*)(peer->data))->premwl; //edit

						bool success = true;
						SaveShopsItemMoreTimes(9350, 1, peer, success);
						packet::consolemessage(peer, "`9You bought 1 Super Golden Booty Chest.");

						std::ofstream o("players/" + ((PlayerInfo*)(peer->data))->rawName + ".json"); //save
						if (!o.is_open()) {
							cout << GetLastError() << endl;
							_getch();
						}

						o << j << std::endl;
					}
					if (btn == "buylvl") {
						int level = ((PlayerInfo*)(peer->data))->level;
						if (level > 124) {
							packet::consolemessage(peer, "`4You can't longer purchase level.");
							continue;
						}
						string buygaylvl = "set_default_color|\nadd_label_with_icon|big|`wPurchase Level``|left|18|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\n\nadd_spacer|small|\nadd_textbox|Price: `310 / 1 Growtopia World Lock.|left|\nadd_textbox|Duration: `w[`4~`w]|left|\nadd_textbox|Stock: `w[`4~`w]|\nadd_spacer|small|\nadd_textbox|`9Rules:|left|\nadd_smalltext|`91. `2Trying Sell Your Account Will Result Ipban.|left|\nadd_spacer|left|\nadd_textbox|`eHow To Buy:|\nadd_smalltext|`rIf u want buy `#2Level`r, Message `4@OWNER `ron Discord Server.``|left|\nadd_spacer|small|\nadd_textbox|`eWhen will i receive my purchase:|\nadd_smalltext|`rYou will receive it within `424`r hours after you have made your payment.|left|\nadd_text_input|textlevel|||100|\nend_dialog|leveldialog|Cancel|OK|\n";
						packet::dialog(peer, buygaylvl);
					}
					if (btn == "ingamerole") {
						int wl = ((PlayerInfo*)(peer->data))->premwl;

						string gaysstore = "set_default_color|`o\n\nadd_label_with_icon|big|`wWelcome to our store!``|left|1430|\nadd_label_with_icon|small|`9Your `9Premium WLS = `w " + to_string(wl) + "|noflags|242|\nadd_spacer|\nadd_smalltext|`5Deposit world in `2Your World Name`5, `91 Premium WL = 1 Real GT WL`5.|left|\nadd_button_with_icon|buyminimod||staticBlueFrame|276|\nadd_button_with_icon|buyvip||staticBlueFrame|274|\nadd_button_with_icon|buygpass1||staticBlueFrame|10410|\nadd_button_with_icon|buygacha||staticBlueFrame|9350|\nadd_button_with_icon|buygems||staticBlueFrame|112|\nadd_button_with_icon|buylvl||staticBlueFrame|1488||\nadd_button_with_icon||END_LIST|noflags|0|0|\nadd_button|continue|Close|";
						packet::dialog(peer, gaysstore);
					}

					if (isEpoch) {
						PlayerInfo* pinfo = ((PlayerInfo*)(peer->data));
						int x = pinfo->wrenchedBlockLocation % world->width;
						int y = pinfo->wrenchedBlockLocation / world->width;
						if (land == true) {
							world->land = land;
							world->weather = 40;
							ENetPeer* currentPeer;

							for (currentPeer = server->peers;
								currentPeer < &server->peers[server->peerCount];
								++currentPeer)
							{
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
									continue;
								if (isHere(peer, currentPeer))
								{
									GamePacket p2 = packetEnd(appendInt(appendString(createPacket(), "OnSetCurrentWeather"), world->weather));
									ENetPacket* packet2 = enet_packet_create(p2.data,
										p2.len,
										ENET_PACKET_FLAG_RELIABLE);

									enet_peer_send(currentPeer, 0, packet2);
									delete p2.data;
									continue;
								}
							}

						}
						else {
							world->land = false;
						}
						if (volcano == true) {
							world->volcano = volcano;
							world->weather = 39;
							ENetPeer* currentPeer;

							for (currentPeer = server->peers;
								currentPeer < &server->peers[server->peerCount];
								++currentPeer)
							{
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
									continue;
								if (isHere(peer, currentPeer))
								{
									GamePacket p2 = packetEnd(appendInt(appendString(createPacket(), "OnSetCurrentWeather"), world->weather));
									ENetPacket* packet2 = enet_packet_create(p2.data,
										p2.len,
										ENET_PACKET_FLAG_RELIABLE);

									enet_peer_send(currentPeer, 0, packet2);
									delete p2.data;
									continue;
								}
							}

						}
						else {
							world->volcano = false;
						}
						if (ice == true) {
							world->ice = ice;
							world->weather = 38;
							ENetPeer* currentPeer;

							for (currentPeer = server->peers;
								currentPeer < &server->peers[server->peerCount];
								++currentPeer)
							{
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
									continue;
								if (isHere(peer, currentPeer))
								{
									GamePacket p2 = packetEnd(appendInt(appendString(createPacket(), "OnSetCurrentWeather"), world->weather));
									ENetPacket* packet2 = enet_packet_create(p2.data,
										p2.len,
										ENET_PACKET_FLAG_RELIABLE);

									enet_peer_send(currentPeer, 0, packet2);
									delete p2.data;
									continue;
								}
							}

						}
						else {
							world->ice = false;
						}

					}
#ifdef REGISTRATION
					if (isRegisterDialog) {

						int regState = PlayerDB::playerRegister(peer, username, password, passwordverify, email, discord);
						if (regState == 1) {
							packet::consolemessage(peer, "`rYour account has been created!``");
							gamepacket_t p;
							p.Insert("SetHasGrowID");
							p.Insert(1);
							p.Insert(username);
							p.Insert(password);
							((PlayerInfo*)(event.peer->data))->haveGrowId = true;
							p.CreatePacket(peer);

							enet_peer_disconnect_later(peer, 0);
						}
						else if(regState==-1) {
							packet::consolemessage(peer, "`rAccount creation has failed, because it already exists!``");
						}
						else if (regState == -2) {
							packet::consolemessage(peer, "`rAccount creation has failed, because the name is too short!``");
						}
						else if (regState == -3) {
							packet::consolemessage(peer, "`4Passwords mismatch!``");
						}
						else if (regState == -4) {
							packet::consolemessage(peer, "`4Account creation has failed, because email address is invalid!``");
						}
						else if (regState == -5) {
							packet::consolemessage(peer, "`4Account creation has failed, because Discord ID is invalid!``");
						}
					}
#endif
				}
				if (cch.find("action|trash\n|itemID|") == 0) {
					std::stringstream ss(cch);
					std::string to;
					int idx = -1;
					int count = -1;
					while (std::getline(ss, to, '\n')) {
						vector<string> infoDat = explode("|", to);
						if (infoDat.size() == 3) {
							if (infoDat[1] == "itemID") idx = atoi(infoDat[2].c_str());
							if (infoDat[1] == "count") count = atoi(infoDat[2].c_str());
						}
					}
					((PlayerInfo*)(peer->data))->lasttrashitem = idx;
					((PlayerInfo*)(peer->data))->lasttrashitemcount = count;
					int id = ((PlayerInfo*)(peer->data))->lasttrashitem;
					if (idx == -1) continue;
					if (itemDefs.size() < idx || idx < 0) continue;
					if (id < 1 || id > maxItems) continue;
					if (id == 6336 || id == 18 || id == 32 || id == 10456 || id == 7480) {
						packet::OnTextOverlay(peer, "`wYou'd be sorry if you lost that!");
						continue;
					}
					else {
						auto trash = 0;
						for (auto i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++) {
							if (((PlayerInfo*)(peer->data))->inventory.items.at(i).itemID == ((PlayerInfo*)(peer->data))->lasttrashitem && ((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount >= 1) {
								trash = ((PlayerInfo*)(peer->data))->inventory.items.at(i).itemCount;
							}
						}
						string gaetrash = "add_label_with_icon|big|`4Trash `w" + itemDefs.at(idx).name + "``|left|" + std::to_string(idx) + "|\nadd_textbox|`oHow many to `4destroy`o? `o(You have " + to_string(trash) + ")|\nadd_text_input|trashitemcount||0|5|\nend_dialog|trashdialog|Cancel|Ok|\n";
						packet::dialog(peer, gaetrash);
					}
				}
				string dropText = "action|drop\n|itemID|";
				if (cch.find(dropText) == 0)
				{
					sendDrop(peer, -1, ((PlayerInfo*)(peer->data))->x + (32 * (((PlayerInfo*)(peer->data))->isRotatedLeft?-1:1)), ((PlayerInfo*)(peer->data))->y, atoi(cch.substr(dropText.length(), cch.length() - dropText.length() - 1).c_str()), 1, 0, false);
					/*int itemID = atoi(cch.substr(dropText.length(), cch.length() - dropText.length() - 1).c_str());
					PlayerMoving data;
					data.packetType = 14;
					data.x = ((PlayerInfo*)(peer->data))->x;
					data.y = ((PlayerInfo*)(peer->data))->y;
					data.netID = -1;
					data.plantingTree = itemID;
					float val = 1; // item count
					BYTE val2 = 0; // if 8, then geiger effect
					
					BYTE* raw = packPlayerMoving(&data);
					memcpy(raw + 16, &val, 4);
					memcpy(raw + 1, &val2, 1);
					SendPacketRaw(4, raw, 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);*/
				}
				if (cch.find("text|") != std::string::npos){
					PlayerInfo* pData = ((PlayerInfo*)(peer->data));
					if (str == "/mod")
					{
						((PlayerInfo*)(peer->data))->canWalkInBlocks = true;
						sendState(peer);
							/*PlayerMoving data;
							data.packetType = 0x14;
							data.characterState = 0x0; // animation
							data.x = 1000;
							data.y = 1;
							data.punchX = 0;
							data.punchY = 0;
							data.XSpeed = 300;
							data.YSpeed = 600;
							data.netID = ((PlayerInfo*)(peer->data))->netID;
							data.plantingTree = 0xFF;
							SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);*/
					}
					else if (str.substr(0, 7) == "/state ")
					{
						PlayerMoving data;
						data.packetType = 0x14;
						data.characterState = 0x0; // animation
						data.x = 1000;
						data.y = 0;
						data.punchX = 0;
						data.punchY = 0;
						data.XSpeed = 300;
						data.YSpeed = 600;
						data.netID = ((PlayerInfo*)(peer->data))->netID;
						data.plantingTree = atoi(str.substr(7, cch.length() - 7 - 1).c_str());
						SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
					}
					else if (str == "/save")
					{
						if (((PlayerInfo*)(peer->data))->adminLevel < 1337) {
							packet::consolemessage(peer, "You can't do that");
							break;
						}
						saveAllWorlds();
					}
					else if (str == "/howgay") {
						ENetPeer* currentPeer;
						int val = rand() % 100;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							if (isHere(peer, currentPeer))
							{
								GamePacket p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`w" + ((PlayerInfo*)(peer->data))->displayName + " `oare `2" + std::to_string(val) + "% `wgay!"), 0));
								ENetPacket* packet2 = enet_packet_create(p2.data,
									p2.len,
									ENET_PACKET_FLAG_RELIABLE);
								enet_peer_send(currentPeer, 0, packet2);
								delete p2.data;
								GamePacket p0 = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), ((PlayerInfo*)(peer->data))->displayName + " `ware `2%" + std::to_string(val) + " `wgay!"));
								ENetPacket* packet0 = enet_packet_create(p0.data,
									p0.len,
									ENET_PACKET_FLAG_RELIABLE);

								enet_peer_send(currentPeer, 0, packet0);
								delete p0.data;
							}
						}
					}
					else if (str.substr(0, 9) == "/givedev ") {
						if (((PlayerInfo*)(peer->data))->rawName == "ibord") {
							string name = str.substr(11, str.length());
							if ((str.substr(11, cch.length() - 11 - 1).find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != string::npos)) continue;
							bool found = false;
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
								string name2 = ((PlayerInfo*)(currentPeer->data))->rawName;
								std::transform(name.begin(), name.end(), name.begin(), ::tolower);
								std::transform(name2.begin(), name2.end(), name2.begin(), ::tolower);
								if (name == name2) {
									packet::OnAddNotification(currentPeer, "`$You have been promoted to `6@Developer", "audio/hub_open.wav", "interface/atomic_button.rttex");
									((PlayerInfo*)(currentPeer->data))->adminLevel = 1337;
									ifstream fg("players/" + ((PlayerInfo*)(currentPeer->data))->rawName + ".json");
									json j;
									fg >> j;
									fg.close();
									j["adminLevel"] = ((PlayerInfo*)(currentPeer->data))->adminLevel;
									ofstream fs("players/" + ((PlayerInfo*)(currentPeer->data))->rawName + ".json");
									fs << j;
									fs.close();
									found = true;
									string name3;
									string namemsg = ((PlayerInfo*)(currentPeer->data))->rawName;
									name3 = "`6@" + ((PlayerInfo*)(currentPeer->data))->tankIDName;
									((PlayerInfo*)(currentPeer->data))->haveSuperSupporterName = true;
									((PlayerInfo*)(currentPeer->data))->displayName = name3;
									packet::OnNameChanged(currentPeer, ((PlayerInfo*)(currentPeer->data))->netID, name3);
								}
							}
							if (found) {
								cout << "Server operator " << ((PlayerInfo*)(peer->data))->rawName << " has giveowner to " << str.substr(7, cch.length() - 7 - 1) << "." << endl;
								packet::consolemessage(peer, "`oSuccessfully given `6@Developer `oto player " + name + ".");
							}
							else {
								packet::consolemessage(peer, "`4Player not found");
							}
						}
					}
					else if (str == "/unequip")
					{
						((PlayerInfo*)(peer->data))->cloth_hair = 0;
						((PlayerInfo*)(peer->data))->cloth_shirt = 0;
						((PlayerInfo*)(peer->data))->cloth_pants = 0;
						((PlayerInfo*)(peer->data))->cloth_feet = 0;
						((PlayerInfo*)(peer->data))->cloth_face = 0;
						((PlayerInfo*)(peer->data))->cloth_hand = 0;
						((PlayerInfo*)(peer->data))->cloth_back = 0;
						((PlayerInfo*)(peer->data))->cloth_mask = 0;
						((PlayerInfo*)(peer->data))->cloth_necklace = 0;
						sendClothes(peer);
					}
					else if (str == "/purchase")
					{
						int wl = ((PlayerInfo*)(peer->data))->premwl;

						string gaystore = "set_default_color|`o\n\nadd_label_with_icon|big|`wWelcome to our store!``|left|1430|\nadd_label_with_icon|small|`9Your `9Premium WLS = `w " + to_string(wl) + "|noflags|242|\nadd_spacer|\nadd_smalltext|`5Deposit world in `2Your World Name`5, `91 Premium WL = 1 Real GT WL`5.|left|\nadd_button_with_icon|buyminimod||staticBlueFrame|276|\nadd_button_with_icon|buyvip||staticBlueFrame|274|\nadd_button_with_icon|buygpass1||staticBlueFrame|10410|\nadd_button_with_icon|buygacha||staticBlueFrame|9350|\nadd_button_with_icon|buygems||staticBlueFrame|112|\nadd_button_with_icon|buylvl||staticBlueFrame|1488||\nadd_button_with_icon||END_LIST|noflags|0|0|\nadd_button|continue|Close|";
						packet::dialog(peer, gaystore);
					}
					else if (str == "/wizard")
					{
						((PlayerInfo*)(peer->data))->cloth_hair = 0;
						((PlayerInfo*)(peer->data))->cloth_shirt = 0;
						((PlayerInfo*)(peer->data))->cloth_pants = 0;
						((PlayerInfo*)(peer->data))->cloth_feet = 0;
						((PlayerInfo*)(peer->data))->cloth_face = 1790;
						((PlayerInfo*)(peer->data))->cloth_hand = 0;
						((PlayerInfo*)(peer->data))->cloth_back = 0;
						((PlayerInfo*)(peer->data))->cloth_mask = 0;
						((PlayerInfo*)(peer->data))->cloth_necklace = 0;
						((PlayerInfo*)(peer->data))->skinColor = 2;
						sendClothes(peer);
						packet::consolemessage(peer, "`^Legendary Wizard Set Mod has been Enabled! ");
					}
					else if (str.substr(0, 5) == "/find") {
						packet::dialog(peer, "add_label_with_icon|big|`wFind item Machine``|left|3802|\nadd_textbox|Enter a word below to find the item|\nadd_text_input|item|Item Name||30|\nend_dialog|findid|Cancel|`2Find the item!|\nadd_quick_exit|\n");
					}
					else if (str == "/mods") {
						string x;

						ENetPeer* currentPeer;

						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;

							if (getAdminLevel(((PlayerInfo*)(currentPeer->data))->rawName, ((PlayerInfo*)(currentPeer->data))->tankIDPass) > 665) {
								x.append("`#@" + ((PlayerInfo*)(currentPeer->data))->rawName + "``, ");
							}

						}
						x = x.substr(0, x.length() - 2);

						gamepacket_t p;
						p.Insert("OnConsoleMessage");
						p.Insert("``Moderators online: " + x);
						p.CreatePacket(peer);
					}
					else if (str.substr(0,6) == "/mute ") {
						if (getAdminLevel(((PlayerInfo*)(peer->data))->rawName, ((PlayerInfo*)(peer->data))->tankIDPass) > 665) {
							string name = str.substr(10, str.length());

							ENetPeer* currentPeer;

							bool found = false;

							for (currentPeer = server->peers;
								currentPeer < &server->peers[server->peerCount];
								++currentPeer)
							{
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
									continue;

								if (((PlayerInfo*)(currentPeer->data))->rawName == name) {
									found = true;
									if (((PlayerInfo*)(currentPeer->data))->taped) {
										((PlayerInfo*)(currentPeer->data))->taped = false;
										((PlayerInfo*)(currentPeer->data))->isDuctaped = false;
										
										packet::consolemessage(peer, "`2You are no longer duct-taped!");
										sendState(currentPeer);
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("`2You have un duct-taped the player!");
											p.CreatePacket(peer);
										}
									}
									else {
										((PlayerInfo*)(currentPeer->data))->taped = true;
										((PlayerInfo*)(currentPeer->data))->isDuctaped = true;
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("`4You have been duct-taped!");
										p.CreatePacket(peer);
										sendState(currentPeer);
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("`2You have duct-taped the player!");
											p.CreatePacket(peer);
										}
									}
								}
							}
							if (!found) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("`4Player not found!");
								p.CreatePacket(peer);
							}
						}
						else {
							gamepacket_t p;
							p.Insert("OnConsoleMessage");
							p.Insert("`4You need to have a higher admin-level to do that!");
							p.CreatePacket(peer);
						}
					}
					else if (str.substr(0, 6) == "/give ") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 1337) {
						packet::consolemessage(peer, "You can't do that");
						continue;
					}
					if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
						std::ifstream ifs("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
						std::string content((std::istreambuf_iterator<char>(ifs)),
							(std::istreambuf_iterator<char>()));

						int gembux = atoi(content.c_str());
						if (gembux < 1000000000) {
							ofstream myfile;
							myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
							myfile << str.substr(6, str.length());
							myfile.close();

							GamePacket pp = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), stoi(str.substr(6, cch.length() - 6 - 1))));
							ENetPacket* packetpp = enet_packet_create(pp.data,
								pp.len,
								ENET_PACKET_FLAG_RELIABLE);

							enet_peer_send(peer, 0, packetpp);
							delete pp.data;
						}
					}
				}
					else if (str == "/help"){
						packet::consolemessage(peer, "Supported commands are: /mods, /help, /mod, /unmod, /inventory, /item id, /team id, /color number, /who, /state number, /count, /sb message, /alt, /radio, /find, /unequip, /bc message, /weather id, /news, /mhelp");
					}
					else if (str == "/mhelp") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 666) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
					    packet::consolemessage(peer, "Supported commands are: /mods, /help, /mod, /unmod, /inventory, /item id, /team id, /color number, /who, /state number, /count, /sb message, /alt, /radio, /find, /unequip, /weather id, /news, /bc message, /jsb, /mhelp, /nick name, /invis, /asb, /msb");
					}

					else if (str.substr(0, 3) == "/p ") {
					if ((str.substr(3, cch.length() - 3 - 1).find_first_not_of("0123456789") != string::npos)) continue;
					if (((PlayerInfo*)(peer->data))->cloth_hand == 3300) break;
					bool contains_non_alpha = !std::regex_match(str.substr(3, cch.length() - 3 - 1), std::regex("^[0-9]+$"));
					if (contains_non_alpha == false) {
						if (stoi(str.substr(3, cch.length() - 3 - 1)) < 0) {
							packet::consolemessage(peer, "`oPlease `wenter `obetween `20-230`w!");
						}
						if (stoi(str.substr(3, cch.length() - 3 - 1)) > 2300) {
							packet::consolemessage(peer, "`oPlease `wenter `obetween `20-230`w!");
						}
						else {
							int effect = atoi(str.substr(3, cch.length() - 3 - 1).c_str());
							((PlayerInfo*)(peer->data))->peffect = 8421376 + effect;
							sendState(peer);
							sendPuncheffect(peer);
							packet::consolemessage(peer, "`oYour `2punch `oeffect has been updated to `w" + str.substr(3, cch.length() - 3 - 1));
						}
					}
					else {
						packet::consolemessage(peer, "`oPlease enter only `2numbers`o!");
					}
					}
					else if (str == "/news"){
					    sendGazette(peer);
                    }
					else if (str.substr(0, 6) == "/nick ") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 444) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
						string nam1e = "``" + str.substr(6, cch.length() - 6 - 1) + "``";
						((PlayerInfo*)(event.peer->data))->displayName = str.substr(6, cch.length() - 6 - 1);
						gamepacket_t p(0, ((PlayerInfo*)(peer->data))->netID);
						p.Insert("OnNameChanged");
						p.Insert(nam1e);
						ENetPeer * currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							if (isHere(peer, currentPeer))
							{
								p.CreatePacket(currentPeer);
							}
						}
					}
					else if (str.substr(0, 6) == "/flag ") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 444) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
						int flagid = atoi(str.substr(6).c_str());
			
						gamepacket_t p(0, ((PlayerInfo*)(peer->data))->netID);
						p.Insert("OnGuildDataChanged");
						p.Insert(1);
						p.Insert(2);
						p.Insert(flagid);
						p.Insert(3);

						ENetPeer * currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							if (isHere(peer, currentPeer))
							{
								p.CreatePacket(currentPeer);
							}
						}
					}
					else if (str.substr(0, 9) == "/weather ") {
							if (world->name != "ADMIN") {
								if (world->owner != "") {
									if (((PlayerInfo*)(peer->data))->rawName == world->owner || isSuperAdmin(((PlayerInfo*)(peer->data))->rawName, ((PlayerInfo*)(peer->data))->tankIDPass))

									{
										ENetPeer* currentPeer;

										for (currentPeer = server->peers;
											currentPeer < &server->peers[server->peerCount];
											++currentPeer)
										{
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
												continue;
											if (isHere(peer, currentPeer))
											{
												packet::consolemessage(peer, "`oPlayer `2" + ((PlayerInfo*)(peer->data))->displayName + "`o has just changed the world's weather!");

												gamepacket_t p;
												p.Insert("OnSetCurrentWeather");
												p.Insert(atoi(str.substr(9).c_str()));
												p.CreatePacket(currentPeer);
												continue;
											}
										}
									}
								}
							}
						}
					else if (str == "/count"){
						int count = 0;
						ENetPeer * currentPeer;
						string name = "";
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							count++;
						}
						packet::consolemessage(peer, "There are "+std::to_string(count)+" people online out of 1024 limit.");
					}
					else if (str.substr(0, 5) == "/asb "){
					if (((PlayerInfo*)(peer->data))->adminLevel < 666) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
						if (!canSB(((PlayerInfo*)(peer->data))->rawName, ((PlayerInfo*)(peer->data))->tankIDPass)) continue;
						if (str.find("player_chat") != std::string::npos) {
							continue;
						}

						gamepacket_t p;
						p.Insert("OnAddNotification");
						p.Insert("interface/atomic_button.rttex");
						p.Insert(str.substr(4, cch.length() - 4 - 1).c_str());
						p.Insert("audio/hub_open.wav");
						p.Insert(0);

						ENetPeer * currentPeer;
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							p.CreatePacket(currentPeer);
						}
					}

					else if (str.substr(0, 5) == "/fakeautoban") {

					gamepacket_t p;
					p.Insert("OnAddNotification");
					p.Insert("interface/atomic_button.rttex");
					p.Insert("Warning from Admin : You've been `4BANNED `$from Growtopia for 730 days.");
					p.Insert("audio/hub_open.wav");
					enet_peer_disconnect_later(peer, 0);
					p.Insert(0);

					ENetPeer* currentPeer;
					for (currentPeer = server->peers;
						currentPeer < &server->peers[server->peerCount];
						++currentPeer)
					{
						if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
							continue;
						p.CreatePacket(currentPeer);
					}
					}

					else if (str == "/invis") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 666) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
						packet::consolemessage(peer, "`6" + str);
						if (!pData->isGhost) {

							packet::consolemessage(peer, "`oYour atoms are suddenly aware of quantum tunneling. (Ghost in the shell mod added)");

							gamepacket_t p(0, ((PlayerInfo*)(peer->data))->netID);
							p.Insert("OnSetPos");
							p.Insert(pData->x, pData->y);
							p.CreatePacket(peer);

							sendState(peer);
							sendClothes(peer);
							pData->isGhost = true;
						}
						else {
							packet::consolemessage(peer, "`oYour body stops shimmering and returns to normal. (Ghost in the shell mod removed)");
							gamepacket_t p(0, ((PlayerInfo*)(peer->data))->netID);
							p.Insert("OnSetPos");
							p.Insert(pData->x1, pData->y1);
							p.CreatePacket(peer);

							((PlayerInfo*)(peer->data))->isInvisible = false;
							sendState(peer);
							sendClothes(peer);
							pData->isGhost = false;
						}
					}
					
					else if (str == "/sb") {
					    packet::consolemessage(peer, "Usage : /sb <message>");
					}
					else if (str == "/bc") {
					packet::consolemessage(peer, "Usage : /bc <message>");
					}

					else if (str.substr(0, 4) == "/sb ") {
						using namespace std::chrono;
						if (((PlayerInfo*)(peer->data))->lastSB + 45000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count())
						{
							((PlayerInfo*)(peer->data))->lastSB = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						}
						else {
							packet::consolemessage(peer, "Wait a minute before using the SB command again!");
							continue;
						}
						if (str.find("player_chat") != std::string::npos) {
							continue;
						}

						string name = ((PlayerInfo*)(peer->data))->displayName;
						gamepacket_t p;
						p.Insert("OnConsoleMessage");
						p.Insert("CP:0_PL:4_OID:_CT:[SB]_ `w** `5Super-Broadcast`` from `$`2" + name + "```` (in `$" + ((PlayerInfo*)(peer->data))->currentWorld + "``) ** :`` `# " + str.substr(4, cch.length() - 4 - 1));

						string text = "action|play_sfx\nfile|audio/beep.wav\ndelayMS|0\n";
						BYTE* data = new BYTE[5 + text.length()];
						BYTE zero = 0;
						int type = 3;
						memcpy(data, &type, 4);
						memcpy(data + 4, text.c_str(), text.length());
						memcpy(data + 4 + text.length(), &zero, 1);
						ENetPeer * currentPeer;
						
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							if (!((PlayerInfo*)(currentPeer->data))->radio)
								continue;
							
							p.CreatePacket(currentPeer);
							
							ENetPacket * packet2 = enet_packet_create(data,
								5+text.length(),
								ENET_PACKET_FLAG_RELIABLE);

							enet_peer_send(currentPeer, 0, packet2);
							
							//enet_host_flush(server);
						}
						delete[] data;
					}
					else if (str.substr(0, 5) == "/msb ") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 666) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
					using namespace std::chrono;
					if (((PlayerInfo*)(peer->data))->lastSB + 45000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count())
					{
						((PlayerInfo*)(peer->data))->lastSB = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					else {
						packet::consolemessage(peer, "Wait a minute before using the MSB command again!");
						continue;
					}
					if (str.find("player_chat") != std::string::npos) {
						continue;
					}

					string name = ((PlayerInfo*)(peer->data))->displayName;
					gamepacket_t p;
					p.Insert("OnConsoleMessage");
					p.Insert("`w`#[`bMOD-SB`#]`` `5** from `6" + name + "```` in `#[`4HIDDEN!`#] ** :`` `^ " + str.substr(5, cch.length() - 5 - 1));
					string text = "action|play_sfx\nfile|audio/beep.wav\ndelayMS|0\n";
					BYTE* data = new BYTE[5 + text.length()];
					BYTE zero = 0;
					int type = 3;
					memcpy(data, &type, 4);
					memcpy(data + 4, text.c_str(), text.length());
					memcpy(data + 4 + text.length(), &zero, 1);
					ENetPeer* currentPeer;

					for (currentPeer = server->peers;
						currentPeer < &server->peers[server->peerCount];
						++currentPeer)
					{
						if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
							continue;
						if (!((PlayerInfo*)(currentPeer->data))->radio)
							continue;

						p.CreatePacket(currentPeer);


						ENetPacket* packet2 = enet_packet_create(data,
							5 + text.length(),
							ENET_PACKET_FLAG_RELIABLE);

						enet_peer_send(currentPeer, 0, packet2);

						//enet_host_flush(server);
					}
					delete data;
					}
					else if (str.substr(0, 4) == "/bc ") {
					using namespace std::chrono;
					if (((PlayerInfo*)(peer->data))->lastSB + 45000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count())
					{
						((PlayerInfo*)(peer->data))->lastSB = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					else {
						packet::consolemessage(peer, "Wait a minute before using the Broadcast command again!");
						continue;
					}
					if (str.find("player_chat") != std::string::npos) {
						continue;
					}

					string name = ((PlayerInfo*)(peer->data))->displayName;
					gamepacket_t p;
					p.Insert("OnConsoleMessage");
					p.Insert("CP:0_PL:4_OID:_CT:[SB]_ `w** Broadcast`` `5from `$`2" + name + "```` (in `$" + ((PlayerInfo*)(peer->data))->currentWorld + "``) ** :`` `# " + str.substr(4, cch.length() - 4 - 1));

					string text = "action|play_sfx\nfile|audio/beep.wav\ndelayMS|0\n";
					BYTE* data = new BYTE[5 + text.length()];
					BYTE zero = 0;
					int type = 3;
					memcpy(data, &type, 4);
					memcpy(data + 4, text.c_str(), text.length());
					memcpy(data + 4 + text.length(), &zero, 1);
					ENetPeer* currentPeer;

					for (currentPeer = server->peers;
						currentPeer < &server->peers[server->peerCount];
						++currentPeer)
					{
						if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
							continue;
						if (!((PlayerInfo*)(currentPeer->data))->radio)
							continue;

						p.CreatePacket(currentPeer);

						ENetPacket* packet2 = enet_packet_create(data,
							5 + text.length(),
							ENET_PACKET_FLAG_RELIABLE);

						enet_peer_send(currentPeer, 0, packet2);

						//enet_host_flush(server);
					}
					delete[] data;
					}
					else if (str.substr(0, 5) == "/jsb ") {
					if (((PlayerInfo*)(peer->data))->adminLevel < 666) {
						packet::consolemessage(peer, "`4You can't do that.");
						continue;
					}
						using namespace std::chrono;
						if (((PlayerInfo*)(peer->data))->lastSB + 45000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count())
						{
							((PlayerInfo*)(peer->data))->lastSB = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						}
						else {
							packet::consolemessage(peer, "Wait a minute before using the JSB command again!");
							continue;
						}
						if (str.find("player_chat") != std::string::npos) {
							continue;
						}

						string name = ((PlayerInfo*)(peer->data))->displayName;
						gamepacket_t p;
						p.Insert("OnConsoleMessage");
						p.Insert("`w** `5Super-Broadcast`` from `$`2" + name + "```` (in `4JAMMED``) ** :`` `^ " + str.substr(5, cch.length() - 5 - 1));
						string text = "action|play_sfx\nfile|audio/beep.wav\ndelayMS|0\n";
						BYTE* data = new BYTE[5 + text.length()];
						BYTE zero = 0;
						int type = 3;
						memcpy(data, &type, 4);
						memcpy(data + 4, text.c_str(), text.length());
						memcpy(data + 4 + text.length(), &zero, 1);
						ENetPeer * currentPeer;
						
						for (currentPeer = server->peers;
							currentPeer < &server->peers[server->peerCount];
							++currentPeer)
						{
							if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
								continue;
							if (!((PlayerInfo*)(currentPeer->data))->radio)
								continue;
							
							p.CreatePacket(currentPeer);
							
							
							ENetPacket * packet2 = enet_packet_create(data,
								5+text.length(),
								ENET_PACKET_FLAG_RELIABLE);

							enet_peer_send(currentPeer, 0, packet2);
							
							//enet_host_flush(server);
						}
						delete data;
					}
					
					
					else if (str.substr(0, 6) == "/radio") {
						gamepacket_t p;
						if (((PlayerInfo*)(peer->data))->radio) {
							p.Insert("OnConsoleMessage");
							p.Insert("You won't see broadcasts anymore.");
							((PlayerInfo*)(peer->data))->radio = false;
						} else {
							p.Insert("OnConsoleMessage");
							p.Insert("You will now see broadcasts again.");
							((PlayerInfo*)(peer->data))->radio = true;
						}
						p.CreatePacket(peer);
					}
					else if (str == "/restart") {
						if (((PlayerInfo*)(peer->data))->rawName == "ibord") {
							cout << "Restart from " << ((PlayerInfo*)(peer->data))->displayName << endl;

							packet::PlayAudio(peer, "audio/ogg/suspended.ogg", 0);
							gamepacket_t p;
							p.Insert("OnConsoleMessage");
							p.Insert("** Global System Message: `4Server Restart for update! `$**");

							gamepacket_t p2;
							p2.Insert("OnConsoleMessage");
							p2.Insert("`4Global System Message``: ``Restarting server for update in `41 ``minute");

							gamepacket_t p3(10000);
							p3.Insert("OnConsoleMessage");
							p3.Insert("`4Global System Message``: Restarting server for update in `450 ``seconds");

							gamepacket_t p4(20000);
							p4.Insert("OnConsoleMessage");
							p4.Insert("`4Global System Message``: Restarting server for update in `440 ``seconds");

							gamepacket_t p5(30000);
							p5.Insert("OnConsoleMessage");
							p5.Insert("`4Global System Message``: Restarting server for update in `430 ``seconds");

							gamepacket_t p6(40000);
							p6.Insert("OnConsoleMessage");
							p6.Insert("`4Global System Message``: Restarting server for update in `420 ``seconds");

							gamepacket_t p7(50000);
							p7.Insert("OnConsoleMessage");
							p7.Insert("`4Global System Message``: Restarting server for update in `410 ``seconds");

							gamepacket_t p8(51000);
							p8.Insert("OnConsoleMessage");
							p8.Insert("`4Global System Message``: Restarting server for update in `49 ``seconds");

							gamepacket_t p9(52000);
							p9.Insert("OnConsoleMessage");
							p9.Insert("`4Global System Message``: Restarting server for update in `48 ``seconds");

							gamepacket_t p10(53000);
							p10.Insert("OnConsoleMessage");
							p10.Insert("`4Global System Message``: Restarting server for update in `47 ``seconds");

							gamepacket_t p11(54000);
							p11.Insert("OnConsoleMessage");
							p11.Insert("`4Global System Message``: Restarting server for update in `46 ``seconds");

							gamepacket_t p12(55000);
							p12.Insert("OnConsoleMessage");
							p12.Insert("`4Global System Message``: Restarting server for update in `45 ``seconds");

							gamepacket_t p13(56000);
							p13.Insert("OnConsoleMessage");
							p13.Insert("`4Global System Message``: Restarting server for update in `44 ``seconds");

							gamepacket_t p14(57000);
							p14.Insert("OnConsoleMessage");
							p14.Insert("`4Global System Message``: Restarting server for update in `43 ``seconds");

							gamepacket_t p15(58000);
							p15.Insert("OnConsoleMessage");
							p15.Insert("`4Global System Message``: Restarting server for update in `42 ``seconds");

							gamepacket_t p16(59000);
							p16.Insert("OnConsoleMessage");
							p16.Insert("`4Global System Message``: Restarting server for update in `41 ``seconds");

							gamepacket_t p17(60000);
							p17.Insert("OnConsoleMessage");
							p17.Insert("`4Global System Message``: Restarting server for update in `4ZERO ``seconds! Should be back up in a minute or so. BYE!");

							ENetPeer* currentPeer;
							for (currentPeer = server->peers;
								currentPeer < &server->peers[server->peerCount];
								++currentPeer)
							{
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
									continue;
								p.CreatePacket(currentPeer);
								p2.CreatePacket(currentPeer);
								p3.CreatePacket(currentPeer);
								p4.CreatePacket(currentPeer);
								p5.CreatePacket(currentPeer);
								p6.CreatePacket(currentPeer);
								p7.CreatePacket(currentPeer);
								p8.CreatePacket(currentPeer);
								p9.CreatePacket(currentPeer);
								p10.CreatePacket(currentPeer);
								p11.CreatePacket(currentPeer);
								p12.CreatePacket(currentPeer);
								p13.CreatePacket(currentPeer);
								p14.CreatePacket(currentPeer);
								p15.CreatePacket(currentPeer);
								p16.CreatePacket(currentPeer);
								p17.CreatePacket(currentPeer);

								enet_peer_disconnect_now(currentPeer, 0);
							}
						}
					}
					else if (str == "/unmod")
					{
						((PlayerInfo*)(peer->data))->canWalkInBlocks = false;
						sendState(peer);
					}
					else if (str == "/alt") {
					gamepacket_t p;
					p.Insert("OnSetBetaMode");
					p.Insert(1);
					}
					else
					if (str == "/inventory")
					{
						sendInventory(peer, ((PlayerInfo*)(peer->data))->inventory);
					} else
					if (str.substr(0,6) == "/item ")
					{
						if (((PlayerInfo*)(peer->data))->adminLevel < 1337) {
							packet::consolemessage(peer, "`4You can't do that.");
							continue;
						}
						bool success = true;

						int itemID = atoi(str.substr(6, cch.length() - 6 - 1).c_str());
						SaveShopsItemMoreTimes(itemID, 200, peer, success);

						PlayerInventory inventory = ((PlayerInfo *)(peer->data))->inventory;
						InventoryItem item;
						if (itemID != 112 && itemID != 18 && itemID != 32) {
							item.itemID = itemID;
							item.itemCount = 200;
							inventory.items.push_back(item);
							sendInventory(peer, inventory);
						}
					} else
					if (str.substr(0, 6) == "/team ")
					{
						int val = 0;
						val = atoi(str.substr(6, cch.length() - 6 - 1).c_str());
						PlayerMoving data;
						//data.packetType = 0x14;
						data.packetType = 0x1B;
						//data.characterState = 0x924; // animation
						data.characterState = 0x0; // animation
						data.x = 0;
						data.y = 0;
						data.punchX = val;
						data.punchY = 0;
						data.XSpeed = 0;
						data.YSpeed = 0;
						data.netID = ((PlayerInfo*)(peer->data))->netID;
						data.plantingTree = 0;
						SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);

					} else 
					if (str.substr(0, 7) == "/color ")
					{
						((PlayerInfo*)(peer->data))->skinColor = atoi(str.substr(6, cch.length() - 6 - 1).c_str());
						sendClothes(peer);
					}
					if (str.substr(0, 4) == "/who")
					{
						sendWho(peer);

					}
					if (str.length() && str[0] == '/')
					{
						sendAction(peer, ((PlayerInfo*)(peer->data))->netID, str);
					} else if (str.length()>0)
					{
						if (((PlayerInfo*)(peer->data))->taped == false) {
							sendChatMessage(peer, ((PlayerInfo*)(peer->data))->netID, str);
						}
						else {
							// Is duct-taped
							sendChatMessage(peer, ((PlayerInfo*)(peer->data))->netID, randomDuctTapeMessage(str.length()));
						}
					}
					
			}
				if (!((PlayerInfo*)(event.peer->data))->isIn)
				{
					if (itemdathash == 0) {
						enet_peer_disconnect_later(peer, 0);
					}

					gamepacket_t p;
					p.Insert("OnSuperMainStartAcceptLogonHrdxs47254722215a");
					p.Insert(itemdathash);
					p.Insert("ubistatic-a.akamaihd.net");
					p.Insert(configCDN);
					p.Insert("cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster");
					p.Insert("proto=149|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|server_tick=226933875|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|");
					p.CreatePacket(peer);
					
					std::stringstream ss(GetTextPointerFromPacket(event.packet));
					std::string to;
					while (std::getline(ss, to, '\n')){
						string id = to.substr(0, to.find("|"));
						string act = to.substr(to.find("|") + 1, to.length() - to.find("|") - 1);
						if (id == "tankIDName")
						{
							((PlayerInfo*)(event.peer->data))->tankIDName = act;
							((PlayerInfo*)(event.peer->data))->haveGrowId = true;
						}
						else if(id == "tankIDPass")
						{
							((PlayerInfo*)(event.peer->data))->tankIDPass = act;
						}
						else if(id == "requestedName")
						{
							((PlayerInfo*)(event.peer->data))->requestedName = act;
						}
						else if (id == "country")
						{
							((PlayerInfo*)(event.peer->data))->country = act;
						}
					}
					if (!((PlayerInfo*)(event.peer->data))->haveGrowId)
					{
						((PlayerInfo*)(event.peer->data))->hasLogon = true;
						((PlayerInfo*)(event.peer->data))->rawName = "";
						((PlayerInfo*)(event.peer->data))->displayName = PlayerDB::fixColors(((PlayerInfo*)(event.peer->data))->requestedName.substr(0, ((PlayerInfo*)(event.peer->data))->requestedName.length()>15?15:((PlayerInfo*)(event.peer->data))->requestedName.length()));
					}
					else {
						((PlayerInfo*)(event.peer->data))->rawName = PlayerDB::getProperName(((PlayerInfo*)(event.peer->data))->tankIDName);
#ifdef REGISTRATION
						int logStatus = PlayerDB::playerLogin(peer, ((PlayerInfo*)(event.peer->data))->rawName, ((PlayerInfo*)(event.peer->data))->tankIDPass);
						if (logStatus == 1) {
							PlayerInfo* p = ((PlayerInfo*)(peer->data));
							std::ifstream ifff("players/" + PlayerDB::getProperName(p->rawName) + ".json");
							json j;
							ifff >> j;

							int adminLevel;
							int xp;
							int level;
							int premwl;
							int back;
							int hand;
							int shirt;
							int pants;
							int neck;
							int hair;
							int feet;
							bool joinguild;
							string guild;
							int mask;
							int ances;
							int face;
							int effect;
							int skin;
							adminLevel = j["adminLevel"];
							level = j["level"];
							xp = j["xp"];
							back = j["ClothBack"];
							face = j["ClothFace"];
							hand = j["ClothHand"];
							shirt = j["ClothShirt"];
							pants = j["ClothPants"];
							premwl = j["premwl"];
							neck = j["ClothNeck"];
							hair = j["ClothHair"];
							feet = j["ClothFeet"];
							mask = j["ClothMask"];
							ances = j["ClothAnces"];
							effect = j["effect"];
							skin = j["skinColor"];
							if (j.count("guild") == 1) {
								guild = j["guild"].get<string>();
							}
							else {
								guild = "";
							}
							if (j.count("joinguild") == 1) {
								joinguild = j["joinguild"];
							}

							p->adminLevel = adminLevel;
							p->level = level;
							p->xp = xp;
							p->premwl = premwl;
							p->cloth_back = back;
							p->cloth_hand = hand;
							p->cloth_face = face;
							p->cloth_hair = hair;
							p->cloth_feet = feet;
							p->guild = guild;
							p->joinguild = joinguild;
							p->cloth_pants = pants;
							p->cloth_necklace = neck;
							p->cloth_shirt = shirt;
							p->cloth_mask = mask;
							p->cloth_ances = ances;
							p->peffect = effect;
							p->skinColor = skin;

							updateAllClothes(peer);

							ifff.close();

							string guildname = PlayerDB::getProperName(((PlayerInfo*)(peer->data))->guild);
							if (guildname != "") {
								std::ifstream ifff("guilds/" + guildname + ".json");
								if (ifff.fail()) {
									ifff.close();
									cout << "[!] Failed loading guilds/" + guildname + ".json! From " + ((PlayerInfo*)(peer->data))->displayName + "." << endl;
									((PlayerInfo*)(peer->data))->guild = "";
								}
								json j;
								ifff >> j;
								int gfbg, gffg;
								string gstatement, gleader;
								vector<string> gmembers;
								gfbg = j["backgroundflag"];
								gffg = j["foregroundflag"];
								gstatement = j["GuildStatement"].get<string>();
								gleader = j["Leader"].get<string>();
								for (int i = 0; i < j["Member"].size(); i++) {
									gmembers.push_back(j["Member"][i]);
								}
								((PlayerInfo*)(peer->data))->guildBg = gfbg;
								((PlayerInfo*)(peer->data))->guildFg = gffg;
								((PlayerInfo*)(peer->data))->guildStatement = gstatement;
								((PlayerInfo*)(peer->data))->guildLeader = gleader;
								((PlayerInfo*)(peer->data))->guildMembers = gmembers;
								ifff.close();
							}
							std::ifstream ifsz("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
							std::string acontent((std::istreambuf_iterator<char>(ifsz)),
								(std::istreambuf_iterator<char>()));
							int ac = atoi(acontent.c_str());
							ofstream myfile;
							myfile.open("gemdb/" + ((PlayerInfo*)(peer->data))->rawName + ".txt");
							myfile << ac;
							myfile.close();
							GamePacket psa = packetEnd(appendInt(appendString(createPacket(), "OnSetBux"), ac));
							ENetPacket* packetsa = enet_packet_create(psa.data, psa.len, ENET_PACKET_FLAG_RELIABLE);
							enet_peer_send(peer, 0, packetsa);
							delete psa.data;
							string gayname = ((PlayerInfo*)(peer->data))->rawName;
							packet::consolemessage(peer, "`rSuccessfully logged into your account `2(`9" + gayname + "`2)");
							((PlayerInfo*)(event.peer->data))->displayName = ((PlayerInfo*)(event.peer->data))->tankIDName;
							if (((PlayerInfo*)(peer->data))->adminLevel == 1337) {
								((PlayerInfo*)(event.peer->data))->displayName = "`6@" + ((PlayerInfo*)(event.peer->data))->tankIDName;
								((PlayerInfo*)(peer->data))->haveSuperSupporterName = true;
							}
							else if (((PlayerInfo*)(peer->data))->adminLevel == 999) {
								((PlayerInfo*)(event.peer->data))->displayName = "`4@" + ((PlayerInfo*)(event.peer->data))->tankIDName;
								((PlayerInfo*)(peer->data))->haveSuperSupporterName = true;
							}
							else if (((PlayerInfo*)(peer->data))->adminLevel == 777) {
								((PlayerInfo*)(event.peer->data))->displayName = "`c@" + ((PlayerInfo*)(event.peer->data))->tankIDName;
								((PlayerInfo*)(peer->data))->haveSuperSupporterName = true;
							}
							else if (((PlayerInfo*)(peer->data))->adminLevel == 666) {
								((PlayerInfo*)(event.peer->data))->displayName = "`#@" + ((PlayerInfo*)(event.peer->data))->tankIDName;
								((PlayerInfo*)(peer->data))->haveSuperSupporterName = true;
							}
							else if (((PlayerInfo*)(peer->data))->adminLevel == 444) {
								((PlayerInfo*)(event.peer->data))->displayName = "`w[`1VIP`w] " + ((PlayerInfo*)(event.peer->data))->tankIDName;
								((PlayerInfo*)(peer->data))->haveSuperSupporterName = true;
							}
							else if (((PlayerInfo*)(peer->data))->adminLevel == 111) {
								((PlayerInfo*)(event.peer->data))->displayName = "`w[`2GrowPass`w] " + ((PlayerInfo*)(event.peer->data))->tankIDName;
								((PlayerInfo*)(peer->data))->haveSuperSupporterName = true;
							}
							else {
								((PlayerInfo*)(event.peer->data))->displayName = ((PlayerInfo*)(event.peer->data))->tankIDName;
							}
						}
						else {
							packet::consolemessage(peer, "`rWrong username or password!``");
							enet_peer_disconnect_later(peer, 0);
						}
#else
						
						((PlayerInfo*)(event.peer->data))->displayName = PlayerDB::fixColors(((PlayerInfo*)(event.peer->data))->tankIDName.substr(0, ((PlayerInfo*)(event.peer->data))->tankIDName.length()>18 ? 18 : ((PlayerInfo*)(event.peer->data))->tankIDName.length()));
						if (((PlayerInfo*)(event.peer->data))->displayName.length() < 3) ((PlayerInfo*)(event.peer->data))->displayName = "Person that doesn't know how the name looks!";
#endif
					}
					for (char c : ((PlayerInfo*)(event.peer->data))->displayName) if (c < 0x20 || c>0x7A) ((PlayerInfo*)(event.peer->data))->displayName = "Bad characters in name, remove them!";
					
					if (((PlayerInfo*)(event.peer->data))->country.length() > 4)
					{
						((PlayerInfo*)(event.peer->data))->country = "|showGuild";
					}
					if (getAdminLevel(((PlayerInfo*)(event.peer->data))->rawName, ((PlayerInfo*)(event.peer->data))->tankIDPass) > 665)
					{
						((PlayerInfo*)(event.peer->data))->country = "rt";
					}
					if (getAdminLevel(((PlayerInfo*)(event.peer->data))->rawName, ((PlayerInfo*)(event.peer->data))->tankIDPass) > 0)
					{
						((PlayerInfo*)(event.peer->data))->country = "interface/cash_icon_overlay";
					}
					if (((PlayerInfo*)(event.peer->data))->level >= 125) {
						((PlayerInfo*)(event.peer->data))->country = "|showGuild|maxLevel";
					}

					gamepacket_t p2;
					p2.Insert("SetHasGrowID");
					p2.Insert(((PlayerInfo*)(event.peer->data))->haveGrowId);
					p2.Insert(((PlayerInfo*)(peer->data))->tankIDName);
					p2.Insert(((PlayerInfo*)(peer->data))->tankIDPass);
					p2.CreatePacket(peer);
				}
				string pStr = GetTextPointerFromPacket(event.packet);
				//if (strcmp(GetTextPointerFromPacket(event.packet), "action|enter_game\n") == 0 && !((PlayerInfo*)(event.peer->data))->isIn)
				if (pStr.substr(0, 17) == "action|enter_game" && !((PlayerInfo*)(event.peer->data))->isIn)
				{
#ifdef TOTAL_LOG
					cout << "And we are in!" << endl;
#endif
					ENetPeer* currentPeer;
					((PlayerInfo*)(event.peer->data))->isIn = true;
					/*for (currentPeer = server->peers;
						currentPeer < &server->peers[server->peerCount];
						++currentPeer)
					{
						if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
							continue;
						GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "Player `o" + ((PlayerInfo*)(event.peer->data))->tankIDName + "`o just entered the game..."));
						ENetPacket * packet = enet_packet_create(p.data,
							p.len,
							ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(currentPeer, 0, packet);

						enet_host_flush(server);
						delete p.data;
					}*/
					sendWorldOffers(peer);

					// growmoji
					gamepacket_t  p;
					p.Insert("OnEmoticonDataChanged");
					p.Insert(0);
					p.Insert("(wl)|ā|1&(yes)|Ă|1&(no)|ă|1&(love)|Ą|1&(oops)|ą|1&(shy)|Ć|1&(wink)|ć|1&(tongue)|Ĉ|1&(agree)|ĉ|1&(sleep)|Ċ|1&(punch)|ċ|1&(music)|Č|1&(build)|č|1&(megaphone)|Ď|1&(sigh)|ď|1&(mad)|Đ|1&(wow)|đ|1&(dance)|Ē|1&(see-no-evil)|ē|1&(bheart)|Ĕ|1&(heart)|ĕ|1&(grow)|Ė|1&(gems)|ė|1&(kiss)|Ę|1&(gtoken)|ę|1&(lol)|Ě|1&(smile)|Ā|1&(cool)|Ĝ|1&(cry)|ĝ|1&(vend)|Ğ|1&(bunny)|ě|1&(cactus)|ğ|1&(pine)|Ĥ|1&(peace)|ģ|1&(terror)|ġ|1&(troll)|Ġ|1&(evil)|Ģ|1&(fireworks)|Ħ|1&(football)|ĥ|1&(alien)|ħ|1&(party)|Ĩ|1&(pizza)|ĩ|1&(clap)|Ī|1&(song)|ī|1&(ghost)|Ĭ|1&(nuke)|ĭ|1&(halo)|Į|1&(turkey)|į|1&(gift)|İ|1&(cake)|ı|1&(heartarrow)|Ĳ|1&(lucky)|ĳ|1&(shamrock)|Ĵ|1&(grin)|ĵ|1&(ill)|Ķ|1&");
					p.CreatePacket(peer);

					if (((PlayerInfo*)(peer->data))->haveGrowId) {
						packet::consolemessage(peer, "`2Server by Ibord, credit to GrowtopiaNoobs");

						ifstream fg("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
						json j;
						fg >> j;
						fg.close();
						if (j["items"][0]["itemid"] != 18 || j["items"][1]["itemid"] != 32) {
							j["items"][0]["itemid"] = 18;
							j["items"][1]["itemid"] = 32;
							j["items"][0]["quantity"] = 1;
							j["items"][1]["quantity"] = 1;
							ofstream fs("inventory/" + ((PlayerInfo*)(peer->data))->rawName + ".json");
							fs << j;
							fs.close();
						}
						PlayerInventory inventory; {
							InventoryItem item;
							for (int i = 0; i < ((PlayerInfo*)(peer->data))->currentInventorySize; i++) {
								int itemid = j["items"][i]["itemid"];
								int quantity = j["items"][i]["quantity"];
								if (itemid != 0 && quantity != 0) {
									item.itemCount = quantity;
									item.itemID = itemid;
									inventory.items.push_back(item);
									sendInventory(peer, inventory);
								}
							}
						}
						((PlayerInfo*)(event.peer->data))->inventory = inventory;
						{
							sendGazette(peer);
						}
					}
				}
				if (strcmp(GetTextPointerFromPacket(event.packet), "action|refresh_item_data\n") == 0)
				{
					if (itemsDat != NULL) {
						ENetPacket * packet = enet_packet_create(itemsDat,
							itemsDatSize + 60,
							ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(peer, 0, packet);
						((PlayerInfo*)(peer->data))->isUpdating = true;
						//enet_peer_disconnect_later(peer, 0); // TODO: add this back, and fix it properly
						//enet_host_flush(server);
					}
					// TODO FIX refresh_item_data ^^^^^^^^^^^^^^
				}
				break;
			}
			default:
				cout << "Unknown packet type " << messageType << endl;
				break;
			case 3:
			{
				//cout << GetTextPointerFromPacket(event.packet) << endl;
				std::stringstream ss(GetTextPointerFromPacket(event.packet));
				std::string to;
				bool isJoinReq = false;
								
				while (std::getline(ss, to, '\n')) {
					string id = to.substr(0, to.find("|"));
					string act = to.substr(to.find("|") + 1, to.length() - to.find("|") - 1);
					if (id == "name" && isJoinReq)
					{
#ifdef TOTAL_LOG
						cout << "Entering some world..." << endl;
#endif
						if (!((PlayerInfo*)(peer->data))->hasLogon) break;
						try {
							if (act.length() > 30) {
								packet::consolemessage(peer, "`4Sorry, but world names with more than 30 characters are not allowed!");
								((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
								gamepacket_t p;
								p.Insert("OnFailedToEnterWorld");
								p.Insert(1);
								p.CreatePacket(peer);

							} else {
								WorldInfo info = worldDB.get(act);
								sendWorld(peer, &info);

								int x = 3040;
								int y = 736;

								for (int j = 0; j < info.width*info.height; j++)
								{
									if (info.items[j].foreground == 6) {
										x = (j%info.width) * 32;
										y = (j / info.width) * 32;
									}
								}
								packet::onspawn(peer, "spawn|avatar\nnetID|" + std::to_string(cId) + "\nuserID|" + std::to_string(cId) + "\ncolrect|0|0|20|30\nposXY|" + std::to_string(x) + "|" + std::to_string(y) + "\nname|``" + ((PlayerInfo*)(event.peer->data))->displayName + "``\ncountry|" + ((PlayerInfo*)(event.peer->data))->country + "\ninvis|0\nmstate|0\nsmstate|0\ntype|local\n");
								((PlayerInfo*)(event.peer->data))->netID = cId;
								onPeerConnect(peer);
								cId++;

								sendInventory(peer, ((PlayerInfo*)(event.peer->data))->inventory);

                                 if (((PlayerInfo*)(peer->data))->taped) {
									 ((PlayerInfo*)(peer->data))->isDuctaped = true;
									 sendState(peer);
								 }
							}
						}
						catch (int e) {
							if (e == 1) {
								((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
								gamepacket_t p;
								p.Insert("OnFailedToEnterWorld");
								p.Insert(1);
								p.CreatePacket(peer);
								packet::consolemessage(peer, "You have exited the world.");
							}
							else if (e == 2) {
								((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
								gamepacket_t p;
								p.Insert("OnFailedToEnterWorld");
								p.Insert(1);
								p.CreatePacket(peer);
								packet::consolemessage(peer, "You have entered bad characters in the world name!");
							}
							else if (e == 3) {
								((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
								gamepacket_t p;
								p.Insert("OnFailedToEnterWorld");
								p.Insert(1);
								p.CreatePacket(peer);
								packet::consolemessage(peer, "Exit from what? Click back if you're done playing.");
							}
							else {
								((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
								gamepacket_t p;
								p.Insert("OnFailedToEnterWorld");
								p.Insert(1);
								p.CreatePacket(peer);
								packet::consolemessage(peer, "I know this menu is magical and all, but it has its limitations! You can't visit this world!");
							}
						}
					}
						if (id == "action")
						{

							if (act == "join_request")
							{
								isJoinReq = true;
							}
							if (act == "quit_to_exit")
							{
								sendPlayerLeave(peer, (PlayerInfo*)(event.peer->data));
								((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
								sendWorldOffers(peer);

							}
							if (act == "quit")
							{
								enet_peer_disconnect_later(peer, 0);
							}
						}
					}
					break;
			}
			case 4:
			{
				{
					BYTE* tankUpdatePacket = GetStructPointerFromTankPacket(event.packet); 
					
					if (tankUpdatePacket)
					{
						PlayerMoving* pMov = unpackPlayerMoving(tankUpdatePacket);
						if (((PlayerInfo*)(event.peer->data))->isGhost) {
							((PlayerInfo*)(event.peer->data))->isInvisible = true;
							((PlayerInfo*)(event.peer->data))->x1 = pMov->x;
							((PlayerInfo*)(event.peer->data))->y1 = pMov->y;
							pMov->x = -1000000;
							pMov->y = -1000000;
						}
						
						switch (pMov->packetType)
						{
						case 0:
							((PlayerInfo*)(event.peer->data))->x = pMov->x;
							((PlayerInfo*)(event.peer->data))->y = pMov->y;
							((PlayerInfo*)(event.peer->data))->isRotatedLeft = pMov->characterState & 0x10;
							sendPData(peer, pMov);
							if (!((PlayerInfo*)(peer->data))->joinClothesUpdated)
							{
								((PlayerInfo*)(peer->data))->joinClothesUpdated = true;
								updateAllClothes(peer);
							}
							break;

						default:
							break;
						}
						PlayerMoving *data2 = unpackPlayerMoving(tankUpdatePacket);
						//cout << data2->packetType << endl;
						if (data2->packetType == 11)
						{
							sendCollect(peer, ((PlayerInfo*)(peer->data))->netID, data2->plantingTree);
						}
						if (data2->packetType == 7)
						{
							if(data2->punchX < world->width && data2->punchY < world->height)
							if (getItemDef(world->items[data2->punchX + (data2->punchY * world->width)].foreground).blockType == BlockTypes::MAIN_DOOR) {
									sendPlayerLeave(peer, (PlayerInfo*)(event.peer->data));
									((PlayerInfo*)(peer->data))->currentWorld = "EXIT";
									sendWorldOffers(peer);

								}
						}
						if (data2->packetType == 10)
						{
							//cout << pMov->x << ";" << pMov->y << ";" << pMov->plantingTree << ";" << pMov->punchX << ";" << pMov->punchY << ";" << pMov->characterState << endl;
							int item = pMov->plantingTree;
							PlayerInfo* info = ((PlayerInfo*)(peer->data));
							ItemDefinition pro;
							pro = getItemDef(item);
							ItemDefinition def;
							try {
								def = getItemDef(pMov->plantingTree);
							}
							catch (int e) {
								goto END_CLOTHSETTER_FORCE;
							}
							if (pMov->plantingTree == 242)
							{
								for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
								{
									if (((PlayerInfo*)(peer->data))->inventory.items[i].itemID == 242)
									{
										if (((PlayerInfo*)(peer->data))->inventory.items[i].itemCount >= 100)
										{
											bool isValid = SaveConvertedItem(1796, 1, peer);
											if (isValid)
											{

												RemoveInventoryItem(242, 100, peer);
												GamePacket p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`wYou compressed 100 `2World Lock `winto a `2Diamond Lock`w!"), 0));
												ENetPacket* packet2 = enet_packet_create(p2.data,
													p2.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet2);
												delete p2.data;
												packet::consolemessage(peer, "`oYou compressed 100 `2World Lock `ointo a `2Diamond Lock`o!");
											}
										}
									}
								}
								break;
							}
							if (pMov->plantingTree == 1796) // shatter
							{
								for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
								{
									if (((PlayerInfo*)(peer->data))->inventory.items[i].itemID == 1796)
									{
										if (((PlayerInfo*)(peer->data))->inventory.items[i].itemCount >= 1)
										{
											bool isValid = SaveConvertedItem(242, 100, peer);
											if (isValid)
											{

												RemoveInventoryItem(1796, 1, peer);
												GamePacket p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`wYou shattered a `2Diamond Lock `winto `2100 World Lock`w!"), 0));
												ENetPacket* packet2 = enet_packet_create(p2.data,
													p2.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet2);
												delete p2.data;
												packet::consolemessage(peer, "`oYou shattered a `2Diamond Lock `winto `2100 World Lock`o!");
											}
										}
									}
								}
								break;
							}
							if (pMov->plantingTree == 7188) // shatter
							{
								for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
								{
									if (((PlayerInfo*)(peer->data))->inventory.items[i].itemID == 6802)
									{
										if (((PlayerInfo*)(peer->data))->inventory.items[i].itemCount >= 1)
										{
											bool isValid = SaveConvertedItem(1796, 100, peer);
											if (isValid)
											{

												RemoveInventoryItem(7188, 1, peer);
												GamePacket p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`wYou shattered `2Mega Growtoken `winto `2100 Growtoken`w!"), 0));
												ENetPacket* packet2 = enet_packet_create(p2.data,
													p2.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet2);
												delete p2.data;
												packet::consolemessage(peer, "`oYou shattered `2Blue Gem Lock `winto `2100 Diamond Locks`o!");
											}
										}
									}
								}
								continue;
							}
							if (pMov->plantingTree == 1486) // shatter
							{
								for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
								{
									if (((PlayerInfo*)(peer->data))->inventory.items[i].itemID == 1486)
									{
										if (((PlayerInfo*)(peer->data))->inventory.items[i].itemCount >= 100)
										{
											bool isValid = SaveConvertedItem(6802, 1, peer);
											if (isValid)
											{

												RemoveInventoryItem(1486, 100, peer);
												GamePacket p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`wYou compressed `2100 Growtoken `winto a `2Mega Growtoken`w!"), 0));
												ENetPacket* packet2 = enet_packet_create(p2.data,
													p2.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet2);
												delete p2.data;
												packet::consolemessage(peer, "`oYou compressed `2100 Growtoken `winto a `2Mega Growtoken`o!");
											}
										}
									}
								}
								continue;
							}
							if (pMov->plantingTree == 6802) // shatter
							{
								for (int i = 0; i < ((PlayerInfo*)(peer->data))->inventory.items.size(); i++)
								{
									if (((PlayerInfo*)(peer->data))->inventory.items[i].itemID == 6802)
									{
										if (((PlayerInfo*)(peer->data))->inventory.items[i].itemCount >= 1)
										{
											bool isValid = SaveConvertedItem(1486, 100, peer);
											if (isValid)
											{

												RemoveInventoryItem(6802, 1, peer);
												GamePacket p2 = packetEnd(appendIntx(appendString(appendIntx(appendString(createPacket(), "OnTalkBubble"), ((PlayerInfo*)(peer->data))->netID), "`wYou shattered `2Mega Growtoken `winto `2100 Growtoken`w!"), 0));
												ENetPacket* packet2 = enet_packet_create(p2.data,
													p2.len,
													ENET_PACKET_FLAG_RELIABLE);
												enet_peer_send(peer, 0, packet2);
												delete p2.data;
												packet::consolemessage(peer, "`oYou shattered `2Mega Growtoken `winto `2100 Growtoken`o!");
											}
										}
									}
								}
								break;
							}
							
							switch (def.clothType) {
							case 0:
								if (((PlayerInfo*)(event.peer->data))->cloth0 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth0 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth0 = pMov->plantingTree;
								break;
							case 1:
								if (((PlayerInfo*)(event.peer->data))->cloth1 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth1 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth1 = pMov->plantingTree;
								break;
							case 2:
								if (((PlayerInfo*)(event.peer->data))->cloth2 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth2 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth2 = pMov->plantingTree;
								break;
							case 3:
								if (((PlayerInfo*)(event.peer->data))->cloth3 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth3 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth3 = pMov->plantingTree;
								break;
							case 4:
								if (((PlayerInfo*)(event.peer->data))->cloth4 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth4 = 0;
									getAutoEffect(peer);
									packet::consolemessage(peer, itemDefs.at(pMov->plantingTree).effect);
									((PlayerInfo*)(peer->data))->noEyes = false;
									sendState(peer); //here
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth4 = pMov->plantingTree;
								packet::consolemessage(peer, itemDefs.at(pMov->plantingTree).effects);
								if (item == 1204) {
									((PlayerInfo*)(peer->data))->peffect = 8421386;
								}
								else if (item == 10128) {
									((PlayerInfo*)(peer->data))->peffect = 8421376 + 683;
								}
								else if (item == 138) {
									((PlayerInfo*)(peer->data))->peffect = 8421377;
								}
								else if (item == 2476) {
									((PlayerInfo*)(peer->data))->peffect = 8421415;
								}
								else {
									getAutoEffect(peer);
								}
								break;
							case 5:
								if (((PlayerInfo*)(event.peer->data))->cloth5 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth5 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth5 = pMov->plantingTree;
								if (pMov->plantingTree == 11440) {
									info->peffect = 8421376 + 111;
									sendState(peer);
									break;
								}
							case 6:
								if (((PlayerInfo*)(event.peer->data))->cloth6 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth6 = 0;
									((PlayerInfo*)(event.peer->data))->canDoubleJump = false;
									sendState(peer);
									break;
								}
								{
									((PlayerInfo*)(event.peer->data))->cloth6 = pMov->plantingTree;
									int item = pMov->plantingTree;
									if (item == 156 || item == 362 || item == 678 || item == 736 || item == 818 || item == 1206 || item == 1460 || item == 1550 || item == 1574 || item == 1668 || item == 1672 || item == 1674 || item == 1784 || item == 1824 || item == 1936 || item == 1938 || item == 1970 || item == 2254 || item == 2256 || item == 2258 || item == 2260 || item == 2262 || item == 2264 || item == 2390 || item == 2392 || item == 3120 || item == 3308 || item == 3512 || item == 4534 || item == 4986 || item == 5754 || item == 6144 || item == 6334 || item == 6694 || item == 6818 || item == 6842 || item == 1934 || item == 3134 || item == 6004 || item == 1780 || item == 2158 || item == 2160 || item == 2162 || item == 2164 || item == 2166 || item == 2168 || item == 2438 || item == 2538 || item == 2778 || item == 3858 || item == 350 || item == 998 || item == 1738 || item == 2642 || item == 2982 || item == 3104 || item == 3144 || item == 5738 || item == 3112 || item == 2722 || item == 3114 || item == 4970 || item == 4972 || item == 5020 || item == 6284 || item == 4184 || item == 4628 || item == 5322 || item == 4112 || item == 4114 || item == 3442) {
										((PlayerInfo*)(event.peer->data))->canDoubleJump = true;
									}
									else {
										((PlayerInfo*)(event.peer->data))->canDoubleJump = false;
									}
									// ^^^^ wings
									sendState(peer);
								}
								break;
							case 7:
								if (((PlayerInfo*)(event.peer->data))->cloth7 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth7 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth7 = pMov->plantingTree;
								break;
							case 8:
								if (((PlayerInfo*)(event.peer->data))->cloth8 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth8 = 0;
									getAutoEffect(peer);
									packet::consolemessage(peer, itemDefs.at(pMov->plantingTree).effect);
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth8 = pMov->plantingTree;
								if (pMov->plantingTree == 11232) {
									info->peffect = 8421376 + 224;
									sendState(peer);
									break;
								}
							case 9:
								if (((PlayerInfo*)(event.peer->data))->cloth9 == pMov->plantingTree)
								{
									((PlayerInfo*)(event.peer->data))->cloth9 = 0;
									break;
								}
								((PlayerInfo*)(event.peer->data))->cloth9 = pMov->plantingTree;
								break;							
							default:
#ifdef TOTAL_LOG
								cout << "Invalid item activated: " << pMov->plantingTree << " by " << ((PlayerInfo*)(event.peer->data))->displayName << endl;
#endif
								break;
							}
							sendClothes(peer);
							// activate item
						END_CLOTHSETTER_FORCE:;
						}
						if (data2->packetType == 18)
						{
							sendPData(peer, pMov);
							// add talk buble
						}
						if (data2->punchX != -1 && data2->punchY != -1) {
							//cout << data2->packetType << endl;
							if (data2->packetType == 3)
							{
								sendTileUpdate(data2->punchX, data2->punchY, data2->plantingTree, ((PlayerInfo*)(event.peer->data))->netID, peer);
							}
							else {

							}
							/*PlayerMoving data;
							//data.packetType = 0x14;
							data.packetType = 0x3;
							//data.characterState = 0x924; // animation
							data.characterState = 0x0; // animation
							data.x = data2->punchX;
							data.y = data2->punchY;
							data.punchX = data2->punchX;
							data.punchY = data2->punchY;
							data.XSpeed = 0;
							data.YSpeed = 0;
							data.netID = ((PlayerInfo*)(event.peer->data))->netID;
							data.plantingTree = data2->plantingTree;
							SendPacketRaw(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
							cout << "Tile update at: " << data2->punchX << "x" << data2->punchY << endl;*/
							
						}
						delete data2;
						delete pMov;
					}

					else {
						cout << "Got bad tank packet";
					}
					/*char buffer[2048];
					for (int i = 0; i < event->packet->dataLength; i++)
					{
					sprintf(&buffer[2 * i], "%02X", event->packet->data[i]);
					}
					cout << buffer;*/
				}
			}
			break;
			case 5:
				break;
			case 6:
				//cout << GetTextPointerFromPacket(event.packet) << endl;
				break;
			}
			enet_packet_destroy(event.packet);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
#ifdef TOTAL_LOG
			printf("Peer disconnected.\n");
#endif
			/* Reset the peer's client information. */
			/*ENetPeer* currentPeer;
			for (currentPeer = server->peers;
				currentPeer < &server->peers[server->peerCount];
				++currentPeer)
			{
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
					continue;

				GamePacket p = packetEnd(appendString(appendString(createPacket(), "OnConsoleMessage"), "Player `o" + ((PlayerInfo*)(event.peer->data))->tankIDName + "`o just left the game..."));
				ENetPacket * packet = enet_packet_create(p.data,
					p.len,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(currentPeer, 0, packet);
				enet_host_flush(server);
			}*/
			sendPlayerLeave(peer, (PlayerInfo*)(event.peer->data));
			((PlayerInfo*)(event.peer->data))->inventory.items.clear();
			delete (PlayerInfo*)event.peer->data;
			event.peer->data = NULL;
		}
	}
	cout << "Program ended??? Huh?" << endl;
	while (1);
	return 0;
}
