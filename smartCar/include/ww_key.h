#ifndef __WUWU_KEY_H
#define __WUWU_KEY_H

#include "headfile.h"

#define KEY_FILE_ADD "/dev/input/event0"

class Key
{
public:
    Key(void);
    ~Key(void);
/*******************************************************************
 * @brief       按键初始化
 * 
 * @return      返回初始化状态
 * @retval      0               初始化成功
 * @retval      -1              初始化失败
 * 
 * @example     //按键初始化
 *              if(key.key_init() < 0) {
 *                  return -1;             
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/
    int key_init(void);

/*******************************************************************
 * @brief       案件监听
 * 
 * @example     //按键阻塞监听
 *              key.key_listeners();
 ******************************************************************/
    void key_listeners(void);

private:
    int fd;

};

#endif
