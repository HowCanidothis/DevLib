#ifndef GTSCENE_H
#define GTSCENE_H

#include "Shared/internal.hpp"

class GtActor;

class GtScene
{
    ArrayPointers<GtActor> objects;

public:
    GtScene();
    ~GtScene();

    void addObject(GtActor* object);
};

#endif // GTSCENE_H
