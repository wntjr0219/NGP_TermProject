#pragma once
#define STRLEN 20
#define RANKERS 10
#define OBSTACLES 18
typedef unsigned char BYTE;

#pragma pack(push, 1)


#define KEYUP 0
#define KEYDOWN 1
#define KEYLEFT 2
#define KEYRIGHT 3


enum packet_type {
    //Client Packet
    CSKEYPACKET,
    CSINITIALPACKET,
    CSRESUMEPACKET,
    CSEXITPACKET,
    //CSDEADPACKET

    //ServerPakcet
    SCCHARACTERPACKET,
    SCENEMYPACKET,
    SCPAUSEPACKET,
    SCWINNERPACKET,
    SCOBSTACLEPACKET,
    SCRANKINGPACKET,
    SCISDEAD,
    SCCOLLIDE
};

struct POSXYZ {
    float posX;
    float posY;
    float posZ;
};

struct RankedInfo {
    int meter;
    char name[STRLEN];
};

//--------------------------------CSPacket------------------------------------------------------
struct CSKeyPacket {
    BYTE type;
    BYTE keytype;
};

struct CSInitialPacket {
    BYTE type;
    int meter;
    char nameInitial[STRLEN];
};

struct CSResumePacket {
    BYTE type;
    bool start;
};

struct CSExitPacket {
    BYTE type;
    bool exit;
};

//struct CSDeadPacket {
//    BYTE type;
//    int cnt;
//    bool dead;
//};

//--------------------------------SCPacket------------------------------------------------------
struct SCCharacterPacket {
    BYTE type;
    bool isCollide;
    POSXYZ characterXYZ;
};

struct SCEnemyPacket {
    BYTE type;
    POSXYZ EnemyXYZ;
};

// winner 패킷으로 대체?
struct SCDeadPacket {
    BYTE type;
    bool isDead;
};

struct SCPausePacket {
    BYTE type;
    BYTE seconds;
};

struct SCWinnerPacket {
    BYTE type;
    bool winner;
};

struct SCObstaclePacket {
    BYTE type;
    POSXYZ obstacleXYZ[OBSTACLES];
};

struct SCRankingPacket {
    BYTE type;
    RankedInfo rankings[RANKERS];
};

#pragma pack(pop)

/*
switch (type)
{
case SCCHARACTERPACKET:

    break;
case SCENEMYPACKET:
    break;
case SCRESUMEPACKET:
    break;
case SCWINNERPACKET:
    break;
case SCOBSTACLEPACKET:
    break;
case SCRANKINGPACKET:
    break;
case SCRANKEDINFO:
    break;
default:
    break;
}
*/