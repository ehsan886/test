#include<stdio.h>
#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<queue>
#include<string.h>
#include<sstream>
#include<unistd.h>

using namespace std;

class queue_util {
    public:
    	string name;
        queue<int> * q;
        int size;
        sem_t empty, full;
        pthread_mutex_t mtx;

        queue_util(int n, string nm){
        	//cout << "queue_util creating of size " << n << endl;
            q = new queue<int>();
            //cout << "queue_util created" << endl;
            size = n;
            name = nm;
            sem_init(&empty,0,n);
            sem_init(&full,0,0);
            pthread_mutex_init(&mtx, 0);
        }

        void enqueue(int x){
            sem_wait(&empty);
            pthread_mutex_lock(&mtx);
            q->push(x);
            //print();
            string s = print();
            const char *c =s.c_str();
            printf("%d inserted into queue %s: %s\n", x, name.c_str(), c);
            sem_post(&full);
            pthread_mutex_unlock(&mtx);

        }

        int dequeue(){
            sem_wait(&full);
            pthread_mutex_lock(&mtx);
            int x = q->front();
            q->pop();
            string s = print();
            const char *c =s.c_str();
            printf("%d popped from queue %s: %s\n", x, name.c_str(), c);
            pthread_mutex_unlock(&mtx);
            sem_post(&empty);
            return x;
        }

        int search(int x){
            queue<int> q2 = *q;
            int c=0;
            while(!q2.empty()){
                if(q2.front()==x) c++;
                q2.pop();
            }
            return c;
        
        }
        
        void remove(int x){
        	queue<int> q2 = *q;
        	while(!q->empty()){
        		q->pop();
        	}
        	while(!q2.empty()){
        		int y = q2.front();
        		q2.pop();
        		if(y != x) q->push(y);
        	}
        	
        	string s = print();
            const char *c =s.c_str();
            printf("%d deleted from queue %s: %s\n", x, name.c_str(), c);
        	
        }
        
        string print(){
        	string s;
        	ostringstream o;
        	queue<int> q2 = *q;
            //int c;
            while(!q2.empty()){
            	o << q2.front();
            	o << " ";
                q2.pop();
            }
            s = o.str();
            return s;
        }

};

queue_util * q_ace,  * q_b, * q_d, * q_bd;

pthread_mutex_t mtx_app, mtx_pass;

int approved[100];

int password[100];

void enqueue_util(queue<int> * q, int x){
    q->push(x);
}

int dequeue_util(queue<int> * q){
    int x = q->front();

    return x;
}

void* studentThread(void * x){
    
    int *i;
    i=(int *)x;
    /*
    string s = "Student thread created by ";
    
    ostringstream o;
    o << *i;
    s += o.str();
    s += "\n";
    cout << s;
    */
    printf("Student Thread #%d\n", *i);
    usleep(1000000);
    while(1){
    	pthread_mutex_lock(&mtx_pass);
    	int pass = password[*i];
    	pthread_mutex_unlock(&mtx_pass);
		q_ace->enqueue(*i);
		while(1){
			q_b->enqueue(*i);
			q_d->enqueue(*i);
			pthread_mutex_lock(&mtx_pass);
			if(pass != password[*i]) {
				printf("Student %d new password %d\n", *i, password[*i]);
				pthread_mutex_unlock(&mtx_pass);	
				break;
			}
    		pthread_mutex_unlock(&mtx_pass);	
			
		}
	}


}

void * aceThread(void * x){
	int *i;
	i=(int *)x;
	char c;
	if(*i==0) c='A';
	else if(*i==1) c= 'C';
	else c= 'E';
	//cout << "Teacher thread created by " << c << endl;
	printf("Teacher thread created by %c\n", c);
	usleep(1000000);
	
	
	
	while(1){
		int y= q_ace->dequeue();
		printf("Teacher %c dequeued %d\n", c, y);
		printf("Teacher %c enqueue-ing %d\n", c, y);
		q_bd->enqueue(y);
		usleep(1000000);
	}
	
}

void * bThread(void * x){
	
	printf("Teacher thread created by B\n");
	while(1){
		int y= q_b->dequeue();
		int n = q_bd->search(y);
		printf("Teacher B found %d, %d times\n", y, n);
		if(n == 1) {
			q_bd->remove(y);
			pthread_mutex_lock(&mtx_app);
			approved[y]=1;
			pthread_mutex_unlock(&mtx_app);
		}
		usleep(1000000);
	}
}

