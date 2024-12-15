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
#define CLIENT_ID 3
#define cls "\e[1;1H\e[2J" // clear console screen

typedef union
{
    struct _pulse pulse;
} my_message_t;

enum states { State0, State1, State2, State3};
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




void *traffic_light_func(void *arg)
{
    enum states *currentState = (enum states *)arg;

    while (1)
    {
        printf("Handling Train Signal\n");
        pthread_mutex_lock(&state_mutex);

        // Forced state change based on the force header value
        switch (force_header)
        {
            case 0:
                *currentState = State0;
                break;
            case 6:  // Train incoming
                printf("TRAIN INCOMING\n");
                *currentState = State1;
                break;
            case 7:  // Boom gate error
                printf("ERROR WITH BOOM GATES\n");
                *currentState = State2;
                break;
            default:
                printf("Unknown force signal. Defaulting to State0.\n");
                *currentState = State0;
                break;
        }

        // Clear the force signal after handling
        force_signal = 0;

        pthread_mutex_unlock(&state_mutex);

        // Notify the client thread that the state has changed
        pthread_cond_signal(&state_change_cond);

        sleep(1);  // Simulate delay between state changes
    }

    return NULL;
}







void trafficLight(enum states *currentState, timer_t timer_id)
{
	pthread_cond_signal(&state_change_cond);
    switch (*currentState)
    {
        case State0:
            printf("State (0): LEVEL CROSSING CLEAR: BOOM GATES: UP... 	: LIGHTS OFF");
            gpio_write(0, 0);
            gpio_write(1, 0);
            gpio_write(2, 0);
            gpio_write(3, 0);
            gpio_write(4, 0);
            gpio_write(5, 0);
            *currentState = State1;
            break;
        case State1:
            printf("State (1): INCOMING TRAIN: BOOM GATES: DOWN...	 	: LIGHTS ON");
            gpio_write(0, 1);
			gpio_write(1, 1);
			gpio_write(2, 1);
			gpio_write(3, 1);

            *currentState = State2;
            break;
        case State2:
            printf("State (2): INCOMING TRAIN: BOOM GATES: ERROR...     : LIGHTS OFF TRAIN EMERGENCY STOP");
            gpio_write(0, 0);
			gpio_write(1, 0);
			gpio_write(2, 0);
			gpio_write(3, 0);
			gpio_write(4, 1);
			gpio_write(5, 1);
            *currentState = State3;
            break;
    }



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
        ts.tv_sec += 3;  // 3-second timeout for sending the state
        pthread_cond_timedwait(&state_change_cond, &state_change_mutex, &ts);

        // After waking up (state change or timeout), send the state to the server
        pthread_mutex_lock(&state_mutex);
		msg.trafficLightState = *currentState;
		pthread_mutex_unlock(&state_mutex);



        printf("Client (ID:%d), sending state: %d\n", msg.ClientID, msg.trafficLightState);

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

//            if (reply.hdr.subtype != 0)
//				{
//					pthread_mutex_lock(&input_mutex);
//					force_header = reply.hdr.subtype;  // Capture the forced state header
//					force_signal = 1;  // Set the force signal
//					pthread_cond_broadcast(&input_cond);
//					pthread_mutex_unlock(&input_mutex);
//				}
        }

        pthread_mutex_unlock(&state_change_mutex);
    }

    return NULL;
}



void *sim_thread_func(void *arg)
{
    traffic_data msg;
    traffic_reply reply;
    msg.ClientID = CLIENT_ID;  // Unique number for this client
    msg.hdr.type = 0x22;
    msg.hdr.subtype = 0x00;

    enum states *currentState = (enum states *)arg;
    int server_coid;

    printf("  ---> Trying to connect to server named: %s\n", SIM_ATTACH_POINT);

    while (1)
    {
        while ((server_coid = name_open(SIM_ATTACH_POINT, 0)) == -1)
        {
            printf("\n    ERROR: Could not connect to server!\nRETRYING\n");
            sleep(3);  // Wait for 3 seconds before retrying
        }

        pthread_mutex_lock(&state_change_mutex);

        // Wait for state change signal or timeout
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 3;  // 3-second timeout for sending the state
        pthread_cond_timedwait(&state_change_cond, &state_change_mutex, &ts);

        // After waking up (state change or timeout), send the state to the server
        pthread_mutex_lock(&state_mutex);
		msg.trafficLightState = *currentState;
		pthread_mutex_unlock(&state_mutex);




        printf("Client (ID:%d), sending state: %d\n", msg.ClientID, msg.trafficLightState);

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
            else
            {
            	pthread_mutex_lock(&input_mutex);
            	force_header = 0;
            	pthread_cond_broadcast(&input_cond);
				pthread_mutex_unlock(&input_mutex);
            }
        }

        pthread_mutex_unlock(&state_change_mutex);
    }

    return NULL;
}


int init_GPIO(void)
{
    if (gpio_init() != 0) {
        printf("GPIO initialization failed.\n");
        return -1;
    }

    // Initialize all pins (from 0 to 27) as outputs and turn them off
    for (int pin = 0; pin <= 27; pin++) {
        gpio_set_output(pin);    // Set as output
        gpio_write(pin, 0);      // Turn off the pin
    }

    return 0;
}
int main(int argc, char *argv[])
{

    printf("Boom Gate machine with server communication\n");
    if (init_GPIO() != 0)
    {
            printf("GPIO initialization failed.\n");
            return -1;
    }
    enum states CurrentState = State0;

    // Create a client thread to send the state to the server
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, client_thread_func, &CurrentState);
    pthread_t traffic_light_thread;
    pthread_create(&traffic_light_thread, NULL, traffic_light_func, &CurrentState);
    pthread_t simthread;
    pthread_create(&simthread, NULL, sim_thread_func, &CurrentState);
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
