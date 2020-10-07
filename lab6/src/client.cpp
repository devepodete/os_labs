#include <zmq.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "structures.hpp"

namespace addr{
    int client_puller_address;
    int stock_puller_address;
}

namespace help{
    zmq::context_t *context;
    bool work;
}

std::ostream &operator<<(std::ostream &os, const node_value_t &nvt){
    os << "{"
    << nvt.id << ", " << nvt.own_puller_address << ", "
    << nvt.child_puller_address << ", " << nvt.own_return_address << ", "
    << nvt.parent_return_address << ", " << nvt.parent_ping_address << ", "
    << nvt.child_ping_address << ", " << "(" << nvt.has_child << ", "
    << nvt.parent_is_client << ")" << ", " << nvt.client_node_address << ", "
    << nvt.client_stock_address << "}";

    return os;
}


class MainNode{
public:

    MainNode(){}

    //created, i, j, pos_type
    std::tuple<bool, int, int, int> create(int new_node_id, int parent_node_id){
        if(new_node_id < 0){
            std::cerr << "Error: server node must has positive id" << std::endl;
            return std::make_tuple(false, -1, -1, -1);
        }
        if(new_node_id == parent_node_id){
            std::cerr << "Error: new and parent nodes must have different id" << std::endl;
            return std::make_tuple(false, -1, -1, -1);
        }
        
        bool new_node_exist = check_id_exist(new_node_id);
        if(new_node_exist){
            std::cerr << "Error: Already exist" << std::endl;
            return std::make_tuple(false, -1, -1, -1);
        }
        node_value_t new_node;
        new_node.id = new_node_id;
        new_node.own_puller_address = generate_new_port();
        new_node.own_return_address = generate_new_port();
        new_node.child_ping_address = generate_new_port();
        new_node.client_node_address = addr::client_puller_address;
        new_node.client_stock_address = addr::stock_puller_address;

        if(parent_node_id == -1){
            id_set.insert(new_node.id);
            new_node.child_puller_address = 0;
            new_node.parent_return_address = addr::client_puller_address;
            new_node.parent_ping_address = 0;
            new_node.has_child = false;
            new_node.parent_is_client = true;

            v.push_back(std::vector<node_value_t>(1, new_node));

            return std::make_tuple(true, v.size()-1, 0, pos::BEGIN);
        }else{
            std::tuple<bool, int, int> parent_search_res = find_node(parent_node_id);
            bool parent_was_found = std::get<0>(parent_search_res);
            int i = std::get<1>(parent_search_res);
            int j = std::get<2>(parent_search_res);
            if(parent_was_found){
                id_set.insert(new_node.id);
                new_node.parent_return_address = v[i][j].own_return_address;
                new_node.parent_ping_address = v[i][j].child_ping_address;
                new_node.parent_is_client = false;
                v[i][j].child_puller_address = new_node.own_puller_address;
                if(j == v[i].size()-1){
                    //parent is last element of vector
                    v[i][j].has_child = true;

                    new_node.child_puller_address = 0;
                    new_node.has_child = false;
                    
                    v[i].push_back(new_node);
                    return std::make_tuple(true, i, j+1, pos::END);
                }else{
                    //parent is not last element of vector 
                    new_node.child_puller_address = v[i][j+1].own_puller_address;
                    new_node.has_child = true;
                    
                    v[i][j+1].parent_return_address = new_node.own_return_address;
                    v[i][j+1].parent_ping_address = new_node.child_ping_address;

                    v[i].insert(v[i].begin()+j+1, new_node);
                    return std::make_tuple(true, i, j+1, pos::MIDDLE);
                }

            }else{
                //parent not found
                std::cerr << "Error: parent node not found" << std::endl;
                return std::make_tuple(false, -1, -1, -1);
            }
        }
    }

