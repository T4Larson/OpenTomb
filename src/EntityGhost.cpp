/*
 * EntityGhost.cpp
 *
 *  Created on: 01.11.2015
 *      Author: T4Larson
 */

#include "EntityGhost.h"


LaraShape_Crouch::LaraShape_Crouch()
{
    btScalar rad = 200.0f;
    btScalar top = 400.0f;
    btScalar low = 255.0f; // tr val
//            low = 112.0f;

    btScalar height = (top-low)/2.0f;
    btScalar offset = low+height;
    btScalar pad = 64.0;    // floor to bottom-tip of cone

    btTransform tr;
    tr.setIdentity();

    btCollisionShape* child;
    child = new btCylinderShapeZ({rad, rad, height});
    tr.setOrigin({0,0, offset});
    addChildShape(tr, child);

    child = new btConeShapeZ(rad, low-pad);
    tr.setRotation(btQuaternion(0, 180 * RadPerDeg, 0));
    tr.setOrigin({0,0, low - (low-pad)/2.0f});
    addChildShape(tr, child);

    m_localOffset.setIdentity();
}

LaraShape_Crouch::~LaraShape_Crouch()
{
    for(int i=0; i<getNumChildShapes(); i++)
    {
        delete getChildShape(i);
    }
}


EntityShape_Skeletal::EntityShape_Skeletal(SSBoneFrame* bf)
{
    for(size_t i = 0; i < bf->bone_tags.size(); i++)
    {
        btVector3 boxsize = COLLISION_GHOST_VOLUME_COEFFICIENT * (bf->bone_tags[i].mesh_base->m_bbMax - bf->bone_tags[i].mesh_base->m_bbMin);
        // btVector3 boxsize = m_bf.bone_tags[i].mesh_base->m_bbMax - m_bf.bone_tags[i].mesh_base->m_bbMin;
        btBoxShape *boxshape = new btBoxShape(boxsize);
        boxshape->setMargin(COLLISION_MARGIN_DEFAULT);

        btTransform tr = bf->bone_tags[i].full_transform;
        tr.setOrigin(tr * bf->bone_tags[i].mesh_base->m_center);

        addChildShape(tr, boxshape);
    }

    m_localOffset.setIdentity();
}

EntityShape_Skeletal::~EntityShape_Skeletal()
{
    for(int i=0; i<getNumChildShapes(); i++)
    {
        delete getChildShape(i);
    }
}


const btTransform& EntityShape_Skeletal::update(const SSBoneFrame* bf)
{
    if(bf->bone_tags.size() != (unsigned)getNumChildShapes())
    {
        // maybe raise some exceptions?
        return m_localOffset;
    }
    for(int i = 0; i < getNumChildShapes(); i++)
    {
        btTransform tr = bf->bone_tags[i].full_transform;
        tr.setOrigin(tr * bf->bone_tags[i].mesh_base->m_center);
        // FIXME: scaling is unstable, 0-size will collapse...
//        if((1<<i) & no_fix_body_parts)
//        {
//            getChildShape(i)->setLocalScaling({0,0,0});
//        }
//        else
//        {
//            getChildShape(i)->setLocalScaling({1,1,1});
//            // need manual margin adjust:
//            //static_cast<btBoxShape*>(cshape)->setImplicitShapeDimensions();
//        }
        updateChildTransform(i, tr, false);
    }
    recalculateLocalAabb();
    return m_localOffset;
}


/*
 * =========================================================
 * ==== EntityGhost
 */


/**
 * Add selectable shape to ghost.
 * !Important: EntityGhost takes ownership of the pointer
 * !           and handles deletion.
 * @param gs    ptr to object with ghostshape interface
 * @return      index of shape (for selectShape)
 */
int EntityGhost::addShape(IEntityGhostShape* gs)
{
    m_shapes.emplace_back(gs);
    if(m_currentShape < 0)
    {
        setCurrentShape(0);
    }
    return m_shapes.size()-1;
}

void EntityGhost::selectShape(int idx)
{
    if(idx != m_selectedShape && idx >= 0 && (unsigned)idx < m_shapes.size())
    {
        m_selectedShape = idx;;
    }
}

void EntityGhost::setCurrentShape(int shapeidx)
{
    if(shapeidx < 0 || (unsigned)shapeidx >= m_shapes.size())
        return;

    setCollisionShape(m_shapes[shapeidx]->getShape());
    m_currentShape = shapeidx;

    if(m_myWorld)
    {
        // TODO: Flushing:
        // Without, separation resolving doesn't work if already intersecting with new shape...
        // bullet bug?: apparently, even removing/re-adding the (same?) ghost object doesn't flush world pairs!? (bullet crashes with stale ptrs)
        //              - does this happen only with compound objects?
        getOverlappingPairCache()->cleanProxyFromPairs( getBroadphaseHandle(), m_myWorld->getDispatcher() );
        // the world cache may also contains stale pairs: need to do this, too:
        m_myWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs( getBroadphaseHandle(), m_myWorld->getDispatcher() );
        // TODO: change shape after flush
    }
}

/**
 * Update ghost object/ghostshapes.
 * Expects the shape's local transform as return value from shape->update()!
 * @param t     global transform (i.e. from entity)
 * @param bf    boneframe (to update anim specific shapes)
 */
void EntityGhost::update(const btTransform& t, const SSBoneFrame* bf)
{
    if(m_currentShape != m_selectedShape)
    {
        setCurrentShape(m_selectedShape);
    }
    if(!bf && m_parent)
    {
        bf = &m_parent->m_bf;
    }
    const btTransform& local = m_shapes[m_currentShape]->update(bf);
    setWorldTransform(t * local);
}

