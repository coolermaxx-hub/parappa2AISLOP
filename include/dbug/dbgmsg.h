#ifndef DBGMSG_H
#define DBGMSG_H

#include "common.h"

#include <eetypes.h>
#include <libgifpk.h>

void DbgMsgInit(void);
void DbgMsgClear(void);
void DbgMsgFlash(void);

void DbgMsgSetColor(u_char r, u_char g, u_char b);
void DbgMsgSetSize(u_short sw, u_short sh);

/* x, y: the text's top left in GS primitive coordinates, in whole pixels (the
 * field's top left is GS_X_COORD(0) >> 4, GS_Y_COORD(0) >> 4: 1728, 1936). */
void DbgMsgPrint(u_char* m_pp, u_short x, u_short y);
void DbgMsgPrintUserPkt(u_char* m_pp, u_short x, u_short y, sceGifPacket* usrPacket_pp);
void DbgMsgClearUserPkt(sceGifPacket* usrPacket_pp);

void DbgMsgSetColorUserPkt(u_char r, u_char g, u_char b, sceGifPacket* usrPacket_pp);
void DbgMsgSetZ(int z);

#endif /* DBGMSG_H */
