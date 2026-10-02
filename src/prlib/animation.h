#ifndef PRLIB_ANIMATION_H
#define PRLIB_ANIMATION_H

#include "common.h"

#include <eetypes.h>

#include "linkedlist.h"
#include "objectset.h"

#define SPA_MAGIC   (0x59238771)
#define SPA_VERSION (2)

class SpmNode;
class SpaNodeAnimation;

class SpaFileHeader {
public:
    u_int m_magic;
    u_short m_version;
    u_short m_flags;
    PR_PADDING(unk8, 0xC);
    float unk14;
    char m_name[32];
    PR_PADDING(unk38, 0x4);
    PrLinkedList<SpaFileHeader> m_list;
    PrObjectSet<SpaFileHeader> *m_obj_set;
    void *m_user_data;
    PR_PADDING(unk4C, 0x4);
    SpaNodeAnimation **unk50;

public:
    void Initialize();

    bool IsNodeVisible(SpmNode *arg0, float arg1) const;

    void ChangePointer();
};

#endif /* PRLIB_ANIMATION_H */
