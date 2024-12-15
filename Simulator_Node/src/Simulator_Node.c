#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define ATTACH_POINT "SIM"  // Server's attach point
#define BUF_SIZE 100
#define cls "\e[1;1H\e[2J" // clear console screen
#define MAX_CLIENTS 10     // Define a maximum number of clients
#define HEARTBEAT_TIMEOUT 5  // Time in seconds before we consider a client disconnected
#define HEARTBEAT_INTERVAL 2  // Time interval for sending heartbeats (client)
int train_active = 0;  // Set to 1 when "train" is entered
timer_t train_timer;  // Timer to handle the 5-second reset


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

enum states { State0, State1, State2, State3, State4, State5, State6, State7, State8, State9, State10, State11, State12, State13 };

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

// Structure to store each client's current state
typedef struct
{
    int ClientID;
    enum states trafficLightState;
    bool is_active;
    time_t last_heartbeat;  // Track the time of the last received heartbeat
} client_info;

// Thread arguments structure
typedef struct
{
    int rcvid;
    traffic_data msg;
    name_attach_t *attach;
} thread_args_t;

typedef struct
{
    char input[BUF_SIZE];
    int forced_red_value;
} InputAction;


InputAction action_table[] =
{
     {"r", 5},
    {"train", 6},
	{"error", 7},
	{"11d",8},
	{"11e",9},
	{"12d",10},
	{"12e",11},
	{"13d",12},
	{"13e",13},
	{"14d",14},
	{"14e",15},
	{"21d",16},
	{"21e",17},
	{"22d",18},
	{"22e",19},
	{"23d",20},
	{"23e",21},
	{"24d",22},
	{"24e",23},

};



// Shared state and protection mechanism
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t state_cond = PTHREAD_COND_INITIALIZER;
client_info client_states[MAX_CLIENTS];  // Array to store client states
int forced_red = 0;   // True if forced to red, False if released

// Function to print all client states at once
void print_all_clients()
{


        printf(cls);  // Clear the screen
        printf("Connected Clients:\n");

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_states[i].is_active)
            {
                int state = client_states[i].trafficLightState;

                // Check if ClientID is 2 to use Old Aerodrome Road/Sturt Street names
                if (client_states[i].ClientID == 2)
                {
                    printf("\nIntersection 2: State: %d\nLast Heartbeat: %ld seconds ago\n",

                           state,
                           time(NULL) - client_states[i].last_heartbeat);
                }
                else if(client_states[i].ClientID == 1)
                {
                    printf("\nIntersection 1: State: %d\nLast Heartbeat: %ld seconds ago\n",

                           state,
                           time(NULL) - client_states[i].last_heartbeat);
                }
                else if(client_states[i].ClientID == 3)
				{
					// Just print the state number for Client 3
					printf("\nTrain Controller: State: %d\nLast Heartbeat: %ld seconds ago\n",

						   state,
						   time(NULL) - client_states[i].last_heartbeat);
				}
            }
        }


    }

// Thread function for handling heartbeats
void *heartbeat_thread(void *arg)
{
    while (1)
    {
        sleep(1);
        time_t current_time = time(NULL);

        pthread_mutex_lock(&state_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_states[i].is_active && (current_time - client_states[i].last_heartbeat > HEARTBEAT_TIMEOUT))
            {
                printf("Client %d has timed out. Marking as disconnected.\n", client_states[i].ClientID);
                client_states[i].is_active = false;
            }
        }
        pthread_mutex_unlock(&state_mutex);
        print_all_clients();


    }
    return NULL;
}

void reset_train_state(union sigval arg)
{
    pthread_mutex_lock(&state_mutex);
    forced_red = 0;
    train_active = 0;  // Reset the train state
    printf("5 seconds elapsed. Train state reset to no incoming train.\n");
    pthread_mutex_unlock(&state_mutex);
    print_all_clients();  // Refresh the client states
}


