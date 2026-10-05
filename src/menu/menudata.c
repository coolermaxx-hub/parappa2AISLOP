#include "menu/menudata.h"

#include "main/main.h"
#include "main/subt.h"

#include "menu/menufont.h"
#include "menu/p3mc.h"

#include "os/mtc.h"
#include "os/system.h"
#include "os/tim2.h"

#include "iop_mdl/tapctrl_rpc.h"

#include <libpad.h>

static int _BankChan1Req = 0;
static int _BankChan1Stat = 0;

MenuMcIcon menumciconL[] = {
    {
        .fileId = P3MC_LOG_ICON_FILE(1),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0xf6, 0x42, 0x00, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
            },
            .Ambient = {
                1.0f, 1.0f, 1.0f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_LOG_ICON_FILE(2),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x19, 0x79, 0xf4, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
            },
            .Ambient = {
                1.0f, 1.0f, 1.0f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_LOG_ICON_FILE(3),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0xf8, 0x46, 0xaf, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
            },
            .Ambient = {
                1.0f, 1.0f, 1.0f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_LOG_ICON_FILE(4),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0xf9, 0xee, 0x03, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
                { 0.0f, 0.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 0.0f, },
            },
            .Ambient = {
                1.0f, 1.0f, 1.0f, 0.0f,
            },
        },
    },
};

