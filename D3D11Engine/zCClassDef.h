#pragma once
#include "zSTRING.h"
#include "zCObject.h"
#include "zCArray.h"

class zCClassDef {
public:
    zSTRING className;
    zSTRING baseClassName;
    zSTRING scriptClassName;
    zCClassDef* baseClassDef;

    /* Some other stuff that I don't need and am too lazy to make compile successfully
        zCObject (*createNewInstance) (void);
        zCObject (*createNewInstanceBackup) (void);
        unsigned int classFlags;
        unsigned int classSize;
        int numLivingObjects;
        int numCtorCalled;
        zCObject **hashtable;
        zCArray<zCObject *> objectList;
        unsigned short archiveVersion;
        unsigned short archiveVersionSum;
    */
};