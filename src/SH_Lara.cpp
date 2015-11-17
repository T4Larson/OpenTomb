/*
 * SH_Lara.cpp
 *
 *  Created on: 12.11.2015
 *      Author: T4Larson
 */

#include "SH_Lara.h"

#include "anim_state_control.h"

#include <initializer_list>

class SH_Lara : public EntityStateHandler
{
public:
    SH_Lara(Entity *pEnt)
    : EntityStateHandler(pEnt)
    {
        registerState(TR_STATE_LARA_WALK_FORWARD, new walk);
        registerState(TR_STATE_LARA_RUN_FORWARD,  new run);
        registerState(TR_STATE_LARA_STOP,         new stand);
    }

    // dev: service methods for state rules:
    struct state : EntityStateHandler::state
    {
        void nextState(int stid) { return; }
        template<typename T>
        bool isInList(const T& needle, std::initializer_list<T> list)
        {
            for(auto e : list) {
                if(e == needle) return true;
            }
            return false;
        }

        bool changeState(int state, std::initializer_list<bool> list)
        {
            for(auto e : list) {
                if(e == false) return false;
            }
            nextState(state);
            return false;
        }

        bool key(int) { return true; };
        int envtype() { return 0; };
/*

*/
    };


    // ======================
    // State Implementations:




