#pragma once

typedef unsigned pos_t;
typedef unsigned cmd_t;

const pthread_attr_t *USE_NO_THREAD_ATTRIBUTES = NULL;
const int MAX_WAITING_TIME_MS = 1000;
const int PING_INTERVAL_TIME_S = 3;

namespace cmd{
	enum {PING, EXEC_ADD, EXEC_FIND, CHANGE_STATE, EXIT, JUNK};
}

namespace pos{
	enum {BEGIN, MIDDLE, END};
}

struct node_value_t{
    int id;
    int own_puller_address;
    int child_puller_address;
    int own_return_address;
    int parent_return_address;
    int parent_ping_address;
    int child_ping_address;
    
    bool has_child;
    bool parent_is_client;

    int client_node_address;
    int client_stock_address;
};

struct send_to_node_t{
	int node_id_to_execute;
	cmd_t command_type;
	node_value_t nvt;

	char exec_key[200];
	int exec_value;
};

struct receive_from_node_t{
	cmd_t command_type;
	char str[300];
};

struct worker_add_t{
	char key[200];
	int val;
};

struct worker_find_t{
	char key[200];
};

struct send_to_worker_add_t{
	zmq::context_t *context;
	worker_add_t w_info;
};

struct send_to_worker_find_t{
	zmq::context_t *context;
	worker_find_t w_info;
};