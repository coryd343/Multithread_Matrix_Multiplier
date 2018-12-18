#include <stdio.h>
#include <stdlib.h>

#define MAX 5
int read = 0, write = 0;
int queue[MAX];

int put(int val) {
    if((read % MAX) != (write + 1) % MAX) {
        printf("Success: Inserting into %d\n", write % MAX);
        queue[write++ % MAX] = val;
        return 1;
    }
    else {
        printf("Error! Cannot insert into %d\n", write % MAX);
        return 0;
    }
}

int get() {
    if((read % MAX) != (write % MAX)) {
        printf("Success: reading from %d\n", read % MAX);
        return queue[read++ % MAX];
    }
    else {
        printf("Error: reading from %d\n", read % MAX);
        exit(0);
    }
}

int main() {
    //Fill all slots, then empty all slots
    /*for(int i = 0; i < MAX - 1; i++) {
        put(10 + i);
    }
    for(int i = 0; i < MAX - 1; i++) {
        printf("Getting %d: %d\n", i,get());
    }*/
    
    //test circular indexing
    for(int i = 0; i < 21; i++) {
        put(10 + i);
        printf("Write: %d\n", write);
        printf("Getting %d: %d\n", i,get());
        printf("Read: %d\n", read);
        
    }
    
    return 0;
}
