#pragma once
#include "StdAfx.h"
#include "GameType.h"

#include <cstdint>

// Control-plane structs + headers (shared with EterLib/NetStream base class)
#include "EterLib/ControlPackets.h"

//
// Packet header constants (uint16_t)
//
// Packet framing: [header:2] [length:2] [payload...]
// - header: one of the constants below
// - length: total packet size including header+length (minimum 4)
// - payload: packet-specific data
//
// Ranges:
//   0x0001-0x00FF  Control (handshake, phase, ping, key exchange)
//   0x0100-0x01FF  Authentication (login, auth, empire)
//   0x0200-0x02FF  Character (create, delete, select, stats)
//   0x0300-0x03FF  Movement (move, sync, warp)
//   0x0400-0x04FF  Combat (attack, skill, damage, pvp)
//   0x0500-0x05FF  Items (use, drop, move, exchange, refine)
//   0x0600-0x06FF  Chat (chat, whisper)
//   0x0700-0x07FF  Social (party, guild, messenger)
//   0x0800-0x08FF  Shop/Trade (shop, safebox, mall)
//   0x0900-0x09FF  Quest (script, quest confirm)
//   0x0A00-0x0AFF  UI/Effect (target, affect, effects)
//   0x0B00-0x0BFF  World (dungeon, fishing, land, time)
//   0x0C00-0x0CFF  Guild Marks
//

// Packet header type (used by dispatch/registration systems)
typedef uint16_t TPacketHeader;

// Minimum packet size: header(2) + length(2)
constexpr uint16_t PACKET_HEADER_SIZE = 4;

// ============================================================================
// CG -- Client -> Game
// ============================================================================
namespace CG {
// Control (PONG, KEY_RESPONSE moved to EterLib/ControlPackets.h)
constexpr uint16_t CLIENT_VERSION = 0x000D;
constexpr uint16_t STATE_CHECKER = 0x000F;
constexpr uint16_t TEXT = 0x0011;

// Authentication
constexpr uint16_t LOGIN2 = 0x0101;
constexpr uint16_t LOGIN3 = 0x0102;
constexpr uint16_t LOGIN_SECURE = 0x0103;
constexpr uint16_t EMPIRE = 0x010A;
constexpr uint16_t CHANGE_NAME = 0x010B;

// Character
constexpr uint16_t CHARACTER_CREATE = 0x0201;
constexpr uint16_t CHARACTER_DELETE = 0x0202;
constexpr uint16_t CHARACTER_SELECT = 0x0203;
constexpr uint16_t ENTERGAME = 0x0204;

// Movement
constexpr uint16_t MOVE = 0x0301;
constexpr uint16_t SYNC_POSITION = 0x0303;
constexpr uint16_t WARP = 0x0305;

// Combat
constexpr uint16_t ATTACK = 0x0401;
constexpr uint16_t USE_SKILL = 0x0402;
constexpr uint16_t SHOOT = 0x0403;
constexpr uint16_t FLY_TARGETING = 0x0404;
constexpr uint16_t ADD_FLY_TARGETING = 0x0405;

// Items
constexpr uint16_t ITEM_USE = 0x0501;
constexpr uint16_t ITEM_DROP = 0x0502;
constexpr uint16_t ITEM_DROP2 = 0x0503;
constexpr uint16_t ITEM_MOVE = 0x0504;
constexpr uint16_t ITEM_PICKUP = 0x0505;
constexpr uint16_t ITEM_USE_TO_ITEM = 0x0506;
constexpr uint16_t ITEM_GIVE = 0x0507;
constexpr uint16_t EXCHANGE = 0x0508;
constexpr uint16_t QUICKSLOT_ADD = 0x0509;
constexpr uint16_t QUICKSLOT_DEL = 0x050A;
constexpr uint16_t QUICKSLOT_SWAP = 0x050B;
constexpr uint16_t REFINE = 0x050C;
constexpr uint16_t DRAGON_SOUL_REFINE = 0x050D;

// Chat
constexpr uint16_t CHAT = 0x0601;
constexpr uint16_t WHISPER = 0x0602;

// Social
constexpr uint16_t PARTY_INVITE = 0x0701;
constexpr uint16_t PARTY_INVITE_ANSWER = 0x0702;
constexpr uint16_t PARTY_REMOVE = 0x0703;
constexpr uint16_t PARTY_SET_STATE = 0x0704;
constexpr uint16_t PARTY_USE_SKILL = 0x0705;
constexpr uint16_t PARTY_PARAMETER = 0x0706;
constexpr uint16_t GUILD = 0x0720;
constexpr uint16_t ANSWER_MAKE_GUILD = 0x0721;
constexpr uint16_t GUILD_SYMBOL_UPLOAD = 0x0722;
constexpr uint16_t SYMBOL_CRC = 0x0723;
constexpr uint16_t MESSENGER = 0x0740;

// Shop / Safebox / Mall
constexpr uint16_t SHOP = 0x0801;
constexpr uint16_t MYSHOP = 0x0802;
constexpr uint16_t SAFEBOX_CHECKIN = 0x0820;
constexpr uint16_t SAFEBOX_CHECKOUT = 0x0821;
constexpr uint16_t SAFEBOX_ITEM_MOVE = 0x0822;
constexpr uint16_t MALL_CHECKOUT = 0x0840;

// Quest
constexpr uint16_t SCRIPT_ANSWER = 0x0901;
constexpr uint16_t SCRIPT_BUTTON = 0x0902;
constexpr uint16_t SCRIPT_SELECT_ITEM = 0x0903;
constexpr uint16_t QUEST_INPUT_STRING = 0x0904;
constexpr uint16_t QUEST_CONFIRM = 0x0905;
constexpr uint16_t QUEST_CANCEL = 0x0906;

// UI / Targeting
constexpr uint16_t TARGET = 0x0A01;
constexpr uint16_t ON_CLICK = 0x0A02;
constexpr uint16_t CHARACTER_POSITION = 0x0A60;

// World
constexpr uint16_t FISHING = 0x0B01;
constexpr uint16_t DUNGEON = 0x0B02;
constexpr uint16_t HACK = 0x0B03;

// Guild Marks
constexpr uint16_t MARK_LOGIN = 0x0C01;
constexpr uint16_t MARK_CRCLIST = 0x0C02;
constexpr uint16_t MARK_UPLOAD = 0x0C03;
constexpr uint16_t MARK_IDXLIST = 0x0C04;
} // namespace CG

