#pragma once

#define _WIN32_WINDOWS

#include <asio.hpp>
#include <pthread.h>
#include <queue>
#include <map>

#include "Common.h"
#include "Utility.h"
#include "Thread.h"
#include "NetworkUtils.h"

using asio::ip::tcp;

class TWT_Peer {
    private:
        tcp::resolver *resolver;
        tcp::acceptor *acceptor;
        void TWT_CloseSocket(tcp::socket *sock);

    public:
        pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t writeLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t transferLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t closeLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t popLock = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_t awaitLock = PTHREAD_MUTEX_INITIALIZER;
		
        pthread_cond_t gotReadJob = PTHREAD_COND_INITIALIZER;
        pthread_cond_t gotWriteJob = PTHREAD_COND_INITIALIZER;
        pthread_cond_t gotTransfer = PTHREAD_COND_INITIALIZER;
        pthread_cond_t gotCloseJob = PTHREAD_COND_INITIALIZER;
        pthread_cond_t gotAwaitSignal = PTHREAD_COND_INITIALIZER;

        //A single listening thread
        TWT_ListenerThread *listener;
        TWT_CloserThread *closer;

        //Queues that read/write threads work from
        std::queue<TWT_Connection*> connections;
        std::queue<tcp::socket*> pendingClosings;
        // std::queue<tcp::socket*> pendingClosings;
        std::queue<TWT_Packet*> pendingData;

        std::map<std::string,tcp::socket*> addressMap;

        TWT_ThreadPool<TWT_ReadThread> readers;
        TWT_ThreadPool<TWT_WriteThread> writers;
        TWT_ThreadPool<TWT_TransferThread> transferers;


		//Basic information
        int port;

        bool active;
        bool listening;
		
		unsigned int last_packet = 0;
		
		//Filename of file currently being received
		//There's probably a better way to do this...?
		std::string fname;

        int numConnections;

        //Server functionality
        void TWT_Listen(TWT_Thread*);
        void TWT_Listen() {
            if(this->listening) {
                print("Already listening");
                return;
            }
            this->listener->start(this);
        }
		//Await a packet of a specified type
		void TWT_Await(DataType type) {
			this->awaiting = true;
			this->awaitType = type;
			pthread_mutex_lock(&this->awaitLock);
			while(this->active and this->awaiting) {
				pthread_cond_wait(&this->gotAwaitSignal,&this->awaitLock);
			}
			pthread_mutex_unlock(&this->awaitLock);
		}
		bool isAwaiting(DataType type) {
			return(this->awaiting and this->awaitType == type);
		}
		bool awaiting;
		DataType awaitType;

        //Thread routine for handling incoming connections
        void TWT_AwaitReadJob(TWT_Thread*);
        void TWT_ServeConnection(TWT_Connection*,TWT_Thread*);
        DataType HandlePacket(TWT_Connection*,TWT_Packet packet);

        void TWT_Link(tcp::socket*);

        //Client functionality
		void TWT_HandleInput();
        void TWT_SendPacket(TWT_Packet*);
        void TWT_PackageAndSend(const std::string&,tcp::socket *sock,DataType type = TWT_TEXT);
        void TWT_PackageAndSend(std::vector<char> data,const std::string&,DataType type = TWT_TEXT);
        void TWT_PackageAndSend(const std::string &message,const std::string&,DataType type = TWT_TEXT);
		void TWT_PackageAndSendForResponse(const std::string &message,const std::string&,DataType type = TWT_TEXT,DataType returnType = TWT_ACK);
		
        // void TWT_PackageAndSend(std::vector<char> data,tcp::socket* sock);

        bool TWT_Connect(const std::string &host);

        //Thread routine for handling outgoing connections
        void TWT_AwaitWriteJob(TWT_Thread*);
		
		//Thread routine for handling transmissions
        void TWT_AwaitTransfer(TWT_Thread*);

        //Thread routine for handling pending socket closings
        void TWT_AwaitCloseJob(TWT_Thread*);

        //Utility
        /*TODO:
         * Pass TWT_SafePop a queue and the correct lock
         * Currently we use the same lock for all queues
        */
        TWT_Connection* TWT_PopReadQueue();
        tcp::socket* TWT_PopClosingQueue();
        TWT_Packet* TWT_PopWriteQueue();

        void TWT_MarkSocketForClosing(tcp::socket *sock);
        void TWT_MarkSocketForClosing(const std::string &sock_id);

        void TWT_Deactivate() {
            this->active = false;

            //Awaken all threads to tell them we're deactivating
            for(int i=0;i<this->readers.numThreads;++i) pthread_cond_signal(&this->gotReadJob);
            for(int i=0;i<this->writers.numThreads;++i) pthread_cond_signal(&this->gotWriteJob);

            this->readers.stop();
            this->writers.stop();
        }

        TWT_Peer(int _port,int numThreads): port(_port),acceptor(new tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), _port))),resolver(new tcp::resolver(io_context)) {

            this->active = true;
            this->listening = false;

            // this->reading = false;

            pthread_cond_init(&this->gotReadJob,NULL);
            pthread_cond_init(&this->gotWriteJob,NULL);
            pthread_cond_init(&this->gotCloseJob,NULL);
            pthread_cond_init(&this->gotTransfer,NULL);

            pthread_mutex_init(&this->readLock,NULL);
            pthread_mutex_init(&this->writeLock,NULL);
            pthread_mutex_init(&this->closeLock,NULL);
            pthread_mutex_init(&this->popLock,NULL);
            pthread_mutex_init(&this->awaitLock,NULL);

            this->closer = new TWT_CloserThread();
            this->listener = new TWT_ListenerThread();

            this->readers = TWT_ThreadPool<TWT_ReadThread>(numThreads,this);
            this->writers = TWT_ThreadPool<TWT_WriteThread>(numThreads,this);
            this->transferers = TWT_ThreadPool<TWT_TransferThread>(numThreads,this);

            this->closer->start(this);
			
			// this->transferers.start();
            this->readers.start();
            this->writers.start();

            this->numConnections = 0;

            print("Peer initialized on port ",_port);
        }
};
