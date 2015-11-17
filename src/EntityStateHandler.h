/*
 * EntityStateHandler.h
 *
 * Simple ID based state handling wrapper
 *
 *  Created on: 12.11.2015
 *      Author: T4Larson
 */

#ifndef ENTITYSTATEHANDLER_H_
#define ENTITYSTATEHANDLER_H_

#include <vector>
#include "entity.h"

/**
 * State handling wrapper for OT Entities/Models
 */
// TODO: Integrate Animation-StateChange handling
class EntityStateHandler {
public:
    EntityStateHandler(Entity *pEnt);
    virtual ~EntityStateHandler();

    struct state {
    protected:
        state() = default;
    public:
        virtual ~state() = default;

        virtual void enter(EntityStateHandler *)  { }
        virtual void action(EntityStateHandler *) { }
        virtual void exit(EntityStateHandler *)   { }
    };

    void registerState(int id, state *pstate);

//    template<class T>
//    void registerState(int id) {
//        registerState(id, new T);
//    }

    void updateState(int stateid);

protected:
    std::vector<state*> m_states;
    Entity* m_entity = nullptr;

    int m_laststate = -1;
};









#endif /* ENTITYSTATEHANDLER_H_ */