    void remove(zmq::socket_t &pusher, zmq::socket_t &puller, int node_id){
        std::tuple<bool, int, int> finded = find_node(node_id);
        if(std::get<0>(finded)){
            int i = std::get<1>(finded);
            int j = std::get<2>(finded);
            
            node_value_t node = get_element(i, j);
            zmq::message_t msg_for_node(sizeof(send_to_node_t));
            zmq::message_t to_receive(sizeof(receive_from_node_t));
            send_to_node_t msg;
            msg.node_id_to_execute = node_id;
            msg.command_type = cmd::EXIT;

            memcpy(msg_for_node.data(), (void*)&msg, sizeof(send_to_node_t));

            std::string to_connect = "tcp://127.0.0.1:" +
                std::to_string(get_element(i, 0).own_puller_address);
            if(j == 0){
                //remove first element
                if(v[i].size() == 1){
                    //only 1 element in vector
                    pusher.connect(to_connect);
                    pusher.send(msg_for_node);
                    pusher.disconnect(to_connect);

                    v.erase(v.begin()+i);
                }else{
                    //several elements in vector
                    zmq::message_t msg_for_child(sizeof(send_to_node_t));
                    send_to_node_t msg;
                    msg.node_id_to_execute = v[i][j+1].id;
                    msg.command_type = cmd::CHANGE_STATE;

                    v[i][j+1].parent_return_address = addr::client_puller_address;
                    v[i][j+1].parent_ping_address = 0;
                    v[i][j+1].parent_is_client = true;

                    msg.nvt = v[i][j+1];

                    memcpy(msg_for_child.data(), (void*)&msg, sizeof(send_to_node_t));

                    std::string to_connect = "tcp://127.0.0.1:" +
                        std::to_string(get_element(i, 0).own_puller_address);
                    
                    pusher.connect(to_connect);
                    pusher.send(msg_for_child);
                    
                    int received_res = puller.recv(&to_receive);
                    
                    pusher.send(msg_for_node);
                    received_res = puller.recv(&to_receive);

                    pusher.disconnect(to_connect);

                    v[i].erase(v[i].begin());
                }
            }else if(j == v[i].size()-1){
                //remove last element
                node_value_t parent = v[i][j-1];

                zmq::message_t msg_for_parent(sizeof(send_to_node_t));
                send_to_node_t msg;
                msg.node_id_to_execute = v[i][j-1].id;
                msg.command_type = cmd::CHANGE_STATE;

                v[i][j-1].has_child = false;
                msg.nvt = v[i][j-1];

                memcpy(msg_for_parent.data(), (void*)&msg, sizeof(send_to_node_t));

                std::string to_connect = "tcp://127.0.0.1:" +
                    std::to_string(get_element(i, 0).own_puller_address);
                
                pusher.connect(to_connect);
                pusher.send(msg_for_node);
                
                int received_res = puller.recv(&to_receive);
                pusher.send(msg_for_parent);
                received_res = puller.recv(&to_receive);
                
                pusher.disconnect(to_connect);

                v[i].pop_back();

            }else{
                //remove middle element
                node_value_t parent = v[i][j-1];
                node_value_t child = v[i][j+1];

                zmq::message_t msg_for_parent(sizeof(send_to_node_t));
                zmq::message_t msg_for_child(sizeof(send_to_node_t));

                send_to_node_t parent_msg;
                send_to_node_t child_msg;

                parent_msg.node_id_to_execute = v[i][j-1].id;
                child_msg.node_id_to_execute = v[i][j+1].id;

                parent_msg.command_type = cmd::CHANGE_STATE;
                child_msg.command_type = cmd::CHANGE_STATE;
            
                v[i][j-1].child_puller_address = v[i][j+1].own_puller_address;
                v[i][j+1].parent_return_address = v[i][j-1].own_return_address;
                v[i][j+1].parent_ping_address = v[i][j-1].child_ping_address;

                parent_msg.nvt = v[i][j-1];
                child_msg.nvt = v[i][j+1];

                memcpy(msg_for_parent.data(), (void*)&parent_msg, sizeof(send_to_node_t));
                memcpy(msg_for_child.data(), (void*)&child_msg, sizeof(send_to_node_t));

                std::string to_connect = "tcp://127.0.0.1:" +
                    std::to_string(get_element(i, 0).own_puller_address);
                
                pusher.connect(to_connect);
                pusher.send(msg_for_child);
                
                int received_res = puller.recv(&to_receive);
                pusher.send(msg_for_node);
                received_res = puller.recv(&to_receive);

                pusher.send(msg_for_parent);
                received_res = puller.recv(&to_receive);

                pusher.disconnect(to_connect);

                v[i].erase(v[i].begin()+j);
            }
            id_set.erase(node_id);
        }else{
            std::cerr << "Error: node not found" << std::endl;
        }

    }