    /* **********************************
     * TR_STATE_LARA_STOP (2)
     *
     * Possible Anim Statechange Transitions (per TR4 Anim Data):
     * (some of these are switched by interaction code)
     *    0 : WALKING
     *    1 : RUNNING
     *    5*: BACKWD_HOP
     *    6*: TURN_RIGHT
     *    7*: TURN_LEFT
     *    8 : FALL_DEATH
     *   15*: JUMP_UP_HIGH
     *   16*: WALK_BACK
     *   21*: SIDESTEP_RIGHT
     *   22*: SIDESTEP_LEFT
     *   28 : JUMP_FLY_UP
     *   38 : PUSHABLE_GRAB
     *   39 : QUICK_PICKING_UP_ITEM
     *   40 : SWITCHING_ON
     *   41 : SWITCHING_OFF
     *   42 : STAND_USE_TURN_KEY
     *   43 : STAND_INSERT_PUZZLE
     *   56 : WALL_CLIMB_IDLE
     *   65*: WATER_WADE
     *   67 : KNEEL_PICKING_UP
     *   70 : SLIDING_DEATHSLIDE
     *   71*: DUCKING
     *   90 : ROPE_TURN_LEFT
     *   98 : GRAB_INTO_WALL_HOLE
     *   99 : BAR_CLIMB_IDLE
     *
     *  Anim/State changes without anim transition:
     *   45 : ROLL_FORWARD (Anim 146)
     */
    struct stand : state
    {
        void action(EntityStateHandler* sh)
        {
            // check skid anims:
//            if(isInList(curAnim(), {226,228}))
            {
                // SkidanimStuff
            }
//            if(useInvItem()) return;

            if(changeState(
                    TR_STATE_LARA_STOP,  // back to self after intermission-anim
                    {
                        key(2), // ROLL
                        envtype() != 4,
                    }))
            {
                //setAnimState(TR_STATE_LARA_ROLL_FORWARD, 146);   // State, Anim, Frame=0
                return;
            }

            if(changeState(
                    TR_STATE_LARA_CROUCH_IDLE,
                    {
                        // Conditions:
                        key(1), // duck
                        envtype() != 4,
                        // currentState() == 2, // !? only when called as substate from elsewhere...avoid?
                        // ( actionstate() == 0 || isInList(currWpn(), {0, Pistol, Revolver, Uzi, Flare}),
                    }))
                return;

            // checkLook();

            /*
                nextState(TR_STATE_LARA_WALK_LEFT);
                nextState(TR_STATE_LARA_WALK_RIGHT);
                nextState(TR_STATE_LARA_TURN_LEFT_SLOW);
                nextState(TR_STATE_LARA_TURN_RIGHT_SLOW);
                nextState(TR_STATE_LARA_JUMP_PREPARE);

                callSubState(TR_STATE_LARA_WADE_FORWARD);
                callSubState(TR_STATE_LARA_WALK_BACK);

                nextState(TR_STATE_LARA_RUN_BACK);  // backward_hop
             */
#if 0

            // Psudocode derived from TR4 behaviour:

            // check skid anims:
            if(isInList(curAnim(), {226,228}))
            {
                // SkidanimStuff
            }
            if(useInvItem()) return;

            if(key(ROLL) && envtype() != 4)
            {
                setAnimState(TR_STATE_LARA_ROLL_FORWARD, 146);   // State, Anim, Frame=0
                nextState(TR_STATE_LARA_STOP);
                return;
            }

            if(    key(DUCK)
                && envtype() != 4
                // && currentState() == 2 ... huh same state?
                && ( actionstate() == 0
                     || isInList( currWpn(), {0, Pistol, Revolver, Uzi, Flare})
                    )
                )
            {
                nextState(TR_STATE_LARA_CROUCH_IDLE);
                return;
            }

            // nextState(2);  // => same state...?

            if(key(LOOK))
            {
                checkLookUpDown();
            }


            if(key(UP))
            {
                v_fwdfloor = scanFloor(fwd,104);
            }
            else if(key(DOWN))
            {
                v_bkwdfloor = scanFloor(bkwd,104);
            }


            if(key(STEP_LEFT))
            {
                floordiff = scanFloorDiff(left,116);
                ceildiff  = scanCeilDiff (left,116,762);
                if( floordiff < 128 && floordiff > -128
                    && floorTilt() != 2
                    && ceildiff <= 0
                    )
                {
                    nextState(TR_STATE_LARA_WALK_LEFT);
                }
                // =>
                if(checkColl(dirvec, maxTilt, floorMin, floorMax, ceilMin)) // use shape? minfloor?
                { nextState(TR_STATE_LARA_WALK_LEFT); }
            }
            else if(key(STEP_RIGHT))
            {
                floordiff = scanFloorDiff(right,116);
                ceildiff  = scanCeilDiff (right,116,762);
                if( floordiff < 128 && floordiff > -128
                    && floorTilt() != 2
                    && ceildiff <= 0
                    )
                {
                    nextState(TR_STATE_LARA_WALK_RIGHT);
                }

            }
            else if(key(LEFT))
            {
                nextState(TR_STATE_LARA_TURN_LEFT_SLOW);
            }
            else if(key(RIGHT))
            {
                nextState(TR_STATE_LARA_TURN_RIGHT_SLOW);
            }



            if(envtype() == ENV_SHALLOW) // 4
            {
                if(key(JUMP))
                {
                    nextState(TR_STATE_LARA_JUMP_PREPARE);
                }

                // v_fwd/bkwdfloor depend on previouse if(UP) !!!
                // i.e., they might have been correlated in the original rule definition...
                if(key(UP))
                {
                    if( v_fwdfloor >= 383
                        || v_fwdfloor <= -383)
                    {
                        /*
                         * Check Wall Climb:
                            wLaraFwdMoveAngle = pItem->wRotation_Y;
                            collflags = pCollInfo->wBitFlags;
                            pCollInfo->dIN_FloorLowest = 32512;
                            pCollInfo->dIN_FloorHighest = -384;
                            pCollInfo->dIN_CeilingMin = 0;
                            pCollInfo->wBitFlags = collflags & 0xfffd | 1;
                            pCollInfo->dFwdRadius = 102;
                            callCollBufCheck(pItem, pCollInfo);
                         */
                        collbufcheck();
                        if ( !checkDoLaraWallClimbAction(apItem, pCollInfo) )
                        {
                            pCollInfo->dFwdRadius = 100;
                        }

                    }
                    else
                    {
                        callSubState(TR_STATE_LARA_WADE_FORWARD);
                    }
                }
                else if(key(DOWN)
                        && v_bkwdfloor < 383
                        && v_bkwdfloor > -383
                        )
                {
                    callSubState(TR_STATE_LARA_WALK_BACK);
                }
            }
            else    // not shallow: (ground)
            {
                if(key(JUMP))
                {
                    nextState(TR_STATE_LARA_JUMP_PREPARE);  // same above...

                }
                else if(key(UP))
                {
                    ...
                }
                else if(key(DOWN))
                {
                    if(key(SHIFT_WALK))
                    {
                        if( v_bkwdfloor < 383
                            && v_bkwdfloor > -383
                            && floorTilt() != 2
                            )
                        {
                            callSubState(TR_STATE_LARA_WALK_BACK);
                        }
                    }
                    else
                    {
                        if( v_bkwdfloor > -383 )
                        {
                            nextState(TR_STATE_LARA_RUN_BACK);  // backward_hop
                        }
                    }
                }
            }
#endif
        }
    };

    struct walk : state
    {

    };


    struct run : state
    {

    };





};


// TODO: See about how to tie this to the Model (BoneFrame) later...
EntityStateHandler* createStateHandler_Lara(Entity* pEnt)
{
    return new SH_Lara(pEnt);
}



