/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNCTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/mman.h>
#include<stdbool.h>
#include <sys/stat.h>
#include<sys/fcntl.h>
#include <unistd.h>


//The Class for the Sub branch
/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this
macro to make the output of all system same and conduct a fair evaluation.
*/
#define PAGE_SIZE 4096
#define PROT (PROT_READ | PROT_WRITE)
#define FLAGS (MAP_ANONYMOUS | MAP_PRIVATE)

typedef struct sub_node{
    int chain_ID;
    int Node_ID;
    size_t size;
    unsigned long VA;
    struct sub_node* prev;
    struct sub_node* next;
    char process_type[10];
}sub_node;
 
typedef struct MainNode{
    int ID;
    int num_of_sub_pages;
    size_t size;
    struct MainNode* prev;
    struct MainNode* next;
    sub_node* sn;
}MainNode;

typedef struct Head{
    struct MainNode* f1;
}Head;

typedef struct mapping{
    unsigned long sva;
    unsigned long eva;
    unsigned long spa;
    unsigned long epa;
    struct mapping* next;
    struct mapping* prev;
}mapping;
//the Class for the Main branch
typedef struct MainBranch{
    MainNode* head;
}MainBranch;

MainNode* get_head(MainBranch* main_branch) {
    return main_branch->head;
}

// Setter method for the head pointer of the main branch
void set_head(MainBranch* main_branch, MainNode* new_head) {
    main_branch->head = new_head;
}

typedef struct mapping_list{
    mapping* head;
}mapping_list;

mapping* get_mhead(mapping_list* m) {
    return m->head;
}

// Setter method for the head pointer of the main branch
void set_mhead(mapping_list* m, mapping* new_head) {
    m->head = new_head;
}

//the Class for the Sub branch
typedef struct subBranch{
    sub_node* sn1;
    sub_node* sn_prev;
    sub_node* sn_next;
}subBranch;

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
size_t starting_VA;
//unsigned long vi[][2];
//unsigned long ph[][2];
int nmp =0;
//Function to split a node into a process and a Hole
void split(sub_node* m1, size_t x,MainNode* w){
    sub_node* ns1 = (sub_node*)mmap(NULL, sizeof(sub_node), PROT, FLAGS, -1, 0);
    sub_node* ns2 = (sub_node*)mmap(NULL, sizeof(sub_node), PROT, FLAGS, -1, 0);
    ns1 -> size = x;
    ns2 -> size = m1->size - x;
    ns1 -> chain_ID = m1 -> chain_ID;
    ns2 -> chain_ID = m1 -> chain_ID;
    ns1->VA = m1->VA;
    ns2 -> VA = m1->VA + x;
    strcpy(ns1 -> process_type , "PROCESS");
    strcpy(ns2 -> process_type ,"HOLE");
    if(m1->prev == NULL && m1->next !=NULL){
        w-> sn = ns1;
        ns1->prev = NULL;
        ns1->next = ns2;
        ns2 -> prev = ns1;
        ns2 -> next = m1-> next;
        m1->next = NULL;
        ns2 -> next -> prev = ns2;
        munmap(m1, sizeof(sub_node));
    }
    if(m1->prev == NULL && m1->next == NULL){
        w->sn = ns1;
        ns1 -> prev = NULL;
        ns1->next = ns2;
        ns2->prev = ns1;
        ns2->next = NULL;
        munmap(m1, sizeof(sub_node));
    }else if(m1->prev!=NULL && m1->next==NULL){
        m1->prev->next = ns1;
        ns1 -> prev = m1->prev;
        m1->prev = NULL;
        ns2 -> next = NULL;
        ns1->next =ns2;
        ns2 -> prev = ns1;
        munmap(m1,sizeof(sub_node));
    }else if(m1->prev != NULL && m1->next !=NULL){
        m1->prev->next = ns1;
        ns1 -> prev = m1-> prev;
        m1 -> prev = NULL;
        ns1 -> next = ns2;
        ns2 -> prev = ns1;
        ns2 -> next = m1->next;
        m1->next->prev = ns2;
        m1-> next = NULL;
        munmap(m1,sizeof(sub_node));
    }
}
MainBranch *Free_maintbu;
mapping_list * mltbu;

void mems_init(void){
    //initialising the starting Virtual Address
    starting_VA = 1000;
    MainBranch *Free_main=(MainBranch*)mmap(NULL, sizeof(MainBranch), PROT, FLAGS, -1, 0);
    mapping_list *mapl=(mapping_list*)mmap(NULL, sizeof(mapping_list), PROT, FLAGS, -1, 0);
    //Initialising the head node
    set_head(Free_main, NULL);
    Free_maintbu = Free_main;
    mltbu = mapl;
    set_mhead(mltbu, NULL);

}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void edging(void){
    MainNode* temp = get_head(Free_maintbu);
    while(temp!=NULL){
        sub_node *suby = temp->sn;
        while(suby!=NULL){
            if(strcmp(suby->process_type,"HOLE")==0 && suby->next!=NULL && strcmp(suby->next->process_type,"HOLE")==0 ){
                sub_node *lala = (sub_node*)mmap(NULL, sizeof(sub_node), PROT, FLAGS, -1, 0);
                lala ->next = suby->next;
                suby->next = suby->next->next;
                if(lala->next->next!=NULL){
                    lala ->next ->next->prev = suby;
                }
                lala->next ->next = NULL;
                lala->next -> prev = NULL;
                suby->size += lala->next->size;
                munmap(lala->next,sizeof(sub_node));
            }
            suby=suby->next;
        }
        temp=temp->next;
    }
}
void mems_finish(void){
    
}


