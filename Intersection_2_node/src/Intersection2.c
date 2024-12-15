#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

// INTERSECTION I1 NODE

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL
#define QNET_ATTACH_POINT "/net/controller/dev/name/local/CONTROLLER"
#define SIM_ATTACH_POINT "/net/sim/dev/name/local/SIM"
#define BUF_SIZE 100
#define CLIENT_ID 2
#define cls "\e[1;1H\e[2J" // clear console screen
#include "gpio_control.h"

typedef union
{
    struct _pulse pulse;
} my_message_t;

enum states { State0, State1, State2, State3, State4, State5, State6, State7, State8, State9, State10, State11, State12, State13 };
enum states *global_state;

typedef struct _Mypulse
{
   _Uint16t type;
   _Uint16t subtype;
   _Int8t code;
   _Uint8t zero[3];
   _Uint32t value;
   _Uint8t zero2[2];
   _Int32t scoid;
} msg_header_t;

typedef struct
{
   msg_header_t hdr;
   int ClientID;
   enum states trafficLightState;
} traffic_data;

typedef struct
{
   msg_header_t hdr;
   char buf[BUF_SIZE];
} traffic_reply;

int force_header = 0;
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t input_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t state_change_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t state_change_cond = PTHREAD_COND_INITIALIZER;
int force_signal = 0;  // Indicates if a forced state change is required

void set_timer_interval(timer_t timer_id, enum states state)
{
    struct itimerspec itime;

    switch (state)
    {
        case State3:
        case State6:
        case State9:
        case State12:
            itime.it_value.tv_sec = 2;
            itime.it_value.tv_nsec = 0;
            itime.it_interval.tv_sec = 2;
            itime.it_interval.tv_nsec = 0;
            break;

        default:
            itime.it_value.tv_sec = 1;
            itime.it_value.tv_nsec = 0;
            itime.it_interval.tv_sec = 1;
            itime.it_interval.tv_nsec = 0;
            break;
    }

    if (timer_settime(timer_id, 0, &itime, NULL) == -1)
    {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}



void *traffic_light_func(void *arg)
{
    enum states *currentState = (enum states *)arg;
    int chid;
    int rcvid;
    my_message_t msg;
    timer_t timer_id;
    struct sigevent event;
    struct sched_param param;

    // Set up timer for the traffic light state machine
    chid = ChannelCreate(0);  // Create a channel for the pulse
    event.sigev_notify = SIGEV_PULSE;
    event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);  // Attach channel to this node
    if (event.sigev_coid == -1)
    {
        perror("ConnectAttach failed");
        exit(EXIT_FAILURE);
    }
    struct sched_param th_param;
    pthread_getschedparam(pthread_self(), NULL, &th_param);
    event.sigev_priority = th_param.sched_curpriority;
    event.sigev_code = MY_PULSE_CODE;

    // Create the timer with pulse event
    if (timer_create(CLOCK_REALTIME, &event, &timer_id) == -1)
    {
        fprintf(stderr, "Couldn't create a timer, errno %d\n", errno);
        perror("timer_create failed");
        exit(EXIT_FAILURE);
    }

    // Set the initial timer interval based on the current state
    set_timer_interval(timer_id, *currentState);

    while (1)
    {
        // Wait for the timer pulse
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == 0 && msg.pulse.code == MY_PULSE_CODE)
        {
            pthread_mutex_lock(&state_mutex);

            if (force_signal)  // If a forced state change is signaled
            {
                printf("Handling forced state change\n");

                // Forced state change based on the force header value
                if (force_signal)  // If a forced state change is signaled
                {
                    printf("Handling forced state change\n");

                    // Forced state change based on the force header value
                    switch (force_header)
                       {
                           case 2:
                               printf("Forcing R2 (Old Areodrome Road / Sturt Street) to green.\n");
                               if (*currentState != State2)  // Ensure it's not already green
                               {
                                   // Cycle current green to yellow first, if needed
                                   *currentState = (*currentState == State5 || *currentState == State10) ? State6 : State3;
                                   printf("Cycling current green to yellow, then forcing R1 to green.\n");
                               }
                               *currentState = State2;  // Set R2 to green
                               break;

                           case 3:  // Forcing R3 (Northern Highway) to green
                               printf("Forcing R3 (Northern Highway) to green.\n");
                               if (*currentState != State5)  // Ensure it's not already green
                               {
                                   // Cycle current green to yellow first, if needed
                                   *currentState = (*currentState == State2 || *currentState == State7) ? State3 : State6;
                                   printf("Cycling current green to yellow, then forcing R3 to green.\n");
                               }
                               *currentState = State5;  // Set R3 to green
                               break;

                           case 4:  // Forcing all lights to red for emergency
                        	   printf("Incoming train: Forcing R2 (Old Areodrome Road / Sturt Street) to green.\n");
                        	   if (*currentState != State2)  // Ensure it's not already green
                        	   {
								  // Cycle current green to yellow first, if needed
                        		   *currentState = (*currentState == State5 || *currentState == State10) ? State6 : State3;
                        		   printf("Cycling current green to yellow, then forcing R1 to green.\n");
                        	   }
                        	   *currentState = State2;  // Set R2 to green
							  break;

                           default:

                               printf("Unknown force signal: %d  Proceeding with normal state transitions.\n", force_header);

                               trafficLight(currentState, timer_id);
                               break;
                       }

                    force_signal = 0;  // Clear the force signal after handling
                }
            }
            else
            {
                // Regular state transitions
                trafficLight(currentState, timer_id);
            }

            // Reset the timer after each state change
            set_timer_interval(timer_id, *currentState);

            // Notify the client thread that the state has changed
            pthread_cond_signal(&state_change_cond);

            pthread_mutex_unlock(&state_mutex);
        }
    }

    timer_delete(timer_id);
    ConnectDetach(event.sigev_coid);
    ChannelDestroy(chid);
    return NULL;
}




