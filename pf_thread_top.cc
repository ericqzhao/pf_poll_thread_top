//
//  client.cc
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <httplib.h>
#include <iostream>
#include <pthread.h>
#include <nlohmann/json.hpp>

using nlohmann::json;
using namespace std;

std::mutex g_thread_lock;

struct thread_stat {
	std::string name;
	uint64_t tid;
	uint64_t last_busy;
	uint64_t busy;
	uint64_t idle;
	uint64_t last_idle;
};

void from_json(const json& j, thread_stat& p) {
	j.at("name").get_to(p.name);
	j.at("tid").get_to(p.tid);
	j.at("busy").get_to(p.busy);
	j.at("idle").get_to(p.idle);
}

#define MAX_THREAD_NUM 256

struct thread_stat g_thread_infos[MAX_THREAD_NUM];

bool inited_g_thread_infos = false;

void get_thread_data() {
	httplib::Client cli("http://127.0.0.1:49181");
	if (auto res = cli.Post("/api?op=get_thread_stats","{\"op\":\"get_thread_stats\"}","application/json")) {
		//cout << res->status << endl;
		//cout << res->get_header_value("Content-Type") << endl;
		//cout << res->body << endl;
		nlohmann::json j = nlohmann::json::parse(res->body);
		auto threads = j.get<std::vector<thread_stat>>();   

		std::lock_guard<std::mutex> lock{ g_thread_lock };
		if (!inited_g_thread_infos) {
			for (int i = 0; i < threads.size(); i++) {
				g_thread_infos[i].name = threads[i].name;
				g_thread_infos[i].tid = threads[i].tid;
				g_thread_infos[i].busy = threads[i].busy;
				g_thread_infos[i].idle = threads[i].idle;
				g_thread_infos[i].last_busy = 0;
				g_thread_infos[i].last_idle = 0;
				
			}
			inited_g_thread_infos = true;
		}
		for (int i = 0; i < threads.size(); i++) {
			for (int j = 0; j < MAX_THREAD_NUM; j++) {
				if (g_thread_infos[j].tid == threads[i].tid) {
					g_thread_infos[j].last_busy = g_thread_infos[j].busy;	
					g_thread_infos[j].last_idle = g_thread_infos[j].idle;
					g_thread_infos[j].busy = threads[i].busy;
					g_thread_infos[j].idle = threads[i].idle;
				}	
			}
			//cout << "thread: " << threads[i].name << " tid: " << threads[i].tid << " busy: " << threads[i].busy << " idle: \n" << threads[i].idle << endl;
		}

	} else {
		cout << "error code: " << res.error() << std::endl;
	}
}

void* refresh_thread_data(void* arg) 
{ 
    // detach the current thread 
    // from the calling thread 
    pthread_detach(pthread_self()); 
  
    printf("Inside the thread\n"); 
  
    while(1) {
	get_thread_data();
	usleep(100000);
    }
 
    // exit the current thread 
    pthread_exit(NULL); 
} 
  
void show_thread_data() {
	std::lock_guard<std::mutex> lock{ g_thread_lock };
	if (!inited_g_thread_infos) {
		return;
	}
	for (int i = 0; i < MAX_THREAD_NUM; i++) {
		auto busy = g_thread_infos[i].busy - g_thread_infos[i].last_busy;
		auto idle = g_thread_infos[i].idle - g_thread_infos[i].last_idle;
		if (busy == 0 && idle == 0) {
			continue;
		}
		float percent = (float)busy / (float)(busy + idle);
		std::string result = std::to_string(percent * 100) + "%";
		cout << "thread: " << g_thread_infos[i].name << " tid: " << g_thread_infos[i].tid << " cpu: " << result << endl;
	}
}

void* refresh_show_data(void* arg) {
    // detach the current thread 
    // from the calling thread 
    pthread_detach(pthread_self()); 
  
    printf("Inside the thread\n"); 
  
    while(1) {
	show_thread_data();
	usleep(100000);
    }
 
    // exit the current thread 
    pthread_exit(NULL); 
}

void start_thread() 
{ 
    pthread_t ptid; 
    pthread_t ptid2; 
  
    // Creating a new thread 
    pthread_create(&ptid, NULL, &refresh_thread_data, NULL); 
    pthread_create(&ptid2, NULL, &refresh_show_data, NULL); 
    printf("This line may be printed"
           " before thread terminates\n"); 
  
    // The following line terminates 
    // the thread manually 
    // pthread_cancel(ptid); 
  
    // Compare the two threads created 
    if(pthread_equal(ptid, pthread_self())) 
        printf("Threads are equal\n"); 
    else
        printf("Threads are not equal\n"); 
  
    // Waiting for the created thread to terminate 
    pthread_join(ptid, NULL); 
    pthread_join(ptid2, NULL); 
  
    printf("This line will be printed"
           " after thread ends\n"); 
  
    pthread_exit(NULL); 
} 

int main(void) {
    start_thread();
    return 0;
}
