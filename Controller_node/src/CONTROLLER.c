#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define ATTACH_POINT "CONTROLLER"  // Server's attach point
#define BUF_SIZE 100
#define cls "\e[1;1H\e[2J" // clear console screen
#define MAX_CLIENTS 3     // Define a maximum number of clients
#define HEARTBEAT_TIMEOUT 5  // Time in seconds before we consider a client disconnected
#define HEARTBEAT_INTERVAL 2  // Time interval for sending heartbeats (client)

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
     {"r", 0},
    {"R1", 1},
    {"R2", 2},
    {"R3", 3},
};



// Shared state and protection mechanism
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t state_cond = PTHREAD_COND_INITIALIZER;
client_info client_states[MAX_CLIENTS];  // Array to store client states
int forced_red = 0;   // True if forced to red, False if released
int forced_red_intersection1 = 0;
int forced_red_intersection2 = 0;
// Function to print all client states at once
void print_all_clients()
{
    const char* state_names_high_street[] = {
        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns: \033[31mRED\033[0m",

        "High Street(R1): \033[32mGREEN\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[33mYELLOW\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[32mGREEN\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[33mYELLOW\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[32mGREEN\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[33mYELLOW\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[32mGREEN\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[33mYELLOW\033[0m",

        "High Street(R1): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "High Street Right Turns(R1): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m"
    };


    const char* state_names_aerodrome[] = {
        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns: \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[32mGREEN\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[33mYELLOW\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[32mGREEN\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[33mYELLOW\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[32mGREEN\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[33mYELLOW\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[32mGREEN\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[33mYELLOW\033[0m",

        "Old Aerodrome Road/Sturt Street(R2): \033[31mRED\033[0m\n"
        "Northern Highway(R3): \033[31mRED\033[0m\n"
        "Old Aerodrome Right Turns(R2): \033[31mRED\033[0m\n"
        "Northern Highway Right Turns(R3): \033[31mRED\033[0m"
    };


    const char* state_names_train[] = {
    		"LEVEL CROSSING CLEAR: BOOM GATES: UP... 	: LIGHTS OFF",
			"INCOMING TRAIN: BOOM GATES: DOWN...	 	: LIGHTS ON",
			"INCOMING TRAIN: BOOM GATES: ERROR...     : LIGHTS OFF TRAIN EMERGENCY STOP",

        };

    int local_forced_red;
       time_t last_heartbeat[MAX_CLIENTS];
       bool is_active[MAX_CLIENTS];
       int traffic_light_state[MAX_CLIENTS];
       int client_ids[MAX_CLIENTS];

       // Copy the state data in a mutex-protected block
       pthread_mutex_lock(&state_mutex);

       local_forced_red = forced_red;  // Copy the forced red state

       for (int i = 0; i < MAX_CLIENTS; i++)
       {
           is_active[i] = client_states[i].is_active;
           traffic_light_state[i] = client_states[i].trafficLightState;
           last_heartbeat[i] = client_states[i].last_heartbeat;
           client_ids[i] = client_states[i].ClientID;
       }

       pthread_mutex_unlock(&state_mutex);

       // Print the copied data
       printf(cls);  // Clear the screen
       printf("Connected Clients:\n");

       for (int i = 0; i < MAX_CLIENTS; i++)
       {
           if (is_active[i])
           {
               int state = traffic_light_state[i];

               // Check if ClientID is 2 to use Old Aerodrome Road/Sturt Street names
               if (client_ids[i] == 2)
               {
                   printf("\nIntersection 2:\n%s\nLast Heartbeat: %ld seconds ago\n",
                          state_names_aerodrome[state],
                          time(NULL) - last_heartbeat[i]);
               }
               else if (client_ids[i] == 1)
               {
                   printf("\nIntersection 1: State:\n%s\nLast Heartbeat: %ld seconds ago\n",
                          state_names_high_street[state],
                          time(NULL) - last_heartbeat[i]);
               }
               else if (client_ids[i] == 3)
               {
                   // Just print the state number for Client 3 (Train Controller)
                   printf("\nTrain Controller: State: \n%s\nLast Heartbeat: %ld seconds ago\n",
                          state_names_train[state],
                          time(NULL) - last_heartbeat[i]);
               }
           }
       }

       // Print the forced red state
       printf("\nForced Red State: %s\n", local_forced_red ? "ON" : "OFF");
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

// Thread function for handling client communication
// Thread function for handling client communication
void *client_handler(void *arg)
{
    thread_args_t *targs = (thread_args_t *)arg;
    traffic_reply replymsg;
    int local_forced_red;
    int received_state;

    pthread_mutex_lock(&state_mutex);
    local_forced_red = forced_red;
    int local_received_state = targs->msg.trafficLightState;
    int local_client = targs->msg.ClientID;

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

    // If train (client 3) sends state 1, set flags for both intersections
    if (local_client == 3 && local_received_state == 1)
    {
        forced_red_intersection1 = 1;  // Set flag for intersection 1
        forced_red_intersection2 = 1;  // Set flag for intersection 2
        replymsg.hdr.subtype = 4;
        sprintf(replymsg.buf, "Train coming: %d", local_received_state);
        MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));
    }

    // Check if it's client 1 or client 2, and if their respective forced red flag is set
    else if (local_client == 1 && forced_red_intersection1)
    {
        replymsg.hdr.subtype = 4;  // Forced red for intersection 1
        sprintf(replymsg.buf, "Forced red for intersection 1 due to train");
        MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));
        forced_red_intersection1 = 0;  // Clear the flag after sending
    }
    else if (local_client == 2 && forced_red_intersection2)
    {
        replymsg.hdr.subtype = 4;  // Forced red for intersection 2
        sprintf(replymsg.buf, "Forced red for intersection 2 due to train");
        MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));
        forced_red_intersection2 = 0;  // Clear the flag after sending
    }

    // Normal handling of other messages
    else if (local_forced_red != 0)
    {
        replymsg.hdr.subtype = local_forced_red;  // Indicate the need to hold the state
        sprintf(replymsg.buf, "Controller got state: %d", local_received_state);
        MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));
    }
    else
    {
        replymsg.hdr.subtype = 0;
        sprintf(replymsg.buf, "Controller got state: %d", local_received_state);
        MsgReply(targs->rcvid, EOK, &replymsg, sizeof(replymsg));
    }

    pthread_mutex_unlock(&state_mutex);

    // Refresh client states on screen


    free(targs);  // Free the argument structure
    return NULL;
}


void *input_handler(void *arg)
{
    char input[BUF_SIZE];
    int new_forced_red;

    while (1)
    {

    	printf("Enter input: ");
		fgets(input, BUF_SIZE, stdin);

		// Remove newline character from input if it exists
		input[strcspn(input, "\n")] = 0;

		// Look up the forced red value based on input
		new_forced_red = lookup_forced_red_value(input);

		if (new_forced_red != -1)
		{
			printf("Input %s selected. Forced red set to %d.\n", input, new_forced_red);
		}
		else
		{
			printf("INVALID INPUT\n");
		}

		// Lock the state mutex, update the forced_red variable if it has changed, and unlock
		pthread_mutex_lock(&state_mutex);
		if (forced_red != new_forced_red)
		{
			forced_red = new_forced_red;
			pthread_cond_broadcast(&state_cond);  // Notify other threads that the state has changed
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