// ============================================================================
// GC -- Game -> Client
// ============================================================================
namespace GC {
// Control (PING, PHASE, KEY_CHALLENGE, KEY_COMPLETE moved to
// EterLib/ControlPackets.h)
constexpr uint16_t RESPOND_CHANNELSTATUS = 0x0010;

// Authentication
constexpr uint16_t LOGIN_SUCCESS3 = 0x0104;
constexpr uint16_t LOGIN_SUCCESS4 = 0x0105;
constexpr uint16_t LOGIN_FAILURE = 0x0106;
constexpr uint16_t LOGIN_KEY = 0x0107;
constexpr uint16_t AUTH_SUCCESS = 0x0108;
constexpr uint16_t EMPIRE = 0x0109;
constexpr uint16_t CHANGE_NAME = 0x010C;

// Character
constexpr uint16_t CHARACTER_ADD = 0x0205;
constexpr uint16_t CHARACTER_ADD2 = 0x0206;
constexpr uint16_t CHAR_ADDITIONAL_INFO = 0x0207;
constexpr uint16_t CHARACTER_DEL = 0x0208;
constexpr uint16_t CHARACTER_UPDATE = 0x0209;
constexpr uint16_t CHARACTER_UPDATE2 = 0x020A;
constexpr uint16_t CHARACTER_POSITION = 0x020B;
constexpr uint16_t PLAYER_CREATE_SUCCESS = 0x020C;
constexpr uint16_t PLAYER_CREATE_FAILURE = 0x020D;
constexpr uint16_t PLAYER_DELETE_SUCCESS = 0x020E;
constexpr uint16_t PLAYER_DELETE_WRONG_SOCIAL_ID = 0x020F;
constexpr uint16_t MAIN_CHARACTER = 0x0210;
constexpr uint16_t PLAYER_POINTS = 0x0214;
constexpr uint16_t PLAYER_POINT_CHANGE = 0x0215;
constexpr uint16_t STUN = 0x0216;
constexpr uint16_t DEAD = 0x0217;
constexpr uint16_t CHANGE_SPEED = 0x0218;
constexpr uint16_t WALK_MODE = 0x0219;
constexpr uint16_t SKILL_LEVEL = 0x021A;
constexpr uint16_t SKILL_LEVEL_NEW = 0x021B;
constexpr uint16_t SKILL_COOLTIME_END = 0x021C;
constexpr uint16_t CHANGE_SKILL_GROUP = 0x021D;
constexpr uint16_t VIEW_EQUIP = 0x021E;

// Movement
constexpr uint16_t MOVE = 0x0302;
constexpr uint16_t SYNC_POSITION = 0x0304;
constexpr uint16_t WARP = 0x0306;
constexpr uint16_t MOTION = 0x0307;
constexpr uint16_t DIG_MOTION = 0x0308;

// Combat
constexpr uint16_t DAMAGE_INFO = 0x0410;
constexpr uint16_t FLY_TARGETING = 0x0411;
constexpr uint16_t ADD_FLY_TARGETING = 0x0412;
constexpr uint16_t CREATE_FLY = 0x0413;
constexpr uint16_t PVP = 0x0414;
constexpr uint16_t DUEL_START = 0x0415;

// Items
constexpr uint16_t ITEM_DEL = 0x0510;
constexpr uint16_t ITEM_SET = 0x0511;
constexpr uint16_t ITEM_USE = 0x0512;
constexpr uint16_t ITEM_DROP = 0x0513;
constexpr uint16_t ITEM_UPDATE = 0x0514;
constexpr uint16_t ITEM_GROUND_ADD = 0x0515;
constexpr uint16_t ITEM_GROUND_DEL = 0x0516;
constexpr uint16_t ITEM_OWNERSHIP = 0x0517;
constexpr uint16_t ITEM_GET = 0x0518;
constexpr uint16_t QUICKSLOT_ADD = 0x0519;
constexpr uint16_t QUICKSLOT_DEL = 0x051A;
constexpr uint16_t QUICKSLOT_SWAP = 0x051B;
constexpr uint16_t EXCHANGE = 0x051C;
constexpr uint16_t REFINE_INFORMATION = 0x051D;
constexpr uint16_t REFINE_INFORMATION_NEW = 0x051E;
constexpr uint16_t DRAGON_SOUL_REFINE = 0x051F;

// Chat
constexpr uint16_t CHAT = 0x0603;
constexpr uint16_t WHISPER = 0x0604;

// Social
constexpr uint16_t PARTY_INVITE = 0x0710;
constexpr uint16_t PARTY_ADD = 0x0711;
constexpr uint16_t PARTY_UPDATE = 0x0712;
constexpr uint16_t PARTY_REMOVE = 0x0713;
constexpr uint16_t PARTY_LINK = 0x0714;
constexpr uint16_t PARTY_UNLINK = 0x0715;
constexpr uint16_t PARTY_PARAMETER = 0x0716;
constexpr uint16_t GUILD = 0x0730;
constexpr uint16_t REQUEST_MAKE_GUILD = 0x0731;
constexpr uint16_t SYMBOL_DATA = 0x0732;
constexpr uint16_t MESSENGER = 0x0741;
constexpr uint16_t LOVER_INFO = 0x0750;
constexpr uint16_t LOVE_POINT_UPDATE = 0x0751;

// Shop / Safebox / Mall
constexpr uint16_t SHOP = 0x0810;
constexpr uint16_t SHOP_SIGN = 0x0811;
constexpr uint16_t SAFEBOX_SET = 0x0830;
constexpr uint16_t SAFEBOX_DEL = 0x0831;
constexpr uint16_t SAFEBOX_WRONG_PASSWORD = 0x0832;
constexpr uint16_t SAFEBOX_SIZE = 0x0833;
constexpr uint16_t SAFEBOX_MONEY_CHANGE = 0x0834;
constexpr uint16_t MALL_OPEN = 0x0841;
constexpr uint16_t MALL_SET = 0x0842;
constexpr uint16_t MALL_DEL = 0x0843;

// Quest
constexpr uint16_t SCRIPT = 0x0910;
constexpr uint16_t QUEST_CONFIRM = 0x0911;
constexpr uint16_t QUEST_INFO = 0x0912;

// UI / Effects / Targeting
constexpr uint16_t TARGET = 0x0A10;
constexpr uint16_t TARGET_UPDATE = 0x0A11;
constexpr uint16_t TARGET_DELETE = 0x0A12;
constexpr uint16_t TARGET_CREATE_NEW = 0x0A13;
constexpr uint16_t AFFECT_ADD = 0x0A20;
constexpr uint16_t AFFECT_REMOVE = 0x0A21;
constexpr uint16_t SEPCIAL_EFFECT = 0x0A30;
constexpr uint16_t SPECIFIC_EFFECT = 0x0A31;
constexpr uint16_t MOUNT = 0x0A40;
constexpr uint16_t OWNERSHIP = 0x0A41;
constexpr uint16_t NPC_POSITION = 0x0A50;

// World
constexpr uint16_t FISHING = 0x0B10;
constexpr uint16_t DUNGEON = 0x0B11;
constexpr uint16_t LAND_LIST = 0x0B12;
constexpr uint16_t TIME = 0x0B13;
constexpr uint16_t CHANNEL = 0x0B14;
constexpr uint16_t MARK_UPDATE = 0x0B15;
constexpr uint16_t OBSERVER_ADD = 0x0B20;
constexpr uint16_t OBSERVER_REMOVE = 0x0B21;
constexpr uint16_t OBSERVER_MOVE = 0x0B22;

// Guild Marks
constexpr uint16_t MARK_BLOCK = 0x0C10;
constexpr uint16_t MARK_IDXLIST = 0x0C11;
constexpr uint16_t MARK_DIFF_DATA = 0x0C12;
} // namespace GC

// --- Phase constants ---

enum EPhases {
  PHASE_CLOSE,
  PHASE_HANDSHAKE,
  PHASE_LOGIN,
  PHASE_SELECT,
  PHASE_LOADING,
  PHASE_GAME,
  PHASE_DEAD,

  PHASE_CLIENT_CONNECTING,
  PHASE_DBCLIENT,
  PHASE_P2P,
  PHASE_AUTH,
};

// ============================================================================
// Subheader enums — grouped by feature
// ============================================================================

namespace GuildSub {
namespace CG {
enum : uint8_t {
  ADD_MEMBER,
  REMOVE_MEMBER,
  CHANGE_GRADE_NAME,
  CHANGE_GRADE_AUTHORITY,
  OFFER,
  POST_COMMENT,
  DELETE_COMMENT,
  REFRESH_COMMENT,
  CHANGE_MEMBER_GRADE,
  USE_SKILL,
  CHANGE_MEMBER_GENERAL,
  GUILD_INVITE_ANSWER,
  CHARGE_GSP,
  DEPOSIT_MONEY,
  WITHDRAW_MONEY,
};
}
namespace GC {
enum : uint8_t {
  LOGIN,
  LOGOUT,
  LIST,
  GRADE,
  ADD,
  REMOVE,
  GRADE_NAME,
  GRADE_AUTH,
  INFO,
  COMMENTS,
  CHANGE_EXP,
  CHANGE_MEMBER_GRADE,
  SKILL_INFO,
  CHANGE_MEMBER_GENERAL,
  GUILD_INVITE,
  WAR,
  GUILD_NAME,
  GUILD_WAR_LIST,
  GUILD_WAR_END_LIST,
  WAR_POINT,
  MONEY_CHANGE,
};
}
} // namespace GuildSub

namespace ShopSub {
namespace CG {
enum : uint8_t {
  END,
  BUY,
  SELL,
  SELL2,
};
}
namespace GC {
enum : uint8_t {
  START,
  END,
  UPDATE_ITEM,
  UPDATE_PRICE,
  OK,
  NOT_ENOUGH_MONEY,
  SOLDOUT,
  INVENTORY_FULL,
  INVALID_POS,
  SOLD_OUT,
  START_EX,
  NOT_ENOUGH_MONEY_EX,
};
}
} // namespace ShopSub

namespace ExchangeSub {
namespace CG {
enum : uint8_t {
  START,
  ITEM_ADD,
  ITEM_DEL,
  ELK_ADD,
  ACCEPT,
  CANCEL,
};
}
namespace GC {
enum : uint8_t {
  START,
  ITEM_ADD,
  ITEM_DEL,
  ELK_ADD,
  ACCEPT,
  END,
  ALREADY,
  LESS_ELK,
};
}
} // namespace ExchangeSub

namespace MessengerSub {
namespace CG {
enum : uint8_t {
  ADD_BY_VID,
  ADD_BY_NAME,
  REMOVE,
  INVITE_ANSWER, // Added to match server packet_headers.h
};
}
namespace GC {
enum : uint8_t {
  LIST,
  LOGIN,
  LOGOUT,
  INVITE,
  REMOVE_FRIEND,
};
}
} // namespace MessengerSub

namespace FishingSub {
namespace GC {
enum : uint8_t {
  START,
  STOP,
  REACT,
  SUCCESS,
  FAIL,
  FISH,
};
}
} // namespace FishingSub

namespace DungeonSub {
namespace GC {
enum : uint8_t {
  TIME_ATTACK_START = 0,
  DESTINATION_POSITION = 1,
};
}
} // namespace DungeonSub

namespace DragonSoulSub {
enum : uint8_t {
  OPEN,
  CLOSE,
  DO_UPGRADE,
  DO_IMPROVEMENT,
  DO_REFINE,
  REFINE_FAIL,
  REFINE_FAIL_MAX_REFINE,
  REFINE_FAIL_INVALID_MATERIAL,
  REFINE_FAIL_NOT_ENOUGH_MONEY,
  REFINE_FAIL_NOT_ENOUGH_MATERIAL,
  REFINE_FAIL_TOO_MUCH_MATERIAL,
  REFINE_SUCCEED,
};
}

