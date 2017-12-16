#include <npheap/tnpheap_ioctl.h>
#include <npheap/npheap.h>
#include <npheap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <sys/queue.h>


typedef struct node{
    long unsigned int offset; 
    unsigned int version; 
    void * data;
    unsigned int size;
    TAILQ_ENTRY(node) nodes;
};



typedef TAILQ_HEAD(head_t,node) head_t;
head_t my_list_head;


__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
    struct tnpheap_cmd cmd;
    
    printf("%d \n",__LINE__);
    cmd.offset=offset;
    printf("%d \n",__LINE__);
    return ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, cmd );
}



int tnpheap_handler(int sig, siginfo_t *si)
{
    return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
    struct node * tmp ;
    
    
    TAILQ_FOREACH(tmp,&my_list_head,nodes)
    {
        if (tmp->offset == offset)
        {
            return tmp->data;
        }
    }
    char * data ;
    data = (void *)malloc(size);
    tmp = (struct node *)malloc(sizeof(struct node));
    tmp->offset = offset;
    tmp->version=0;
    tmp->data = data;
    tmp->size=size;
    TAILQ_INSERT_TAIL(&my_list_head,tmp,nodes);
    return data;     
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
    
    TAILQ_INIT(&my_list_head);
    return 0;
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
    
    
    struct node * tmp;
    struct node * next = NULL;
    
    printf("%d \n",__LINE__);
    TAILQ_FOREACH(tmp,&my_list_head,nodes)
    {
        struct tnpheap_cmd user_cmd;
        user_cmd.offset = tmp->offset;
        unsigned int original_version = ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, user_cmd );
        if(original_version != tmp->version)
        {
            return -1;
        }
    }
    printf("%d \n",__LINE__);
    TAILQ_FOREACH(tmp,&my_list_head,nodes)
    {
        npheap_lock(npheap_dev, tmp->offset);
        struct tnpheap_cmd user_cmd;
        void * mapped_data;
        user_cmd.offset = tmp->offset;
        user_cmd.version = tmp->version;
        user_cmd.size=tmp->size;
        user_cmd.data = tmp->data;
        unsigned int original_version = ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, user_cmd);
        mapped_data = (char *)npheap_alloc(npheap_dev,tmp->offset,tmp->size);
        //sprintf(mapped_data,"%s",tmp->data);
        memcpy(mapped_data,tmp->data,tmp->size);
        npheap_unlock(npheap_dev, tmp->offset);
    }
   
    return 0;
}

