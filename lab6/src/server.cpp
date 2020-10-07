#include <zmq.hpp>
#include <unistd.h>
#include <map>
#include <iostream>
#include <sstream>
#include "structures.hpp"

class CalculusNode;

namespace work{
	CalculusNode *cn;
	bool run;
};

class CalculusNode{
public:
	CalculusNode(int argc, char *argv[]){	
		if(argc < 11){
			std::cerr << "server: Error in node: not enough data to create node" << std::endl;
			throw int();
		}
		sscanf(argv[0], "%d", &_id);
		sscanf(argv[1], "%d", &_own_puller_address);
		sscanf(argv[2], "%d", &_child_puller_address);
		sscanf(argv[3], "%d", &_own_return_address);
		sscanf(argv[4], "%d", &_parent_return_address);
		sscanf(argv[5], "%d", &_parent_ping_address);
		sscanf(argv[6], "%d", &_child_ping_address);
		sscanf(argv[7], "%d", &_has_child);
		sscanf(argv[8], "%d", &_parent_is_client);
		sscanf(argv[9], "%d", &_client_node_address);
		sscanf(argv[10], "%d", &_client_stock_address);
	}

	std::pair<bool, int> get_value(std::string key){
		std::map<std::string, int>::iterator it;
		it = mp.find(key);
		if(it != mp.end()){
			return {true, it->second};
		}else{
			return {false, 0};
		}
	}

	void insert_value(std::pair<std::string, int> p){
		mp.insert(p);
	}

	void change_state(node_value_t nvt){
		_id = nvt.id;
	    _own_puller_address = nvt.own_puller_address;
	    _child_puller_address = nvt.child_puller_address;
	    _own_return_address = nvt.own_return_address;
	    _parent_return_address = nvt.parent_return_address;
	    _parent_ping_address = nvt.parent_ping_address;
	    _child_ping_address = nvt.child_ping_address;
	    
	    _has_child = nvt.has_child;
	    _parent_is_client = nvt.parent_is_client;

	    _client_node_address = nvt.client_node_address;
	    _client_stock_address = nvt.client_stock_address;
	}

	void tie_to_parent(){
		_parent_return_address = _client_node_address;
		_parent_ping_address = 0;
		_parent_is_client = true;
	}

	int get_id(){
		return _id;
	}

	std::string construct_connect_port(int val){
		return std::string(std::string("tcp://127.0.0.1:") + std::to_string(val));
	}
	std::string construct_bind_port(int val){
		return std::string(std::string("tcp://127.0.0.1:") + std::to_string(val));
	}
	std::string get_own_puller(){
		return construct_bind_port(_own_puller_address);
	}
	std::string get_child_puller(){
		return construct_connect_port(_child_puller_address);
	}
	std::string get_own_bind_return(){
		return construct_bind_port(_own_return_address);
	}
	std::string get_own_connect_return(){
		return construct_connect_port(_own_return_address);
	}
	std::string get_parent_return(){
		return construct_connect_port(_parent_return_address);
	}
	std::string get_parent_ping(){
		return construct_bind_port(_parent_ping_address);
	}
	int get_parent_ping_address(){
		return _parent_ping_address;
	}
	std::string get_child_bind_ping(){
		return construct_bind_port(_child_ping_address);
	}
	std::string get_child_connect_ping(){
		return construct_connect_port(_child_ping_address);
	}
	std::string get_client(){
		return construct_connect_port(_client_node_address);
	}
	std::string get_stock(){
		return construct_connect_port(_client_stock_address);
	}
	bool has_child(){
		return _has_child;
	}
	bool parent_is_client(){
		return _parent_is_client;
	}

private:
	std::map<std::string, int> mp;

	int _id;
    int _own_puller_address;
    int _child_puller_address;
    int _own_return_address;
    int _parent_return_address;
    int _parent_ping_address;
    int _child_ping_address;
    
    bool _has_child;
    bool _parent_is_client;

    int _client_node_address;
    int _client_stock_address;
};


