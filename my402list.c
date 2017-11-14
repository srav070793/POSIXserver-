#include <stdio.h>
#include <stdlib.h>
#include "my402list.h"
#include "cs402.h"

int  My402ListLength(My402List *list){
	int count=0;
	My402ListElem *elem;
	elem = My402ListFirst(list);	
	while (elem!=NULL){
		count++;
		elem=My402ListNext(list, elem);
	}
	return count;
}
int  My402ListEmpty(My402List *list){
	if (My402ListLength(list) == 0)	
		return 1;
	else 
		return 0;
}
int  My402ListAppend(My402List *list, void *nobj){
	if (My402ListEmpty(list)){
	My402ListElem *nnode=(My402ListElem *)malloc(sizeof(My402ListElem));	
	nnode->obj=nobj;
	//append to the anchor
		(list->anchor).next=nnode;
		nnode->prev=&(list->anchor);
		nnode->next=&(list->anchor);
	(list->anchor).prev=nnode;			
		return 1;	
	}
	else{
	//append in the end
		My402ListElem *nnode=(My402ListElem *)malloc(sizeof(My402ListElem));	
		nnode->obj=nobj;
		My402ListElem *elem = My402ListLast(list);
		elem->next=nnode;
		(list->anchor).prev=nnode;
		nnode->next=&(list->anchor);
		nnode->prev=elem;
		return 1;
	}
	return 0;
}
int  My402ListPrepend(My402List *list, void *nobj){
	if (My402ListEmpty(list)){
	My402ListElem *nnode=(My402ListElem *)malloc(sizeof(My402ListElem));	
		nnode->obj=nobj;
		(list->anchor).next=nnode;
		nnode->prev=&(list->anchor);
		nnode->next=&(list->anchor);
	(list->anchor).prev=nnode;			
		return 1;	
	}
	else{
                My402ListElem *nnode=(My402ListElem *)malloc(sizeof(My402ListElem));	
		nnode->obj=nobj;
		My402ListElem *elem = My402ListFirst(list);
		nnode->prev=elem->prev;		
		elem->prev=nnode;
		nnode->next=elem;
		(list->anchor).next=nnode;		
		return 1;	
	}
	return 0;
}
void My402ListUnlink(My402List *list, My402ListElem *elem){
	My402ListElem *belem=elem->prev;
	//printf("before:%d\n", (int)belem->obj);
	My402ListElem *aelem=elem->next;
	//printf("after:%d\n", (int)aelem->obj);	
	elem->next=NULL;
	elem->prev=NULL;
	belem->next=aelem;
	aelem->prev=belem;	
}
void My402ListUnlinkAll(My402List *list){
	My402ListElem *curr= My402ListFirst(list);
	My402ListElem *nextcurr=My402ListNext(list,curr);	
	while (curr!=NULL){
		My402ListUnlink(list, curr);
		curr=nextcurr;
		nextcurr=My402ListNext(list, nextcurr);
	}
	(list->anchor).next=&(list->anchor);
	(list->anchor).prev=&(list->anchor);
}
int  My402ListInsertAfter(My402List *list, void *nobj, My402ListElem *lelem){
	if (lelem == NULL){
		My402ListAppend(list, nobj);
		return 1;	
	}
	else{
		My402ListElem *nnode=(My402ListElem *)malloc(sizeof(My402ListElem));	
		nnode->obj=nobj;
		nnode->next=lelem->next;
		(lelem->next)->prev=nnode;
		lelem->next=nnode;
		nnode->prev=lelem;
		return 1;
	}
	return 0;	
}
int  My402ListInsertBefore(My402List *list, void *nobj, My402ListElem *lelem){
	if(lelem==NULL){
		My402ListPrepend(list, nobj);
		return 1;
	}
	else{
		My402ListElem *nnode=(My402ListElem *)malloc(sizeof(My402ListElem));	
		nnode->obj=nobj;
		nnode->prev=lelem->prev;
		(lelem->prev)->next=nnode;
		nnode->next=lelem;
		lelem->prev=nnode;
		return 1;
	}
	return 0;
}
My402ListElem *My402ListFirst(My402List *list){
	if((list->anchor).next == &(list->anchor)){	
		return NULL;
	}
	else 
		return (list->anchor).next;
}
My402ListElem *My402ListLast(My402List *list){
	if((list->anchor).next == &(list->anchor)){	
		return NULL;
	}
	else
		return (list->anchor).prev;
}
My402ListElem *My402ListNext(My402List *list, My402ListElem *elem){
	if (elem == My402ListLast(list))
		return NULL;
	else 
		return elem->next;
}
My402ListElem *My402ListPrev(My402List *list, My402ListElem *elem){	
	if (elem == My402ListFirst(list))
		return NULL;
	else 
		return elem->prev;	
	
}
My402ListElem *My402ListFind(My402List *list, void *lobj){
	My402ListElem *curr=My402ListFirst(list);
	while(curr!=NULL && curr->obj!=lobj){		
		curr=My402ListNext(list,curr);		
	}
	//printf("Inside My402ListFind: %d\n", (int)curr);
	return curr;
}
int My402ListInit(My402List *list){
	(list->anchor).next= &(list->anchor);
	(list->anchor).prev= &(list->anchor);
	(list->anchor).obj=NULL;	
	return 1;
}

