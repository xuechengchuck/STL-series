#if 1
#include<iostream>
#include<new>
#include<malloc.h>
using namespace std;
//#define __THROW_BAD_ALLOC   throw   bad_alloc
#define __THROW_BAD_ALLOC  cerr<<"Throw bad alloc, Out Of Memory."<<endl; exit(1)  //�����쳣,�������һ�仰,���ҽ�������.
#elif  !defined  (__THROW_BAD_ALLOC) //���û�ж�������쳣,����Ͷ���
#include<iostream.h>
#define __THROW_BAD_ALLOC   cerr<<"out of memory"<<endl; exit(1);
#endif

template<int inst>
class __malloc_alloc_template{
private:
	static void* oom_malloc(size_t);  //������ռ�ʧ�ܵĴ�����
	static void* oom_realloc(void *, size_t);  //����չ�ռ�ʧ�ܴ���ĺ���
	static void(*__malloc_alloc_oom_handler)(); //����һ������ָ��

public:
	static void* allocate(size_t n){   //����ռ�
		void *result = malloc(n);
		if (0 == result)
			result = oom_malloc(n); //����ռ�ʧ�ܣ�����oom_malloc()����
		return result;  //������Ŀռ�ĵ�ַ����
	}
	static void   deallocate(void *p, size_t){
		free(p);  //�ͷſռ�
	}
	static void* reallocate(void *p, size_t, size_t new_sz){
		void *result = realloc(p, new_sz);  //��չ�¿ռ�;
		if (0 == result) //��չʧ��
			oom_realloc(p, new_sz); //������չ�ռ�ʧ�ܵĴ�����
		return result;
	}
public:
	//set_new_handler(Out_Of_Memory);
	static void(*set_malloc_handler(void(*f)()))(){  //����һ��ָ�뺯��,��������:set_malloc_handler,����:��һ������ָ��,����ֵ:��һ������ָ��;
		void(*old)() = __malloc_alloc_oom_handler;  //��ԭ�пռ�ĵ�ַ������old��;
		__malloc_alloc_oom_handler = f;  //���Լ�����ĺ�����ַ��__malloc_alloc_oom_handler;
		return old;  //ÿ�ο��Ա�������һ�εĵ�ַ.
	}
};

template<int inst>
void(*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0; //�Զ���ľ�̬����ָ���ʼ��Ϊ0;

template<int inst>
void* __malloc_alloc_template<inst>::oom_malloc(size_t n){ //����ռ�ʧ�ܵ�����
	void *result;
	void(*my_malloc_handler)(); //����һ������ָ��;

	for (;;){
		my_malloc_handler = __malloc_alloc_oom_handler;
		if (0 == my_malloc_handler){ //�Լ�û�ж��崦��ռ�ʧ�ܵ��º���
			__THROW_BAD_ALLOC;  //�쳣�׳�;������ֹ;
		}
		(*my_malloc_handler)(); //�����Լ���д�Ĵ�����(һ�㶼�ǻ��տռ�֮���);
		result = malloc(n);  //�ڴ��������ռ�
		if (result){ //����ɹ�
			return result; //����ַ����;
		}//��ô��������򽫻�һֱ�������ռ����ɹ������շ���.
	}
}


template<int inst>
void* __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n){
	void(*my_malloc_handler)();  //����ָ��
	void *result;
	for (;;){
		my_malloc_handler = __malloc_alloc_oom_handler; //��������丳ֵ
		if (0 == my_malloc_handler){  //����û�ж��崦��ĺ���
			__THROW_BAD_ALLOC;  //�쳣�׳�,�������.
		}
		(*my_malloc_handler)();  //�����Լ���д�Ĵ�����(һ�㶼�ǻ��տռ�֮���);
		result = realloc(p, n);  //�ٴ���չ�ռ����;
		if (result){  //��չ�ɹ�,�ͷ���
			return result;
		}  //һֱ�����ɹ���֪����չ�ռ����ɹ��ŷ���;
	}
}

typedef __malloc_alloc_template<0> malloc_alloc;   //һ���ռ�������:malloc_alloc;

/////////////////////////////////////////////////////////////////////////////////////
//�����ռ�������������������ڴ�����;
enum { __ALIGN = 8 };  //һ������8B
enum { __MAX_BYTES = 128 };  //С��128B���ö�����
enum { __NFREELISTS = __MAX_BYTES / __ALIGN }; //һ������16����������,����16�ִη�������.

