/*********************************************************************************************************
* 模块名称：SendDataToHost.c
* 摘    要：SendDataToHost模块
* 当前版本：1.0.0
* 作    者：SZLY(COPYRIGHT 2018 - 2020 SZLY. All rights reserved.)
* 完成日期：2020年01月01日 
* 内    容：
* 注    意：                                                                  
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "SendDataToHost.h"
#include "PackUnpack.h"
#include "UART1.h"
#include "Route.h"
#include "RADIO.h"
#include "string.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
 
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  SendPackToHost(uint8 addh, uint8 addl, uint8 channel, StructPackType* pt);  //打包数据，并将数据发送到主机

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：SendPackToHost
* 函数功能：打包数据，并将数据发送到主机
* 输入参数：pPackSent，指向结构体变量的地址
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年11月07日
* 注    意：
*********************************************************************************************************/
static  void  SendPackToHost(uint8 addh, uint8 addl, uint8 channel, StructPackType* pt)
{
  SentStructPackType sspt2;
  uint8 packValid  = 0;  //打包正确标志位，默认值为0
  
  sspt2.addh = addh;  //广播地址高位,1
  sspt2.addl = addl;  //广播地址低位,2
  sspt2.channel = channel;  //信道地址,3
  packValid = PackData(pt); //打包数据，加校验和
  
  if(0 < packValid)         //如果打包正确
  {
    sspt2.spt2 = *pt; //复制数据包
    RadioSendData((uint8*)&sspt2+1, PACKLEN+3);  //无线发送数据 uint8--_4Byte;uint8*---4Byte;short---2Byte
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitSendDataToHost
* 函数功能：初始化SendDataToHost模块 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
void  InitSendDataToHost(void)
{
  
}

/*********************************************************************************************************
* 函数名称：SendAckPack
* 函数功能：发送命令应答数据包到主机
* 输入参数：ackMsg为应答消息
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月11日
* 注    意：
*********************************************************************************************************/
void SendAckPack(uint8 addh, uint8 addl,uint8 channel, uint8* ackMsg, uint8 len)
{
  StructPackType  pt;              //包结构体2变量
  
  pt.packType = TYPE_DATA;
  memcpy(pt.arrData, ackMsg, len);
  
  SendPackToHost(addh,addl,channel,&pt);
}

/*********************************************************************************************************
* 函数名称：SendRouteToNeighbor
* 函数功能：广播发送路由信息
* 输入参数：pRouteData-路由数据存放的地址
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年11月6日
* 注    意：
*********************************************************************************************************/
void  SendRouteToNeighbor(uint8* pRouteData, uint8 len)
{
  StructPackType  pt;  //包结构体变量
  
  pt.packType = TYPE_ROUTE;
  memcpy(pt.arrData, pRouteData, len);
  
  SendPackToHost(0xff,0xff,0x00,&pt);
}

/*********************************************************************************************************
* 函数名称：SendDateToParent
* 函数功能：给父结点发送数据
* 输入参数：pSentData-待发送数据存放的地址
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年02月12日
* 注    意：数据数组长度最大为61个字节，太长只发前61个，无应答，不保证传输成功
*********************************************************************************************************/
#if (defined SINK) && (SINK == TRUE)//汇聚节点

#else  //普通节点
void  SendDateToParent(uint8* pSentData, uint8 len)
{
  uint16 P_Add;  //父结点地址
  StructPackType  pt;  //包结构体变量
  memset(&pt, '\0', sizeof(StructPackType));
  
  pt.packType = TYPE_DATA;
  memcpy(pt.arrData, pSentData, len);
  P_Add = GetParentAddr();
  
  if(P_Add == 0xffff)
  {
    debug("未连接Lora网络");
    return;
  }
  SendPackToHost(P_Add>>8, P_Add, 0x00, &pt);
}
#endif
/*********************************************************************************************************
* 函数名称：SendDateToE20
* 函数功能：给E20发送数据
* 输入参数：pSentData-待发送数据存放的地址
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年03月10日
* 注    意：
*********************************************************************************************************/
#if (defined SINK) && (SINK == TRUE)//汇聚节点
void  SendDateToE20(uint8* pSentData, uint8 len)
{
  
}
#endif
