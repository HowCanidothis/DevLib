#ifndef GTSCENE_H
#define GTSCENE_H

#include "SharedModule/internal.hpp"

class GtActor;

class GtScene
{
    ArrayPointers<GtActor> m_objects;

public:
    GtScene();
    ~GtScene();

    void AddObject(GtActor* object);
};

#endif // GTSCENE_H
