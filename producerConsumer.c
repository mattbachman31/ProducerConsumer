#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>

typedef struct{
	int hours;
	int profNum;
	int assignmentNum;
	int numStudents;
	int* completedStudents;
}assignment;

typedef struct{
	int ai,wi,Wi,ni,Ni,hi,Hi,pi,si,di,qi;
}firstSend;

typedef struct{
	pthread_mutex_t* buffMutex;
	pthread_mutex_t* profsCanExitMutex;
	pthread_cond_t* condition;
	int profNum;
	int numAssignings;
	int min_num_assignments;
	int max_num_assignments;
	int min_assignment_hours;
	int max_assignment_hours;
	int Wi;
	int wi;
	int studentsPerAssignment;
	int bufferSize;
	assignment* buffer;
	int* bufferWriteIndex;
	int* bufferReadIndex;
	int* numExitedProfs;
	int totalNumProfs;
	int totalNumStudents;
}profStruct;

typedef struct{
	pthread_mutex_t* readMutex;
	pthread_mutex_t* buffMutex;
	int studentNum;
	assignment* buffer;
	int* bufferWriteIndex;
	int* bufferReadIndex;
	int bufferSize;
	int totalNumProfs;
	int* numExitedProfs;
	int* stillGo;
}studentStruct;

void* top_level(void* arg);

void* prof(void* parg);

void* student(void* sarg);

int main(int argc, char** argv){
	char c;
	int aFlag=0,wFlag=0,WFlag=0,nFlag=0,NFlag=0,hFlag=0,HFlag=0,pFlag=0,sFlag=0,dFlag=0,qFlag=0,uFlag=0;
	char *a, *w, *W, *n, *N, *h, *H, *p, *s, *d, *q;
	int ai,wi,Wi,ni,Ni,hi,Hi,pi,si,di,qi;
	while((c = getopt(argc, argv, "a:w:W:n:N:h:H:p:s:d:q:")) != -1){
		switch(c){
			case 'a':
				aFlag = 1;
				a = strdup(optarg);
				break;
			case 'w':
				wFlag = 1;
				w = strdup(optarg);
				break;
			case 'W':
				WFlag = 1;
				W = strdup(optarg);
				break;
			case 'n':
				nFlag = 1;
				n = strdup(optarg);
				break;
			case 'N':
				NFlag = 1;
				N = strdup(optarg);
				break;
			case 'h':
				hFlag = 1;
				h = strdup(optarg);
				break;
			case 'H':
				HFlag = 1;
				H = strdup(optarg);
				break;
			case 'p':
				pFlag = 1;
				p = strdup(optarg);
				break;
			case 's':
				sFlag = 1;
				s = strdup(optarg);
				break;
			case 'd':
				dFlag = 1;
				d = strdup(optarg);
				break;
			case 'q':
				qFlag = 1;
				q = strdup(optarg);
				break;
			default:
				uFlag = 1;
				break;
		}
	}
	if(aFlag){
		ai = atoi(a);
	}
	else{
		ai = 10;
	}
	if(wFlag){
		wi = atoi(w);
	}
	else{
		wi = 1;
	}
	if(WFlag){
		Wi = atoi(W);
	}
	else{
		Wi = 5;
	}
	if(nFlag){
		ni = atoi(n);
	}
	else{
		ni = 1;
	}
	if(NFlag){
		Ni = atoi(N);
	}
	else{
		Ni = 10;
	}
	if(hFlag){
		hi = atoi(h);
	}
	else{
		hi = 1;
	}
	if(HFlag){
		Hi = atoi(H);
	}
	else{
		Hi = 5;
	}
	if(pFlag){
		pi = atoi(p);
	}
	else{
		pi = 2;
	}
	if(sFlag){
		si = atoi(s);
	}
	else{
		si = 2;
	}
	if(dFlag){
		di = atoi(d);
	}
	else{
		di = si;
	}
	if(qFlag){
		qi = atoi(q);
	}
	else{
		qi = 8;
	}
	//error checking
	if(ai > 100000){
		fprintf(stderr, "<num_assignings> too large; max value is 100000. Terminating.\n");
		exit(1);
	}
	if(wi > 10){
		fprintf(stderr, "<min_prof_wait> too large; max value is 10. Terminating.\n");
		exit(1);
	}
	if(Wi > 100){
		fprintf(stderr, "<max_prof_wait> too large; max value is 100. Terminating.\n");
		exit(1);
	}
	if(ni > 10){
		fprintf(stderr, "<min_num_assignments> too large; max value is 10. Terminating.\n");
		exit(1);
	}
	if(Ni > 100){
		fprintf(stderr, "<max_num_assignments> too large; max value is 100. Terminating.\n");
		exit(1);
	}
	if(hi > 5){
		fprintf(stderr, "<min_assignment_hours> too large; max value is 5. Terminating.\n");
		exit(1);
	}
	if(Hi > 10){
		fprintf(stderr, "<max_assignment_hours> too large; max value is 10. Terminating.\n");
		exit(1);
	}
	if(pi > 10){
		fprintf(stderr, "<num_professors> too large; max value is 10. Terminating.\n");
		exit(1);
	}
	if(si > 10){
		fprintf(stderr, "<num_students> too large; max value is 10. Terminating.\n");
		exit(1);
	}
	if(di > 10){
		fprintf(stderr, "<students_per_assignment> too large; max value is 10. Terminating.\n");
		exit(1);
	}
	if(qi > 256){
		fprintf(stderr, "<queue_size> too large; max value is 256. Terminating.\n");
		exit(1);
	}
	if(wi > Wi){
		fprintf(stderr, "<min_prof_wait> must not be larger than <max_prof_wait>. Terminating.\n");
		exit(1);
	}
	if(ni > Ni){
		fprintf(stderr, "<min_num_assignments> must not be larger than <max_num_assignments>. Terminating.\n");
		exit(1);
	}
	if(hi > Hi){
		fprintf(stderr, "<min_assignment_hours> must not be larger than <max_assignment_hours>. Terminating.\n");
		exit(1);
	}
	if(si < di){
		fprintf(stderr, "<students_per_assignment> cannot be larger than <num_students>. Terminating.\n");
		exit(1);
	}
	if(uFlag){
		fprintf(stderr, "Usage: ./program1 [-a <num_assignings>] [-w <min_prof_wait>] [-W <max_prof_wait>] [-n <min_num_assignments>] [-N <max_num_assignments>] [-h <min_assignment_hours>] [-H <max_assignment_hours>] [-p <num_professors>] [-s <num_students>] [-d <students_per_assignment>] [-q <queue_size>]\n");
		exit(1);
	}
	pthread_t top_thread;
	firstSend* arg;
	arg->pi = pi;
	arg->wi = wi;
	arg->Wi = Wi;
	arg->ni = ni;
	arg->Ni = Ni;
	arg->hi = hi;
	arg->Hi = Hi;
	arg->di = di;
	arg->qi = qi;
	arg->si = si;
	arg->ai = ai;
	pthread_create(&top_thread, NULL, top_level, (void*)arg);
	pthread_join(top_thread, NULL);
	return 0;
}

