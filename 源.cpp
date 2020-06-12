#include<iostream>
#include<stdlib.h>
#include"stl_alloc.h"
using namespace std;

int main(){
	int *p = (int *)__default_alloc_template<0, 0>::allocate(sizeof(int));
	int *s = (int *)__default_alloc_template<0, 0>::allocate(sizeof(int) * 4);
	int *t = (int *)__default_alloc_template<0, 0>::allocate(sizeof(int) * 80);
	int *m = (int *)__default_alloc_template<0, 0>::allocate(sizeof(double) * 10);
	int *n = (int *)__default_alloc_template<0, 0>::allocate(sizeof(int) * 100);
	int *u = (int *)__default_alloc_template<0, 0>::allocate(sizeof(int));
	int *k = (int *)__default_alloc_template<0, 0>::allocate(sizeof(int) * 2);
	return 0;
}


/*
void Out_Of_Memory(){
cout<<"Out Of Memory."<<endl;
exit(1);
}

int main(){
//__malloc_alloc_template<0>::set_malloc_handler(OMG);
//set_new_handler()
//int *p = (int*)malloc(sizeof(int)*10);
void (*pfun)() = __malloc_alloc_template<0>::set_malloc_handler(Out_Of_Memory);
int *p = (int*)__malloc_alloc_template<0>::allocate(sizeof(int) * 2073741824);


__malloc_alloc_template<0>::set_malloc_handler(pfun);

if(p == NULL)
{
cout<<"Error."<<endl;
exit(1);
}
cout<<"OK"<<endl;
return 0;
}
*/