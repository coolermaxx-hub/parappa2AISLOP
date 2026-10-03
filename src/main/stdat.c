#include "main/stdat.h"

#include "os/mtc.h"

FILE_STR file_str_logo_file = { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\LOGO.INT;1", {} };
FILE_STR file_str_menu_file = { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\STMENU.INT;1", {} };
FILE_STR file_str_extra_file[10] = {
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT00.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT01.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT02.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT03.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT04.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT05.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT06.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT07.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT08.WP2;1", {} },
    { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\EXT09.WP2;1", {} },
};
static STDAT_DAT stdat_dat_st00[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        93.0f,
        0,
        (EVENTREC *)0x1CA9138,
        (struct SCR_MAIN *)0x1CA9000,
        (JIMAKU_STR *)0x1CA9130,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST00SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST00SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
};
static STDAT_DAT stdat_dat_title[] = {
    {
        PSTEP_SERIAL,
        "TITLE",
        100.0f,
        0,
        (EVENTREC *)0x1CA45B0,
        (struct SCR_MAIN *)0x1CA4598,
        (JIMAKU_STR *)0x1CA45A8,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\TITLE.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_SERIAL,
        "TITLE DERA",
        100.0f,
        0,
        (EVENTREC *)0x1CA8FB0,
        (struct SCR_MAIN *)0x1CA8F98,
        (JIMAKU_STR *)0x1CA8FA8,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\TITLE.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\EXT\\TITLE.WP2;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
};
static STDAT_DAT stdat_dat_st01[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        93.0f,
        1,
        (EVENTREC *)0x1CA0140,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA0138,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST01SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST01SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        1,
        (EVENTREC *)0x1CB2348,
        (struct SCR_MAIN *)0x1CB22E0,
        (JIMAKU_STR *)0x1CB22F0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST01HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D2AEF0,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        93.0f,
        1,
        (EVENTREC *)0x1CA0358,
        (struct SCR_MAIN *)0x1CA0190,
        (JIMAKU_STR *)0x1CA0350,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST01SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST01SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        108.0f,
        1,
        (EVENTREC *)0x1D2BAF0,
        (struct SCR_MAIN *)0x1D2A9C8,
        (JIMAKU_STR *)0x1D2AEA8,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST01GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST01GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST01GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST01GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D2AEF0,
    },
};
static STDAT_DAT stdat_dat_st02[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        2,
        (EVENTREC *)0x1CA03C0,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA03B8,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST02SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST02SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        2,
        (EVENTREC *)0x1CB4E38,
        (struct SCR_MAIN *)0x1CB4DD0,
        (JIMAKU_STR *)0x1CB4DE0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST02HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D30CA8,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        2,
        (EVENTREC *)0x1CA0558,
        (struct SCR_MAIN *)0x1CA0410,
        (JIMAKU_STR *)0x1CA0550,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST02SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST02SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        93.4f,
        2,
        (EVENTREC *)0x1D318A8,
        (struct SCR_MAIN *)0x1D30660,
        (JIMAKU_STR *)0x1D30C60,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST02GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST02GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST02GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST02GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D30CA8,
    },
};
static STDAT_DAT stdat_dat_st03[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        93.0f,
        3,
        (EVENTREC *)0x1CA0210,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA0208,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST03SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST03SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        3,
        (EVENTREC *)0x1CB8BA0,
        (struct SCR_MAIN *)0x1CB8B38,
        (JIMAKU_STR *)0x1CB8B48,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST03HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D484B8,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        93.0f,
        3,
        (EVENTREC *)0x1CA04E8,
        (struct SCR_MAIN *)0x1CA0260,
        (JIMAKU_STR *)0x1CA04E0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST03SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST03SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        78.0f,
        3,
        (EVENTREC *)0x1D490B8,
        (struct SCR_MAIN *)0x1D47D80,
        (JIMAKU_STR *)0x1D48470,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST03GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST03GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST03GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST03GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D484B8,
    },
};
static STDAT_DAT stdat_dat_st04[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        93.0f,
        4,
        (EVENTREC *)0x1CA0250,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA0248,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST04SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST04SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        4,
        (EVENTREC *)0x1CB3D80,
        (struct SCR_MAIN *)0x1CB3D18,
        (JIMAKU_STR *)0x1CB3D28,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST04HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D3F1D0,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        93.0f,
        4,
        (EVENTREC *)0x1CA0428,
        (struct SCR_MAIN *)0x1CA02A0,
        (JIMAKU_STR *)0x1CA0420,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST04SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST04SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        102.0f,
        4,
        (EVENTREC *)0x1D3FDD0,
        (struct SCR_MAIN *)0x1D3EB08,
        (JIMAKU_STR *)0x1D3F188,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST04GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST04GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST04GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST04GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D3F1D0,
    },
};
static STDAT_DAT stdat_dat_st05[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        5,
        (EVENTREC *)0x1CA0150,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA0148,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST05SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST05SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        5,
        (EVENTREC *)0x1CAFBE8,
        (struct SCR_MAIN *)0x1CAFB80,
        (JIMAKU_STR *)0x1CAFB90,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST05HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D72B78,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        5,
        (EVENTREC *)0x1CA03D8,
        (struct SCR_MAIN *)0x1CA01A0,
        (JIMAKU_STR *)0x1CA03D0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST05SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST05SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        105.0f,
        5,
        (EVENTREC *)0x1D73778,
        (struct SCR_MAIN *)0x1D72620,
        (JIMAKU_STR *)0x1D72B30,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST05GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST05GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST05GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST05GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D72B78,
    },
};
static STDAT_DAT stdat_dat_st06[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        6,
        (EVENTREC *)0x1CA01D0,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA01C8,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST06SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST06SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        6,
        (EVENTREC *)0x1CA90E8,
        (struct SCR_MAIN *)0x1CA9080,
        (JIMAKU_STR *)0x1CA9090,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST06HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D20140,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        6,
        (EVENTREC *)0x1CA03B8,
        (struct SCR_MAIN *)0x1CA0220,
        (JIMAKU_STR *)0x1CA03B0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST06SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST06SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        102.0f,
        6,
        (EVENTREC *)0x1D20D40,
        (struct SCR_MAIN *)0x1D1FA88,
        (JIMAKU_STR *)0x1D200F8,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST06GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST06GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST06GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST06GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D20140,
    },
};
static STDAT_DAT stdat_dat_st07[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        7,
        (EVENTREC *)0x1CA0170,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA0168,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST07SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST07SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_HOOK,
        "HOOK 0",
        68.0f,
        7,
        (EVENTREC *)0x1CA84E8,
        (struct SCR_MAIN *)0x1CA8480,
        (JIMAKU_STR *)0x1CA8490,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST07HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        (TAPLVL_STR *)0x1D14AD8,
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        7,
        (EVENTREC *)0x1CA04E8,
        (struct SCR_MAIN *)0x1CA01C0,
        (JIMAKU_STR *)0x1CA04E0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST07SR1.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST07SR1.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        95.0f,
        7,
        (EVENTREC *)0x1D156D8,
        (struct SCR_MAIN *)0x1D14380,
        (JIMAKU_STR *)0x1D14A90,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST07GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST07GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST07GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST07GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D14AD8,
    },
};
static STDAT_DAT stdat_dat_st08[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        8,
        (EVENTREC *)0x1CA0290,
        (struct SCR_MAIN *)0x1CA0048,
        (JIMAKU_STR *)0x1CA0288,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST08SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST08SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
    {
        PSTEP_GAME,
        "GAME 0",
        99.0f,
        8,
        (EVENTREC *)0x1D82D28,
        (struct SCR_MAIN *)0x1D81A90,
        (JIMAKU_STR *)0x1D820E0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST08GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST08GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST08GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST08GM0N.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D82128,
    },
};
static STDAT_DAT stdat_dat_st09[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        9,
        (EVENTREC *)0x1CA0258,
        (struct SCR_MAIN *)0x1CA0170,
        (JIMAKU_STR *)0x1CA0250,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATAS\\ST09SR0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_XTR, 2, 0, "\\XTR\\ST09SR0.XTR;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
};
static STDAT_DAT stdat_dat_bonus[] = {
    {
        PSTEP_BONUS,
        "BONUS",
        108.6f,
        128,
        (EVENTREC *)0x1CA41A8,
        (struct SCR_MAIN *)0x1CA40E8,
        (JIMAKU_STR *)0x1CA4188,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\STBN.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\STBN0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\STBN1.WP2;1", {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        NULL,
    },
};
static STDAT_DAT stdat_dat_vs01[] = {
    {
        PSTEP_VS,
        "VS 0",
        108.0f,
        1,
        (EVENTREC *)0x1D70328,
        (struct SCR_MAIN *)0x1D6F3F0,
        (JIMAKU_STR *)0x1D6F680,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS01VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS01VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS01VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS01VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D6F728,
    },
};
static STDAT_DAT stdat_dat_vs02[] = {
    {
        PSTEP_VS,
        "VS 0",
        93.4f,
        2,
        (EVENTREC *)0x1D757F8,
        (struct SCR_MAIN *)0x1D748C0,
        (JIMAKU_STR *)0x1D74B50,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS02VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS02VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS02VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS02VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D74BF8,
    },
};
static STDAT_DAT stdat_dat_vs03[] = {
    {
        PSTEP_VS,
        "VS 0",
        78.0f,
        3,
        (EVENTREC *)0x1D91CE8,
        (struct SCR_MAIN *)0x1D90DB0,
        (JIMAKU_STR *)0x1D91040,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS03VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS03VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS03VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS03VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D910E8,
    },
};
static STDAT_DAT stdat_dat_vs04[] = {
    {
        PSTEP_VS,
        "VS 0",
        102.0f,
        4,
        (EVENTREC *)0x1D8FA40,
        (struct SCR_MAIN *)0x1D8EB08,
        (JIMAKU_STR *)0x1D8ED98,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS04VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS04VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS04VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS04VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D8EE40,
    },
};
static STDAT_DAT stdat_dat_vs05[] = {
    {
        PSTEP_VS,
        "VS 0",
        105.0f,
        5,
        (EVENTREC *)0x1DC2B30,
        (struct SCR_MAIN *)0x1DC1BF8,
        (JIMAKU_STR *)0x1DC1E88,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS05VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS05VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS05VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS05VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1DC1F30,
    },
};
static STDAT_DAT stdat_dat_vs06[] = {
    {
        PSTEP_VS,
        "VS 0",
        102.0f,
        6,
        (EVENTREC *)0x1D73120,
        (struct SCR_MAIN *)0x1D721E8,
        (JIMAKU_STR *)0x1D72478,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS06VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS06VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS06VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS06VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D72520,
    },
};
static STDAT_DAT stdat_dat_vs07[] = {
    {
        PSTEP_VS,
        "VS 0",
        95.0f,
        7,
        (EVENTREC *)0x1D5C3B8,
        (struct SCR_MAIN *)0x1D5B480,
        (JIMAKU_STR *)0x1D5B710,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS07VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS07VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS07VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS07VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1D5B7B8,
    },
};
static STDAT_DAT stdat_dat_vs08[] = {
    {
        PSTEP_VS,
        "VS 0",
        99.0f,
        8,
        (EVENTREC *)0x1DD6D58,
        (struct SCR_MAIN *)0x1DD5E20,
        (JIMAKU_STR *)0x1DD60B0,
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS08VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS08VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS08VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS08VS2.WP2;1", {} },
        },
        (TAPLVL_STR *)0x1DD6158,
    },
};
STDAT_REC stdat_rec[20] = {
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG00.OLM;1", {} },
        1,
        stdat_dat_st00,
        "STAGE 0",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG01.OLM;1", {} },
        4,
        stdat_dat_st01,
        "STAGE 1",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG02.OLM;1", {} },
        4,
        stdat_dat_st02,
        "STAGE 2",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG03.OLM;1", {} },
        4,
        stdat_dat_st03,
        "STAGE 3",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG04.OLM;1", {} },
        4,
        stdat_dat_st04,
        "STAGE 4",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG05.OLM;1", {} },
        4,
        stdat_dat_st05,
        "STAGE 5",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG06.OLM;1", {} },
        4,
        stdat_dat_st06,
        "STAGE 6",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG07.OLM;1", {} },
        4,
        stdat_dat_st07,
        "STAGE 7",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG08.OLM;1", {} },
        2,
        stdat_dat_st08,
        "STAGE 8",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG09.OLM;1", {} },
        1,
        stdat_dat_st09,
        "STAGE 9",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STGBN.OLM;1", {} },
        1,
        stdat_dat_bonus,
        "BONUS GAME",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG01.OLM;1", {} },
        1,
        stdat_dat_vs01,
        "VS 1",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG02.OLM;1", {} },
        1,
        stdat_dat_vs02,
        "VS 2",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG03.OLM;1", {} },
        1,
        stdat_dat_vs03,
        "VS 3",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG04.OLM;1", {} },
        1,
        stdat_dat_vs04,
        "VS 4",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG05.OLM;1", {} },
        1,
        stdat_dat_vs05,
        "VS 5",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG06.OLM;1", {} },
        1,
        stdat_dat_vs06,
        "VS 6",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG07.OLM;1", {} },
        1,
        stdat_dat_vs07,
        "VS 7",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG08.OLM;1", {} },
        1,
        stdat_dat_vs08,
        "VS 8",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG00.OLM;1", {} },
        2,
        stdat_dat_title,
        "TITLE",
    },
};

int stdat_rec_num = 20;

static void stDatFileSearch(FILE_STR *fs_pp) {
    if (fs_pp == NULL || fs_pp->fname == NULL) {
        return;
    }

    while (!CdctrlSerch(fs_pp)) {
        MtcWait(1);
    }
}

void stDatFirstFileSearch(void) {
    int i, j, k;

    stDatFileSearch(&file_str_logo_file);
    stDatFileSearch(&file_str_menu_file);

    for (i = 0; i < 10u; i++) {
        stDatFileSearch(&file_str_extra_file[i]);
    }

    for (i = 0; i < stdat_rec_num; i++) {
        stDatFileSearch(&stdat_rec[i].ovlfile);

        for (j = 0; j < stdat_rec[i].stdat_dat_num; j++) {
            stDatFileSearch(&stdat_rec[i].stdat_dat_pp[j].intfile);

            for (k = 0; k < 3; k++) {
                stDatFileSearch(&stdat_rec[i].stdat_dat_pp[j].sndfile[k]);
            }
        }
    }
}

