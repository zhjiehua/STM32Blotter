/*! 
 *  \file cmd_queue.h
 *  \brief ����ָ�����
 *  \version 1.0
 *  \date 2012-2015
 *  \copyright ���ݴ�ʹ��Ƽ����޹�˾
 */

#ifndef _CMD_QUEUE
#define _CMD_QUEUE
#include "hmi_driver.h"

typedef unsigned char qdata;
typedef unsigned short qsize;

#define CMD_HEAD 0XEE  //֡ͷ
#define CMD_TAIL 0XFFFCFFFF //֡β

typedef struct _QUEUE
{
	qsize _head; //����ͷ
	qsize _tail;  //����β
	qdata _data[QUEUE_MAX_SIZE];	//�������ݻ�����
}QUEUE;

/*! 
 *  \brief  ���ָ������
 */
void queue_reset(void);

/*! 
 * \brief  ���ָ������
 * \detial ���ڽ��յ����ݣ�ͨ���˺�������ָ����� 
 *  \param  _data ָ������
 */
void queue_push(qdata _data);

/*! 
 *  \brief  ��ָ�������ȡ��һ��������ָ��
 *  \param  cmd ָ����ջ�����
 *  \param  buf_len ָ����ջ�������С
 *  \return  ָ��ȣ�0��ʾ������������ָ��
 */
qsize queue_find_cmd(qdata *cmd,qsize buf_len);

void queue_pop(qdata* _data);

extern QUEUE que;

#endif
