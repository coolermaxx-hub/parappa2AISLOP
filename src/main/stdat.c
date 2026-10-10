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
        STAGE_OVERLAY_PTR(EVENTREC, 0x9138),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x9000),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x9130),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x45b0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x4598),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x45a8),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x8fb0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x8f98),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x8fa8),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x140),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x138),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x12348),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x122e0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x122f0),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST01HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x8aef0),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        93.0f,
        1,
        STAGE_OVERLAY_PTR(EVENTREC, 0x358),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x190),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x350),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x8baf0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x8a9c8),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x8aea8),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST01GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST01GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST01GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST01GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x8aef0),
    },
};
static STDAT_DAT stdat_dat_st02[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        2,
        STAGE_OVERLAY_PTR(EVENTREC, 0x3c0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x3b8),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x14e38),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x14dd0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x14de0),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST02HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x90ca8),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        2,
        STAGE_OVERLAY_PTR(EVENTREC, 0x558),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x410),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x550),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x918a8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x90660),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x90c60),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST02GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST02GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST02GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST02GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x90ca8),
    },
};
static STDAT_DAT stdat_dat_st03[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        93.0f,
        3,
        STAGE_OVERLAY_PTR(EVENTREC, 0x210),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x208),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x18ba0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x18b38),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x18b48),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST03HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xa84b8),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        93.0f,
        3,
        STAGE_OVERLAY_PTR(EVENTREC, 0x4e8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x260),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x4e0),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0xa90b8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xa7d80),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xa8470),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST03GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST03GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST03GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST03GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xa84b8),
    },
};
static STDAT_DAT stdat_dat_st04[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        93.0f,
        4,
        STAGE_OVERLAY_PTR(EVENTREC, 0x250),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x248),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x13d80),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x13d18),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x13d28),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST04HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x9f1d0),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        93.0f,
        4,
        STAGE_OVERLAY_PTR(EVENTREC, 0x428),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x2a0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x420),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x9fdd0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x9eb08),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x9f188),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST04GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST04GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST04GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST04GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x9f1d0),
    },
};
static STDAT_DAT stdat_dat_st05[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        5,
        STAGE_OVERLAY_PTR(EVENTREC, 0x150),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x148),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0xfbe8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xfb80),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xfb90),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST05HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xd2b78),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        5,
        STAGE_OVERLAY_PTR(EVENTREC, 0x3d8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x1a0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x3d0),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0xd3778),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xd2620),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xd2b30),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST05GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST05GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST05GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST05GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xd2b78),
    },
};
static STDAT_DAT stdat_dat_st06[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        6,
        STAGE_OVERLAY_PTR(EVENTREC, 0x1d0),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x1c8),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x90e8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x9080),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x9090),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST06HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x80140),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        6,
        STAGE_OVERLAY_PTR(EVENTREC, 0x3b8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x220),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x3b0),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x80d40),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x7fa88),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x800f8),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST06GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST06GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST06GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST06GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x80140),
    },
};
static STDAT_DAT stdat_dat_st07[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        7,
        STAGE_OVERLAY_PTR(EVENTREC, 0x170),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x168),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x84e8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x8480),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x8490),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST07HK0.INT;1", {} },
        {
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
            { FRMODE_CD, 0, 0, 0, NULL, {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x74ad8),
    },
    {
        PSTEP_XTR,
        "SER XTR 1",
        100.0f,
        7,
        STAGE_OVERLAY_PTR(EVENTREC, 0x4e8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x1c0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x4e0),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x756d8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x74380),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x74a90),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST07GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST07GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST07GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST07GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x74ad8),
    },
};
static STDAT_DAT stdat_dat_st08[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        8,
        STAGE_OVERLAY_PTR(EVENTREC, 0x290),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x48),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x288),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0xe2d28),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xe1a90),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xe20e0),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\ST08GM0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST08GM0C.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\SND\\ST08GM0G.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\SND\\ST08GM0N.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xe2128),
    },
};
static STDAT_DAT stdat_dat_st09[] = {
    {
        PSTEP_XTR,
        "SER XTR 0",
        100.0f,
        9,
        STAGE_OVERLAY_PTR(EVENTREC, 0x258),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x170),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x250),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0x41a8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x40e8),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x4188),
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
        STAGE_OVERLAY_PTR(EVENTREC, 0xd0328),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xcf3f0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xcf680),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS01VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS01VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS01VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS01VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xcf728),
    },
};
static STDAT_DAT stdat_dat_vs02[] = {
    {
        PSTEP_VS,
        "VS 0",
        93.4f,
        2,
        STAGE_OVERLAY_PTR(EVENTREC, 0xd57f8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xd48c0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xd4b50),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS02VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS02VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS02VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS02VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xd4bf8),
    },
};
static STDAT_DAT stdat_dat_vs03[] = {
    {
        PSTEP_VS,
        "VS 0",
        78.0f,
        3,
        STAGE_OVERLAY_PTR(EVENTREC, 0xf1ce8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xf0db0),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xf1040),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS03VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS03VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS03VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS03VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xf10e8),
    },
};
static STDAT_DAT stdat_dat_vs04[] = {
    {
        PSTEP_VS,
        "VS 0",
        102.0f,
        4,
        STAGE_OVERLAY_PTR(EVENTREC, 0xefa40),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xeeb08),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xeed98),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS04VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS04VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS04VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS04VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xeee40),
    },
};
static STDAT_DAT stdat_dat_vs05[] = {
    {
        PSTEP_VS,
        "VS 0",
        105.0f,
        5,
        STAGE_OVERLAY_PTR(EVENTREC, 0x122b30),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x121bf8),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x121e88),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS05VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS05VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS05VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS05VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x121f30),
    },
};
static STDAT_DAT stdat_dat_vs06[] = {
    {
        PSTEP_VS,
        "VS 0",
        102.0f,
        6,
        STAGE_OVERLAY_PTR(EVENTREC, 0xd3120),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xd21e8),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xd2478),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS06VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS06VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS06VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS06VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xd2520),
    },
};
static STDAT_DAT stdat_dat_vs07[] = {
    {
        PSTEP_VS,
        "VS 0",
        95.0f,
        7,
        STAGE_OVERLAY_PTR(EVENTREC, 0xbc3b8),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0xbb480),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0xbb710),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS07VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS07VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS07VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS07VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0xbb7b8),
    },
};
static STDAT_DAT stdat_dat_vs08[] = {
    {
        PSTEP_VS,
        "VS 0",
        99.0f,
        8,
        STAGE_OVERLAY_PTR(EVENTREC, 0x136d58),
        STAGE_OVERLAY_PTR(struct SCR_MAIN, 0x135e20),
        STAGE_OVERLAY_PTR(JIMAKU_STR, 0x1360b0),
        { FRMODE_CD, FTMODE_INTG, 0, 0, "\\DATA\\VS08VS0.INT;1", {} },
        {
            { FRMODE_CD, FTMODE_WP2, 2, 0, "\\VS\\VS08VS0.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS08VS1.WP2;1", {} },
            { FRMODE_CD, FTMODE_WP2, 4, 0, "\\VS\\VS08VS2.WP2;1", {} },
        },
        STAGE_OVERLAY_PTR(TAPLVL_STR, 0x136158),
    },
};
STDAT_REC stdat_rec[STDAT_STAGE_MAX] = {
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG00.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st00),
        stdat_dat_st00,
        "STAGE 0",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG01.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st01),
        stdat_dat_st01,
        "STAGE 1",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG02.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st02),
        stdat_dat_st02,
        "STAGE 2",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG03.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st03),
        stdat_dat_st03,
        "STAGE 3",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG04.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st04),
        stdat_dat_st04,
        "STAGE 4",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG05.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st05),
        stdat_dat_st05,
        "STAGE 5",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG06.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st06),
        stdat_dat_st06,
        "STAGE 6",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG07.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st07),
        stdat_dat_st07,
        "STAGE 7",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG08.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st08),
        stdat_dat_st08,
        "STAGE 8",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG09.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_st09),
        stdat_dat_st09,
        "STAGE 9",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STGBN.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_bonus),
        stdat_dat_bonus,
        "BONUS GAME",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG01.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs01),
        stdat_dat_vs01,
        "VS 1",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG02.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs02),
        stdat_dat_vs02,
        "VS 2",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG03.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs03),
        stdat_dat_vs03,
        "VS 3",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG04.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs04),
        stdat_dat_vs04,
        "VS 4",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG05.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs05),
        stdat_dat_vs05,
        "VS 5",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG06.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs06),
        stdat_dat_vs06,
        "VS 6",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG07.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs07),
        stdat_dat_vs07,
        "VS 7",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG08.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_vs08),
        stdat_dat_vs08,
        "VS 8",
    },
    {
        { FRMODE_CD, FTMODE_ETC, 0, 0, "\\MDL\\STG00.OLM;1", {} },
        PR_ARRAYSIZE(stdat_dat_title),
        stdat_dat_title,
        "TITLE",
    },
};

int stdat_rec_num = STDAT_STAGE_MAX;

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

    for (i = 0; i < PR_ARRAYSIZEU(file_str_extra_file); i++) {
        stDatFileSearch(&file_str_extra_file[i]);
    }

    for (i = 0; i < stdat_rec_num; i++) {
        stDatFileSearch(&stdat_rec[i].ovlfile);

        for (j = 0; j < stdat_rec[i].stdat_dat_num; j++) {
            stDatFileSearch(&stdat_rec[i].stdat_dat_pp[j].intfile);

            for (k = 0; k < PR_ARRAYSIZE(stdat_rec[i].stdat_dat_pp[j].sndfile); k++) {
                stDatFileSearch(&stdat_rec[i].stdat_dat_pp[j].sndfile[k]);
            }
        }
    }
}