template<bool threads, int inst>  //�����Ƕ��߳�״̬;
class __default_alloc_template{
public:
	static void* allocate(size_t n);  //����ռ�
	static void  deallocate(void *p, size_t n);  //���ٿռ�
	static void* reallocate(void *p, size_t, size_t new_sz);  //��չ�ռ�
private:
	static size_t  ROUND_UP(size_t bytes){  //���ϵ�������;
		return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1)); //��Ϊ��ǰ�ֽ���8��������.
	}
private:
	union obj{  //������
		union obj * free_list_link;  //���������ָ��
		char client_data[1];
	};
private:
	static obj* volatile free_list[__NFREELISTS];  //������һ��ָ������;
	static size_t FREELIST_INDEX(size_t bytes){  //��ǰ�ֽڵ�����������±�;
		return ((bytes)+__ALIGN - 1) / __ALIGN - 1;
	}
private:
	static char *start_free; //��ʼ�ռ���±�
	static char *end_free;   //�����ռ���±�
	static size_t heap_size;  //�ѿռ��С
	static void *refill(size_t n);  //��亯��
	static char* chunk_alloc(size_t size, int &nobjs);  //
};


template<bool threads, int inst> //���¶��ǶԾ�̬�����ĳ�ʼ��,��Ϊ0;
char* __default_alloc_template<threads, inst>::start_free = 0;
template<bool threads, int inst>
char* __default_alloc_template<threads, inst>::end_free = 0;
template<bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;
template<bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj* volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

template<bool threads, int inst>
void* __default_alloc_template<threads, inst>::allocate(size_t n){ //����ռ�ĺ���
	obj * volatile *my_free_list;
	obj *result;

	if (n > __MAX_BYTES){  //����ռ�Ĵ�С����128B�Ļ����͵���һ���ռ�������
		return malloc_alloc::allocate(n);
	}

	my_free_list = free_list + FREELIST_INDEX(n); //free_list�Ƕ�ά��������,�����ĸ����¹ң�
	result = *my_free_list;  //ȡ����ֵ,��Ϊmy_free_list�Ƕ���ָ��;

	if (result == 0){  //û���ڴ�ؿռ�;
		void *r = refill(ROUND_UP(n));  //����refill()����
		return r;
	}

	*my_free_list = result->free_list_link;  //���й�������;
	return result;
}

template<bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n){ //û�п�������ʱ,�͵���refill()����;
	int nobjs = 20;//����Ҫ��20��;
	char *chunk = chunk_alloc(n, nobjs); //�����ڴ�غ���;
	obj * volatile *my_free_list;

	obj *result;
	obj *current_obj, *next_obj;
	int i;

	if (1 == nobjs){  //�����䵽ֻ��һ��ռ�ʱ,ֱ�ӷ���.
		return chunk;
	}

	my_free_list = free_list + FREELIST_INDEX(n); //�ҵ���Ӧ���±꣬�������ӵĹ���;
	result = (obj*)chunk;
	*my_free_list = next_obj = (obj*)(chunk + n);

	for (i = 1;; ++i){
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if (nobjs - 1 == i){  //�������ӹ���;
			current_obj->free_list_link = 0;
			break;
		}
		else{
			current_obj->free_list_link = next_obj;
		}
	}
	return result;
}

template<bool threads, int inst>
char* __default_alloc_template<threads, inst>::chunk_alloc(size_t size, int &nobjs){ //�ڴ�غ���
	char *result; //�ؼ�Ҫ�������µĸ������;
	size_t total_bytes = size * nobjs;   //�����size�Ѿ����ϵ������ֽ�;��ȡ20*size���ֽڵĴ�С
	size_t bytes_left = end_free - start_free; //�տ�ʼ�������ֽ�Ϊ0;
	if (bytes_left >= total_bytes){  //������
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else if (bytes_left >= size){ //������
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else{  //�ߵľ������������·��
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4); //����2����total_bytes��
		if (bytes_left > 0){ //�����ֽ���=0,����������䲻����;
			obj * volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj *)start_free;
		}

		start_free = (char *)malloc(bytes_to_get); //����ռ�;
		if (0 == start_free){
			int i;
			obj * volatile *my_free_list, *p;
			for (i = size; i <= __MAX_BYTES; i += __ALIGN){
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (0 != p){
					*my_free_list = p->free_list_link;
					start_free = (char *)p;
					end_free = start_free + i;
					return chunk_alloc(size, nobjs);
				}
			}
			end_free = 0;
			start_free = (char *)malloc_alloc::allocate(bytes_to_get);
		}

		heap_size += bytes_to_get;  //��¼��ʱ�ѿռ�Ĵ�С;
		end_free = start_free + bytes_to_get;  //ָ�������;
		return chunk_alloc(size, nobjs);  //��ȥ�ڴ˵����������;
	}
}