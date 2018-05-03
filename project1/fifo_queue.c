// Node of a Double linked list
typedef struct Node {
	void* data;
	struct Node* next;
	struct Node* prev;
} Node;

// The usable double linked list interface. This internally manages the doubly linked list node.
typedef struct List{
	struct Node* head;
	struct Node* tail;
	int size;
} List;

typedef enum {
    CHAR,
    INT16,
} TYPE;

List* getDoublyLinkedList(){
	// initialize buffer to queue up user inputs
	List* L = malloc(sizeof(List));
	L->head = NULL;
	L->tail = NULL;
	L->size = 0;
	return L;
}

// Used by Q_push().
void appendLeft(List* L, void* data, size_t data_size, TYPE type_t){
	// createa node
	Node* n = malloc(sizeof(Node));

	// copy data into node->data; cast them to correct datatype manually defined by enum TYPE
	n->data = malloc(data_size);
	switch(type_t){
		case CHAR:
		    *(char *)(n->data) = *(char*)data;
		    break;
		case INT16:
		    *(int16_t *)(n->data) = *(int16_t*)data;;
		    break;
	}

	// Set the list's head pointer and tail pointer to the first and last nodes, respectively.
	n->next = L->head;
	n->prev = NULL;
	L->head = n;
	L->size++;
	if(n->next==NULL){
		L->tail=n;
	}else{
		(n->next)->prev = n;
	}
}


// Used by Q_pop(). User of this method must manage free()ing the Node.
Node* popRight(List *L){
	if(L->size > 0){
		Node* lastNode = (L->tail);
		//int16_t result = (L->tail)->data;

		if(L->size==1){
			L->head = NULL;
			//free(L->tail);
			L->tail = NULL;
			lastNode->next=NULL;
		}else{
			L->tail = (L->tail)->prev;
			//free( (L->tail)->next );
			(L->tail)->next = NULL;
		}
		L->size--;
		lastNode->prev = NULL;
		return lastNode;
	}
	return NULL;
}

void printInterpolatorQueue(List *L){
	int i = 0;
	Node* it = L->head;
	printf("List: ");
	while ( i<(L->size) ){
		int16_t* result = (int16_t *)(it->data);
		printf("%d ", *result);
		it = it->next;
		i++;
	}
	printf("\r\n");
}

void Q_push(List* Q, void* data, size_t data_size, TYPE type_t){
	appendLeft(Q, data, data_size, type_t);
}

int16_t Q_pop_int16t(List* Q){
	Node* n = popRight(Q);
	if (n!=NULL){
		int16_t result = *(int16_t *)(n->data);
		free(n->data);
		free(n);
		return result;
	}
	return NULL;
}