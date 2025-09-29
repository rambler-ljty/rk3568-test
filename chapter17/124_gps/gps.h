/*
 * GPS 模块头文件
 * 功能：定义 GPS 数据结构和函数声明
 */

#ifndef __GPS_H__
#define __GPS_H__

/**
 * GPRMC 数据包结构体
 * GPRMC (Recommended Minimum Specific GPS/Transit Data) 是 GPS 模块输出的标准 NMEA 数据格式
 */
struct gprmc_data{
	char id;        // 数据包标识符（通常为 'R'）
	int time;       // UTC 时间（格式：HHMMSS）
	char state;     // 数据状态（'A'=有效，'V'=无效）
	float latitude; // 纬度（度）
	char NS;        // 北纬/南纬标识（'N'=北纬，'S'=南纬）
	float longitude;// 经度（度）
	char EW;        // 东经/西经标识（'E'=东经，'W'=西经）
	float speed;    // 地面速度（节）
	int date;       // 日期（格式：DDMMYY）
	char mode;      // 模式指示符（'A'=自主，'D'=差分，'E'=估算，'N'=无效）
	char check;     // 校验和
};

/**
 * 配置串口参数
 * @param fd 串口文件描述符
 * @param speed 波特率（9600 或 115200）
 * @param bits 数据位数（7 或 8）
 * @param check 校验位类型（'N'无校验，'O'奇校验，'E'偶校验）
 * @param stop 停止位数（1 或 2）
 * @return 0 成功，-1 获取属性失败，-2 设置属性失败
 */
extern int set_uart(int fd,int speed,int bits,char check,int stop);

/**
 * 解析 GPS 数据
 * @param buff 包含 NMEA 数据的字符串缓冲区
 * @param gps_data 用于存储解析后 GPS 数据的结构体指针
 */
extern void get_gps_data(char *buff,struct gprmc_data *gps_data);

#endif

