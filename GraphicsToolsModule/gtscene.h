#ifndef GTSCENE_H
#define GTSCENE_H

#include "Shared/internal.hpp"

class GtObjectActor;

class GtScene
{
    ArrayPointers<GtObjectActor> objects;
public:
    GtScene();

    void addObject(GtObjectActor* object);
};

#endif // GTSCENE_H