MenuMcIcon menumciconR[] = {
    {
        .fileId = P3MC_REPLAY_ICON_FILE(1),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(2),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(3),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(4),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(5),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(6),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(7),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
    {
        .fileId = P3MC_REPLAY_ICON_FILE(8),
        .iconSys = {
            .Head = {'P', 'S', '2', 'D'},
            .Reserv1 = 0,
            .OffsLF = 0x10,
            .Reserv2 = 0,
            .TransRate = 0x40,
            .BgColor = {
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x50, 0xeb, 0x14, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
                { 0x80, 0x80, 0x80, 0x00 },
            },
            .LightDir = {
                {  0.0f, -1.0f, -1.0f, 0.0f, },
                {  1.0f, -1.0f, -1.0f, 0.0f, },
                { -1.0f, -1.0f, -1.0f, 0.0f, },
            },
            .LightColor = {
                { 0.5f, 0.5f, 0.5f, 0.0f, },
                { 0.3f, 0.3f, 0.3f, 0.0f, },
                { 0.2f, 0.2f, 0.2f, 0.0f, },
            },
            .Ambient = {
                0.3f, 0.3f, 0.3f, 0.0f,
            },
        },
    },
};

static SNDTAP sndtap_menu[] = {
    { .prg = 0, .key = 12, .volume = 127 },
    { .prg = 0, .key = 13, .volume = 127 },
    { .prg = 0, .key = 14, .volume = 127 },
    { .prg = 0, .key = 15, .volume = 127 },
    { .prg = 0, .key = 16, .volume = 127 },
    { .prg = 0, .key = 17, .volume = 127 },
    { .prg = 0, .key = 18, .volume = 127 },
    { .prg = 0, .key = 19, .volume = 127 },
    { .prg = 0, .key = 20, .volume = 127 },
    { .prg = 0, .key = 21, .volume = 127 },
    { .prg = 0, .key = 22, .volume = 127 },
    { .prg = 0, .key = 23, .volume = 127 },

    { .prg = 1, .key = 12, .volume = 127 },
    { .prg = 1, .key = 13, .volume = 127 },
    { .prg = 1, .key = 14, .volume = 127 },
    { .prg = 1, .key = 15, .volume = 127 },
    { .prg = 1, .key = 16, .volume = 127 },
    { .prg = 1, .key = 17, .volume = 127 },
    { .prg = 1, .key = 18, .volume = 127 },
    { .prg = 1, .key = 19, .volume = 127 },
    { .prg = 1, .key = 20, .volume = 127 },
    { .prg = 1, .key = 21, .volume = 127 },
    { .prg = 1, .key = 22, .volume = 127 },

    { .prg = 2, .key = 12, .volume = 108 },
    { .prg = 2, .key = 13, .volume = 74  },
    { .prg = 2, .key = 14, .volume = 108 },
    { .prg = 2, .key = 15, .volume = 108 },
    { .prg = 2, .key = 16, .volume = 108 },
    { .prg = 2, .key = 17, .volume = 108 },
    { .prg = 2, .key = 18, .volume = 108 },
    { .prg = 2, .key = 19, .volume = 74  },

    { .prg = 0, .key = 0, .volume = 0 },
};

static SNDTAP sndtap_map[] = {
    { .prg = 2, .key = 20, .volume = 127 },
    { .prg = 2, .key = 21, .volume = 127 },
    { .prg = 2, .key = 22, .volume = 127 },

    { .prg = 0, .key = 0,  .volume = 0   },
};

static SNDTAP sndtap_chall[] = {
    { .prg = 1, .key = 23, .volume = 127 },
    { .prg = 1, .key = 24, .volume = 127 },
    { .prg = 1, .key = 25, .volume = 127 },
    { .prg = 1, .key = 26, .volume = 127 },
    { .prg = 1, .key = 27, .volume = 127 },
    { .prg = 1, .key = 28, .volume = 127 },
    { .prg = 1, .key = 29, .volume = 127 },
    { .prg = 1, .key = 30, .volume = 127 },
    { .prg = 1, .key = 31, .volume = 127 },
    { .prg = 1, .key = 32, .volume = 127 },
    { .prg = 1, .key = 33, .volume = 127 },
    { .prg = 1, .key = 34, .volume = 127 },
    { .prg = 1, .key = 35, .volume = 127 },
    { .prg = 1, .key = 36, .volume = 127 },
    { .prg = 1, .key = 37, .volume = 127 },
    { .prg = 1, .key = 38, .volume = 127 },
    { .prg = 1, .key = 39, .volume = 127 },
    { .prg = 1, .key = 40, .volume = 127 },
    { .prg = 1, .key = 41, .volume = 127 },
    { .prg = 1, .key = 42, .volume = 127 },
};

static VOICE_TBL VoiceSet[] = {
    { .bnkNo = 0, .pTap = &sndtap_menu[0]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[1]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[2]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[3]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[4]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[5]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[6]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[7]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[8]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[9]   },
    { .bnkNo = 0, .pTap = &sndtap_menu[10]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[11]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[12]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[13]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[14]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[15]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[16]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[17]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[18]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[19]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[20]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[21]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[22]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[30]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[24]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[23]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[25]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[26]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[27]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[28]  },
    { .bnkNo = 0, .pTap = &sndtap_menu[29]  },

    { .bnkNo = 0, .pTap = &sndtap_map[0]    },
    { .bnkNo = 0, .pTap = &sndtap_map[1]    },
    { .bnkNo = 0, .pTap = &sndtap_map[2]    },

    { .bnkNo = 0, .pTap = &sndtap_chall[0]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[1]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[2]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[3]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[4]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[5]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[6]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[7]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[8]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[9]  },
    { .bnkNo = 0, .pTap = &sndtap_chall[10] },
    { .bnkNo = 0, .pTap = &sndtap_chall[11] },
    { .bnkNo = 0, .pTap = &sndtap_chall[12] },
    { .bnkNo = 0, .pTap = &sndtap_chall[13] },
    { .bnkNo = 0, .pTap = &sndtap_chall[14] },
    { .bnkNo = 0, .pTap = &sndtap_chall[15] },
    { .bnkNo = 0, .pTap = &sndtap_chall[16] },
    { .bnkNo = 0, .pTap = &sndtap_chall[17] },
    { .bnkNo = 0, .pTap = &sndtap_chall[18] },
    { .bnkNo = 0, .pTap = &sndtap_chall[19] },
};

MESS mess_mc[] = {
    /* MCA_NO_CARD */
    {
        "No MEMORY CARD(PS2) inserted@in MEMORY CARD slot 1.@A MEMORY CARD(PS2)@with a minimum of 140KB@of memory available@is required to save system file.@@Proceed anyway?",
        "メモリーカード（ＰＳ２）が＠スロット１に差さっていません。＠このゲームのシステムファイルをセーブするには＠１４０ＫＢ以上の空き容量のある＠メモリーカード（ＰＳ２）が必要です。＠このまま開始する場合は＠○ボタンを押して下さい。＠またはメモリーカード（ＰＳ２）を＠スロット１に差し込んで下さい。",
    },
    /* MCA_NO_SPACE */
    {
        "Insufficient memory available@on MEMORY CARD(PS2).@It requires more than 140KB of memory@in  MEMORY CARD(PS2)@to save system file.@@ Proceed anyway?",
        "メモリーカード（ＰＳ２）の＠空き容量が不足しています。＠このゲームのシステムファイルをセーブするには＠１４０ＫＢ以上の空き容量が必要です。＠このまま開始する場合は＠○ボタンを押して下さい。",
    },
    /* MCA_NO_CARD2 */
    {
        "No MEMORY CARD(PS2)@inserted in MEMORY CARD slot 1.@Please insert MEMORY CARD(PS2)@in MEMORY CARD slot 1.@@× Exit",
        "メモリーカード（ＰＳ２）が＠差さってないぞ。スロット１に＠メモリーカード（ＰＳ２）を差しとくれ。＠＠× もどる",
    },
    /* MCA_CHECK */
    {
        "Checking MEMORY CARD slot 1.@Do not remove MEMORY CARD(PS2) .",
        "スロット１をチェック中じゃ。＠メモリーカード（ＰＳ２）を＠抜いてはだめじゃよ！",
    },
    /* MCA_CHECK_ERR */
    {
        "Check unsuccessful.@@× Exit",
        "チェック失敗じゃ。＠＠× もどる",
    },
    /* MCA_NO_DATA_S */
    {
        "No system file present@on this MEMORY CARD(PS2) .@@× Exit",
        "システムファイルがないぞ。＠＠× もどる",
    },
    /* MCA_NO_DATA_R */
    {
        "No replay file present@on this MEMORY CARD(PS2) .@@× Exit",
        "リプレイファイルがないぞ。＠＠× もどる",
    },
    /* MCA_LOAD */
    {
        "Loading file.@Do not remove MEMORY CARD(PS2)@from MEMORY CARD slot 1.",
        "ロード中じゃよ。＠メモリーカード（ＰＳ２）を＠スロット１から抜いてはだめじゃよ！",
    },
    /* MCA_LOAD_ERR */
    {
        "Load unsuccessful.@@× Exit",
        "ロード失敗じゃ。＠＠× もどる",
    },
    /* MCA_LOAD_ERR2 */
    {
        "Load unsuccessful. File is corrupt.@@× Exit",
        "ロード失敗じゃ。＠ファイルが壊れているかもしれないぞ。＠＠× もどる",
    },
    /* MCA_LOAD_COMP */
    {
        "Load completed.",
        "ロード完了じゃ。",
    },
    /* MCA_NO_CARD3 */
    {
        "No MEMORY CARD(PS2)@inserted in MEMORY CARD slot 1.@It requires more than 140KB of memory@in  MEMORY CARD(PS2)@to save system file.@Please insert MEMORY CARD(PS2)@in MEMORY CARD slot 1.@@× Exit",
        "メモリーカード（ＰＳ２）が差さってないぞ。＠システムファイルをセーブするには＠１４０ＫＢ以上の空き容量のある＠メモリーカード（ＰＳ２）が必要なんじゃ。＠スロット１にメモリーカード（ＰＳ２）を＠差しとくれ。＠＠× もどる",
    },
    /* MCA_NO_CARD4 */
    {
        "No MEMORY CARD(PS2)@inserted in MEMORY CARD slot 1.@It requires more than 160KB of memory@in  MEMORY CARD(PS2)@to save replay file.@Please insert MEMORY CARD(PS2)@in MEMORY CARD slot 1.@@× Exit",
        "メモリーカード（ＰＳ２）が差さってないぞ。＠リプレイファイルをセーブするには＠１６０ＫＢ以上の空き容量のある＠メモリーカード（ＰＳ２）が必要なんじゃ。＠スロット１にメモリーカード（ＰＳ２）を＠差しとくれ。＠＠× もどる",
    },
    /* MCA_SAVE_ERR */
    {
        "Insufficient memory@on MEMORY CARD(PS2) .@It requires more than 140KB of memory@in MEMORY CARD(PS2)@ to save system file.@@× Exit",
        "メモリーカード（ＰＳ２）の＠空き容量が足りないぞ。＠新しいシステムファイルをセーブするには＠１４０ＫＢ以上の空き容量のある＠メモリーカード（ＰＳ２）が必要なんじゃ。＠＠× もどる",
    },
    /* MCA_SAVE_ERR2 */
    {
        "Insufficient memory@on MEMORY CARD(PS2).@It requires more than 160KB of memory@in MEMORY CARD(PS2)@to save replay file.@@× Exit",
        "メモリーカード（ＰＳ２）の＠空き容量が足りないぞ。＠新しいリプレイファイルをセーブするには＠１６０ＫＢ以上の空き容量のある＠メモリーカード（ＰＳ２）が必要なんじゃ。＠＠× もどる",
    },
    /* MCA_REWRITE_CONF */
    {
        "Overwriting this area@will cause previous file@to be lost.@Overwrite previous file?@@○ YES   × NO",
        "そこには前にセーブしたファイルがあるぞ。＠そこにセーブすると＠古いファイルは消えてしまうぞ。＠よいかな？＠＠○ はい　　× もどる",
    },
    /* MCA_INITIAL_CONF */
    {
        "MEMORY CARD(PS2) unformatted.@Save after formatting@MEMORY CARD(PS2)?@@○ YES   × NO",
        "メモリーカード（ＰＳ２）が＠フォーマットされてないぞ。＠フォーマットしてからセーブするぞ。＠よいかな？＠＠○ はい　　× もどる",
    },
    /* MCA_INITIAL */
    {
        "Formatting MEMORY CARD(PS2).@Do not remove MEMORY CARD(PS2)@from MEMORY CARD slot 1.",
        "フォーマット中じゃよ。＠メモリーカード（ＰＳ２）を＠スロット１から抜いてはだめじゃよ！",
    },
    /* MCA_INITIAL_ERR */
    {
        "Format unsuccessful.@@× Exit",
        "フォーマット失敗じゃ。＠＠× もどる",
    },
    /* MCA_SAVE */
    {
        "Saving file.@Do not remove MEMORY CARD(PS2)@from MEMORY CARD slot 1.",
        "セーブ中じゃよ。＠メモリーカード（ＰＳ２）を＠スロット１から抜いてはだめじゃよ！",
    },
    /* MCA_SAVE_ERR3 */
    {
        "Save unsuccessful.@@× Exit",
        "セーブ失敗じゃ。＠＠× もどる",
    },
    /* MCA_SAVE_COMP */
    {
        "Save completed.",
        "セーブ完了じゃ。",
    },
    /* MCA_LOAD_CONF */
    {
        "Load file from MEMORY CARD(PS2)?@@○ YES   × NO",
        "ファイルをロードするぞ。よいかな？＠＠○ はい　　× もどる",
    },
    /* MCA_RANK_ERR */
    {
        "Something might be wrong with the file.",
        "読めないファイルがあるぞ。",
    },
    /* MCA_RANK_ERR2 */
    {
        "Check unsuccessful.",
        "チェック失敗じゃ。",
    },
    /* MCA_DUMY_NUM */
    {
        "",
        "０１２３４５６７８９ＫＢ",
    },
};

MESS mess_menu[] = {
    /* MENU_MAP1_HALL */
    {
        "City hall (Load & Save, Option)@＊Move  ○OK ×Cancel",
        "シティホール（ロード＆セーブ、オプション）＠＊移動　○決定　×もどる",
    },
    /* MENU_MAP1_ST1 */
    {
        "Stage 1@＊Move  ○OK ×Cancel",
        "ステージ１＠＊移動　○決定　×もどる"
    },
    /* MENU_HALL */
    {
        "City hall (Load & Save, Option)@＊Move  ○OK ×Cancel",
        "シティホール（ロード＆セーブ、オプション）＠＊移動　○決定　×もどる",
    },
    /* MENU_ST1 */
    {
        "Stage 1@＊Move  ○OK ×Cancel",
        "ステージ１＠＊移動　○決定　×もどる",
    },
    /* MENU_ST2 */
    {
        "Stage 2@＊Move  ○OK ×Cancel",
        "ステージ２＠＊移動　○決定　×もどる",
    },
    /* MENU_ST3 */
    {
        "Stage 3@＊Move  ○OK ×Cancel",
        "ステージ３＠＊移動　○決定　×もどる",
    },
    /* MENU_ST4 */
    {
        "Stage 4@＊Move  ○OK ×Cancel",
        "ステージ４＠＊移動　○決定　×もどる",
    },
    /* MENU_ST5 */
    {
        "Stage 5@＊Move  ○OK ×Cancel",
        "ステージ５＠＊移動　○決定　×もどる",
    },
    /* MENU_ST6 */
    {
        "Stage 6@＊Move  ○OK ×Cancel",
        "ステージ６＠＊移動　○決定　×もどる",
    },
    /* MENU_ST7 */
    {
        "Stage 7@＊Move  ○OK ×Cancel",
        "ステージ７＠＊移動　○決定　×もどる",
    },
    /* MENU_ST8 */
    {
        "Stage 8@＊Move  ○OK ×Cancel",
        "ステージ８＠＊移動　○決定　×もどる",
    },
    /* MENU_RECORD */
    {
        "Record shop@＊Move  ○OK ×Cancel",
        "レコードショップ＠＊移動　○決定　×もどる",
    },
    /* MENU_REC1 */
    {
        "Toasty Buns@＊Select  ○OK ×Cancel",
        "Ｔｏａｓｔｙ　Ｂｕｎｓ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC2 */
    {
        "Romantic Love@＊Select  ○OK ×Cancel",
        "Ｒｏｍａｎｔｉｃ　Ｌｏｖｅ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC3 */
    {
        "BIG@＊Select  ○OK ×Cancel",
        "ＢＩＧ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC4 */
    {
        "Sista Moosesha@＊Select  ○OK ×Cancel",
        "Ｓｉｓｔａ　Ｍｏｏｓｅｓｈａ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC5 */
    {
        "Hair Scare@＊Select  ○OK ×Cancel",
        "Ｈａｉｒ　Ｓｃａｒｅ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC6 */
    {
        "\"FOOD COURT\"@＊Select  ○OK ×Cancel",
        "”ＦＯＯＤ　ＣＯＵＲＴ”＠＊選択　○決定　×もどる",
    },
    /* MENU_REC7 */
    {
        "Noodles Can't Be Beat@＊Select  ○OK ×Cancel",
        "Ｎｏｏｄｌｅｓ　Ｃａｎ’ｔ　Ｂｅ　Ｂｅａｔ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC8 */
    {
        "Always Believe !@＊Select  ○OK ×Cancel",
        "Ａｌｗａｙｓ　Ｂｅｌｉｅｖｅ！＠＊選択　○決定　×もどる",
    },
    /* MENU_REC9 */
    {
        "Come A Long Way@＊Select  ○OK ×Cancel",
        "Ｃｏｍｅ　Ａ　Ｌｏｎｇ　Ｗａｙ＠＊選択　○決定　×もどる",
    },
    /* MENU_REC10 */
    {
        "Say \"I Gotta Believe !\"@＊Select  ○OK ×Cancel",
        "Ｓａｙ　”Ｉ　Ｇｏｔｔａ　Ｂｅｌｉｅｖｅ！”＠＊選択　○決定　×もどる",
    },
    /* MENU_POPUP_S */
    {
        "Single Play@＊Select  ○OK ×Cancel",
        "シングルプレイ＠＊選択　○決定　×ステージ選択にもどる",
    },
    /* MENU_POPUP_VCOMP */
    {
        "VS Computer@＊Select  ○OK ×Cancel",
        "コンピュータと対戦＠＊選択　○決定　×ステージ選択にもどる",
    },
    /* MENU_POPUP_LEVEL */
    {
        " ＝Select level  ○OK ×Cancel",
        "　＝レベルを選択　○決定　×もどる",
    },
    /* MENU_POPUP_VP2 */
    {
        "VS Player 2@＊Select  ○OK ×Cancel",
        "プレイヤー２と対戦＠＊選択　○決定　×ステージ選択にもどる",
    },
    /* MENU_POPUP_SRANK */
    {
        "Single Play Ranking@＊Select  ○OK ×Cancel",
        "シングルプレイランキング＠＊選択　○決定　×ステージ選択にもどる",
    },
    /* MENU_POPUP_VRANK */
    {
        "VS Computer Ranking@＊Select  ○OK ×Cancel",
        "対戦コンピュータランキング＠＊選択　○決定　×ステージ選択にもどる",
    },
    /* MENU_LIST */
    {
        "｜Scroll Ranking List   × Close List",
        "｜リストをスクロール　×リストを閉じる",
    },
    /* MENU_LIST_1PAGE */
    {
        "× Close List",
        "×リストを閉じる",
    },
    /* MENU_LIST_VS */
    {
        " ＝Change Sheet × Close List",
        "　＝シートを移動　×リストを閉じる",
    },
    /* MENU_HALL_INSIDE */
    {
        "May I Help You?",
        "ご用件は？",
    },
    /* MENU_LOGLOAD_CT */
    {
        "Load / Save system file@＊Select  ○OK ×Exit city hall",
        "システムファイルをロード／セーブ＠＊選択　○決定　×シティホールから出る",
    },
    /* MENU_REPLOAD_CT */
    {
        "Load replay file@＊Select  ○OK ×Exit city hall",
        "リプレイファイルをロード＠＊選択　○決定　×シティホールから出る",
    },
    /* MENU_OPT_CT */
    {
        "Option@＊Select  ○OK ×Exit city hall",
        "オプション＠＊選択　○決定　×シティホールから出る",
    },
    /* MENU_LOGLOAD_CAM */
    {
        "You want to load or save?",
        "システムファイルのロードかね？セーブかね？",
    },
    /* MENU_LOGLOAD */
    {
        "Select loading file.@｜Select ○OK ×Cancel  →Go to Save",
        "ロードするファイルを選ぶ＠｜選択　○決定　×もどる　→セーブへ",
    },
    /* MENU_REPLOAD_CAM */
    {
        "You want to load the replay?",
        "リプレイのロードじゃね。",
    },
    /* MENU_REPLOAD_CEL */
    {
        "Select loading file.@｜Select  ○OK ×Cancel",
        "ロードするファイルを選ぶ＠｜選択　○決定　×もどる",
    },
    /* MENU_OPT_CAM */
    {
        "You wanna go to options?",
        "オプションを変えるんじゃね。",
    },
    /* MENU_OPT_LANG */
    {
        "Which language?@＝Change ｜Select ○OK ×Cancel",
        "ことば＠＝選択　｜移動　○決定　×もどる",
    },
    /* MENU_OPT_SUBTITLE */
    {
        "Subtitle@＝Change ｜Select ○OK ×Cancel",
        "字幕＠＝選択　｜移動　○決定　×もどる",
    },
    /* MENU_OPT_VIB */
    {
        "Vibration@＝Change ｜Select ○OK ×Cancel",
        "バイブレーション＠＝選択　｜移動　○決定　×もどる",
    },
    /* MENU_OPT_BUTTON */
    {
        "Easy mode@＝Change ｜Select ○OK ×Cancel",
        "かんたんモード＠＝選択　｜移動　○決定　×もどる",
    },
    /* MENU_SAVE_STAGE */
    {
        "Save system file@＝Select ○OK ×Cancel",
        "システムファイルをセーブ＠＝選択　○決定　×セーブしない",
    },
    /* MENU_SAVE_REPLAY */
    {
        "Save replay file@＝Select ○OK ×Cancel",
        "リプレイファイルをセーブ＠＝選択　○決定　×セーブしない",
    },
    /* MENU_SAVE_CEL */
    {
        "Select save position.@｜Select ○OK ×Cancel",
        "セーブ場所を選ぶ＠｜選択　○決定　×もどる",
    },
    /* MENU_SAVE_NAME */
    {
        "Input your name. ×Cancel@｜Character Change  ＝Select Position",
        "名前入力。ＯＫで○を押すと決定＠｜文字スクロール　＝移動　×もどる",
    },
    /* MENU_DUMY_CHAR */
    {
        "",
        "←→↓↑",
    },
    /* MENU_SAVE_CEL2 */
    {
        "Select save position.@｜Select ○OK ×Cancel  ←Go to Load",
        "セーブ場所を選ぶ＠｜選択　○決定　×もどる　←ロードへ",
    },
};

static int rt2t_r1[] = {
    0x165, 0x166, 0x167, 0,
};

static int rt2t_r2[] = {
    0x168, 0x169, 0x16a, 0,
};

static int rt2t_r3[] = {
    0x16b, 0x16c, 0x16d, 0,
};

static int rt2t_r4[] = {
    0x16e, 0x16f, 0x170, 0,
};

static RT2TRANS_STR rt2trans_str[4] = {
    { .num = 3, .data_pp = rt2t_r1 },
    { .num = 3, .data_pp = rt2t_r2 },
    { .num = 3, .data_pp = rt2t_r3 },
    { .num = 3, .data_pp = rt2t_r4 },
};

static int ocl1_r1[] = {
    0xb9, 0xbd, 0xc1, 0xc5,
    0xc9, 0xcd, 0xd1, 0xd5,
};

static int ocl1_r2[] = {
    0xba, 0xbe, 0xc2, 0xc6,
    0xca, 0xce, 0xd2, 0xd6,
};

static int ocl1_r3[] = {
    0xbb, 0xbf, 0xc3, 0xc7,
    0xcb, 0xcf, 0xd3, 0xd7,
};

static int ocl1_r4[] = {
    0xbc, 0xc0, 0xc4, 0xc8,
    0xcc, 0xd0, 0xd4, 0xd8,
};

static int *ocl1_rndTbl[] = {
    ocl1_r1, ocl1_r2, ocl1_r3, ocl1_r4,
};

static MN_CRWCL1_TBL ocl1_st1 = {
    .fno = {
        { 0xd9, 0xda, 0xdb, 0xdc },
        { 0xdd, 0xde, 0xdf, 0xe0 },
        { 0xe1, 0xe2, 0xe3, 0xe4 },
        { 0xe5, 0xe6, 0xe7, 0xe8 },
    },
};

static MN_CRWCL1_TBL ocl1_st2 = {
    .fno = {
        { 0xe9, 0xea, 0xeb, 0xec },
        { 0xed, 0xee, 0xef, 0xf0 },
        { 0xf1, 0xf2, 0xf3, 0xf4 },
        { 0xf5, 0xf6, 0xf7, 0xf8 },
    },
};

static MN_CRWCL1_TBL ocl1_st3 = {
    .fno = {
        { 0xf9,  0xfa,  0xfb,  0xfc  },
        { 0xfd,  0xfe,  0xff,  0x100 },
        { 0x101, 0x102, 0x103, 0x104 },
        { 0x105, 0x106, 0x107, 0x108 },
    },
};

static MN_CRWCL1_TBL ocl1_st4 = {
    .fno = {
        { 0x109, 0x10a, 0x10b, 0x10c },
        { 0x10d, 0x10e, 0x10f, 0x110 },
        { 0x111, 0x112, 0x113, 0x114 },
        { 0x115, 0x116, 0x117, 0x118 },
    },
};

static MN_CRWCL1_TBL ocl1_st5 = {
    .fno = {
        { 0x119, 0x11a, 0x11b, 0x11c },
        { 0x11d, 0x11e, 0x11f, 0x120 },
        { 0x121, 0x122, 0x123, 0x124 },
        { 0x125, 0x126, 0x127, 0x128 },
    },
};

static MN_CRWCL1_TBL ocl1_st6 = {
    .fno = {
        { 0x129, 0x12a, 0x12b, 0x12c },
        { 0x12d, 0x12e, 0x12f, 0x130 },
        { 0x131, 0x132, 0x133, 0x134 },
        { 0x135, 0x136, 0x137, 0x138 },
    },
};

static MN_CRWCL1_TBL ocl1_st7 = {
    .fno = {
        { 0x139, 0x13a, 0x13b, 0x13c },
        { 0x13d, 0x13e, 0x13f, 0x140 },
        { 0x141, 0x142, 0x143, 0x144 },
        { 0x145, 0x146, 0x147, 0x148 },
    },
};

static MN_CRWCL1_TBL ocl1_st8 = {
    .fno = {
        { 0x149, 0x14a, 0x14b, 0x14c },
        { 0x14d, 0x14e, 0x14f, 0x150 },
        { 0x151, 0x152, 0x153, 0x154 },
        { 0x155, 0x156, 0x157, 0x158 },
    },
};

static MN_CRWCL1_TBL *ocl1_stageTbl[] = {
    &ocl1_st1, &ocl1_st2, &ocl1_st3, &ocl1_st4,
    &ocl1_st5, &ocl1_st6, &ocl1_st7, &ocl1_st8,
};

void* MenuDataGetIconSysHed(int mode, int stageNo, int *size) {
    void *ptr;

    stageNo -= 1;
    ptr = NULL;

    if (size != NULL) {
        *size = 0;
    }

    switch (mode) {
    case 0:
        if (stageNo >= 0 && stageNo <= 3) {
            ptr = &menumciconL[stageNo].iconSys;
            if (size != NULL) {
                *size = sizeof(MenuMcIconSys);
            }
        }
        break;
    case 1:
        if (stageNo >= 0 && stageNo <= 7) {
            ptr = &menumciconR[stageNo].iconSys;
            if (size != NULL) {
                *size = sizeof(MenuMcIconSys);
            }
        }
        break;
    }

    return ptr;
}

void MenuDataSndInit(void) {
    _BankChan1Req = 0;
    _BankChan1Stat = 0;

    TapCt(TAPCT_ALLOCSPU | 1, 0x135010, TAPCT_NONE);
    TapCt(TAPCT_SETMASTERVOL, SPU_VOLUME_LR(SPU_VOLUME_MAX, SPU_VOLUME_MAX), TAPCT_NONE);
}

void MenuDataSndReq(int chanId, int req) {
    SNDTAP *sndtap_pp = &sndtap_menu[req];

    TapCt(TAPCT_TAPVOLUME, chanId, sndtap_pp->volume);
    TapCt(TAPCT_TAPREQ,    chanId, sndtap_pp->prg + sndtap_pp->key * 256);
}

void MenuDataSndStop(int chanId) {
    TapCt(TAPCT_TAPSTOP, chanId, TAPCT_NONE);
}

void MenuDataSndQuit(void) {
    int i;

    for (i = 0; i < 3; i++) {
        TapCt(TAPCT_CHANCLOSE | i, TAPCT_NONE, TAPCT_NONE);
    }
}

void MenuDataSndSetVol(int chanId, int req, int vol0) {
    SNDTAP *sndtap_pp = &sndtap_menu[req];

    TapCt(TAPCT_TAPVOLUMECHANGE, chanId, sndtap_pp->volume * vol0 >> 8); /* Should be a division rather than a shift? */
}

MENU_SPU_ENUM MenuDataSndTrans(int bdId, int hdId, MENU_SPU_ENUM trId) {
    TapCt(TAPCT_BDSPUTRANS | trId, (int)GetIntAdrsCurrent(bdId), (int)GetIntSizeCurrent(bdId));
    TapCt(TAPCT_HDIOPTRANS | trId, (int)GetIntAdrsCurrent(hdId), (int)GetIntSizeCurrent(hdId)); 
    return trId;
}

int MenuDataSndTransCheck(void) {
    return TapCt(TAPCT_TRANSCHECK, TAPCT_NONE, TAPCT_NONE);
}

void MenuDataSndReqChan(int chanId, int req, MENU_SPU_ENUM trId) {
    SNDTAP *sndtap_pp = &sndtap_menu[req];

    TapCt(TAPCT_TAPVOLUME | trId, chanId, sndtap_pp->volume);
    TapCt(TAPCT_TAPREQ    | trId, chanId, sndtap_pp->prg + sndtap_pp->key * 256);
}

void MenuDataSpuVolume(int vol) {
    if ((vol * 128) <= SPU_VOLUME_MAX) {
        vol = vol << 7;
    } else {
        vol = SPU_VOLUME_MAX;
    }

    TapCt(TAPCT_SETMASTERVOL, SPU_VOLUME_LR(vol, vol), TAPCT_NONE);
}

void MenuDataDiskVolume(u_int vol) {
    CdctrlWP2SetVolume(vol);
}

void MenuDataDiskSndReq(MENU_DISKSND_ENUM sndId) {
    CdctrlWP2Set(&file_str_extra_file[sndId]);
}

int MenuDataDiskSndReady(void) {
    return CdctrlWP2CheckBuffer();
}

void MenuDataDiskSndPlay(void) {
    CdctrlWP2Play();
}

void MenuDataDiskSndEnd(void) {
    CdctrlWp2FileEnd();
}

void menuDiskSndDebug(void *x) {
    int sndId = MDISK_00;

    while (1) {
        do {
            MtcWait(1);
        } while (!(pad[1].one & SCE_PADRdown));

        MenuDataDiskSndReq(sndId);

        do {
            MtcWait(1);
        } while (MenuDataDiskSndReady());

        MenuDataDiskSndPlay();
        MenuDataDiskVolume(128);

        do {
            MtcWait(1);
        } while (!(pad[1].one & SCE_PADRdown));

        MenuDataDiskSndEnd();
        sndId++;

        if (sndId >= MDISK_MAX) {
            sndId = MDISK_00;
        }
    }
}

int MenuVoiceBankSet(int bnkNo) {
    _BankChan1Req = bnkNo;
    _BankChan1Stat = 0;
    return 0;
}

void MenuVoicePlayVol(int chanId, int vsetIdx, int vol0) {
    SNDTAP        *sndtap_pp;
    MENU_SPU_ENUM  trId;
    int            bnkNo;

    bnkNo = VoiceSet[vsetIdx].bnkNo;
    if (bnkNo != 0) {
        if (_BankChan1Req != bnkNo) {
            return;
        }
        trId = MENU_SPU_CHAN1;
        if (_BankChan1Stat != 0 && MenuVoiceBankSet(_BankChan1Req) != 0) {
            return;
        }
    } else {
        trId = MENU_SPU_CHAN;
    }

    sndtap_pp = VoiceSet[vsetIdx].pTap;
    TapCt(TAPCT_TAPVOLUME | trId, chanId, (sndtap_pp->volume * vol0) >> 8);
    TapCt(TAPCT_TAPREQ    | trId, chanId, sndtap_pp->prg + (sndtap_pp->key * 256));
}

void MenuVoicePlay(int chanId, int vsetIdx) {
    MenuVoicePlayVol(chanId, vsetIdx, 0x100);
}

void MenuVoiceStop(int chanId) {
    TapCt(TAPCT_TAPSTOP, chanId, TAPCT_NONE);
}

void MenuVoiceSetVol(int chanId, int vsetIdx, int vol0) {
    SNDTAP *sndtap_pp = VoiceSet[vsetIdx].pTap;
    TapCt(TAPCT_TAPVOLUMECHANGE, chanId, (sndtap_pp->volume * vol0) >> 8);
}

void MenuMsgInit(void) {
    SubtMenuCtrlInit(GetIntAdrsCurrent(INTNUM_SUBT_CODE));
    MENUSubtSetKanji(GetIntAdrsCurrent(INTNUM_SUBT_CODE));
}

void MenuMsgPrintSub(int id, int xp, int yp, int flg) {
    SubtMenuCtrlPrint(mess_menu[id][flg], xp, yp, flg);
}

void MenuMsgPrintMc(int id, int xp, int yp, int flg) {
    SubtMenuCtrlPrint(mess_mc[id][flg], xp, yp, flg);
}

char* MenuMsgGetMessageMc(int id, int flg) {
    return mess_mc[id][flg];
}

char* MenuMsgGetMessageSub(int id, int flg) {
    return mess_menu[id][flg];
}

int MenuRoundTim2Trans(TAP_ROUND_ENUM round) {
    RT2TRANS_STR *rt2trans_str_pp;
    int           i;

    if (round < TRND_R4) {
        hat_change_enum = HCNG_AUTO;
    }
    if (hat_change_enum != HCNG_AUTO) {
        round = (TAP_ROUND_ENUM)hat_change_enum;
    }
    if (round >= TRND_MAX) {
        return 1;
    }

    rt2trans_str_pp = &rt2trans_str[round];
    for (i = 0; i < rt2trans_str_pp->num; i++) {
        Tim2Trans(GetIntAdrsCurrent(rt2trans_str_pp->data_pp[i]));
    }

    return 0;
}

int MenuStageCl1Trans(int nStage, int nRound) {
    if (!(nStage >= 0 && nStage <= 7)) {
        return 1;
    }

    if (nRound >= 0 && nRound <= 3) {
        Tim2Trans(GetIntAdrsCurrent(ocl1_rndTbl[nRound][nStage]));
    }

    return 0;
}

int MenuCoolCl1Trans(int nStage, int nPos, int nRound) {
    if (!(nStage >= 0 && nStage <= 7)) {
        return 1;
    }
    if (!(nRound >= 0 && nRound <= 3)) {
        return 1;
    }
    if (!(nPos >= 0 && nPos <= 3)) {
        return 1;
    }

    Tim2Trans(GetIntAdrsCurrent(ocl1_stageTbl[nStage]->fno[nPos][nRound]));
    return 0;
}