// Thread function for handling client communication
void *client_handler(void *arg)
{
    thread_args_t *targs = (thread_args_t *)arg;
    traffic_reply replymsg;
    int local_forced_red;




    pthread_mutex_lock(&state_mutex);
    local_forced_red = forced_red;
    int received_state = targs->msg.trafficLightState;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_states[i].ClientID == targs->msg.ClientID || !client_states[i].is_active)
        {
            client_states[i].ClientID = targs->msg.ClientID;
            client_states[i].trafficLightState = targs->msg.trafficLightState;
            client_states[i].is_active = true;
            client_states[i].last_heartbeat = time(NULL);  // Update heartbeat time
            break;
        }
    }
    pthread_mutex_unlock(&state_mutex);

    // Send response
    if (local_forced_red!=0)
	{
		replymsg.hdr.subtype = local_forced_red;  // Indicate the need to hold the state
		sprintf(replymsg.buf, "Simulator got state: %d", received_state);
		MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));
	}
	else
	{
		replymsg.hdr.subtype = 0;
		sprintf(replymsg.buf, "Simulator got state: %d", received_state);
		MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));

	}


      // Refresh client states on screen




    free(targs);  // Free the argument structure
    return NULL;
}



void start_train_timer() {
    struct sigevent sev;
    struct itimerspec its;

    // Set up the timer to call reset_train_state after 5 seconds
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = &train_timer;
    sev.sigev_notify_function = reset_train_state;
    sev.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_REALTIME, &sev, &train_timer) == -1) {
        perror("Failed to create timer");
        exit(EXIT_FAILURE);
    }

    // Set the timer for 5 seconds
    its.it_value.tv_sec = 10;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(train_timer, 0, &its, NULL) == -1) {
        perror("Failed to set timer");
        exit(EXIT_FAILURE);
    }
}

void *input_handler(void *arg)
{
    char input[BUF_SIZE];
    int new_forced_red;

    while (1) {
        printf("Enter input: ");
        fgets(input, BUF_SIZE, stdin);

        // Remove newline character from input if it exists
        input[strcspn(input, "\n")] = 0;

        // Look up the forced red value based on input
        new_forced_red = lookup_forced_red_value(input);

        if (new_forced_red != -1) {
            printf("Input %s selected. Forced red set to %d.\n", input, new_forced_red);
        } else {
            printf("INVALID INPUT\n");
            continue;
        }

        // Lock the state mutex, update the forced_red variable if it has changed, and unlock
        pthread_mutex_lock(&state_mutex);
        if (forced_red != new_forced_red) {
            forced_red = new_forced_red;
            pthread_cond_broadcast(&state_cond);  // Notify other threads that the state has changed

            // If the input is "train", start the 5-second timer
            if (new_forced_red == 6)
            {  // Assuming 6 corresponds to the "train" state
                train_active = 1;
                start_train_timer();  // Start/reset the 5-second timer
            }
        }
        pthread_mutex_unlock(&state_mutex);
    }

    return NULL;
}



int lookup_forced_red_value(char *input)
{
    int table_size = sizeof(action_table) / sizeof(action_table[0]);

    // Iterate through the table to find a matching input
    for (int i = 0; i < table_size; i++)
    {
        if (strcmp(input, action_table[i].input) == 0)
        {
            return action_table[i].forced_red_value;  // Return the corresponding value
        }
    }
    return -1;  // Return -1 if the input is invalid
}




// Main thread for receiving client messages
int main(int argc, char *argv[])
{
    name_attach_t *attach;
    traffic_data msg;

    printf("Server starting, waiting for clients...\n");

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_states[i].is_active = false;
    }

    if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL)
    {
        printf("Failed to attach to name: %s\n", ATTACH_POINT);

        return EXIT_FAILURE;
    }

    printf("Server Listening on %s\n", ATTACH_POINT);

    // Create heartbeat thread
    pthread_t input_thread;
    pthread_t heartbeat_thread_id;
    pthread_create(&heartbeat_thread_id, NULL, heartbeat_thread, NULL);
    pthread_create(&input_thread, NULL, input_handler, NULL);

    int rcvid;
    pthread_t client_thread;
    while (1)
    {
        thread_args_t *targs = malloc(sizeof(thread_args_t));
        if (targs == NULL)
        {
            perror("Failed to allocate memory for thread args");
            continue;
        }

        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1)
        {
            perror("MsgReceive failed");
            free(targs);
            continue;
        }

        targs->rcvid = rcvid;
        targs->msg = msg;
        targs->attach = attach;

        if (pthread_create(&client_thread, NULL, client_handler, (void *)targs) != 0)
        {
            perror("pthread_create failed");
            free(targs);
        } else {
            pthread_detach(client_thread);
        }
    }

    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
