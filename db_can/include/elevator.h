/*
 * elevator.h
 *
 * File to with elevator prototypes
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */

#ifndef _elevator_h_
#define _elevator_h_

#define 	NODE_CAR	0
#define 	NODE_AT_1	1
#define 	NODE_AT_2	2
#define 	NODE_AT_3	3

#define		FLOOR_TO_NODE	0
#define		FLOOR_TO_1	1
#define		FLOOR_TO_2	2
#define		FLOOR_TO_3	3
#define		FLOOR_GO_DOWN	4
#define		FLOOR_GO_UP	5

class Elevator
{
public:
    typedef enum class elevatorState {
        at_1,
        at_2,
        at_3,
        go_up,
        go_dn,
        COND,           // Need to figure this
        First = at_1,
        Last = go_dn
    } state;

    typedef enum class elevatorEvent {
        to_1,
        to_2,
        to_3,
        up_from_1,
        dn_from_2,
        up_from_2,
        dn_from_3,
        First = to_1,
        Last = dn_from_3
    } event;

    int transition(int, int);
    event* translateEvent(int, int);

private:
    state CurState = state::at_3;

    // Transition Table
    const state NextState[(int)state::Last + 1][(int)event::Last + 1] = {
        /* to_1,         to_2,         to_3,         up_from_1,    dn_from_2,    up_from_2,    dn_from_3    */
        {  state::at_1,  state::go_up, state::go_up, state::go_up, state::go_up, state::go_up, state::go_up }, // at_1
        {  state::go_dn, state::at_2,  state::go_up, state::go_dn, state::go_dn, state::go_up, state::go_up }, // at_2
        {  state::go_dn, state::go_dn, state::at_3,  state::go_dn, state::go_dn, state::go_dn, state::go_dn }, // at_3
        {  state::go_up, state::COND,  state::at_3,  state::go_up, state::go_up, state::COND,  state::at_3  }, // go_up
        {  state::at_1,  state::COND,  state::go_dn, state::at_1,  state::COND,  state::go_dn, state::go_dn }  // go_dn
    };
};

#endif