enum {
  ID_MAX_NUM = 30,
  PASS_MAX_NUM = 16,
  CHAT_MAX_NUM = 128,
  PATH_NODE_MAX_NUM = 64,
  SHOP_SIGN_MAX_LEN = 32,

  PLAYER_PER_ACCOUNT3 = 3,
  PLAYER_PER_ACCOUNT4 = 4,

  PLAYER_ITEM_SLOT_MAX_NUM = 20, // 플래이어의 슬롯당 들어가는 갯수.

  QUICKSLOT_MAX_LINE = 4,
  QUICKSLOT_MAX_COUNT_PER_LINE = 8, // 클라이언트 임의 결정값
  QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,

  QUICKSLOT_MAX_NUM = 36, // 서버와 맞춰져 있는 값

  SHOP_HOST_ITEM_MAX_NUM = 40,

  METIN_SOCKET_COUNT = 6,

  PARTY_AFFECT_SLOT_MAX_NUM = 7,

  GUILD_GRADE_NAME_MAX_LEN = 8,
  GUILD_NAME_MAX_LEN = 12,
  GUILD_GRADE_COUNT = 15,
  GULID_COMMENT_MAX_LEN = 50,

  MARK_CRC_NUM = 8 * 8,
  MARK_DATA_SIZE = 16 * 12,
  SYMBOL_DATA_SIZE = 128 * 256,
  QUEST_INPUT_STRING_MAX_NUM = 64,

  PRIVATE_CODE_LENGTH = 8,

  REFINE_MATERIAL_MAX_NUM = 5,

  WEAR_MAX_NUM = 11,

  SHOP_TAB_NAME_MAX = 32,
  SHOP_TAB_COUNT_MAX = 3,
};

