#ifndef shared_memory_h
#define shared_memory_h

#include <iostream>
#include "../plugins/WzSharememory.h"




namespace plugins {



  /**
   * @brief 创建共享内存，仅保存int
   * 
   * @param shared_memory_key 
   * @param data 
   * @return int 
   */
  int shared_memory_create(std::string shared_memory_key, int data);


  /**
   * @brief 读取共享内存数据，仅读取int
   * 
   * @param shared_memory_key 
   * @param data 
   * @return int 
   */
  int shared_memory_get(std::string shared_memory_key, int &data);

  ///===================================================


  inline int shared_memory_create(std::string shared_memory_key, int val){
    try
    {
      WzSharememory shm;
      shm.setName(shared_memory_key.c_str());
      if(shm.create(4)){
        // 共享内存创建成功，则证明共享内存不存在
        int* data=(int*)shm.open();
        *data = val;
      }else{
        // 共享内存创建失败，证明内存已创建
        int* data=(int*)shm.open();
        *data = val;
      }
      return 1;
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    return 0;
  }

  inline int shared_memory_get(std::string shared_memory_key, int &val){
    try
    {
      WzSharememory shm;
      shm.setName(shared_memory_key.c_str());
      if(shm.create(4)){
        // 共享内存创建成功，则证明共享内存不存在
        shm.destroy();
        return -1;
      }else{
        // 共享内存创建失败，证明内存已创建
        int* data=(int*)shm.open();
        val = *data;
        return 1;
      }
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    return 0;
  }

};


#endif