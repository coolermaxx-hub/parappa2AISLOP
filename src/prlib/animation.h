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
    float m_duration;
    char m_name[32];
    int *unk38;
    PrLinkedList<SpaFileHeader> m_list;
    PrObjectSet<SpaFileHeader> *m_obj_set;
    void *m_user_data;
    u_int m_nodeCount;
    SpaNodeAnimation **m_nodes;
    SpaNodeAnimation *m_inlineNodes[1];

public:
    void Initialize();

    bool IsNodeVisible(SpmNode *node, float time) const;

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
