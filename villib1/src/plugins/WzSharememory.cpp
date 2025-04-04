
#include "WzSharememory.h"

struct WzSharememoryHandle
{
#ifdef _WIN32
    HANDLE file_mapper = NULL;
    LPVOID sharememory_pointer = NULL;
#else
    key_t key = -1;
    int id = -1;
    void* sharememory_pointer = NULL;
#endif
};

WzSharememory::WzSharememory()
    : handle(NULL),
      name("")
{
    init();
}

WzSharememory::WzSharememory(std::string name)
    : handle(NULL),
      name(name)
{
    init();
}

WzSharememory::~WzSharememory()
{
    release();
}

std::string WzSharememory::getName()
{
    return name;
}

void WzSharememory::setName(std::string name)
{
    resetHandle();
    this->name = name;
}

bool WzSharememory::create(unsigned int size)
{
    if(size <= 0)
    {
        printf("[WzSharememory::create] can not create sharememory,"
                "size you pass must greater than 0.\n");
        return false;
    }

#ifdef _WIN32
    /* 
        the sharememory you want is existed.
        it happens with open success before and create again. 
    */
    if(NULL != handle->file_mapper)
    {
        printf("[WzSharememory::create] the sharememory you want is existed.\n");
        return false;
    }

    handle->file_mapper = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                                             NULL,
                                                   PAGE_READWRITE,
                                                                0,
                                                             size,
                                              name.c_str());
	
    if(NULL == handle->file_mapper || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        printf("[WzSharememory::create] create sharememory failed,"
               "maybe it is existed or an unkown error occurs.\n");
        return false;
    }
#else
    /* 
        the sharememory you want is existed.
        it happens with open success before and create again. 
    */
    if(-1 != handle->key && -1 != handle->id )
    {
        printf("[WzSharememory::create] the sharememory you want is existed.\n");
        return false;
    }

    handle->key = std::hash<std::string>()(name);
    handle->id = shmget(handle->key, size,IPC_CREAT|IPC_EXCL|0666);
    if(-1 == handle->key || -1 == handle->id)
    {
        printf("[WzSharememory::create] create sharememory failed,"
               "maybe it is existed or an unkown error occurs.\n");
        return false;
    }
#endif

    return true;
}

void* WzSharememory::open()
{
#ifdef _WIN32
    /* open again */
    if(NULL != handle->sharememory_pointer)
    {
        return handle->sharememory_pointer;
    }
    /* open without create or with create failed  */
    if(NULL == handle->file_mapper)
    {
        handle->file_mapper = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,
                                                                  0, 
                                                        name.c_str());
        if(NULL == handle->file_mapper)
        {
            printf("[WzSharememory::open] open sharememory failed,"
                   "maybe it is not existed.\n");
            return NULL;
        }
    }

    handle->sharememory_pointer = ::MapViewOfFile(handle->file_mapper, 
                                                  FILE_MAP_ALL_ACCESS, 
                                                                    0, 
                                                                    0, 
                                                                    0);
    if(NULL == handle->sharememory_pointer)
    {
        printf("[WzSharememory::open] open sharememory failed with "
               "an unkown error occurs.\n");
    }
    return handle->sharememory_pointer;
#else
    /* open again */
    if(NULL != handle->sharememory_pointer)
    {
        return handle->sharememory_pointer;
    }
    /* open without create or with create failed  */
    if(-1 == handle->key && -1 == handle->id)
    {
        handle->key = std::hash<std::string>()(name);
        handle->id = shmget(handle->key,0,IPC_CREAT|0666);
        handle->sharememory_pointer = shmat(handle->id,NULL,0);

        if(-1 == handle->key || 
           -1 == handle->id ||
           (void*)-1 == handle->sharememory_pointer)
        {
            printf("[WzSharememory::open] open sharememory failed,"
                   "maybe it is not existed.\n");
            return NULL;
        }
    }

    handle->id = shmget(handle->key,0,IPC_CREAT|0666);
    handle->sharememory_pointer = shmat(handle->id,NULL,0);
    if(-1 == handle->id || (void*)-1 == handle->sharememory_pointer)
    {
        printf("[WzSharememory::open] open sharememory failed with "
               "an unkown error occurs.\n");
        return NULL;
    }

    return handle->sharememory_pointer;
#endif
}

void WzSharememory::destroy()
{
#ifdef _WIN32
    if(NULL != handle->file_mapper)
    {
        ::CloseHandle(handle->file_mapper);
    }
#else
    if(-1 != handle->id)
    {
        shmctl(handle->id, IPC_RMID,NULL);
    }
#endif
}

void WzSharememory::init()
{
    if(NULL == handle)
    {
        handle = new WzSharememoryHandle;
    }
}

void WzSharememory::release()
{
    if(NULL != handle)
    {
#ifdef _WIN32
        if(NULL != handle->sharememory_pointer)
        {
            ::UnmapViewOfFile(handle->sharememory_pointer);
        }
#else
        if(NULL != handle->sharememory_pointer)
        {
            shmdt(handle->sharememory_pointer);
        }
#endif
        delete handle;
    }
}

void WzSharememory::resetHandle()
{
#ifdef _WIN32
    if(NULL != handle->sharememory_pointer)
    {
        ::UnmapViewOfFile(handle->sharememory_pointer);
    }
    handle->file_mapper = NULL;
    handle->sharememory_pointer = NULL;
#else
    if(NULL != handle->sharememory_pointer)
    {
        shmdt(handle->sharememory_pointer);
    }
    handle->key = -1;
    handle->id = -1;
    handle->sharememory_pointer = NULL;
#endif   
}