void *parent_sender_function(void *arg){
	zmq::context_t *context = (zmq::context_t*)arg;
	
	zmq::socket_t puller(*context, ZMQ_PULL);
	puller.bind(work::cn->get_own_bind_return());
	puller.setsockopt(ZMQ_RCVTIMEO, MAX_WAITING_TIME_MS/1);
	zmq::socket_t pusher(*context, ZMQ_PUSH);

	while(work::run){
		zmq::message_t puller_msg;
		int received = puller.recv(&puller_msg);
		if(received == 0){
			continue;
		}
		receive_from_node_t *msg = (receive_from_node_t*)(puller_msg.data());
		
		if(work::cn->parent_is_client()){
			//check if message must be sended to client or to stock
			if(msg->command_type == cmd::PING){
				std::string to_connect = work::cn->get_client();
				pusher.connect(to_connect);
				pusher.send(puller_msg);
				pusher.disconnect(to_connect);
			}else{
				std::string to_connect = work::cn->get_stock();
				pusher.connect(to_connect);
				pusher.send(puller_msg);
				pusher.disconnect(to_connect);
			}
		}else{
			//message send to parent
			std::string to_connect = work::cn->get_parent_return();
			pusher.connect(to_connect);
			pusher.send(puller_msg);
			pusher.disconnect(to_connect);
		}
	}
	return nullptr;
}

//TODO: make 2 different sockets for ping
void *parent_ping_function(void *arg){
	zmq::context_t *context = (zmq::context_t*)arg;
	
	zmq::socket_t puller(*context, ZMQ_PULL);
	puller.setsockopt(ZMQ_RCVTIMEO, MAX_WAITING_TIME_MS);
	
	
	sleep(1);
	while(work::run){
		if(!work::cn->parent_is_client()){
			int addr = work::cn->get_parent_ping_address();
			std::string binded_str = "tcp://127.0.0.1:" + std::to_string(addr);
			puller.connect(binded_str);

			zmq::message_t msg(1);
			int res = puller.recv(&msg);
			if(res == 0){
				//timeout
				work::cn->tie_to_parent();
			}
		}
		sleep(PING_INTERVAL_TIME_S);
	}
	return nullptr;
}

//TODO: make 2 different sockets for ping
void *child_ping_function(void *arg){
	zmq::context_t *context = (zmq::context_t*)arg;
	
	zmq::socket_t pusher(*context, ZMQ_PUSH);
	pusher.bind(work::cn->get_child_bind_ping());

	while(work::run){
		if(work::cn->has_child()){
			zmq::message_t msg(1);
			zmq::message_t reply;
			pusher.send(reply);
		}
		sleep(PING_INTERVAL_TIME_S);
	}
	return nullptr;
}

void *worker_add(void *arg){
	send_to_worker_add_t *received = (send_to_worker_add_t*)arg; 

	work::cn->insert_value(
		std::make_pair<std::string, int> (std::string(received->w_info.key),
			std::move(received->w_info.val))
		);

	std::string str = "OK:" + std::to_string(work::cn->get_id());

	receive_from_node_t answer;
	answer.command_type = cmd::JUNK;
	memcpy(answer.str, str.c_str(), str.length()+1);

	zmq::message_t message(sizeof(receive_from_node_t));
	memcpy(message.data(), &answer, sizeof(receive_from_node_t));

	zmq::context_t *context = received->context;

	zmq::socket_t temp_socket(*context, ZMQ_PUSH);
	std::string to_connect = work::cn->get_own_connect_return();
	temp_socket.connect(to_connect);
	temp_socket.send(message);
	temp_socket.disconnect(to_connect);

	return nullptr;
}

void *worker_find(void *arg){
	send_to_worker_find_t *received = (send_to_worker_find_t*)arg; 

	std::pair<bool, int> find_res = work::cn->get_value(
		std::string(received->w_info.key)
		);
	std::string str = "OK:" + std::to_string(work::cn->get_id()) + ": ";
	if(find_res.first){
		str += std::to_string(find_res.second);
	}else{
		str += '\'' + std::string(received->w_info.key) + '\'' + " not found";
	}
	if(str.length() > 300){
		std::cerr << "server " << work::cn->get_id()  << ": answer length is bigger than 300" << std::endl;
		throw int();
	}
	receive_from_node_t answer;
	answer.command_type = cmd::JUNK;
	memcpy(answer.str, str.c_str(), str.length()+1);

	zmq::message_t message(sizeof(receive_from_node_t));
	memcpy(message.data(), &answer, sizeof(receive_from_node_t));

	zmq::context_t *context = received->context;

	zmq::socket_t temp_socket(*context, ZMQ_PUSH);
	std::string to_connect = work::cn->get_own_connect_return();
	temp_socket.connect(to_connect);
	temp_socket.send(message);
	temp_socket.disconnect(to_connect);
	return nullptr;
}

