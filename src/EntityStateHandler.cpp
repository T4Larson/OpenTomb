/*
 * EntityStateHandler.cpp
 *
 * Simple ID based state handling wrapper
 *
 *  Created on: 12.11.2015
 *      Author: T4Larson
 */

#include "EntityStateHandler.h"

EntityStateHandler::EntityStateHandler(Entity* pEnt)
: m_entity(pEnt)
{}

EntityStateHandler::~EntityStateHandler()
{
    for(auto s : m_states)
    {
        delete s;
    }
}

void EntityStateHandler::registerState(int id, state* pstate)
{
    if(id < 0) return;
    if((unsigned)id >= m_states.size())
    {
        m_states.resize(id+1);
    }
    if(m_states[id])
    {
        delete m_states[id];
    }
    m_states[id] = pstate;
}


void EntityStateHandler::updateState(int stateid)
{
    if(stateid != m_laststate && (unsigned)stateid < m_states.size())
    {
        if(m_laststate > 0)
        {
            m_states[m_laststate]->exit(this);
        }
        m_states[stateid]->enter(this);
        m_laststate = stateid;
    }

    if(m_laststate > 0)
    {
        m_states[m_laststate]->action(this);
    }
}


