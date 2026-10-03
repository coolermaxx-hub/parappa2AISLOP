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
    /* Length in seconds; PrGetAnimationEndFrame converts it to frames */
    float m_length;
    char m_name[32];
    int *unk38;
    PrLinkedList<SpaFileHeader> m_list;
    PrObjectSet<SpaFileHeader> *m_obj_set;
    void *m_user_data;
    /* One animation per model node, indexed by SpmNode::m_index */
    u_int m_node_num;
    SpaNodeAnimation **m_nodes;
    SpaNodeAnimation *m_node_table[1];

public:
    void Initialize();

    bool IsNodeVisible(SpmNode *arg0, float arg1) const;

    void ChangePointer();

    template <typename T>
    T* CalculatePointer(T *offset) {
        if (!offset) {
            return NULL;
        }
        return reinterpret_cast<T*>(reinterpret_cast<int>(this) + reinterpret_cast<int>(offset));
    }
};

#endif /* PRLIB_ANIMATION_H */
