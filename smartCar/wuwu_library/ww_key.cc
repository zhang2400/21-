#include "ww_key.h" 

#define KEY_NUM         (5)

static void KEY1_CallBack(void)
{
    
}

static void KEY2_CallBack(void)
{
    
}

static void KEY3_CallBack(void)
{
    
}

static void KEY4_CallBack(void)
{
    
}

static void KEY5_CallBack(void)
{
    
}

Key::Key(void) {}

Key::~Key(void)
{
    if(fd < 0) return;
    close(fd);
}

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
int Key::key_init(void)
{
    fd = open(KEY_FILE_ADD, O_RDONLY);
    if(fd < 0) {
        perror("key init error!\n");
        return -1;
    } 
    std::cout << "按键初始化成功" << std::endl;
    return 0;
}

/*******************************************************************
 * @brief       案件监听
 * 
 * @example     //按键阻塞监听
 *              key.key_listeners();
 ******************************************************************/
void Key::key_listeners(void)
{
    struct input_event event;
    if(read(fd, &event, sizeof(event)) != sizeof(event)) 
        return;

    if(event.type == EV_KEY && (event.value == 1 || event.value == 2)) {
        int keycode = event.code;

        switch (keycode)
        {
        case 2: KEY1_CallBack(); break;
        case 3: KEY2_CallBack(); break;
        case 4: KEY3_CallBack(); break;
        case 5: KEY4_CallBack(); break;
        case 6: KEY5_CallBack(); break;
        //...(可以根据设备树继续添加)
        default: break;
        }
    }
}