/*main(){
	My402List mylist;
	My402ListElem *curr;
	int appe;
	My402ListInit(&mylist); //no error
	curr = &(mylist.anchor);	
	printf("%d\n",(int)curr->obj);
	curr= My402ListFirst(&mylist); //no error
	//printf("%d\n", (int)curr);
	curr= My402ListLast(&mylist); //no error
	printf("%d\n", My402ListLength(&mylist)); // no error
	printf("%d\n", My402ListEmpty(&mylist));//no error
	appe=My402ListAppend(&mylist,(void *)3); //no error
	appe= My402ListAppend(&mylist,(void *)4);	
	curr= My402ListFirst(&mylist);	
	printf("%d\n", (int)curr->obj);
	curr= My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	appe=My402ListPrepend(&mylist, (void *)2);//works for nonempty list
	curr= My402ListFirst(&mylist);
	printf("%d\n", (int)curr->obj);
	curr=My402ListFirst(&mylist);
	curr=My402ListNext(&mylist, curr);
	//curr=My402ListNext(&mylist, curr);
	printf("printing unlinking\n");
	//curr=My402ListFirst(&mylist);
	My402ListUnlink(&mylist,curr); //deleting 3, no error
	curr=My402ListFirst(&mylist);	
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	printf("Insert after\n");	
	curr=My402ListFirst(&mylist);	
	My402ListInsertAfter(&mylist, (void *)5, curr); //no error	
	My402ListInsertAfter(&mylist, (void *)6, NULL);
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	My402ListInsertBefore(&mylist, (void *)7, NULL); //no error
	curr=My402ListFirst(&mylist);
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	My402ListInsertBefore(&mylist, (void *)8, curr);//no error
	printf("Inserting before\n");	
	curr=My402ListFirst(&mylist);
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	curr=My402ListNext(&mylist, curr);
	printf("%d\n", (int)curr->obj);
	printf("printing previous element\n");
	curr= My402ListPrev(&mylist,curr); //no error
	printf("%d\n", (int)curr->obj);	
	printf("testing find operation\n");
	curr=NULL;
	curr=My402ListFind(&mylist, (void *)5); //no error	
	printf("found object:%d\n", (int) curr->obj);
	printf("Unlinking all\n");
	My402ListUnlinkAll(&mylist); //no error
	curr=My402ListFirst(&mylist);
	printf("%d\n", (int)curr);
	printf("length: %d\n", My402ListLength(&mylist));
//-----------------------------------
}*/
