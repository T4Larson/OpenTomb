/*
 * EntityGhost.h
 *
 *  Created on: 01.11.2015
 *      Author: T4Larson
 */

#ifndef ENTITYGHOST_H_
#define ENTITYGHOST_H_


// TODO: split incs:
#include "entity.h"



class IEntityGhostShape
{
public:
    virtual ~IEntityGhostShape() {};
    // FIXME: update used mainly for localOffset, rarely needs shape change
    virtual const btTransform& update(const SSBoneFrame* bf) = 0;
    virtual btCollisionShape* getShape() = 0;
};


// -----------------------------------------------------
// ---- Some base shapes (used by default shapes below):

class EntityBaseShape_Cylinder : public btCylinderShapeZ, public IEntityGhostShape
{
public:
    EntityBaseShape_Cylinder(btScalar radius, btScalar top, btScalar bottom)
    : btCylinderShapeZ({radius, radius, (top-bottom)/btScalar(2.0)})
    , m_radius(radius), m_top(top), m_bottom(bottom)
    {
        const btScalar height = (top-bottom) / 2.0;
        const btScalar offset = bottom + height;
        m_localOffset.setIdentity();
        m_localOffset.setOrigin({0,0,offset});
    }

    const btTransform& update(const SSBoneFrame*)
    {
        return m_localOffset;
    }
    btCollisionShape* getShape() { return this; }
protected:
    btScalar m_radius, m_top, m_bottom;
    btTransform m_localOffset;
};

class EntityBaseShape_Capsule : public btCapsuleShapeZ, public IEntityGhostShape
{
public:
    EntityBaseShape_Capsule(btScalar radius, btScalar top, btScalar bottom)
    : btCapsuleShapeZ(radius, top-bottom - 2.0*radius)
    , m_radius(radius), m_top(top), m_bottom(bottom)
    {
        // const btScalar height = top-bottom - 2.0*radius;
        const btScalar offset = bottom + (top - bottom)/2.0;
        m_localOffset.setIdentity();
        m_localOffset.setOrigin({0,0,offset});
        m_localOffset = btTransform(btQuaternion(0, -90 * RadPerDeg, 0)) * m_localOffset;    // ypr
    }

    const btTransform& update(const SSBoneFrame*)
    {
        return m_localOffset;
    }
    btCollisionShape* getShape() { return this; }
protected:
    btScalar m_radius, m_top, m_bottom;
    btTransform m_localOffset;
};

// ---------------------------------
// --- Shapes for misc. Lara states:

class LaraShape_Default : public EntityBaseShape_Cylinder
{
public:
    LaraShape_Default()
    : EntityBaseShape_Cylinder(100.0, 762.0, 384.0)
    {}
};

class LaraShape_Monkey : public EntityBaseShape_Cylinder
{
public:
    LaraShape_Monkey()
    : EntityBaseShape_Cylinder(100.0, 600.0, 0.0)
    {}
};

class LaraShape_WaterSurf : public EntityBaseShape_Cylinder
{
public:
    LaraShape_WaterSurf()
    : EntityBaseShape_Cylinder(100.0, 128.0, -384.0)
    {}
};

class LaraShape_WaterDive : public EntityBaseShape_Capsule
{
public:
    LaraShape_WaterDive()
    : EntityBaseShape_Capsule(100.0, 381.0, -381.0)
    {}
};

/*
 * TODO:
 * - transitional crouch Shape (cylinder with bottom=0),
 *   to push lara free only during stand->crouch transition
 */

class LaraShape_Crouch : public btCompoundShape, public IEntityGhostShape
{
public:
    LaraShape_Crouch();
    ~LaraShape_Crouch();
    const btTransform& update(const SSBoneFrame*)
    {
        return m_localOffset;
    }
    btCollisionShape* getShape() { return this; }
protected:
    btTransform m_localOffset;

};


class EntityShape_Skeletal : public btCompoundShape, public IEntityGhostShape
{
public:
    EntityShape_Skeletal(SSBoneFrame *bf);
    ~EntityShape_Skeletal();

    const btTransform& update(const SSBoneFrame *bf);
    btCollisionShape* getShape() { return this; }
protected:
    std::vector<btCollisionShape*> subShapes;
    btTransform m_localOffset;
};



/*
 * =========================================================
 * ==== EntityGhost
 */
extern btDiscreteDynamicsWorld *bt_engine_dynamicsWorld;
/**
 * EntityGhost - GhostObject manager with switchable shapes
 * 1. after creation, EntityGhost should be bound to an entity
 * via setParent() (or during construction).
 * 2. add at least one shape via addShape().
 * 3. call addToWorld() to add the ghost to the physics world.
 */
//class EntityGhost : protected btPairCachingGhostObject
// FIXME: Bullet's overloaded new() allocator doesn't handle proteced base classes...!
class EntityGhost : public btPairCachingGhostObject
{
public:
    EntityGhost()
    {
        setCollisionFlags(getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        getWorldTransform().setIdentity();
        // TODO: should have a dummy shape to reduce misusage problems
    };
    EntityGhost(Entity *parent)   // todo: hierarchy resolve: ent->world->engine
    : EntityGhost()
    {
        setParent(parent);
    }
    virtual ~EntityGhost()
    {
        removeFromWorld();
        for(auto p : m_shapes)
            delete p;
    };

    void setParent(Entity *parent);

    int addShape(IEntityGhostShape* gs);  // return idx

    void selectShape(int idx);

    void update(const btTransform& t, const SSBoneFrame *bf = nullptr);
    void update();

    void addToWorld(btDynamicsWorld *w);
    void removeFromWorld();

    void checkCollision();
    void checkCollision(btTransform& t);

    const btVector3& getLastFixVector() { return m_lastFixVector; };

    // fixme: compat:
    int getPenetrationFixVector(btVector3* reaction);

protected:
    void setCurrentShape(int selected);

    bool recoverFromPenetration();

    Entity *m_parent = nullptr;
    btDynamicsWorld* m_myWorld = nullptr;

    std::vector<IEntityGhostShape*> m_shapes;
    btManifoldArray m_manifoldArray;

    int m_selectedShape = -1;
    int m_currentShape = -1;
    btVector3 m_lastFixVector;

    // int m_collisionflags = 0;
    // ...
};








#endif /* ENTITYGHOST_H_ */
