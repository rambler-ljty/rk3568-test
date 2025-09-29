/*
 * GPS 数据处理模块
 * 功能：解析 NMEA 格式的 GPS 数据
 */

#include <stdio.h>      // 标准输入输出库
#include <termios.h>    // 终端控制结构体定义
#include <string.h>     // 字符串处理函数
#include <sys/types.h>  // 系统数据类型定义
#include <sys/stat.h>   // 文件状态结构体定义
#include <fcntl.h>      // 文件控制选项
#include <unistd.h>     // UNIX 标准函数定义
#include "gps.h"        // GPS 模块头文件

/**
 * 解析 GPRMC 格式的 GPS 数据
 * GPRMC 是 NMEA 0183 标准中推荐的最小特定 GPS/Transit 数据格式
 * 格式：$GPRMC,时间,状态,纬度,北纬/南纬,经度,东经/西经,速度,日期,磁偏角,磁偏角方向,模式指示符*校验和
 * 
 * @param buff 包含 NMEA 数据的字符串缓冲区
 * @param gps_data 用于存储解析后 GPS 数据的结构体指针
 */
void get_gps_data(char *buff,struct gprmc_data *gps_data){
	char *p=NULL;  // 用于定位 GPRMC 数据包的指针

	// 在缓冲区中查找 GPRMC 数据包
	// GPRMC 是 GPS 模块输出的标准数据格式
	p=strstr(buff,"$GPRMC");

	// 检查是否找到有效的 GPRMC 数据包
	if(p == NULL){
		printf("未找到有效的 GPRMC 数据包\n");
		return;
	}

	// 使用 sscanf 解析 GPRMC 数据包
	// 格式：$GPRMC,时间.00,状态,纬度,北纬/南纬,经度,东经/西经,速度,,日期,,,模式指示符,校验和
	sscanf(p,"$GPRMC,%d.00,%c,%f,%c,%f,%c,%c,%f,,%d,,,%c,%*c",
		&(gps_data->time),        // 时间（HHMMSS.00 格式）
		&(gps_data->state),       // 数据状态（'A'=有效，'V'=无效）
		&(gps_data->latitude),    // 纬度（度）
		&(gps_data->NS),          // 北纬/南纬标识
		&(gps_data->longitude),   // 经度（度）
		&(gps_data->EW),          // 东经/西经标识
		&(gps_data->speed),       // 地面速度（节）
		&(gps_data->date),        // 日期（DDMMYY 格式）
		&(gps_data->mode));       // 模式指示符
	
	// 打印解析后的关键 GPS 信息
	printf("GPS 状态: %c, 纬度: %c%f, 经度: %c%f\n", 
		gps_data->state,          // 数据有效性状态
		gps_data->NS,             // 北纬/南纬
		gps_data->latitude,       // 纬度值
		gps_data->EW,             // 东经/西经
		gps_data->longitude);     // 经度值
}