int main(int argc, char *argv[]){
	CalculusNode cn(argc, argv);
	work::cn = &cn;
	work::run = true;

	zmq::context_t context(1);
	
	zmq::socket_t own_puller(context, ZMQ_PULL);
	own_puller.bind(cn.get_own_puller());

	zmq::socket_t child_puller(context, ZMQ_PUSH);

	pthread_t parent_return;
	pthread_t parent_ping;
	pthread_t child_ping;
	
	pthread_t tree_worker;
	
	int th1 = pthread_create(
		&parent_return, 
		USE_NO_THREAD_ATTRIBUTES,
		parent_sender_function,
		(void*)(&context)
		);
	int th2 = pthread_create(
		&parent_ping,
		USE_NO_THREAD_ATTRIBUTES,
		parent_ping_function,
		(void*)(&context)
		);
	int th3 = pthread_create(
		&child_ping,
		USE_NO_THREAD_ATTRIBUTES,
		child_ping_function,
		(void*)(&context)
		);

	if(th1 != 0 || th2 != 0 || th3 != 0){
		std::cerr << "server " << work::cn->get_id()  << ": Error in node: can not create thread(s)" << std::endl;
		throw int();
	}

	bool main_run = true;

	while(main_run){
		zmq::message_t from_parent;
		own_puller.recv(&from_parent);
	
		send_to_node_t *msg = (send_to_node_t*)from_parent.data();
		int id_to_execute = msg->node_id_to_execute;
		
		if(id_to_execute == cn.get_id()){
			cmd_t command = msg->command_type;

			zmq::message_t return_ping(sizeof(receive_from_node_t));
			zmq::socket_t return_ping_pusher(context, ZMQ_PUSH);
			
			std::string returner_address = cn.get_parent_return();
			receive_from_node_t msg_ping;
			msg_ping.command_type = cmd::PING;
			std::string sended_string = "OK:" + std::to_string(cn.get_id());
			memcpy((void*)msg_ping.str, (void*)sended_string.c_str() , sended_string.length()+1);

			memcpy(return_ping.data(), (void*)&msg_ping, sizeof(receive_from_node_t));
			
			return_ping_pusher.connect(returner_address);
			return_ping_pusher.send(return_ping);
			return_ping_pusher.disconnect(returner_address);

			switch(command){
				case cmd::PING:{
					//do nothing
					break;
				}
				case cmd::EXEC_ADD:{
					send_to_worker_add_t to_send;
					to_send.context = &context;
					to_send.w_info.val = msg->exec_value;
					memcpy(to_send.w_info.key, msg->exec_key, strlen(msg->exec_key)+1);

					pthread_create(&tree_worker, NULL, worker_add, (void*)&to_send);
					pthread_join(tree_worker, NULL);
					break;
				}
				case cmd::EXEC_FIND:{
					send_to_worker_find_t to_send;
					to_send.context = &context;
					memcpy(to_send.w_info.key, msg->exec_key, strlen(msg->exec_key)+1);

					pthread_create(&tree_worker, NULL, worker_find, (void*)&to_send);
					pthread_join(tree_worker, NULL);
					break;
				}
				case cmd::CHANGE_STATE:{
					cn.change_state(msg->nvt);
					break;
				}
				case cmd::EXIT:{
					work::run = false;
					main_run = false;
					pthread_join(th1, NULL);
					pthread_join(th2, NULL);
					pthread_join(th3, NULL);
					break;
				}
				case cmd::JUNK:{
					std::cerr << "server " << work::cn->get_id()  << ": Error in node: JUNK received" << std::endl;
					throw 1;
					break;
				}
				default:{
					std::cerr << "server " << work::cn->get_id()  << ": Error in node: incorrect command" << std::endl;
					throw 1;
					break;
				}
			};
		}else if(cn.has_child()){
			child_puller.connect(cn.get_child_puller());
			child_puller.send(from_parent);
			child_puller.disconnect(cn.get_child_puller());
		}
	}

	//without sleep crash probably because of some context/sockets does not close at this time
	sleep(2);
	return 0;
}