#pragma pack(push)
#pragma pack(1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mark
typedef struct command_mark_login {
  uint16_t header;
  uint16_t length;
  uint32_t handle;
  uint32_t random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload {
  uint16_t header;
  uint16_t length;
  uint32_t gid;
  uint8_t image[16 * 12 * 4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist {
  uint16_t header;
  uint16_t length;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist {
  uint16_t header;
  uint16_t length;
  uint8_t imgIdx;
  uint32_t crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist {
  uint16_t header;
  uint16_t length;
  uint32_t bufSize;
  uint16_t count;
  // 뒤에 size * (uint16_t + uint16_t)만큼 데이터 붙음
} TPacketGCMarkIDXList;

typedef struct packet_mark_block {
  uint16_t header;
  uint16_t length;
  uint32_t bufSize;
  uint8_t imgIdx;
  uint32_t count;
  // 뒤에 64 x 48 x 픽셀크기(4바이트) = 12288만큼 데이터 붙음
} TPacketGCMarkBlock;

typedef struct packet_mark_update {
  uint16_t header;
  uint16_t length;
  uint32_t guildID;
  uint16_t imgIdx;
} TPacketGCMarkUpdate;

typedef struct command_symbol_upload {
  uint16_t header;
  uint16_t length;
  uint32_t handle;
} TPacketCGSymbolUpload;

typedef struct command_symbol_crc {
  uint16_t header;
  uint16_t length;
  uint32_t dwGuildID;
  uint32_t dwCRC;
  uint32_t dwSize;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data {
  uint16_t header;
  uint16_t length;
  uint32_t guild_id;
} TPacketGCGuildSymbolData;

//
//
//
typedef struct packet_observer_add {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint16_t x;
  uint16_t y;
} TPacketGCObserverAdd;

typedef struct packet_observer_move {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint16_t x;
  uint16_t y;
} TPacketGCObserverMove;

typedef struct packet_observer_remove {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketGCObserverRemove;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// To Server

typedef struct command_checkin {
  uint16_t header;
  uint16_t length;
  char name[ID_MAX_NUM + 1];
  char pwd[PASS_MAX_NUM + 1];
} TPacketCGCheckin;

// start - 권한 서버 접속을 위한 패킷들
typedef struct command_login2 {
  uint16_t header;
  uint16_t length;
  char name[ID_MAX_NUM + 1];
  uint32_t login_key;
} TPacketCGLogin2;

typedef struct command_login3 {
  uint16_t header;
  uint16_t length;
  char name[ID_MAX_NUM + 1];
  char pwd[PASS_MAX_NUM + 1];
} TPacketCGLogin3;

typedef struct command_direct_enter {
  uint16_t header;
  uint16_t length;
  char login[ID_MAX_NUM + 1];
  char passwd[PASS_MAX_NUM + 1];
  uint8_t index;
} TPacketCGDirectEnter;

typedef struct command_player_select {
  uint16_t header;
  uint16_t length;
  uint8_t player_index;
} TPacketCGSelectCharacter;

typedef struct command_attack {
  uint16_t header;
  uint16_t length;
  uint8_t bType;        // 공격 유형
  uint32_t dwVictimVID; // 적 VID
  uint8_t bCRCMagicCubeProcPiece;
  uint8_t bCRCMagicCubeFilePiece;
} TPacketCGAttack;

typedef struct command_chat {
  uint16_t header;
  uint16_t length;
  uint8_t type;
} TPacketCGChat;

typedef struct command_whisper {
  uint16_t header;
  uint16_t length;
  char szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

enum EBattleMode {
  BATTLEMODE_ATTACK = 0,
  BATTLEMODE_DEFENSE = 1,
};

typedef struct command_EnterFrontGame {
  uint16_t header;
  uint16_t length;
} TPacketCGEnterFrontGame;

typedef struct command_item_use {
  uint16_t header;
  uint16_t length;
  TItemPos pos;
} TPacketCGItemUse;

typedef struct command_item_use_to_item {
  uint16_t header;
  uint16_t length;
  TItemPos source_pos;
  TItemPos target_pos;
} TPacketCGItemUseToItem;

typedef struct command_item_drop {
  uint16_t header;
  uint16_t length;
  TItemPos pos;
  uint32_t elk;
} TPacketCGItemDrop;

typedef struct command_item_drop2 {
  uint16_t header;
  uint16_t length;
  TItemPos pos;
  uint32_t gold;
  uint8_t count;
} TPacketCGItemDrop2;

typedef struct command_item_move {
  uint16_t header;
  uint16_t length;
  TItemPos pos;
  TItemPos change_pos;
  uint8_t num;
} TPacketCGItemMove;

typedef struct command_item_pickup {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketCGItemPickUp;

typedef struct command_quickslot_add {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
  TQuickSlot slot;
} TPacketCGQuickSlotAdd;

typedef struct command_quickslot_del {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
} TPacketCGQuickSlotDel;

typedef struct command_quickslot_swap {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
  uint8_t change_pos;
} TPacketCGQuickSlotSwap;

typedef struct command_on_click {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketCGOnClick;

typedef struct command_shop {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
} TPacketCGShop;

typedef struct command_exchange {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
  uint32_t arg1;
  uint8_t arg2;
  TItemPos Pos;
} TPacketCGExchange;

typedef struct command_position {
  uint16_t header;
  uint16_t length;
  uint8_t position;
} TPacketCGPosition;

typedef struct command_script_answer {
  uint16_t header;
  uint16_t length;
  uint8_t answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button {
  uint16_t header;
  uint16_t length;
  uint32_t idx;
} TPacketCGScriptButton;

typedef struct command_target {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
} TPacketCGTarget;

typedef struct command_move {
  uint16_t header;
  uint16_t length;
  uint8_t bFunc;
  uint8_t bArg;
  uint8_t bRot;
  int32_t lX; // Signed to match server (can be negative coordinates)
  int32_t lY; // Signed to match server
  uint32_t dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element {
  uint32_t dwVID;
  int32_t lX;
  int32_t lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position {
  uint16_t header;
  uint16_t length;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting {
  uint16_t header;
  uint16_t length;
  uint32_t dwTargetVID;
  int32_t lX;
  int32_t lY;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting {
  uint16_t header;
  uint16_t length;
  uint32_t dwShooterVID;
  uint32_t dwTargetVID;
  int32_t lX;
  int32_t lY;
} TPacketGCFlyTargeting;

typedef struct packet_shoot {
  uint16_t header;
  uint16_t length;
  uint8_t bType;
} TPacketCGShoot;

typedef struct command_warp {
  uint16_t header;
  uint16_t length;
} TPacketCGWarp;

typedef struct packet_messenger {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_list_offline {
  uint8_t connected; // always 0
  uint8_t length;
} TPacketGCMessengerListOffline;

enum {
  MESSENGER_CONNECTED_STATE_OFFLINE,
  MESSENGER_CONNECTED_STATE_ONLINE,
};

typedef struct packet_messenger_list_online {
  uint8_t connected;
  uint8_t length;
  // uint8_t length_char_name;
} TPacketGCMessengerListOnline;

typedef struct packet_messenger_login {
  // uint8_t length_login;
  // uint8_t length_char_name;
  uint8_t length;
} TPacketGCMessengerLogin;

typedef struct packet_messenger_logout {
  uint8_t length;
} TPacketGCMessengerLogout;

typedef struct command_messenger {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
} TPacketCGMessenger;

typedef struct command_messenger_remove {
  uint8_t length;
} TPacketCGMessengerRemove;

enum {
  SAFEBOX_MONEY_STATE_SAVE,
  SAFEBOX_MONEY_STATE_WITHDRAW,
};

typedef struct command_safebox_money {
  uint16_t header;
  uint16_t length;
  uint8_t bState;
  int32_t lMoney; // Changed from uint32_t to int32_t to match server
                  // packet_structs.h
} TPacketCGSafeboxMoney;

typedef struct command_safebox_checkout {
  uint16_t header;
  uint16_t length;
  uint8_t bSafePos;
  TItemPos ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin {
  uint16_t header;
  uint16_t length;
  uint8_t bSafePos;
  TItemPos ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_mall_checkout {
  uint16_t header;
  uint16_t length;
  uint8_t bMallPos;
  TItemPos ItemPos;
} TPacketCGMallCheckout;

///////////////////////////////////////////////////////////////////////////////////
// Party

typedef struct command_use_skill {
  uint16_t header;
  uint16_t length;
  uint32_t dwVnum;
  uint32_t dwTargetVID;
} TPacketCGUseSkill;

typedef struct command_party_invite {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer {
  uint16_t header;
  uint16_t length;
  uint32_t leader_pid;
  uint8_t accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  uint8_t byState;
  uint8_t byFlag;
} TPacketCGPartySetState;

typedef struct packet_party_link {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
  uint32_t vid;
} TPacketGCPartyLink;

typedef struct packet_party_unlink {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
  uint32_t vid;
} TPacketGCPartyUnlink;

typedef struct command_party_use_skill {
  uint16_t header;
  uint16_t length;
  uint8_t bySkillIndex;
  uint32_t dwTargetVID;
} TPacketCGPartyUseSkill;

typedef struct command_guild {
  uint16_t header;
  uint16_t length;
  uint8_t bySubHeader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild {
  uint16_t header;
  uint16_t length;
  char guild_name[GUILD_NAME_MAX_LEN + 1];
} TPacketCGAnswerMakeGuild;

typedef struct command_give_item {
  uint16_t header;
  uint16_t length;
  uint32_t dwTargetVID;
  TItemPos ItemPos;
  uint8_t byItemCount;
} TPacketCGGiveItem;

typedef struct SPacketCGHack {
  uint16_t header;
  uint16_t length;
  char szBuf[255 + 1];
} TPacketCGHack;

typedef struct command_dungeon {
  uint16_t header;
  uint16_t length;
} TPacketCGDungeon;

// Private Shop
typedef struct SShopItemTable {
  uint32_t vnum;
  uint8_t count;

  TItemPos pos;        // PC 상점에만 이용
  uint32_t price;      // PC 상점에만 이용
  uint8_t display_pos; //	PC 상점에만 이용, 보일 위치.
} TShopItemTable;

typedef struct SPacketCGMyShop {
  uint16_t header;
  uint16_t length;
  char szSign[SHOP_SIGN_MAX_LEN + 1];
  uint8_t bCount; // count of TShopItemTable, max 39
} TPacketCGMyShop;

typedef struct SPacketCGRefine {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
  uint8_t type;
} TPacketCGRefine;

typedef struct SPacketCGChangeName {
  uint16_t header;
  uint16_t length;
  uint8_t index;
  char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGChangeName;

typedef struct command_client_version {
  uint16_t header;
  uint16_t length;
  char filename[32 + 1];
  char timestamp[32 + 1];
} TPacketCGClientVersion;

typedef struct command_crc_report {
  uint16_t header;
  uint16_t length;
  uint8_t byPackMode;
  uint32_t dwBinaryCRC32;
  uint32_t dwProcessCRC32;
  uint32_t dwRootPackCRC32;
} TPacketCGCRCReport;

enum EPartyExpDistributionType {
  PARTY_EXP_DISTRIBUTION_NON_PARITY,
  PARTY_EXP_DISTRIBUTION_PARITY,
};

typedef struct command_party_parameter {
  uint16_t header;
  uint16_t length;
  uint8_t bDistributeMode;
} TPacketCGPartyParameter;

typedef struct command_quest_input_string {
  uint16_t header;
  uint16_t length;
  char szString[QUEST_INPUT_STRING_MAX_NUM + 1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm {
  uint16_t header;
  uint16_t length;
  uint8_t answer;
  uint32_t requestPID;
} TPacketCGQuestConfirm;

typedef struct command_quest_cancel {
  uint16_t header;
  uint16_t length;
} TPacketCGQuestCancel;

typedef struct command_script_select_item {
  uint16_t header;
  uint16_t length;
  uint32_t selection;
} TPacketCGScriptSelectItem;

// TPacketGCPhase moved to EterLib/ControlPackets.h

typedef struct packet_blank {
  uint16_t header;
  uint16_t length;
} TPacketGCBlank;

typedef TPacketGCBlank TPacketGCBlankDynamic;

typedef struct packet_header_dynamic_size {
  uint16_t header;
  uint16_t length;
} TDynamicSizePacketHeader;

typedef struct SSimplePlayerInformation {
  uint32_t dwID;
  char szName[CHARACTER_NAME_MAX_LEN + 1];
  uint8_t byJob;
  uint8_t byLevel;
  uint32_t dwPlayMinutes;
  uint8_t byST, byHT, byDX, byIQ;
  //	uint16_t				wParts[CRaceData::PART_MAX_NUM];
  uint16_t wMainPart;
  uint8_t bChangeName;
  uint16_t wHairPart;
  uint8_t bDummy[4];
  int32_t x, y;
  uint32_t lAddr;
  uint16_t wPort;
  uint8_t bySkillGroup;
} TSimplePlayerInformation;

typedef struct packet_login_success3 {
  uint16_t header;
  uint16_t length;
  TSimplePlayerInformation akSimplePlayerInformation[PLAYER_PER_ACCOUNT3];
  uint32_t guild_id[PLAYER_PER_ACCOUNT3];
  char guild_name[PLAYER_PER_ACCOUNT3][GUILD_NAME_MAX_LEN + 1];
  uint32_t handle;
  uint32_t random_key;
} TPacketGCLoginSuccess3;

typedef struct packet_login_success4 {
  uint16_t header;
  uint16_t length;
  TSimplePlayerInformation akSimplePlayerInformation[PLAYER_PER_ACCOUNT4];
  uint32_t guild_id[PLAYER_PER_ACCOUNT4];
  char guild_name[PLAYER_PER_ACCOUNT4][GUILD_NAME_MAX_LEN + 1];
  uint32_t handle;
  uint32_t random_key;
} TPacketGCLoginSuccess4;

enum { LOGIN_STATUS_MAX_LEN = 8 };
typedef struct packet_login_failure {
  uint16_t header;
  uint16_t length;
  char szStatus[LOGIN_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct command_player_create {
  uint16_t header;
  uint16_t length;
  uint8_t index;
  char name[CHARACTER_NAME_MAX_LEN + 1];
  uint16_t job;
  uint8_t shape;
  uint8_t CON;
  uint8_t INT;
  uint8_t STR;
  uint8_t DEX;
} TPacketCGCreateCharacter;

typedef struct command_player_create_success {
  uint16_t header;
  uint16_t length;
  uint8_t bAccountCharacterSlot;
  TSimplePlayerInformation kSimplePlayerInfomation;
} TPacketGCPlayerCreateSuccess;

typedef struct command_create_failure {
  uint16_t header;
  uint16_t length;
  uint8_t bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete {
  uint16_t header;
  uint16_t length;
  uint8_t index;
  char szPrivateCode[PRIVATE_CODE_LENGTH];
} TPacketCGDestroyCharacter;

typedef struct packet_player_delete_success {
  uint16_t header;
  uint16_t length;
  uint8_t account_index;
} TPacketGCDestroyCharacterSuccess;

enum {
  ADD_CHARACTER_STATE_DEAD = (1 << 0),
  ADD_CHARACTER_STATE_SPAWN = (1 << 1),
  ADD_CHARACTER_STATE_GUNGON = (1 << 2),
  ADD_CHARACTER_STATE_KILLER = (1 << 3),
  ADD_CHARACTER_STATE_PARTY = (1 << 4),
};

enum EPKModes {
  PK_MODE_PEACE,
  PK_MODE_REVENGE,
  PK_MODE_FREE,
  PK_MODE_PROTECT,
  PK_MODE_GUILD,
  PK_MODE_MAX_NUM,
};

// 2004.11.20.myevan.CRaceData::PART_MAX_NUM 사용안하게 수정 - 서버에서
// 사용하는것과 일치하지 않음
enum ECharacterEquipmentPart {
  CHR_EQUIPPART_ARMOR,
  CHR_EQUIPPART_WEAPON,
  CHR_EQUIPPART_HEAD,
  CHR_EQUIPPART_HAIR,

  CHR_EQUIPPART_NUM,
};

typedef struct packet_char_additional_info {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  char name[CHARACTER_NAME_MAX_LEN + 1];
  uint16_t awPart[CHR_EQUIPPART_NUM];
  uint8_t bEmpire;
  uint32_t dwGuildID;
  uint32_t dwLevel;
  int16_t sAlignment; // 선악치
  uint8_t bPKMode;
  uint32_t dwMountVnum;
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_add_char {
  uint16_t header;
  uint16_t length;

  uint32_t dwVID;

  // char        name[CHARACTER_NAME_MAX_LEN + 1];

  float angle;
  int32_t x;
  int32_t y;
  int32_t z;

  uint8_t bType;
  uint16_t wRaceNum;
  // uint16_t        awPart[CHR_EQUIPPART_NUM];
  uint8_t bMovingSpeed;
  uint8_t bAttackSpeed;

  uint8_t bStateFlag;
  uint32_t dwAffectFlag[2]; // ??
                            // uint8_t      bEmpire;
  // uint32_t     dwGuild;
  // int16_t     sAlignment;
  // uint8_t		bPKMode;
  // uint32_t		dwMountVnum;
} TPacketGCCharacterAdd;

typedef struct packet_add_char2 {
  uint16_t header;
  uint16_t length;

  uint32_t dwVID;

  char name[CHARACTER_NAME_MAX_LEN + 1];

  float angle;
  int32_t x;
  int32_t y;
  int32_t z;

  uint8_t bType;
  uint16_t wRaceNum;
  uint16_t awPart[CHR_EQUIPPART_NUM];
  uint8_t bMovingSpeed;
  uint8_t bAttackSpeed;

  uint8_t bStateFlag;
  uint32_t dwAffectFlag[2]; // ??
  uint8_t bEmpire;

  uint32_t dwGuild;
  int16_t sAlignment;
  uint8_t bPKMode;
  uint32_t dwMountVnum;
} TPacketGCCharacterAdd2;

typedef struct packet_update_char {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;

  uint16_t awPart[CHR_EQUIPPART_NUM];
  uint8_t bMovingSpeed;
  uint8_t bAttackSpeed;

  uint8_t bStateFlag;
  uint32_t dwAffectFlag[2];

  uint32_t dwGuildID;
  int16_t sAlignment;
  uint8_t bPKMode;
  uint32_t dwMountVnum;
} TPacketGCCharacterUpdate;

typedef struct packet_update_char2 {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;

  uint16_t awPart[CHR_EQUIPPART_NUM];
  uint8_t bMovingSpeed;
  uint8_t bAttackSpeed;

  uint8_t bStateFlag;
  uint32_t dwAffectFlag[2];

  uint32_t dwGuildID;
  int16_t sAlignment;
  uint8_t bPKMode;
  uint32_t dwMountVnum;
} TPacketGCCharacterUpdate2;

typedef struct packet_del_char {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
} TPacketGCCharacterDelete;

typedef struct packet_GlobalTime {
  uint16_t header;
  uint16_t length;
  float GlobalTime;
} TPacketGCGlobalTime;

enum EChatType {
  CHAT_TYPE_TALKING, /* 그냥 채팅 */
  CHAT_TYPE_INFO,    /* 정보 (아이템을 집었다, 경험치를 얻었다. 등) */
  CHAT_TYPE_NOTICE,  /* 공지사항 */
  CHAT_TYPE_PARTY,   /* 파티말 */
  CHAT_TYPE_GUILD,   /* 길드말 */
  CHAT_TYPE_COMMAND, /* 명령 */
  CHAT_TYPE_SHOUT,   /* 외치기 */
  CHAT_TYPE_WHISPER, // 서버와는 연동되지 않는 Only Client Enum
  CHAT_TYPE_BIG_NOTICE,
  CHAT_TYPE_MAX_NUM,
};

typedef struct packet_chatting {
  uint16_t header;
  uint16_t length;
  uint8_t type;
  uint32_t dwVID;
  uint8_t bEmpire;
} TPacketGCChat;

typedef struct packet_whisper // 가변 패킷
{
  uint16_t header;
  uint16_t length;
  uint8_t bType;
  char szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCWhisper;

typedef struct packet_stun {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketGCStun;

typedef struct packet_dead {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketGCDead;

typedef struct packet_main_character {
  enum { MUSIC_NAME_MAX_LEN = 24 };

  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  uint16_t wRaceNum;
  char szName[CHARACTER_NAME_MAX_LEN + 1];
  char szBGMName[MUSIC_NAME_MAX_LEN + 1];
  float fBGMVol;
  int32_t lX, lY, lZ;
  uint8_t byEmpire;
  uint8_t bySkillGroup;
} TPacketGCMainCharacter;

enum EPointTypes {
  POINT_NONE,        // 0
  POINT_LEVEL,       // 1
  POINT_VOICE,       // 2
  POINT_EXP,         // 3
  POINT_NEXT_EXP,    // 4
  POINT_HP,          // 5
  POINT_MAX_HP,      // 6
  POINT_SP,          // 7
  POINT_MAX_SP,      // 8
  POINT_STAMINA,     // 9  스테미너
  POINT_MAX_STAMINA, // 10 최대 스테미너

  POINT_GOLD,            // 11
  POINT_ST,              // 12 근력
  POINT_HT,              // 13 체력
  POINT_DX,              // 14 민첩성
  POINT_IQ,              // 15 정신력
  POINT_ATT_POWER,       // 16 공격력
  POINT_ATT_SPEED,       // 17 공격속도
  POINT_EVADE_RATE,      // 18 회피율
  POINT_MOV_SPEED,       // 19 이동속도
  POINT_DEF_GRADE,       // 20 방어등급
  POINT_CASTING_SPEED,   // 21 주문속도 (쿨다운타임*100) / (100 + 이값) = 최종
                         // 쿨다운 타임
  POINT_MAGIC_ATT_GRADE, // 22 마법공격력
  POINT_MAGIC_DEF_GRADE, // 23 마법방어력
  POINT_EMPIRE_POINT,    // 24 제국점수
  POINT_LEVEL_STEP, // 25 한 레벨에서의 단계.. (1 2 3 될 때 보상, 4 되면 레벨
                    // 업)
  POINT_STAT,       // 26 능력치 올릴 수 있는 개수
  POINT_SUB_SKILL,  // 27 보조 스킬 포인트
  POINT_SKILL,      // 28 액티브 스킬 포인트
  //    POINT_SKILL_PASV,           // 27 패시브 기술 올릴 수 있는 개수
  //    POINT_SKILL_ACTIVE,         // 28 액티브 스킬 포인트
  POINT_MIN_ATK,  // 29 최소 파괴력
  POINT_MAX_ATK,  // 30 최대 파괴력
  POINT_PLAYTIME, // 31 플레이시간
  POINT_HP_REGEN, // 32 HP 회복률
  POINT_SP_REGEN, // 33 SP 회복률

  POINT_BOW_DISTANCE, // 34 활 사정거리 증가치 (meter)

  POINT_HP_RECOVERY, // 35 체력 회복 증가량
  POINT_SP_RECOVERY, // 36 정신력 회복 증가량

  POINT_POISON_PCT,    // 37 독 확률
  POINT_STUN_PCT,      // 38 기절 확률
  POINT_SLOW_PCT,      // 39 슬로우 확률
  POINT_CRITICAL_PCT,  // 40 크리티컬 확률
  POINT_PENETRATE_PCT, // 41 관통타격 확률
  POINT_CURSE_PCT,     // 42 저주 확률

  POINT_ATTBONUS_HUMAN,   // 43 인간에게 강함
  POINT_ATTBONUS_ANIMAL,  // 44 동물에게 데미지 % 증가
  POINT_ATTBONUS_ORC,     // 45 웅귀에게 데미지 % 증가
  POINT_ATTBONUS_MILGYO,  // 46 밀교에게 데미지 % 증가
  POINT_ATTBONUS_UNDEAD,  // 47 시체에게 데미지 % 증가
  POINT_ATTBONUS_DEVIL,   // 48 마귀(악마)에게 데미지 % 증가
  POINT_ATTBONUS_INSECT,  // 49 벌레족
  POINT_ATTBONUS_FIRE,    // 50 화염족
  POINT_ATTBONUS_ICE,     // 51 빙설족
  POINT_ATTBONUS_DESERT,  // 52 사막족
  POINT_ATTBONUS_UNUSED0, // 53 UNUSED0
  POINT_ATTBONUS_UNUSED1, // 54 UNUSED1
  POINT_ATTBONUS_UNUSED2, // 55 UNUSED2
  POINT_ATTBONUS_UNUSED3, // 56 UNUSED3
  POINT_ATTBONUS_UNUSED4, // 57 UNUSED4
  POINT_ATTBONUS_UNUSED5, // 58 UNUSED5
  POINT_ATTBONUS_UNUSED6, // 59 UNUSED6
  POINT_ATTBONUS_UNUSED7, // 60 UNUSED7
  POINT_ATTBONUS_UNUSED8, // 61 UNUSED8
  POINT_ATTBONUS_UNUSED9, // 62 UNUSED9

  POINT_STEAL_HP, // 63 생명력 흡수
  POINT_STEAL_SP, // 64 정신력 흡수

  POINT_MANA_BURN_PCT, // 65 마나 번

  /// 피해시 보너스 ///

  POINT_DAMAGE_SP_RECOVER, // 66 공격당할 시 정신력 회복 확률

  POINT_BLOCK, // 67 블럭율
  POINT_DODGE, // 68 회피율

  POINT_RESIST_SWORD,   // 69
  POINT_RESIST_TWOHAND, // 70
  POINT_RESIST_DAGGER,  // 71
  POINT_RESIST_BELL,    // 72
  POINT_RESIST_FAN,     // 73
  POINT_RESIST_BOW,     // 74  화살   저항   : 대미지 감소
  POINT_RESIST_FIRE,    // 75  화염   저항   : 화염공격에 대한 대미지 감소
  POINT_RESIST_ELEC,    // 76  전기   저항   : 전기공격에 대한 대미지 감소
  POINT_RESIST_MAGIC,   // 77  술법   저항   : 모든술법에 대한 대미지 감소
  POINT_RESIST_WIND,    // 78  바람   저항   : 바람공격에 대한 대미지 감소

  POINT_REFLECT_MELEE, // 79 공격 반사

  /// 특수 피해시 ///
  POINT_REFLECT_CURSE, // 80 저주 반사
  POINT_POISON_REDUCE, // 81 독데미지 감소

  /// 적 소멸시 ///
  POINT_KILL_SP_RECOVER,   // 82 적 소멸시 MP 회복
  POINT_EXP_DOUBLE_BONUS,  // 83
  POINT_GOLD_DOUBLE_BONUS, // 84
  POINT_ITEM_DROP_BONUS,   // 85

  /// 회복 관련 ///
  POINT_POTION_BONUS,    // 86
  POINT_KILL_HP_RECOVER, // 87

  POINT_IMMUNE_STUN, // 88
  POINT_IMMUNE_SLOW, // 89
  POINT_IMMUNE_FALL, // 90
  //////////////////

  POINT_PARTY_ATT_GRADE, // 91
  POINT_PARTY_DEF_GRADE, // 92

  POINT_ATT_BONUS, // 93
  POINT_DEF_BONUS, // 94

  POINT_ATT_GRADE_BONUS,       // 95
  POINT_DEF_GRADE_BONUS,       // 96
  POINT_MAGIC_ATT_GRADE_BONUS, // 97
  POINT_MAGIC_DEF_GRADE_BONUS, // 98

  POINT_RESIST_NORMAL_DAMAGE, // 99

  // MR-10: Added missing POINT_* values
  POINT_HIT_HP_RECOVERY, // 100
  POINT_HIT_SP_RECOVERY, // 101
  POINT_MANASHIELD,      // 102 흑신수호 스킬에 의한 마나쉴드 효과 정도

  POINT_PARTY_BUFFER_BONUS,       // 103
  POINT_PARTY_SKILL_MASTER_BONUS, // 104

  POINT_HP_RECOVER_CONTINUE, // 105
  POINT_SP_RECOVER_CONTINUE, // 106

  POINT_STEAL_GOLD, // 107
  POINT_POLYMORPH,  // 108 변신한 몬스터 번호
  POINT_MOUNT,      // 109 타고있는 몬스터 번호

  POINT_PARTY_HASTE_BONUS,    // 110
  POINT_PARTY_DEFENDER_BONUS, // 111
  // MR-10: -- END OF -- Added missing POINT_* values

  POINT_STAT_RESET_COUNT = 112,
  POINT_HORSE_SKILL = 113,

  POINT_MALL_ATTBONUS,  // 114 공격력 +x%
  POINT_MALL_DEFBONUS,  // 115 방어력 +x%
  POINT_MALL_EXPBONUS,  // 116 경험치 +x%
  POINT_MALL_ITEMBONUS, // 117 아이템 드롭율 x/10배
  POINT_MALL_GOLDBONUS, // 118 돈 드롭율 x/10배
  POINT_MAX_HP_PCT,     // 119 최대생명력 +x%
  POINT_MAX_SP_PCT,     // 120 최대정신력 +x%

  POINT_SKILL_DAMAGE_BONUS,      // 121 스킬 데미지 *(100+x)%
  POINT_NORMAL_HIT_DAMAGE_BONUS, // 122 평타 데미지 *(100+x)%

  POINT_SKILL_DEFEND_BONUS,      // 123 스킬 방어 데미지
  POINT_NORMAL_HIT_DEFEND_BONUS, // 124 평타 방어 데미지
  POINT_PC_BANG_EXP_BONUS,       // 125
  POINT_PC_BANG_DROP_BONUS,      // 126 PC방 전용 드롭률 보너스
  // MR-10: Added missing POINT_* values
  POINT_RAMADAN_CANDY_BONUS_EXP, // 라마단 사탕 경험치 증가용
  // MR-10: -- END OF -- Added missing POINT_* values

  POINT_ENERGY = 128, // 128 기력

  // 기력 ui 용.
  // 이렇게 하고 싶지 않았지만,
  // uiTaskBar에서는 affect에 접근할 수 없고,
  // 더구나 클라리언트에서는 blend_affect는 관리하지 않아,
  // 임시로 이렇게 둔다.
  POINT_ENERGY_END_TIME = 129, // 129 기력 종료 시간

  // MR-10: Added missing POINT_* values
  POINT_COSTUME_ATTR_BONUS = 130,
  POINT_MAGIC_ATT_BONUS_PER = 131,
  POINT_MELEE_MAGIC_ATT_BONUS_PER = 132,

  // 추가 속성 저항
  POINT_RESIST_ICE = 133,   //   냉기 저항   : 얼음공격에 대한 대미지 감소
  POINT_RESIST_EARTH = 134, //   대지 저항   : 얼음공격에 대한 대미지 감소
  POINT_RESIST_DARK = 135,  //   어둠 저항   : 얼음공격에 대한 대미지 감소

  POINT_RESIST_CRITICAL = 136, // 크리티컬 저항	: 상대의 크리티컬 확률을 감소
  POINT_RESIST_PENETRATE =
      137, // 관통타격 저항	: 상대의 관통타격 확률을 감소
  // MR-10: -- END OF -- Added missing POINT_* values

  // 클라이언트 포인트
  POINT_MIN_WEP = 200,
  POINT_MAX_WEP,
  POINT_MIN_MAGIC_WEP,
  POINT_MAX_MAGIC_WEP,
  POINT_HIT_RATE,

  // POINT_MAX_NUM = 255,=>stdafx.h 로/
};

typedef struct packet_points {
  uint16_t header;
  uint16_t length;
  int32_t points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_point_change {
  uint16_t header;
  uint16_t length;

  uint32_t dwVID;
  uint8_t Type;

  int32_t amount; // 바뀐 값
  int32_t value;  // 현재 값
} TPacketGCPointChange;

typedef struct packet_motion {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint32_t victim_vid;
  uint16_t motion;
} TPacketGCMotion;

typedef struct packet_del_item {
  uint16_t header;
  uint16_t length;
  TItemPos pos;
} TPacketGCItemDel;

typedef struct packet_set_item {
  uint16_t header;
  uint16_t length;
  TItemPos pos;
  uint32_t vnum;
  uint8_t count;
  uint32_t flags;      // 플래그 추가
  uint32_t anti_flags; // 플래그 추가
  uint8_t highlight;
  int32_t alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
  TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TPacketGCItemSet;

typedef struct packet_item_get {
  uint16_t header;
  uint16_t length;
  uint32_t dwItemVnum;
  uint8_t bCount;
  uint8_t bArg; // 0: normal, 1: from party member
  char szFromName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemGet;

typedef struct packet_use_item {
  uint16_t header;
  uint16_t length;
  TItemPos Cell;
  uint32_t ch_vid;
  uint32_t victim_vid;

  uint32_t vnum;
} TPacketGCItemUse;

typedef struct packet_update_item {
  uint16_t header;
  uint16_t length;
  TItemPos Cell;
  uint8_t count;
  int32_t alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
  TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TPacketGCItemUpdate;

typedef struct packet_ground_add_item {
  uint16_t header;
  uint16_t length;
  int32_t lX;
  int32_t lY;
  int32_t lZ;

  uint32_t dwVID;
  uint32_t dwVnum;
} TPacketGCItemGroundAdd;

typedef struct packet_ground_del_item {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
} TPacketGCItemGroundDel;

typedef struct packet_item_ownership {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  char szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_quickslot_add {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
  TQuickSlot slot;
} TPacketGCQuickSlotAdd;

typedef struct packet_quickslot_del {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
} TPacketGCQuickSlotDel;

typedef struct packet_quickslot_swap {
  uint16_t header;
  uint16_t length;
  uint8_t pos;
  uint8_t change_pos;
} TPacketGCQuickSlotSwap;

typedef struct packet_shop_start {
  struct packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCShopStart;

typedef struct packet_shop_start_ex // 다음에 TSubPacketShopTab* shop_tabs 이
                                    // 따라옴.
{
  typedef struct sub_packet_shop_tab {
    char name[SHOP_TAB_NAME_MAX];
    uint8_t coin_type;
    packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
  } TSubPacketShopTab;
  uint32_t owner_vid;
  uint8_t shop_tab_count;
} TPacketGCShopStartEx;

typedef struct packet_shop_update_item {
  uint8_t pos;
  struct packet_shop_item item;
} TPacketGCShopUpdateItem;

typedef struct packet_shop_update_price {
  int32_t iElkAmount;
} TPacketGCShopUpdatePrice;

typedef struct packet_shop {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
} TPacketGCShop;

typedef struct packet_exchange {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
  uint8_t is_me;
  uint32_t arg1;
  TItemPos arg2;
  uint32_t arg3;
  int32_t alValues[ITEM_SOCKET_SLOT_MAX_NUM];
  TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TPacketGCExchange;

typedef struct packet_position {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint8_t position;
} TPacketGCPosition;

// TPacketGCPing, TPacketCGPong moved to EterLib/ControlPackets.h

typedef struct packet_script {
  uint16_t header;
  uint16_t length;
  uint8_t skin;
  uint16_t src_size;
} TPacketGCScript;

typedef struct packet_target {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  uint8_t bHPPercent;
} TPacketGCTarget;

typedef struct packet_damage_info {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  uint8_t flag;
  int32_t damage;
} TPacketGCDamageInfo;

typedef struct packet_mount {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint32_t mount_vid;
  uint8_t pos;
  uint32_t _x, _y;
} TPacketGCMount;

typedef struct packet_change_speed {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint16_t moving_speed;
} TPacketGCChangeSpeed;

typedef struct packet_move {
  uint16_t header;
  uint16_t length;
  uint8_t bFunc;
  uint8_t bArg;
  uint8_t bRot;
  uint32_t dwVID;
  int32_t lX;
  int32_t lY;
  uint32_t dwTime;
  uint32_t dwDuration;
} TPacketGCMove;

enum {
  QUEST_SEND_IS_BEGIN = 1 << 0,
  QUEST_SEND_TITLE = 1 << 1,      // 28자 까지
  QUEST_SEND_CLOCK_NAME = 1 << 2, // 16자 까지
  QUEST_SEND_CLOCK_VALUE = 1 << 3,
  QUEST_SEND_COUNTER_NAME = 1 << 4, // 16자 까지
  QUEST_SEND_COUNTER_VALUE = 1 << 5,
  QUEST_SEND_ICON_FILE = 1 << 6, // 24자 까지
};

typedef struct packet_quest_info {
  uint16_t header;
  uint16_t length;
  uint16_t index;
  uint8_t flag;
} TPacketGCQuestInfo;

typedef struct packet_quest_confirm {
  uint16_t header;
  uint16_t length;
  char msg[64 + 1];
  int32_t timeout;
  uint32_t requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_attack {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  uint32_t dwVictimVID; // 적 VID
  uint8_t bType;        // 공격 유형
} TPacketGCAttack;

typedef struct packet_c2c {
  uint16_t header;
  uint16_t length;
} TPacketGCC2C;

typedef struct packetd_sync_position_element {
  uint32_t dwVID;
  int32_t lX;
  int32_t lY;
} TPacketGCSyncPositionElement;

typedef struct packetd_sync_position {
  uint16_t header;
  uint16_t length;
} TPacketGCSyncPosition;

typedef struct packet_ownership {
  uint16_t header;
  uint16_t length;
  uint32_t dwOwnerVID;
  uint32_t dwVictimVID;
} TPacketGCOwnership;

#define SKILL_MAX_NUM 255

typedef struct packet_skill_level {
  uint16_t header;
  uint16_t length;
  uint8_t abSkillLevels[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct SPlayerSkill {
  uint8_t bMasterType;
  uint8_t bLevel;
  time_t tNextRead;
} TPlayerSkill;

typedef struct packet_skill_level_new {
  uint16_t header;
  uint16_t length;
  TPlayerSkill skills[SKILL_MAX_NUM];
} TPacketGCSkillLevelNew;

// fly
typedef struct packet_fly {
  uint16_t header;
  uint16_t length;
  uint8_t bType;
  uint32_t dwStartVID;
  uint32_t dwEndVID;
} TPacketGCCreateFly;

enum EPVPModes {
  PVP_MODE_NONE,
  PVP_MODE_AGREE,
  PVP_MODE_FIGHT,
  PVP_MODE_REVENGE,
};

typedef struct packet_duel_start {
  uint16_t header;
  uint16_t length;
} TPacketGCDuelStart;

typedef struct packet_pvp {
  uint16_t header;
  uint16_t length;
  uint32_t dwVIDSrc;
  uint32_t dwVIDDst;
  uint8_t bMode;
} TPacketGCPVP;

typedef struct packet_skill_cooltime_end {
  uint16_t header;
  uint16_t length;
  uint8_t bSkill;
} TPacketGCSkillCoolTimeEnd;

typedef struct packet_warp {
  uint16_t header;
  uint16_t length;
  int32_t lX;
  int32_t lY;
  int32_t lAddr;
  uint16_t wPort;
} TPacketGCWarp;

typedef struct packet_party_invite {
  uint16_t header;
  uint16_t length;
  uint32_t leader_pid;
} TPacketGCPartyInvite;

typedef struct packet_party_add {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
  char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCPartyAdd;

typedef struct packet_party_update {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
  uint8_t state;
  uint8_t percent_hp;
  int16_t affects[PARTY_AFFECT_SLOT_MAX_NUM];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
} TPacketGCPartyRemove;

typedef TPacketCGSafeboxCheckout TPacketGCSafeboxCheckout;
typedef TPacketCGSafeboxCheckin TPacketGCSafeboxCheckin;

typedef struct packet_safebox_wrong_password {
  uint16_t header;
  uint16_t length;
} TPacketGCSafeboxWrongPassword;

typedef struct packet_safebox_size {
  uint16_t header;
  uint16_t length;
  uint8_t bSize;
} TPacketGCSafeboxSize;

typedef struct packet_safebox_money_change {
  uint16_t header;
  uint16_t length;
  int32_t lMoney; // Signed to match server (uses int32_t lMoney)
} TPacketGCSafeboxMoneyChange;

typedef struct command_empire {
  uint16_t header;
  uint16_t length;
  uint8_t bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire {
  uint16_t header;
  uint16_t length;
  uint8_t bEmpire;
} TPacketGCEmpire;

typedef struct command_fishing {
  uint16_t header;
  uint16_t length;
  uint8_t dir;
} TPacketCGFishing;

typedef struct packet_fishing {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
  uint32_t info;
  uint8_t dir;
} TPacketGCFishing;

typedef struct paryt_parameter {
  uint16_t header;
  uint16_t length;
  uint8_t bDistributeMode;
} TPacketGCPartyParameter;

//////////////////////////////////////////////////////////////////////////
// Guild

typedef struct packet_guild {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
} TPacketGCGuild;

// SubHeader - Grade
enum {
  GUILD_AUTH_ADD_MEMBER = (1 << 0),
  GUILD_AUTH_REMOVE_MEMBER = (1 << 1),
  GUILD_AUTH_NOTICE = (1 << 2),
  GUILD_AUTH_SKILL = (1 << 3),
};

typedef struct packet_guild_sub_grade {
  char grade_name[GUILD_GRADE_NAME_MAX_LEN + 1]; // 8+1 길드장, 길드원 등의 이름
  uint8_t auth_flag;
} TPacketGCGuildSubGrade;

typedef struct packet_guild_sub_member {
  uint32_t pid;
  uint8_t byGrade;
  uint8_t byIsGeneral;
  uint8_t byJob;
  uint8_t byLevel;
  uint32_t dwOffer;
  uint8_t byNameFlag;
  // if NameFlag is TRUE, name is sent from server.
  //	char szName[CHARACTER_ME_MAX_LEN+1];
} TPacketGCGuildSubMember;

typedef struct packet_guild_sub_info {
  uint16_t member_count;
  uint16_t max_member_count;
  uint32_t guild_id;
  uint32_t master_pid;
  uint32_t exp;
  uint8_t level;
  char name[GUILD_NAME_MAX_LEN + 1];
  uint32_t gold;
  uint8_t hasLand;
} TPacketGCGuildInfo;

enum EGuildWarState {
  GUILD_WAR_NONE,
  GUILD_WAR_SEND_DECLARE,
  GUILD_WAR_REFUSE,
  GUILD_WAR_RECV_DECLARE,
  GUILD_WAR_WAIT_START,
  GUILD_WAR_CANCEL,
  GUILD_WAR_ON_WAR,
  GUILD_WAR_END,

  GUILD_WAR_DURATION = 2 * 60 * 60, // 2시간
};

typedef struct packet_guild_war {
  uint32_t dwGuildSelf;
  uint32_t dwGuildOpp;
  uint8_t bType;
  uint8_t bWarState;
} TPacketGCGuildWar;

typedef struct SPacketGuildWarPoint {
  uint32_t dwGainGuildID;
  uint32_t dwOpponentGuildID;
  int32_t lPoint;
} TPacketGuildWarPoint;

typedef struct packet_dungeon {
  uint16_t header;
  uint16_t length;
  uint8_t subheader;
} TPacketGCDungeon;

// Private Shop
typedef struct SPacketGCShopSign {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  char szSign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopSign;

typedef struct SPacketGCTime {
  uint16_t header;
  uint16_t length;
  time_t time;
} TPacketGCTime;

enum {
  WALKMODE_RUN,
  WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint8_t mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup {
  uint16_t header;
  uint16_t length;
  uint8_t skill_group;
} TPacketGCChangeSkillGroup;

struct TMaterial {
  uint32_t vnum;
  int32_t count;
};

typedef struct SRefineTable {
  uint32_t src_vnum;
  uint32_t result_vnum;
  uint8_t material_count;
  int32_t cost; // 소요 비용
  int32_t prob; // 확률
  TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGCRefineInformation {
  uint16_t header;
  uint16_t length;
  uint8_t type;
  uint8_t pos;
  TRefineTable refine_table;
} TPacketGCRefineInformation;

typedef struct SPacketGCRefineInformationNew {
  uint16_t header;
  uint16_t length;
  uint8_t type;
  uint8_t pos;
  TRefineTable refine_table;
} TPacketGCRefineInformationNew;

enum SPECIAL_EFFECT {
  SE_NONE,
  SE_HPUP_RED,
  SE_SPUP_BLUE,
  SE_SPEEDUP_GREEN,
  SE_DXUP_PURPLE,
  SE_CRITICAL,
  SE_PENETRATE,
  SE_BLOCK,
  SE_DODGE,
  SE_CHINA_FIREWORK,
  SE_SPIN_TOP,
  SE_SUCCESS,
  SE_FAIL,
  SE_FR_SUCCESS,
  SE_LEVELUP_ON_14_FOR_GERMANY,    // 레벨업 14일때 ( 독일전용 )
  SE_LEVELUP_UNDER_15_FOR_GERMANY, // 레벨업 15일때 ( 독일전용 )
  SE_PERCENT_DAMAGE1,
  SE_PERCENT_DAMAGE2,
  SE_PERCENT_DAMAGE3,
  SE_AUTO_HPUP,
  SE_AUTO_SPUP,
  SE_EQUIP_RAMADAN_RING,    // 초승달의 반지를 착용하는 순간에 발동하는 이펙트
  SE_EQUIP_HALLOWEEN_CANDY, // 할로윈 사탕을 착용(-_-;)한 순간에 발동하는 이펙트
  SE_EQUIP_HAPPINESS_RING,  // 크리스마스 행복의 반지를 착용하는 순간에 발동하는
                            // 이펙트
  SE_EQUIP_LOVE_PENDANT,    // 발렌타인 사랑의 팬던트(71145) 착용할 때 이펙트
                            // (발동이펙트임, 지속이펙트 아님),
  SE_AGGREGATE_MONSTER,
};

typedef struct SPacketGCSpecialEffect {
  uint16_t header;
  uint16_t length;
  uint8_t type;
  uint32_t vid;
} TPacketGCSpecialEffect;

typedef struct SPacketGCNPCPosition {
  uint16_t header;
  uint16_t length;
  uint16_t count;
} TPacketGCNPCPosition;

struct TNPCPosition {
  uint8_t bType;
  uint32_t dwVnum;
  char name[CHARACTER_NAME_MAX_LEN + 1];
  int32_t x;
  int32_t y;
};

typedef struct SPacketGCChangeName {
  uint16_t header;
  uint16_t length;
  uint32_t pid;
  char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCChangeName;

enum EBlockAction {
  BLOCK_EXCHANGE = (1 << 0),
  BLOCK_PARTY_INVITE = (1 << 1),
  BLOCK_GUILD_INVITE = (1 << 2),
  BLOCK_WHISPER = (1 << 3),
  BLOCK_MESSENGER_INVITE = (1 << 4),
  BLOCK_PARTY_REQUEST = (1 << 5),
};

typedef struct packet_login_key {
  uint16_t header;
  uint16_t length;
  uint32_t dwLoginKey;
} TPacketGCLoginKey;

typedef struct packet_auth_success {
  uint16_t header;
  uint16_t length;
  uint32_t dwLoginKey;
  uint8_t bResult;
} TPacketGCAuthSuccess;

typedef struct packet_channel {
  uint16_t header;
  uint16_t length;
  uint8_t channel;
} TPacketGCChannel;

typedef struct SEquipmentItemSet {
  uint32_t vnum;
  uint8_t count;
  int32_t alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
  TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TEquipmentItemSet;

typedef struct pakcet_view_equip {
  uint16_t header;
  uint16_t length;
  uint32_t dwVID;
  TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketGCViewEquip;

typedef struct {
  uint32_t dwID;
  int32_t x, y;
  int32_t width, height;
  uint32_t dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list {
  uint16_t header;
  uint16_t length;
} TPacketGCLandList;

typedef struct {
  uint16_t header;
  uint16_t length;
  int32_t lID;
  char szTargetName[32 + 1];
} TPacketGCTargetCreate;

enum {
  CREATE_TARGET_TYPE_NONE,
  CREATE_TARGET_TYPE_LOCATION,
  CREATE_TARGET_TYPE_CHARACTER,
};

typedef struct {
  uint16_t header;
  uint16_t length;
  int32_t lID;
  char szTargetName[32 + 1];
  uint32_t dwVID;
  uint8_t byType;
} TPacketGCTargetCreateNew;

typedef struct {
  uint16_t header;
  uint16_t length;
  int32_t lID;
  int32_t lX, lY;
} TPacketGCTargetUpdate;

typedef struct {
  uint16_t header;
  uint16_t length;
  int32_t lID;
} TPacketGCTargetDelete;

typedef struct {
  uint32_t dwType;
  uint8_t bPointIdxApplyOn;
  int32_t lApplyValue;
  uint32_t dwFlag;
  int32_t lDuration;
  int32_t lSPCost;
} TPacketAffectElement;

typedef struct {
  uint16_t header;
  uint16_t length;
  TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct {
  uint16_t header;
  uint16_t length;
  uint32_t dwType;
  uint8_t bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_mall_open {
  uint16_t header;
  uint16_t length;
  uint8_t bSize;
} TPacketGCMallOpen;

typedef struct packet_lover_info {
  uint16_t header;
  uint16_t length;
  char szName[CHARACTER_NAME_MAX_LEN + 1];
  uint8_t byLovePoint;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update {
  uint16_t header;
  uint16_t length;
  uint8_t byLovePoint;
} TPacketGCLovePointUpdate;

typedef struct packet_dig_motion {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  uint32_t target_vid;
  uint8_t count;
} TPacketGCDigMotion;

typedef struct SPacketGCOnTime {
  uint16_t header;
  uint16_t length;
  int32_t ontime; // sec
} TPacketGCOnTime;

typedef struct SPacketGCResetOnTime {
  uint16_t header;
  uint16_t length;
} TPacketGCResetOnTime;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client To Client

typedef struct packet_state {
  uint16_t header;
  uint16_t length;
  uint8_t bFunc;
  uint8_t bArg;
  uint8_t bRot;
  uint32_t dwVID;
  uint32_t dwTime;
  TPixelPosition kPPos;
} TPacketCCState;

// AUTOBAN
typedef struct packet_autoban_quiz {
  uint16_t header;
  uint16_t length;
  uint8_t bDuration;
  uint8_t bCaptcha[64 * 32];
  char szQuiz[256];
} TPacketGCAutoBanQuiz;
// END_OF_AUTOBAN

// TPacketGCKeyChallenge, TPacketCGKeyResponse, TPacketGCKeyComplete
// moved to EterLib/ControlPackets.h

// Secure authentication packets (libsodium/XChaCha20-Poly1305)
#pragma pack(push, 1)

// Client -> Server: Secure login
struct TPacketCGLoginSecure {
  uint16_t header; // CG::LOGIN_SECURE
  uint16_t length;
  char name[ID_MAX_NUM + 1];
  char pwd[PASS_MAX_NUM + 1];
  uint8_t session_token[32]; // Session token from KeyComplete
};

#pragma pack(pop)

typedef struct SPacketGCSpecificEffect {
  uint16_t header;
  uint16_t length;
  uint32_t vid;
  char effect_file[128];
} TPacketGCSpecificEffect;

// 용혼석
enum EDragonSoulRefineWindowRefineType {
  DragonSoulRefineWindow_UPGRADE,
  DragonSoulRefineWindow_IMPROVEMENT,
  DragonSoulRefineWindow_REFINE,
};

typedef struct SPacketCGDragonSoulRefine {
  SPacketCGDragonSoulRefine()
      : header(CG::DRAGON_SOUL_REFINE),
        length(sizeof(SPacketCGDragonSoulRefine)) {}
  uint16_t header;
  uint16_t length;
  uint8_t bSubType;
  TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine {
  SPacketGCDragonSoulRefine()
      : header(GC::DRAGON_SOUL_REFINE),
        length(sizeof(SPacketGCDragonSoulRefine)) {}
  uint16_t header;
  uint16_t length;
  uint8_t bSubType;
  TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SChannelStatus {
  int16_t nPort;
  uint8_t bStatus;
} TChannelStatus;

#pragma pack(pop)
