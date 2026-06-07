#ifndef __WW_TCP_CLIENT_H__
#define __WW_TCP_CLIENT_H__

#include "headfile.h"

/*******************************************************************
 * [父类] Tcp Client: 底层通信驱动
 * 职责：只负责 TCP 连接、断开、原始字节的发送和检查连接状态
 * 特点：可用于连接任何 TCP 服务器
 ******************************************************************/
class TcpClient
{
public:
    // 默认连接的服务器 IP 和端口 Port (根据用户个人电脑设置修改)
    static constexpr const char*    DEFAULT_SERVER_IP = "192.168.2.10";
    static constexpr int            DEFAULT_SERVER_PORT = 2233;


    TcpClient(void);
    ~TcpClient(void);
/*******************************************************************
 * @brief       连接TCP服务器 
 * 
 * @param       ip              服务器IP地址
 * @param       port            服务器端口号
 * 
 * @return      返回连接状态
 * @retval      0               连接成功
 * @retval      -1              连接失败
 * 
 * @example     //连接到主机的VOFA+服务器
 *              if(tcp_client.connect_server("192.168.1.101", 2233) < 0) {
 *                  return -1;
 *              }
 * 
 * @note        建立与TCP服务器的连接，供后续数据传输使用
 ******************************************************************/
    int connect_server(const char* ip = DEFAULT_SERVER_IP, int port = DEFAULT_SERVER_PORT);

/*******************************************************************
 * @brief       发送原始字节数据
 * 
 * @param       data    待发送数据指针
 * @param       len     数据长度
 * 
 * @return      true:发送成功, false:发送失败
 * 
 * @note        将上层 TCP 设备协议打包来通信
 ******************************************************************/
    bool send_bytes(const void* data, size_t len);

/*******************************************************************
 * @brief       发送字符串
 * 
 * @param       s       待发送字符串
 * 
 * @return      true:发送成功, false:发送失败
 * 
 * @example     tcp_client.send_string("Hello, TCP Server!");
 * 
 * @note        将上层 TCP 设备协议打包来通信
 ******************************************************************/
    bool send_string(const std::string& s);

/*******************************************************************
 * @brief       断开与TCP服务器的连接
 * 
 * @example     tcp_client.disconnect_server();
 * 
 * @note        断开连接并释放Socket资源
 ******************************************************************/
    void disconnect_server(void);

/*******************************************************************
 * @brief       检查服务器是否已连接
 * 
 * @return      返回服务器运行状态
 * @retval      true            服务器已连接
 * @retval      false           服务器未连接
 * 
 * @example     if(tcp_client.is_connected()) {
 *                  //服务器已连接
 *              }
 ******************************************************************/
    bool is_connected(void);

protected:
    // Socket文件描述符
    int sock_fd;

    // 连接状态
    bool connected;

    // Socket互斥锁
    pthread_mutex_t sock_mutex;

};

/*******************************************************************
 * [子类] VofaClient: VOFA+ 协议层封装
 * 负责：将发送数据封装为 VOFA+ 的 FireWater 协议格式帧
 ******************************************************************/
class VofaClient : public TcpClient
{
public:
    VofaClient(void);
    ~VofaClient(void);

/*******************************************************************
 * @brief        发送格式化数据到VOFA+服务器 (FireWater协议)
 * 
 * @param        format  格式化字符串 (如 "data0: %f, data1: %f\n")
 * @param        ...     可变参数列表
 * 
 * @example      // 发送浮点数
 * client.send_firewater("sin: %.2f, %.2f\n", sin_val, cos_val);
 * 
 * @example      // 发送混合数据
 * client.send_firewater("temp: %.1f, %d\n", 25.5, 1);
 * 
 * @note         底层使用 vsnprintf 格式化，完全解耦，不限制数据类型
 *               发送数据格式说明: FireWater 协议格式:"name:csv_numbers\n"
 *******************************************************************/
    void send_firewater(const char* format, ...);
 
private:
    // FireWater 格式化缓冲区
    char firewater_buffer[1024];

};


#endif