void* top_level(void* arg){
	firstSend* temp = (firstSend*)arg;
	firstSend first = *temp;
	pthread_mutex_t writeMutex;
	pthread_mutex_t profsCanExit;
	pthread_mutex_t readMutex;
	pthread_cond_t condition;
	int init = pthread_mutex_init(&writeMutex, NULL);
	if(init != 0){
		perror("");
		exit(1);
	}
	init = pthread_mutex_init(&profsCanExit, NULL);
	if(init != 0){
		perror("");
		exit(1);
	}
	init = pthread_mutex_init(&readMutex, NULL);
	if(init != 0){
		perror("");
		exit(1);
	}
	init = pthread_cond_init(&condition, NULL);
	if(init != 0){
		perror("");
		exit(1);
	}
	assignment buff[first.qi];
	int writeIndex = 0;
	int readIndex = 0;
	int profExit = 0;
	srand(time(NULL));
	int i;
	int stillGo = 1;
	pthread_t array[first.pi];
	pthread_t array2[first.si];
	profStruct parray[first.pi];
	studentStruct sarray[first.si];
	for(i=1; i<=first.pi; i++){
		parray[i-1].profNum = i;
		parray[i-1].buffMutex = &writeMutex;
		parray[i-1].profsCanExitMutex = &profsCanExit;
		parray[i-1].buffer = buff;
		parray[i-1].bufferReadIndex = &readIndex;
		parray[i-1].bufferWriteIndex = &writeIndex;
		parray[i-1].bufferSize = first.qi;
		parray[i-1].studentsPerAssignment = first.di;
		parray[i-1].numExitedProfs = &profExit;
		parray[i-1].Wi = first.Wi;
		parray[i-1].wi = first.wi;
		//parray[i-1].waitTime = (rand() % (first.Wi-first.wi+1)) + first.wi;
		parray[i-1].numAssignings =  first.ai;
		parray[i-1].min_num_assignments = first.ni;
		parray[i-1].max_num_assignments = first.Ni;
		parray[i-1].min_assignment_hours = first.hi;
		parray[i-1].max_assignment_hours = first.Hi;
		//parray[i-1].numHours = (rand() % (first.Hi - first.hi+1)) + first.hi;
		parray[i-1].totalNumProfs = first.pi;
		parray[i-1].condition = &condition;
		parray[i-1].totalNumStudents = first.si;
		pthread_create(&array[i-1],NULL,prof,(void*)&parray[i-1]);
	}
	for(i=1; i<=first.si; i++){
		sarray[i-1].studentNum = i;
		sarray[i-1].buffer = buff;
		sarray[i-1].bufferSize = first.qi;
		sarray[i-1].bufferWriteIndex = &writeIndex;
		sarray[i-1].bufferReadIndex = &readIndex;
		sarray[i-1].numExitedProfs = &profExit;
		sarray[i-1].totalNumProfs = first.pi;
		sarray[i-1].readMutex = &readMutex;
		sarray[i-1].buffMutex = &writeMutex;
		sarray[i-1].stillGo = &stillGo;
		pthread_create(&array2[i-1],NULL,student,(void*)&sarray[i-1]);
	}
	pthread_cond_wait(&condition, &profsCanExit);
	stillGo--;
	for(i=1; i<= first.si; i++){
		pthread_join(array2[i-1], NULL);
	}
	fprintf(stdout, "Completed Execution!\n");
	pthread_exit(0);
}