void trafficLight(enum states *currentState, timer_t timer_id)
{
	pthread_cond_signal(&state_change_cond);
    switch (*currentState)
    {
        case State0:
            printf("State (0): Old Areodrome Road / Sturt Street: RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State1;
            break;
        case State1:
            printf("State (1): Old Areodrome Road / Sturt Street: RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State2;
            break;
        case State2:
            printf("State (2): Old Areodrome Road / Sturt Street: GREEN, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State3;
            break;
        case State3:
            printf("State (3): Old Areodrome Road / Sturt Street: YELLOW, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State4;
            break;
        case State4:
            printf("State (4): Old Areodrome Road / Sturt Street: RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State5;
            break;
        case State5:
            printf("State (5): Old Areodrome Road / Sturt Street: RED, Northern Highway: GREEN\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State6;
            break;
        case State6:
            printf("State (6): Old Areodrome Road / Sturt Street RED, Northern Highway: YELLOW\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State7;
            break;
        case State7:
            printf("State (7): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State8;
            break;
        case State8:
            printf("State (8): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: GREEN, Northern Highway Right Turns: RED\n\n");
            *currentState = State9;
            break;
        case State9:
            printf("State (9): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: YELLOW, Northern Highway Right Turns: RED\n\n");
            *currentState = State10;
            break;
        case State10:
            printf("State (10): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State11;
            break;
        case State11:
            printf("State (11): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: GREEN\n\n");
            *currentState = State12;
            break;
        case State12:
            printf("State (12): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: YELLOW\n\n");
            *currentState = State13;
            break;
        case State13:
            printf("State (13): Old Areodrome Road / Sturt Street RED, Northern Highway: RED\nOld Areodrome Road / Sturt Street Right Turns: RED, Northern Highway Right Turns: RED\n\n");
            *currentState = State2;
            break;
        default:
            printf("Unknown state!\n");
            break;
    }

    // After printing the state, signal the client thread to send the updated state
    set_timer_interval(timer_id, *currentState);

}

// Thread for client communication
void *client_thread_func(void *arg)
{
    traffic_data msg;
    traffic_reply reply;
    msg.ClientID = CLIENT_ID;  // Unique number for this client
    msg.hdr.type = 0x22;
    msg.hdr.subtype = 0x00;

    enum states *currentState = (enum states *)arg;
    int server_coid;

    printf("  ---> Trying to connect to server named: %s\n", QNET_ATTACH_POINT);

    while (1)
    {
        while ((server_coid = name_open(QNET_ATTACH_POINT, 0)) == -1)
        {
            printf("\n    ERROR: Could not connect to server!\nRETRYING");
            sleep(3);  // Wait for 3 seconds before retrying
        }

        pthread_mutex_lock(&state_change_mutex);

        // Wait for state change signal or timeout
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5;  // 5-second timeout for sending the state
        pthread_cond_timedwait(&state_change_cond, &state_change_mutex, &ts);

        // After waking up (state change or timeout), send the state to the server
        msg.trafficLightState = *currentState;
        printf("Client (ID:%d), sending traffic light state: %d\n", msg.ClientID, msg.trafficLightState);

        if (MsgSend(server_coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
        {
            printf("Error: State '%d' NOT sent to server. Connection lost.\n", msg.trafficLightState);
            name_close(server_coid);  // Close connection and retry
            server_coid = -1;  // Mark server connection as lost
            pthread_mutex_unlock(&state_change_mutex);
            continue;  // Retry sending state when reconnected
        }
        else
        {
            printf("   -->Reply: '%s'\n", reply.buf);

            if (reply.hdr.subtype != 0)
            {
                pthread_mutex_lock(&input_mutex);
                force_header = reply.hdr.subtype;  // Capture the forced state header
                force_signal = 1;  // Set the force signal
                pthread_cond_broadcast(&input_cond);
                pthread_mutex_unlock(&input_mutex);
            }
        }

        pthread_mutex_unlock(&state_change_mutex);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    printf("Traffic lights machine with periodic timers and server communication\n");

    enum states CurrentState = State0;

    // Create a client thread to send the state to the server
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, client_thread_func, &CurrentState);
    pthread_t traffic_light_thread;
    pthread_create(&traffic_light_thread, NULL, traffic_light_func, &CurrentState);
    // Set the initial timer interval
    // Main loop to handle the timer pulses
    while (1)
    {
//        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
//        if (rcvid == 0 && msg.pulse.code == MY_PULSE_CODE)
//        {
//            // Change traffic light state based on the timer
//            trafficLight(&CurrentState, timer_id);
//        }

    }

    // Cleanup

    pthread_join(client_thread, NULL);

    printf("Main terminating...\n");

    return EXIT_SUCCESS;
}
