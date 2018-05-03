typedef struct{
	int (*funptr)();
	int period;
	uint32_t expireTime; // next release time 
	int missedDeadlines;
	int ID;
	int priority;
	int state;		//0=ready, 1=blocking, 2=executing
	int jobsInQueue;
	int qMaxSeen;
} Task;

void setUpATask(
	Task *t,
	int (*f)(),
	int period,
	uint32_t expireTime,
	int missedDeadlines,
	int ID,
	int priority,
	int state,
	int jobsInQueue,
	int qMaxSeen){

	t->funptr = f;
	t->period = period;
	t->expireTime = expireTime;
	t->missedDeadlines = missedDeadlines;
	t->ID = ID;
	t->priority = priority;
	t->state = state;
	t->jobsInQueue = jobsInQueue;
	t->qMaxSeen = qMaxSeen;
}

void releaseTask(Task *t){ int x = t->funptr(); }
void setReady(Task *t){ t->state = 0; }
void setBlocking(Task *t){ t->state = 1; }
void setExecuting(Task *t){ t->state = 2; }
void decrementIfQueued(Task *t){
	if(t->jobsInQueue > 0){ 
		t->jobsInQueue--; 
	} 
}
bool taskIsReadyForRelease(Task *t, uint32_t time_ms){ return time_ms >= (t->expireTime); }
void setTimeOfNextRelease(Task *t){ t->expireTime += t->period; }
bool stateIsReady(Task *t){ return (t->state==0); }
bool stateIsExecuting(Task *t){ return (t->state==2); }
void incrementMissedDeadline(Task *t){ t->missedDeadlines++; }
void incrementQueue(Task *t){ t->jobsInQueue++; }
void updateMaxQueue(Task *t){
	if (t->jobsInQueue > t->qMaxSeen){
		t->qMaxSeen = t->jobsInQueue;
	}
}
Task* getHighestPriorityReadyTask(Task *tArrSorted[], int size){
	volatile int i;
	Task *t;
	for(i=0; i<size; i++){
		t = tArrSorted[i]; // already sorted by priority descending
		if(t->state == 0){
			return t;
		}
	}
	return NULL;
}

void refreshTasksReadiness(Task* tArrSorted[], int size, uint32_t time_ms){
	volatile int i;
	Task *t;

	// For each task, check if any task is ready to be released
	for(i=0; i<size; i++){
		t = tArrSorted[i];
		// if task is ready to be released
		if(taskIsReadyForRelease(t, time_ms)){
			setTimeOfNextRelease(t);
			if(stateIsReady(t) || stateIsExecuting(t)){
				// Task misses a deadline. Increment deadline. Increcrement queue.
				incrementMissedDeadline(t);
				incrementQueue(t);
				updateMaxQueue(t);
			}else{ // state is blocked
				// otherwise set it to ready state
				setReady(t);
			}
		}
	}
}

void runHighestPriorityReadyTask(Task* tArrSorted[], int size){
	Task *t = getHighestPriorityReadyTask(tArrSorted, size);
    if(t){    // if so release it and decrement queue, set state to executing
    	setExecuting(t);
    	decrementIfQueued(t);
    	releaseTask(t);
    	setBlocking(t); //A task that has completed its current job and is waiting for the next release should be in the state "blocked."
    }
}


Task t1, t2, t3, t4;
Task *tArrSorted[4]  = {&t1, &t2, &t3, &t4}; // array of pointers to task