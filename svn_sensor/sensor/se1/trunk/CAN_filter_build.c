/******************************************************************************
* File Name          : CAN_filter_build.c
* Date First Issued  : 04/06/2018
* Board              :
* Description        : Build a composite list for CAN addresses for CAN hw filter
*******************************************************************************/
#include "common.h"
#include "CAN_filter_build.h"
#include "db/gen_db.h"
#include "can_driver_filter.h"

#ifndef NULL
#define NULL 0
#endif

struct CANFILTERFILTERTBL
{
	uint32_t fifo[CANFILTERLISTSIZE];
	uint32_t* pfifo;
	uint32_t* pfifo_begin;
	uint32_t* pfifo_end;
	uint32_t* pfifo_fetch;
};

 struct CANFILTERFILTERTBL f1;
 struct CANFILTERFILTERTBL f0;
static uint8_t flag_init = 0;

/* **************************************************************************************
 * static void build(struct CANFILTERFILTERTBL* p, uint32_t canid);
 * @brief	: Add CAN filtering for CAN fifo
 * @param	: p =  pointer to fifo0 or fifo1 tables
 * @param	: canid = CAN id to be added to list
 * ************************************************************************************** */
static void init(struct CANFILTERFILTERTBL* p)
{
   p->pfifo_end   = &p->fifo[CANFILTERLISTSIZE];
	p->pfifo       = &p->fifo[0];
	p->pfifo_begin = p->pfifo;
	p->pfifo_fetch = p->pfifo_begin;
	return;
}
static void build(struct CANFILTERFILTERTBL* p, uint32_t canid)
{
	uint32_t* px;
	/* One time initialization */
	if (flag_init == 0)
	{
		flag_init = 1;
		init (&f1);
		init (&f0);
	}
	/* Ignore place-holder canid */
	if (canid == CANID_DUMMY) return;

	/* Check for duplicates */
	px = p->pfifo_begin;
	while (px <= p->pfifo)
	{
		if (*px++ == canid)
			return;	// Duplicate found
	}
	*p->pfifo++ = canid; // Add to list

	/* Prevent runaway, jic */
	if (p->pfifo == p->pfifo_end) p->pfifo--;
	return;
}

/* **************************************************************************************
 * void CAN_filter_build_fifo1(uint32_t canid);
 * @brief	: Add CAN filtering for CAN fifo1
 * @param	: canid = CAN id to be added to list
 * ************************************************************************************** */
void CAN_filter_build_fifo1(uint32_t canid)
{
	build(&f1, canid);
	return;
}
/* **************************************************************************************
 * void CAN_filter_build_fifo0(uint32_t canid);
 * @brief	: Add CAN filtering for CAN fifo0
 * @param	: canid = CAN id to be added to list
 * ************************************************************************************** */
void CAN_filter_build_fifo0(uint32_t canid)
{
	build(&f0, canid);
	return;
}
/* **************************************************************************************
 * static uint32_t* fetch(struct CANFILTERFILTERTBL* p);
 * @brief	: fetch canid from list
 * @param	: p =  pointer to fifo0 or fifo1 tables
 * @return	: pointer to canid, or NULL when list exhausted
 * ************************************************************************************** */
static uint32_t* fetch(struct CANFILTERFILTERTBL* p)
{
	if (p->pfifo_fetch == p->pfifo)
		return NULL;
	return p->pfifo_fetch++;
}
/* **************************************************************************************
 * uint32_t* CAN_filter_build_fifo1_fetch(uint8_t r);
 * @brief	: fetch canid from list
 * @param	: r: 1 to re-start from beginning, 0 to continue
 * @return	: pointer to canid, or NULL when list exhausted
 * ************************************************************************************** */
uint32_t* CAN_filter_build_fifo1_fetch(uint8_t r)
{
	if (r == 1) f1.pfifo_fetch = f1.pfifo_begin;
	return fetch(&f1);
}
/* **************************************************************************************
 * uint32_t* CAN_filter_build_fifo0_fetch(uint8_t r);
 * @brief	: fetch canid from list
 * @param	: r: 1 to re-start from beginning, 0 to continue
 * @return	: pointer to canid, or NULL when list exhausted
 * ************************************************************************************** */
uint32_t* CAN_filter_build_fifo0_fetch(uint8_t r)
{
	if (r == 1) f0.pfifo_fetch = f0.pfifo_begin;
	return fetch(&f0);
}
/*******************************************************************************
 * int can_driver_filter_add_one_32b_id(u32 CANnum, u32 id1, u32 fifo);
 * @brief 	: Add one CAN ID into next available slot and designate FIFO
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @param	: id1 = CAN ID
 * @param	: fifo = 0|1 for fifo0|fifo1
 * @return	:  0 = success; 
 *		: -1 = filter edit failed; 
 *		: -2 = CANnum out of bounds
 *		: -3 = fifo out of bounds
 * NOTE: FIFO 0|1 applies to pairs of CAN IDs for the "list mode" (ID-ID)
*******************************************************************************/
/* **************************************************************************************
 * int CAN_filter_build_list_add(void);
 * @brief	: Add list previously built to fifos
 * ************************************************************************************** */
int CAN_filter_build_list_add(void)
{
	uint32_t* pcan;
	int ret;
	uint8_t sw = 1; 
	int i;
	for (i = 1; i <= CANFILTERLISTSIZE; i++)
	{
		pcan = CAN_filter_build_fifo0_fetch(sw);
		if (pcan == NULL) break;
		sw = 0;
		ret = can_driver_filter_add_one_32b_id(0, *pcan, 0);
		if (ret != 0) return ret;
	}
	sw = 1; 
	for (i = 1; i <= CANFILTERLISTSIZE; i++)
	{
		pcan = CAN_filter_build_fifo1_fetch(sw);
		if (pcan == NULL) break;
		sw = 0;
		ret = can_driver_filter_add_one_32b_id(0, *pcan, 1);
		if (ret != 0) return ret;
	}
	return ret;

}

