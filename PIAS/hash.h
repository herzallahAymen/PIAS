#ifndef HASH_H
#define HASH_H

#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/vmalloc.h>

#include "flow.h" //Get definition of Flow structure

#define HASH_RANGE 1024
#define QUEUE_SIZE 128

//Node of Flow
struct FlowNode{
        struct Flow f;         				//content of flow
        struct FlowNode* next; 	//pointer to next node 
};

//List of Flows
struct FlowList{
        struct FlowNode* head; 	//pointer to head node of this link list
        int len;              						 //current length of this list (max: QUEUE_SIZE)
};

//Hash Table of Flows
struct FlowTable{
        struct FlowList* table; 		//many FlowList (HASH_RANGE)
        int size;              					 //total number of nodes in this table
};

//Hash function, given a Flow node, calculate it should be inserted into which FlowList
static unsigned int Hash(struct Flow* f)
{
	//return a value in [0,HASH_RANGE-1]
	return ((f->local_ip/(256*256*256)+1)*(f->remote_ip/(256*256*256)+1)*(f->local_port/(256*256*256)+1)*(f->remote_port/(256*256*256)+1))%HASH_RANGE;
}

//Determine whether two Flows are equal (same flow) 
static int Equal(struct Flow* f1,struct Flow* f2)
{
	//<local_ip,local_port,remote_ip,remote_port> determines a TCP flow
	if((f1->local_ip==f2->local_ip)&&(f1->remote_ip==f2->remote_ip)&&(f1->local_port==f2->local_port)&&(f1->remote_port==f2->remote_port))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//Initialize a TCP flow information entry
static void Init_Information(struct Information* info)
{
	info->start_time=0;
	info->latest_receive_time=0;
	info->srtt=0;
	info->seq=0;
	info->ack=0;
	info->receive_data=0;
	info->send_data=0;
	info->status=0;
}

//Initialize a complete TCP flow 
static void Init_Flow(struct Flow* f)
{
	f->direction=0;
	f>remote_ip=0;
	f->local_ip=0;
	f->remote_port=0;
	f->local_port=0;
	Init_Information(&(f->info));
}

//Initialize a FlowNode
static void Init_Node(struct FlowNode* fn)
{
	//Initialize the pointer to next node as NULL
	fn->next=NULL;
	Init_Flow(&(fn->f));
}

//Initialize a FlowList
static void Init_List(struct FlowList* fl)
{
	//No node in current list
	fl->len=0;
	fl->head=vmalloc(sizeof(struct  FlowNode));
	Init_Node(fl->head);
} 

//Initialize FlowTable
static void Init_Table(struct FlowTable* ft)
{
	int i=0;
	//Allocate space for FlowLists
	ft->table=vmalloc(HASH_RANGE*sizeof(struct FlowList));
	//Initialize each FlowList
	for(i=0;i<HASH_RANGE;i++)
	{
		Init_List(&(ft->table[i]));
	}
	//No nodes in current table
	ft->size=0;
}

//Insert a new Flow entry into a FlowList
//If the Flow entry is inserted successfully, return 1
//Else, fl->len>=QUEUE_SIZE or the same Flow entry exists, return 0
static int Insert_List(struct FlowList* fl, struct Flow* f)
{
	//Reach maximum queue size in this list
	if(fl->len>=QUEUE_SIZE)
	{
		//printk(KERN_INFO "No enough space in this FlowList\n");
		return 0;
	}
	else
	{
		struct FlowNode* tmp=fl->head;
		
		//Find the tail of FlowList
		while(1)
		{
			//If pointer to next node is NULL, we find the tail of this FlowList. Here we can insert our new Flow entry
			if(tmp->next==NULL)
			{
				//Allocate space for next FlowNode
				tmp->next=vmalloc(sizeof(struct FlowNode));
				//Copy data for this new FlowNode
				tmp->next->f=*f;
				//Pointer to next node is NULL
				tmp->next->next=NULL;
				//Increase the length of current FlowList
				fl->len++;
				//Finish the insert, return 1
				return 1;
			}
			//If we find the same Flow entry, we sould return 0 
			else if(Equal(&(tmp->next->f),f))
			{
				return 0;
			}
			//Move to next FlowNode
			else
			{
				tmp=tmp->next;
			}
		}
	}
	
	//By default, return 0
	return 0;
}