void EntityGhost::update()
{
    if(m_parent)
    {
        update(m_parent->m_transform, &m_parent->m_bf);
    }
}


/**
 * Set parent entity for this ghost:
 * Without parent entity, the ghost can still be used
 * for collision checks, but it will not filter collisions
 * with the same entity, doesn't follow entity position on update
 * (unless given as param) and does not update the skeletalmesh.
 * @param parent    Entity to which this ghost is attached
 */
void EntityGhost::setParent(Entity* parent)
{
    if(m_parent || !parent) return;

    m_parent = parent;
    setUserPointer(m_parent->m_self.get());
    setWorldTransform(m_parent->m_transform);
}

/**
 * Add ghost to physics world. Without being added
 * to a world, no collision tests will be done..
 * @param w     Physics world
 */
void EntityGhost::addToWorld(btDynamicsWorld* world)
{
    if(m_myWorld || m_currentShape<0 ) return;

    world->addCollisionObject(this, COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
    m_myWorld = world;
}


void EntityGhost::removeFromWorld()
{
    if(!m_myWorld) return;
    m_myWorld->removeCollisionObject(this);
    m_myWorld = nullptr;
}


void EntityGhost::checkCollision()
{

}

void EntityGhost::checkCollision(btTransform& t)
{

}

bool EntityGhost::recoverFromPenetration()
{
    if(!m_myWorld) return false;

    btCollisionShape* convexShape = getCollisionShape();

    btVector3 currentPosition;

    // From the docs:
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    // TODO: First iteration collision update after physics-step may be obsolete.
    btVector3 minAabb, maxAabb;
    convexShape->getAabb(getWorldTransform(), minAabb,maxAabb);
    m_myWorld->getBroadphase()->setAabb(getBroadphaseHandle(),
                         minAabb,
                         maxAabb,
                         m_myWorld->getDispatcher());

    m_myWorld->getDispatcher()->dispatchAllCollisionPairs(getOverlappingPairCache(), m_myWorld->getDispatchInfo(), m_myWorld->getDispatcher());

    currentPosition = getWorldTransform().getOrigin();

//    btScalar maxPen = btScalar(0.0);
//    btVector3 touchingNormal;
    bool penetration = false;
    for(int i = 0; i < getOverlappingPairCache()->getNumOverlappingPairs(); i++)
    {
        btBroadphasePair* collisionPair = &getOverlappingPairCache()->getOverlappingPairArray()[i];

        btCollisionObject* obj0 = static_cast<btCollisionObject*>(collisionPair->m_pProxy0->m_clientObject);
        btCollisionObject* obj1 = static_cast<btCollisionObject*>(collisionPair->m_pProxy1->m_clientObject);

        if((obj0 && !obj0->hasContactResponse()) && (obj1 && !obj1->hasContactResponse()))
        {
            continue;
        }

        // FIXME: EngineContainer is shared as some kind of "parent id"
        //        I'm not sure if that was intended, but comparing the pointer
        //        will fail when a new container with the same object reference
        //        is used... This should just be Object pointer...(ent,room->object)
        EngineContainer* cont0 = static_cast<EngineContainer*>(obj0->getUserPointer());
        EngineContainer* cont1 = static_cast<EngineContainer*>(obj1->getUserPointer());

        // Unbound ghost should collide anyway:
//        if(!cont0 || !cont1) continue;
        if(cont0 && cont1)
        {
            if(cont0->collision_type == COLLISION_TYPE_GHOST    // fixme: only the other must not be ghost..?
               || cont1->collision_type == COLLISION_TYPE_GHOST
               || (cont0->object == m_parent && cont1->object == m_parent))
            {
                continue;
            }
        }
        m_manifoldArray.resize(0);
        if(collisionPair->m_algorithm)
           collisionPair->m_algorithm->getAllContactManifolds(m_manifoldArray);

        for(int j=0;j<m_manifoldArray.size();j++)
        {
            btPersistentManifold* manifold = m_manifoldArray[j];
            btScalar directionSign = manifold->getBody0() == this ? btScalar(-1.0) : btScalar(1.0);
            for(int p=0;p<manifold->getNumContacts();p++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(p);

                btScalar dist = pt.getDistance();

                if(dist < 0.0)
                {
//                    if(dist < maxPen)
//                    {
//                        maxPen = dist;
//                        touchingNormal = pt.m_normalWorldOnB * directionSign;
//
//                    }
                    currentPosition += pt.m_normalWorldOnB * directionSign * dist * btScalar(0.2);
                    penetration = true;
                }
                else
                {
                    //printf("touching %f\n", dist);
                }
            }
        }
    }

    btTransform newTrans = getWorldTransform();
    newTrans.setOrigin(currentPosition);
    setWorldTransform(newTrans);

    return penetration;
}

// fixme: compat call:
int EntityGhost::getPenetrationFixVector(btVector3* reaction)
{
    if(!m_myWorld) return 0;
//    if( m_bt.no_fix_all) return 0;

    update();

    btVector3 start_pos = getWorldTransform().getOrigin();

    int numPenetrationLoops = 0;
    bool touchingContact = false;

    while(recoverFromPenetration())
    {
        numPenetrationLoops++;
        touchingContact = true;
        if(numPenetrationLoops > 4) // max recovery attempts
        {
            //printf("character could not recover from penetration = %d\n", numPenetrationLoops);
            break;
        }
    }

    *reaction = getWorldTransform().getOrigin() - start_pos;

    m_lastFixVector = *reaction;
    return touchingContact;
}