void * dThread(void * x){
	printf("Teacher thread created by D\n");
	
	while(1){
		int y= q_d->dequeue();
		pthread_mutex_lock(&mtx_app);
		if(approved[y]){
			printf("Teacher D found %d approved\n", y);
			approved[y]=0;
			pthread_mutex_lock(&mtx_pass);
			
			password[y] = rand();
			printf("Teacher D changed password of %d\n", y);
			pthread_mutex_unlock(&mtx_pass);
		}
		pthread_mutex_unlock(&mtx_app);
		usleep(1000000);
		
	}
	
}



int main(){

    int st_c, thr_c;

    cin >> st_c >> thr_c;

	//st_c = 20;
	//thr_c =30;
	
    //pthread_t std_thr[100];

	pthread_t * std_thr;
	
	std_thr = new pthread_t[thr_c];

    int student[100];
    
    int j=thr_c;
    
    pthread_mutex_init(&mtx_app,0);
    pthread_mutex_init(&mtx_pass,0);
   
    /*
    while(j--){
    	student[j] = rand()%4 + 1;
        cout << student[j] << " ";
    }
    
    cout << endl;
	*/
    
    for(int i=0; i<thr_c; i++){
        student[i] =  rand() % st_c + 1;
        cout << student[i] << " ";
    }
    
    cout << endl;
    
    
    
    //cout << "random student initialized" << endl;

	queue_util q(10,"test");
    q_ace = new queue_util(10,"ace");
    q_b = new queue_util(1,"b");
    q_d = new queue_util(1000,"d");
    q_bd = new queue_util(100,"bd");
    
    cout << "queue initialized" << endl;
	
	
	
	int rc;
	
	
	pthread_t * teacher_thr;
	
	teacher_thr = new pthread_t[5];
	
	int t[3] = {0,1,2};
	
	for(int i=0; i<3; i++){
		int  x;
        x = i;
        //cout << "pre pthread_create" << endl;
        rc = pthread_create(&teacher_thr[i],NULL,aceThread,(void *)&t[i]);
        if(rc) cout << "Error Code " << rc << endl;
	}
	
	
	
	
	int x;
	x = 3;
	
	rc = pthread_create(&teacher_thr[3],NULL,bThread,(void *)&x);
    if(rc) cout << "Error Code " << rc << endl;
    
    
    
    int y=4;
    rc = pthread_create(&teacher_thr[4],NULL,dThread,(void *)&x);
    if(rc) cout << "Error Code " << rc << endl;
    
    
	
	
	
	j=thr_c-1;
	
	//cout << "111111111" << endl;
	
	//int x;
	x = 0;
	//cout << "22222222" << endl;
	//rc = pthread_create(&std_thr[0],NULL,studentThread,(void *)x);
	
	/*
	while(j--){
		int * x;
        *x = student[j];
        cout << "pre thread creation" << endl;
        rc = pthread_create(&std_thr[j],NULL,studentThread,(void *)x);
        if(rc) cout << "Error Code " << rc << endl;
	}
	
	j=thr_c;
	while(j--){
		pthread_join(std_thr[j],NULL);
	}
	*/
	
	

    for(int i=0; i<thr_c; i++){
        //int * x;
        //int y = student[i];
        //*x = y;
        //cout << i << " " << student[i] << endl;
        //printf("%d %d\n",i,student[i]);
        rc = pthread_create(&std_thr[i],NULL,studentThread,(void *)&student[i]);
        if(rc) cout << "Error Code " << rc << endl;
    }
    
    void *status;
    
    for(int i=0; i<2; i++){
		pthread_join(teacher_thr[i],NULL);
	}
	
	pthread_join(teacher_thr[3],NULL);
	
	pthread_join(teacher_thr[4],NULL);
    
    for(int i=0; i<thr_c-1; i++){
    	pthread_join(std_thr[i],NULL);
    }
    
    pthread_exit(NULL);
    
    

    /*
    queue<int> * q;
    q = new queue<int>();
    q->push(1);
    enqueue_util(q,10);

    cout << q->size() << endl;

    for(int i=0; i<10; i++){

    }




    cout << q->front() << endl;
    q->pop();
    cout << q->front() << endl;


    */



}