void* prof(void* parg){
	profStruct* temp = (profStruct*)parg;
	profStruct arg = *(temp);
	fprintf(stdout,"STARTING Professor %d\n", arg.profNum);
	int assignmentNum;
	int i;
	for(i=0; i<arg.numAssignings; i++){
		sleep((rand() % (arg.Wi-arg.wi+1)) + arg.wi);
		int n = (rand() % (arg.max_num_assignments-arg.min_num_assignments+1)) + arg.min_num_assignments;
		//printf("DHJSDGJSHDGHJSGD%d\n",n);
		int j,k;
		for(j=0; j<n; j++){
			assignmentNum++;
			int hours = (rand() % (arg.max_assignment_hours - arg.min_assignment_hours+1)) + arg.min_assignment_hours;
			pthread_mutex_lock(arg.buffMutex);
			/* critical section */
			while(*(arg.bufferReadIndex) == ((*(arg.bufferWriteIndex) + 1) % arg.bufferSize)); //busy wait
			fprintf(stdout, "ASSIGN Professor %d adding Assignment %d: %d hours.\n",arg.profNum,assignmentNum,hours);
			assignment ass;
			ass.profNum = arg.profNum;
			ass.hours = hours;
			ass.numStudents = arg.studentsPerAssignment;
			ass.assignmentNum = assignmentNum;
			ass.completedStudents = malloc(sizeof(int) * arg.totalNumStudents);
			for(k=0; k<arg.totalNumStudents; k++){
				ass.completedStudents[k] = 0;
			}
			arg.buffer[*(arg.bufferWriteIndex)] = ass;
			(*arg.bufferWriteIndex) = (*(arg.bufferWriteIndex) + 1) % arg.bufferSize;
			//printf("Write Index %d     Read Index %d\n",(*arg.bufferWriteIndex), (*arg.bufferReadIndex));
			pthread_mutex_unlock(arg.buffMutex);
		}
	}
	pthread_mutex_lock(arg.profsCanExitMutex);
	fprintf(stdout, "EXITING Professor %d\n", arg.profNum);
	(*arg.numExitedProfs)++;
	if((*arg.numExitedProfs) == arg.totalNumProfs){
		//fprintf(stdout, "WORKED\n");
		pthread_cond_signal(arg.condition);
	}
	pthread_mutex_unlock(arg.profsCanExitMutex);
	pthread_exit(0);
}

void* student(void* sarg){
	studentStruct* temp = (studentStruct*)sarg;
	studentStruct arg = *(temp);
	fprintf(stdout,"STARTING Student %d\n", arg.studentNum);
	int startHours;
	while((*arg.stillGo) || (*arg.bufferWriteIndex) != (*arg.bufferReadIndex)){
		pthread_mutex_lock(arg.readMutex);
		while((*arg.bufferWriteIndex) == (*arg.bufferReadIndex));
		//critical section
		assignment toDo = arg.buffer[(*arg.bufferReadIndex)];
		if(arg.buffer[(*arg.bufferReadIndex)].completedStudents[arg.studentNum-1] == 0){
			arg.buffer[(*arg.bufferReadIndex)].completedStudents[arg.studentNum-1] = 1;
			fprintf(stdout, "BEGIN Student %d working on Assignment %d from Professor %d\n", arg.studentNum, toDo.assignmentNum, toDo.profNum);
			(arg.buffer[(*arg.bufferReadIndex)].numStudents)--;
			if((arg.buffer[(*arg.bufferReadIndex)].numStudents) == 0){
				free(arg.buffer[(*arg.bufferReadIndex)].completedStudents);
				(*arg.bufferReadIndex) = ((*arg.bufferReadIndex) + 1) % arg.bufferSize;
			}
			startHours = toDo.hours;
			pthread_mutex_unlock(arg.readMutex);
			while(toDo.hours > 0){
				fprintf(stdout, "WORK Student %d working on Assignment %d Hour %d from Professor %d\n", arg.studentNum, toDo.assignmentNum, (startHours - toDo.hours) + 1, toDo.profNum);
				sleep(1);
				toDo.hours--;
			}
		}
		else{
			pthread_mutex_unlock(arg.readMutex);
		}
	}
	fprintf(stdout, "EXITING Student %d\n", arg.studentNum);
	pthread_exit(0);
}
