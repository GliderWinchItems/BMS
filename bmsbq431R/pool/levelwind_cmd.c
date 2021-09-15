/******************************************************************************
* File Name          : levelwind_cmd.c
* Date First Issued  : 11/11/2020
* Description        : Levelwind handle command msgs
*******************************************************************************/
/*
11/11/2020 drum:levelwind (repo:branch)
*/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "malloc.h"
#include "stm32f4xx_hal.h"
#include "morse.h"
#include "yprintf.h"
#include "main.h"
#include "levelwind_items.h"
#include "LevelwindTask.h"
#include "DTW_counter.h"
#include "drum_items.h"
#include "levelwind_switches.h"
#include "levelwind_cmd.h"
#include "../../../GliderWinchCommons/embed/svn_common/trunk/db/gen_db.h"

static void cmd_get_reading(struct CANRCVBUF* pcan);

/* Union for sending various types of four byte CAN payloads. */
union X4
{
   uint8_t  u8[4];
   uint16_t u16[2];
   int32_t  s32;
   uint32_t u32;
   float    ff;
};

/* *************************************************************************
 * void levelwind_CANrcv_cid_cmd_levelwind_i1(struct CANRCVBUF* pcan);
 * @brief   : Incoming (LW receives) CAN command
 * *************************************************************************/
void levelwind_CANrcv_cid_cmd_levelwind_i1(struct CANRCVBUF* pcan)
{
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer

   /* Is this command for us, i.e. 'mydrum'? */
   if ((p->mydrumbit & pcan->cd.uc[0]) == 0) return;

   /* Command dispatch */
   switch (pcan->cd.uc[1]) // Dispatch sub-commands
   {
      case CMD_GET_READING: // Send a reading as the response
         cmd_get_reading(pcan); // Set up and send the reading
         break;
   }
   return;
}   
/* *************************************************************************
 * static void load_x4(struct LEVELWINDFUNCTION* p, union X4 x4);
 * @brief   : Send a reading based on sub-command code
 * *************************************************************************/
static void load_x4(struct LEVELWINDFUNCTION* p, union X4 x4)
{
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[3] = x4.u8[0];
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[4] = x4.u8[1];
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[5] = x4.u8[2];
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[6] = x4.u8[3];
   return;
}
/* *************************************************************************
 * void static void_cmd_get_reading(struct CANRCVBUF* pcan);
 * @brief   : Send a reading based on sub-command code
 * *************************************************************************/
static void cmd_get_reading(struct CANRCVBUF* pcan)
{
   struct LEVELWINDFUNCTION* p = &levelwindfunction; // Convenience pointer

   union X4 x4;

   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[0] = p->mydrumbit;
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[1] = pcan->cd.uc[1];
   p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[2] = pcan->cd.uc[2];

   /* Sub-command determines reading sent. */
   switch (pcan->cd.uc[2])
   {
   case LWCMD_SWITCHES: // 0  Levelwind switches (uint32_t)
      x4.u32 = (GPIOE->IDR >> 7); // PE14-PE7 right justified
      break;

   case LWCMD_BUS12V:   // 1  CAN bus voltage (float)
      x4.ff = 12.0; // Dummy
      break;

   case LWCMD_STEPVOLT: // 2  Stepper Controller voltage (float)
         x4.ff = 96.0; // Dummy
      break;

   case LWCMD_POSACC_MS:// 3  Position accumulator at motor-side limit sw closure
         x4.u32 = 1234; // dummy
      break;

   case LWCMD_PSACC_MSN:// 4  Position accumulator at not-motor-side limit sw closure
         x4.u32 = 5678; // dummy
      break;

   default: // Sub-command is not in these case statements
      x4.u32 = 0;  // Zero out any previous payload
      p->canmsg[IDX_CID_CMD_LEVELWIND_R1].can.cd.uc[2] = LWCMD_UNDEF;
      break;   
   }
   /* Load response value into four byte payload. */
   load_x4(p,x4);

  /* Queue CAN msg to send. */
   xQueueSendToBack(CanTxQHandle,&p->canmsg[IDX_CID_CMD_LEVELWIND_R1],4);   
   return;   

}

         