    bool wait_answer(zmq::socket_t &puller){
        zmq::message_t receive(sizeof(receive_from_node_t));
        int receive_res = puller.recv(&receive);
        if(receive_res == 0){
            std::cerr << "Error: node is unavailable" << std::endl;
            return false;
        }else{
            receive_from_node_t *received_msg = (receive_from_node_t*)receive.data();
            std::cout << received_msg->str << std::endl;
            return true;
        }
    }

    bool wait_silent_answer(zmq::socket_t &puller){
        zmq::message_t receive(sizeof(receive_from_node_t));
        int receive_res = puller.recv(&receive);
        if(receive_res == 0){
            return false;
        }else{
            return true;
        }
    }

    void print_vector(){
        for(int i = 0; i < v.size(); i++){
            std::cout << i << ": ";
            for(int j = 0; j < v[i].size(); j++){
                std::cout << v[i][j].id << " ";
            }
            std::cout << std::endl;
        }
    }

    void print_vector_full(){
        for(int i = 0; i < v.size(); i++){
            std::cout << i << ": ";
            for(int j = 0; j < v[i].size(); j++){
                std::cout << v[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    node_value_t get_element(int i, int j){
        return (v.at(i)).at(j);
    }

    bool check_id_exist(int node_id){
        return id_set.count(node_id) > 0;
    }

    void exec_add(int node_id, std::string key, int value){
        std::tuple<bool, int, int> find_res = find_node(node_id);

        if(std::get<0>(find_res)){
            zmq::socket_t pusher(*help::context, ZMQ_PUSH);
            zmq::message_t msg(sizeof(send_to_node_t));
            
            send_to_node_t to_send;
            to_send.node_id_to_execute = node_id;
            to_send.command_type = cmd::EXEC_ADD;
            memcpy(to_send.exec_key, key.c_str(), key.length()+1);
            to_send.exec_value = value;
                        
            memcpy(msg.data(), (void*)&to_send, sizeof(send_to_node_t));

            std::string addr = "tcp://127.0.0.1:" + 
                std::to_string(v[std::get<1>(find_res)][0].own_puller_address);

            pusher.connect(addr);
            pusher.send(msg);
            pusher.disconnect(addr);
        }else{
            std::cerr << "Error: node not found" << std::endl;
        }
    }

    void exec_find(int node_id, std::string key){
        std::tuple<bool, int, int> find_res = find_node(node_id);

        if(std::get<0>(find_res)){
            zmq::socket_t pusher(*help::context, ZMQ_PUSH);
            zmq::message_t msg(sizeof(send_to_node_t));
            
            send_to_node_t to_send;
            to_send.node_id_to_execute = node_id;
            to_send.command_type = cmd::EXEC_FIND;
            memcpy(to_send.exec_key, key.c_str(), key.length()+1);
     
            memcpy(msg.data(), (void*)&to_send, sizeof(send_to_node_t));

            std::string addr = "tcp://127.0.0.1:" + 
                std::to_string(v[std::get<1>(find_res)][0].own_puller_address);

            pusher.connect(addr);
            pusher.send(msg);
            pusher.disconnect(addr);
        }else{
            std::cerr << "Error: node not found" << std::endl;
        }
    }

    std::tuple<bool, int, int> find_node(int node_id){
        if(id_set.count(node_id) > 0){
            for(int i = 0; i < v.size(); i++){
                for(int j = 0; j < v[i].size(); j++){
                    if(v[i][j].id == node_id){
                        return std::make_tuple(true, i, j);
                    }
                }
            }
        }
        return std::make_tuple(false, -1, -1);
    }

    void exit(zmq::socket_t &puller){
        zmq::socket_t pusher(*help::context, ZMQ_PUSH);
        zmq::message_t msg(sizeof(send_to_node_t));
    
        send_to_node_t to_send;
        to_send.command_type = cmd::EXIT;

        for(int i = 0; i < v.size(); i++){
            for(int j = v[i].size()-1; j >= 0; j--){ 
                to_send.node_id_to_execute = v[i][j].id;
                
                memcpy(msg.data(), (void*)&to_send, sizeof(send_to_node_t));

                std::string to_connect = "tcp://127.0.0.1:" + std::to_string(v[i][0].own_puller_address);
                pusher.connect(to_connect);
                pusher.send(msg);
                pusher.disconnect(to_connect);

                zmq::message_t received_msg;
                int res = puller.recv(&received_msg);
                if(res == 0){
                    std::cerr << "Error while exiting: can not send command to " << v[i][j].id << std::endl;
                }else{
                    std::cout << "Exit received by " << v[i][j].id << std::endl;
                }
            }
        }
    }

private:
    std::vector<std::vector<node_value_t>> v;
    std::set<int> id_set;
    std::set<int> address_set;

    //split string by spaces
    std::vector<std::string> parse_string(const std::string &str){
        std::vector<std::string> res;
        int last_space = -1;
        for(int i = 0; i < str.size(); i++){
            if(str[i] == ' '){
                res.push_back(str.substr(last_space+1, i-last_space));
                last_space = i;
            }else if(i == str.size()-1){
                res.push_back(str.substr(last_space+1, i-last_space));
            }
        }
        return res;
    }

    int generate_new_port(){
        int res;
        int k = 0;
        do{
            res = 10000+std::rand()%10000;
            k++;
        }while( (address_set.count(res) != 0) && (k < 10000)
            && (res != addr::client_puller_address) && (res != addr::stock_puller_address));
        if(k == 10000){
            std::cerr << "Error: Can not generate free port" << std::endl;
            throw int();
        }
        address_set.insert(res);
        return res;
    }

};

void print(const std::string &str, std::filebuf &fb, bool use_custom_output){
    if(use_custom_output){
        if(!fb.is_open()){
            std::cerr << "Error. OUTPUT_STREAM is closed" << std::endl;
            return;
        }
        std::ostream os(&fb);
        os << str << std::endl;
    }else{
        std::cout << str << std::endl;
    }
    
}

void *stock_function(void *arg){
    zmq::context_t *context = (zmq::context_t*)arg;
    zmq::socket_t stock(*context, ZMQ_PULL);
    stock.setsockopt(ZMQ_RCVTIMEO, MAX_WAITING_TIME_MS/3);

    //trying to reserve some address for stock
    bool binded_stock = false;
    while(!binded_stock){
        try{
            unsigned int socket_number = 10000 + std::rand()%10000;
            std::string s = "tcp://*:" + std::to_string(socket_number);
            stock.bind(s);
            addr::stock_puller_address = socket_number;
            binded_stock = true;
        }catch(...){}
    }
    while(help::work){
        zmq::message_t msg;
        int received = stock.recv(&msg);
        if(received != 0){
            receive_from_node_t *received= (receive_from_node_t*)(msg.data());
            std::cout << received->str << std::endl;
        }
    }

    return nullptr;
}


int main(int argc, char *argv[]){
    std::cout << "Warning: maximum length of key value is set by 200" << std::endl;

    std::srand(static_cast<unsigned int> (std::time(0)));

    std::filebuf fb;
    bool use_custom_output = false;

    if(argc == 1){
        std::cout << "Default OUTPUT_STREAM is set by std::cout\n";
    }else if(argc == 2){
        fb.open(argv[1], std::ios::out);
        if(!fb.is_open()){
            std::cerr << "Error. Can not open OUTPUT_STREAM.\n";
            return 1;
        }
        use_custom_output = true;
    }else{
        std::cerr << "Error. Wrong number of arguments.\n";
        std::cerr << "Usage: " << argv[0] << " [OUTPUT_STREAM FOR SERVER RESPONSE]\n";
        return 1;
    }

    const char *server_name = "server.out";
    std::srand( static_cast<unsigned int>(std::time(0)) );
    
    zmq::context_t context(1);
    help::context = &context;
    help::work = true;

    zmq::socket_t pusher(context, ZMQ_PUSH);

    zmq::socket_t puller(context, ZMQ_PULL);
    puller.setsockopt(ZMQ_RCVTIMEO, MAX_WAITING_TIME_MS);

    //trying to reserve some address for client
    bool binded_puller = false;
    while(!binded_puller){
        try{
            unsigned int socket_number = 10000 + std::rand()%10000;
            std::string s = "tcp://*:" + std::to_string(socket_number);
            puller.bind(s);
            addr::client_puller_address = socket_number;
            binded_puller = true;
        }catch(...){}
    }
    
    pthread_t stock_worker;
    int create_thread_res = pthread_create(&stock_worker, USE_NO_THREAD_ATTRIBUTES,
                                            stock_function, (void*)(&context));
    if(create_thread_res != 0){
        std::cerr << "Error. Stock thread can not be created.\n";
        std::cerr << "Error number: " << create_thread_res << std::endl;
        return 1;
    }

    MainNode mn;
    std::string action("");
    
    while(action != "exit"){
        std::cin >> action;
        if(action == "create"){
            //std::cout << "Create action" << std::endl;
            int new_node_id, parent_node_id;
            std::cin >> new_node_id;
            std::cin >> parent_node_id;
            std::tuple<bool, int, int, int> create_result = mn.create(new_node_id, parent_node_id);
            if(std::get<0>(create_result)){
                //node was created in vector
                int i_id = std::get<1>(create_result);
                int j_id = std::get<2>(create_result);
                send_to_node_t info_for_node;

                info_for_node.nvt = mn.get_element(i_id, j_id);
                info_for_node.exec_key[0] = '\0';
                info_for_node.exec_value = 0;

                pos_t position = std::get<3>(create_result);

                if(position == pos::BEGIN){
                    //no need to send any sockets
                }else if(position == pos::MIDDLE || position == pos::END){
                    zmq::message_t receive;
                    std::string addr_to_push = "tcp://127.0.0.1:" + 
                        std::to_string(mn.get_element(i_id, 0).own_puller_address);
                    if(position == pos::MIDDLE){
                        zmq::message_t change_child_config(sizeof(send_to_node_t));
                        node_value_t new_child_node_config = mn.get_element(i_id, j_id+1);
                        send_to_node_t temp_message;
                        temp_message.node_id_to_execute = new_child_node_config.id;
                        temp_message.command_type = cmd::CHANGE_STATE;
                        temp_message.nvt = new_child_node_config;

                        memcpy(change_child_config.data(), (void*)&temp_message, sizeof(send_to_node_t));
                        
                        pusher.connect(addr_to_push);
                        pusher.send(change_child_config);
                        pusher.disconnect(addr_to_push);


                        bool received_res = mn.wait_answer(puller);
                        if(!received_res){
                            mn.remove(pusher, puller, new_node_id);
                            continue;
                        }
                    }

                    zmq::message_t change_parent_config(sizeof(send_to_node_t));
                    node_value_t new_parent_node_config = mn.get_element(i_id, j_id-1);
                    
                    send_to_node_t temp_message;
                    temp_message.node_id_to_execute = new_parent_node_config.id;
                    temp_message.command_type = cmd::CHANGE_STATE;
                    temp_message.nvt = new_parent_node_config;
                    
                    memcpy(change_parent_config.data(), (void*)&temp_message, sizeof(send_to_node_t));

                    pusher.connect(addr_to_push);
                    pusher.send(change_parent_config);
                    pusher.disconnect(addr_to_push);

                    bool received_res = mn.wait_silent_answer(puller);
                    if(!received_res){
                        continue;
                    }                   
                }

                int fork_res = fork();
                if(fork_res < 0){
                    std::cerr << "Fatal error: Can not fork" << std::endl;
                    exit(1);
                }else if(fork_res == 0){
                    //child
                    int executed = execl(server_name, 
                        std::to_string(info_for_node.nvt.id).c_str(),
                        std::to_string(info_for_node.nvt.own_puller_address).c_str(),
                        std::to_string(info_for_node.nvt.child_puller_address).c_str(),
                        std::to_string(info_for_node.nvt.own_return_address).c_str(),
                        std::to_string(info_for_node.nvt.parent_return_address).c_str(),
                        std::to_string(info_for_node.nvt.parent_ping_address).c_str(),
                        std::to_string(info_for_node.nvt.child_ping_address).c_str(),
                        std::to_string(info_for_node.nvt.has_child).c_str(),
                        std::to_string(info_for_node.nvt.parent_is_client).c_str(),
                        std::to_string(info_for_node.nvt.client_node_address).c_str(),
                        std::to_string(info_for_node.nvt.client_stock_address).c_str(),
                        nullptr
                        );
                }else{
                    //parent
                    std::cout << "OK: " << fork_res << std::endl;
                }
            }
            //mn.print_vector_full();
        }else if(action == "exec_find"){
            int node_id;
            std::string key;

            std::cin >> node_id >> key;
            if(key.length()+1 > 200){
                std::cerr << "Error: length of key must not be bigger than 201" << std::endl;
                continue;
            }
            mn.exec_find(node_id, key);
            bool received_res = mn.wait_silent_answer(puller);
            if(!received_res){
                continue;
            }
        }else if(action == "exec_add"){
            //std::cout << "Exec add action" << std::endl;
            int node_id;
            std::string key;
            int value;

            std::cin >> node_id >> key >> value;
            if(key.length()+1 > 200){
                std::cerr << "Error: length of key must not be bigger than 201" << std::endl;
                continue;
            }

            mn.exec_add(node_id, key, value);
            bool received_res = mn.wait_silent_answer(puller);
            if(!received_res){
                continue;
            }
        
        }else if (action == "ping"){
            int node_id;
            std::cin >> node_id;
            
                zmq::message_t msg(sizeof(send_to_node_t));
                send_to_node_t stnt;
                stnt.node_id_to_execute = node_id;

                memcpy(msg.data(), (void*)&stnt, sizeof(send_to_node_t));

                std::tuple<bool, int, int> find_node = mn.find_node(node_id);

                if(std::get<0>(find_node)){
                    int to_send = mn.get_element(std::get<1>(find_node), 0).own_puller_address;
                    std::string ping_address = "tcp://127.0.0.1:" + std::to_string(to_send);

                    pusher.connect(ping_address);
                    pusher.send(msg);
                    pusher.disconnect(ping_address);
                    bool received_res = mn.wait_answer(puller);
                    if(!received_res){
                        continue;
                    }
                }else{
                    std::cout << "Error: node not found" << std::endl;
                }
        }else if(action == "print"){
            mn.print_vector();
        }else if(action == "print_full"){
            mn.print_vector_full();
        }else if(action == "remove"){
            int node_id;
            std::cin >> node_id;
            mn.remove(pusher, puller, node_id);
        }else if(action == "exit"){
            mn.exit(puller);
            help::work = false;
        }else{
            std::cout << "Error: incorrect command" << std::endl;
        }
    }

    std::cout << "Exit complete" << std::endl;
    pthread_join(stock_worker, NULL);
    std::cout << "Stock complete" << std::endl;

    if(fb.is_open()){
        fb.close();
    }

    return 0;
}