/*
Allocates memory of the specified size by reusing a segment from the free list if
a sufficiently large segment is available.

Else, uses the mmap system call to allocate more memory on the heap and updates
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/
void* mems_malloc(size_t size){
    edging();
    //I'll recurse every main node of the free list and check for a sufficiently large HOLE in its subchain.
    bool found = false;
    unsigned long nva = 0;
    MainNode* temp = get_head(Free_maintbu);
    if(get_head(Free_maintbu)==NULL){
        char *mfheap = mmap(0,PAGE_SIZE,PROT,FLAGS,-1,0);
        nmp++;
        if(mfheap == MAP_FAILED){
            printf("ERROR");
            return NULL;
        }
        memset(mfheap,0,PAGE_SIZE);
        sub_node* t = (sub_node*)mmap(NULL,sizeof(sub_node),PROT,FLAGS,-1,0);
        MainNode *n = (MainNode*)mmap(NULL, sizeof(MainNode), PROT, FLAGS, -1, 0);
         if (n == MAP_FAILED) {
             printf("Error");
             exit(EXIT_FAILURE);
         } else {
             n->ID = 1;
             n->num_of_sub_pages = 1;
             n->size = PAGE_SIZE;
             n->prev = NULL;
             n->next = NULL;
             n->sn = t;
             t->chain_ID = n->ID;
             t->Node_ID = 1;
             t->size = PAGE_SIZE;
             t->VA = starting_VA;
             t->prev = NULL;
             t->next = NULL;
             strcpy(n->sn->process_type,"PROCESS");
             split(n->sn, size, n);
         }
        set_head(Free_maintbu, n);
        mapping* m1 = (mapping*)mmap(NULL,sizeof(mapping),PROT,FLAGS,-1,0);
        m1->sva = n->sn->VA;
        m1->eva = n->sn->next->VA -1;
        m1->spa = (unsigned long)(int *)mfheap;
        m1->epa = (unsigned long)(int *)mfheap + size - 1;
        m1->prev = NULL;
        m1->next = NULL;
        set_mhead(mltbu,m1);
        unsigned long virt_addr = m1->sva;
        found = true;
        return (void*)virt_addr;
    }else{
        // sub_node* temp_sub = get_head(Free_maintbu)->sn;
        sub_node* pic = (sub_node*)mmap(NULL,sizeof(sub_node),PROT,FLAGS,-1,0);
        unsigned long via = 0;
        while(temp!=NULL){
            sub_node* temp_sub = temp->sn;
            while(temp_sub!=NULL){
                if(temp_sub->size>size && strcmp(temp_sub->process_type,"HOLE")==0 && temp_sub->prev!=NULL && found == false){
                    pic->next = temp_sub->prev;
                    split(temp_sub,size,temp);
                    via = pic->next->next->VA;
                    mapping *tempm = get_mhead(mltbu);
                    mapping *newmapping = (mapping*)mmap(NULL,sizeof(mapping),PROT,FLAGS,-1,0);
                    while(tempm!=NULL){
                        if(tempm->next==NULL){
                            tempm->next = newmapping;
                            newmapping->prev = tempm;
                            newmapping -> next = NULL;
                            break;
                        }
                        tempm=tempm->next;
                    }
                    newmapping->sva = pic->next->next->VA;
                    newmapping->eva = newmapping->sva + size -1;
                    newmapping->spa = (unsigned long)(void*)pic->next->next;
                    newmapping->epa = (unsigned long)(void*)pic->next->next + size -1;
                    found = true;
                    break;
                }
                else if(temp_sub->size == size && strcmp(temp_sub->process_type, "HOLE")==0 && found == false){
                    strcpy(temp_sub->process_type,"PROCESS");
                    via = temp_sub->VA;
                    mapping *tempm = get_mhead(mltbu);
                    mapping *newmapping = (mapping*)mmap(NULL,sizeof(mapping),PROT,FLAGS,-1,0);
                    while(tempm!=NULL){
                        if(tempm->next==NULL){
                            tempm->next = newmapping;
                            newmapping->prev = tempm;
                            newmapping -> next = NULL;
                            break;
                        }
                        tempm=tempm->next;
                    }
                    newmapping->sva = temp_sub->VA;
                    newmapping->eva = newmapping->sva + size -1;
                    newmapping->spa = (unsigned long)(void*)temp_sub;
                    newmapping->epa = (unsigned long)(void*)temp_sub + size -1;
                    found = true;
                    break;
            }else if(temp_sub->size<size && strcmp(temp_sub->process_type, "HOLE")==0 && temp_sub->next!=NULL){
                continue;
            }else if(temp_sub->next == NULL && (strcmp(temp_sub->process_type, "HOLE")==0 || strcmp(temp_sub->process_type, "PROCESS")==0) && temp_sub->size<size){
                    nva = temp_sub->VA +temp_sub->size - 1;
            }

                    temp_sub = temp_sub->next;
        }
        if(temp_sub==NULL && temp->next == NULL && found==false){
                char *mfheap = mmap(0,PAGE_SIZE,PROT,FLAGS,-1,0);
                nmp++;
                if(mfheap == MAP_FAILED){
                    printf("ERROR");
                    return NULL;
                }
                memset(mfheap,0,PAGE_SIZE);
                MainNode *nmn = (MainNode*)mmap(NULL, sizeof(MainNode), PROT, FLAGS, -1, 0);
                nmn ->ID = temp ->ID+1;
                nmn ->size = PAGE_SIZE;
                temp->next = nmn;
                nmn->prev = temp;
                nmn->next = NULL;
                sub_node *n1 = (sub_node*)mmap(NULL,sizeof(sub_node),PROT,FLAGS,-1,0);
                nmn -> sn = n1;
                n1 -> size = PAGE_SIZE;
                strcpy(n1->process_type,"HOLE");
                n1->prev = NULL;
                n1->next = NULL;
                n1->Node_ID = nmn->ID;
                n1 -> VA = nva+1;
                split(n1,size, nmn);
                mapping *tempm = get_mhead(mltbu);
                    mapping *newmapping = (mapping*)mmap(NULL,sizeof(mapping),PROT,FLAGS,-1,0);
                    while(tempm!=NULL){
                        if(tempm->next==NULL){
                            tempm->next = newmapping;
                            newmapping->prev = tempm;
                            newmapping -> next = NULL;
                            break;
                        }
                        tempm=tempm->next;
                    }
                newmapping ->sva= n1->VA;
                newmapping ->spa = (unsigned long)(void*)n1;
                newmapping ->eva = n1->VA + n1->size -1;
                newmapping -> epa = (unsigned long)(void*)n1 + n1->size -1;
                via = nmn -> sn -> VA;
                break;
        }
        temp=temp->next;
    }
    return (void*)via;
    }
}



/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(void){
    printf("Number of Mapped Pages: %d\n",nmp);
    unsigned long un_mem = 0;
    MainNode* temp = get_head(Free_maintbu);
    while(temp!=NULL){
        sub_node* calcsub = temp->sn;
        while(calcsub!=NULL){
            if(strcmp(calcsub->process_type,"HOLE")==0){
                un_mem+=calcsub->size;
            }
            calcsub=calcsub->next;
        }
        temp = temp->next;
    }
    temp = get_head(Free_maintbu);
    printf("Unused Memory: %lu bytes\n", un_mem);
    printf("\n");
    int i =1;
    printf("Sub-Chain Details\n");
    while(temp!=NULL){
        int k = 1;
        sub_node *ss = temp->sn;
        printf("Main Node ID: %d\n", i);
        while(ss!=NULL){
            printf("Sub-Node: %d | PROCESS TYPE: %s | SIZE: %lu bytes\n",k,ss->process_type,ss->size);
            ss=ss->next;
            k++;
        }
        printf("---------------------------------------------------\n");
        i++;
        temp = temp->next;
    }
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){
    unsigned long memsva = (unsigned long)v_ptr;
    unsigned long phyaddr = 0 ;
    mapping *temp = get_mhead(mltbu);
    while(temp!=NULL){
        if(temp->sva<=memsva && temp->eva>=memsva){
            phyaddr = temp->spa + memsva - temp->sva;
            break;
        }
        temp = temp->next;
    }
    return (void *)phyaddr;
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing
*/
void mems_free(void *v_ptr){
    unsigned long memsva = (unsigned long)v_ptr;
    unsigned long phyaddr = 0 ;
    mapping *temp = get_mhead(mltbu);
    while(temp!=NULL){
        if(temp->sva<=memsva && temp->eva>=memsva){
            phyaddr = temp->spa + memsva - temp->sva;
            if(temp->prev!=NULL && temp->next!=NULL){
                temp ->prev ->next = temp->next;
                temp->next->prev = temp->prev;
                temp->next = NULL;
                temp->prev = NULL;
                munmap(temp, sizeof(mapping));
            }else if(temp->prev == NULL && temp->next !=NULL){
                set_mhead(mltbu, temp->next);
                temp->next = NULL;
                temp ->next ->prev = NULL;
                munmap(temp, sizeof(mapping));
            }else if(temp->next == NULL && temp->prev!=NULL){
                temp->prev ->next = NULL;
                temp ->prev = NULL;
                munmap(temp, sizeof(mapping));
            }
            break;
        }
        temp = temp->next;
    }
    MainNode *tem = get_head(Free_maintbu);
    while(tem!=NULL){
        sub_node *tms = tem->sn;
        while(tms!=NULL){
            if(tms->VA<=memsva && tms->VA + tms->size -1 >=memsva){
                strcpy(tms->process_type, "HOLE");
                break;
            }
            tms=tms->next;
        }
        tem=tem->next;
    }
    edging();
    
}

