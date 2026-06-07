#ifndef __WUWU_LCD_H_
#define __WUWU_LCD_H_

#include "headfile.h"
#include "font.h"

/* 免责声明:显示部份移植逐飞科技TC264开源库，如有雷同那是真雷同，开源库地址: https://gitee.com/seekfree/TC264_Library*/
/* 免责声明:显示部份移植逐飞科技TC264开源库，如有雷同那是真雷同，开源库地址: https://gitee.com/seekfree/TC264_Library*/
/* 免责声明:显示部份移植逐飞科技TC264开源库，如有雷同那是真雷同，开源库地址: https://gitee.com/seekfree/TC264_Library*/

#define     FB_LCD_DEVICE                   "/dev/fb0"
#define     LCD_DEFAULT_BGCOLOR             (RGB565_WHITE)                              // 默认的背景颜色
#define     LCD_DEFAULT_PENCOLOR            (RGB565_BLACK)                              // 默认的画笔颜色
#define     LCD_DEFAULT_DISPLAY_FONT        (LCD_6X8_FONT)                              // 默认的字体模式
#define     func_abs(x)                     ((x) >= 0 ? (x): -(x))

class LCD
{
public:

    LCD(void);
    ~LCD();
/*******************************************************************
 * @brief       显示屏初始化
 * 
 * @return      返回初始化状态
 * @retval      0               初始化成功
 * @retval      -1              初始化失败
 * 
 * @example     //显示屏初始化
 *              if(lcd.lcd_init() < 0) {
 *                  return -1;             
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/
    int lcd_init(void);

/*******************************************************************
 * @brief   画点
 * 
 * @param   x           横坐标
 * @param   y           纵坐标
 * @param   color       画点颜色
 * 
 * @example lcd.drawPoint(0, 0, 0x0000);
 ******************************************************************/
    void drawPoint(int x, int y, unsigned short color);

/*******************************************************************
 * @brief   清屏
 * 
 * @example lcd.clearScreen();
 ******************************************************************/
    void clearScreen(void);

/*******************************************************************
 * @brief   显示字符
 * 
 * @param   x           字符左上角横坐标
 * @param   y           字符左上角纵坐标
 * @param   dat         字符
 * 
 * @example lcd.showChar(0, 0, '6');
 ******************************************************************/
    void showChar(int x, int y, const char dat);

/*******************************************************************
 * @brief   显示字符串
 * 
 * @param   x           字符串左上角横坐标
 * @param   y           字符串左上角纵坐标
 * @param   dat         字符串
 * 
 * @example lcd.showString(0, 0, "hello world");
 ******************************************************************/
    void showString(int x, int y, const char dat[]);

/*******************************************************************
 * @brief   显示有符号整形
 * 
 * @param   x           整形左上角横坐标
 * @param   y           整形左上角纵坐标
 * @param   dat         整形
 * @param   num         需要显示的位数
 * 
 * @example lcd.showInt(0, 0, dat, 5);
 ******************************************************************/
    void showInt(int x, int y, int dat, int num);

/*******************************************************************
 * @brief   显示无符号整形
 * 
 * @param   x           无符号整形左上角横坐标
 * @param   y           无符号整形左上角纵坐标
 * @param   dat         无符号整形
 * @param   num         需要显示的位数
 * 
 * @example lcd.showUInt(0, 0, dat, 5);
 ******************************************************************/
    void showUInt(int x, int y, int dat, int num);

/*******************************************************************
 * @brief   显示浮点数
 * 
 * @param   x           浮点数左上角横坐标
 * @param   y           浮点数左上角纵坐标
 * @param   dat         浮点数
 * @param   num         需要显示的位数
 * 
 * @example lcd.showDouble(0, 0, dat, 3, 3);
 ******************************************************************/   
    void showDouble (int x, int y, const double dat, unsigned char num, unsigned char pointnum);

/*******************************************************************
 * @brief   显示OpenCV Mat图像
 * 
 * @param   x                   图像左上角横坐标
 * @param   y                   图像左上角纵坐标
 * @param   image               需要显示的图像
 * @param   target_width        显示宽度
 * @param   target_height       显示高度
 * 
 * @example lcd.showCVImage(0, 0, img, 128, 80);
 ******************************************************************/
    void showCVImage(int x, int y, const cv::Mat& image, int target_width, int target_height);

private:
    int fd;                                     //设备是否存在
    struct fb_fix_screeninfo fix_screeninfo;    //屏幕固定信息
    struct fb_var_screeninfo var_screeninfo;    //屏幕可变信息
    long int screensize;                        //缓冲帧大小
    char *fbp = nullptr;                        //帧缓冲内存指针

    unsigned short      lcd_bgcolor         =   LCD_DEFAULT_BGCOLOR;            // 背景颜色
    unsigned short      lcd_pencolo         =   LCD_DEFAULT_PENCOLOR;           // 画笔颜色(字体色)
    lcd_font_size_enum  lcd_display_font    =   LCD_DEFAULT_DISPLAY_FONT;       // 显示字体类型

    void drawPixel(int x, int y, unsigned short color);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     整形数字转字符串 数据范围是 [-32768,32767]
// 参数说明     *str            字符串指针
// 参数说明     number          传入的数据
// 返回参数     void
// 使用示例     func_int_to_str(data_buffer, -300);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
    void func_int_to_str (char *str, int number);
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     整形数字转字符串 数据范围是 [0,65535]
// 参数说明     *str            字符串指针
// 参数说明     number          传入的数据
// 返回参数     void
// 使用示例     func_uint_to_str(data_buffer, 300);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
    void func_uint_to_str (char *str, unsigned int number);
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     浮点数字转字符串
// 参数说明     *str            字符串指针
// 参数说明     number          传入的数据
// 参数说明     point_bit       小数点精度
// 返回参数     void
// 使用示例     func_double_to_str(data_buffer, 3.1415, 2);                     // 结果输出 data_buffer = "3.14"
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
    void func_double_to_str (char *str, double number, unsigned char point_bit);
};

#endif
