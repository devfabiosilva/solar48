#ifndef HAL_H
 #define HAL_H

#include <stm32f103x6.h>
/**
  * @brief  HAL Lock structures definition
  */
typedef enum
{
  HAL_UNLOCKED = 0x00U,
  HAL_LOCKED   = 0x01U
} HAL_LockTypeDef;

/**
  * @brief  HAL Status structures definition
  */
typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

/**
  * @brief  PCD State structure definition
  */
typedef enum
{
  HAL_PCD_STATE_RESET   = 0x00,
  HAL_PCD_STATE_READY   = 0x01,
  HAL_PCD_STATE_ERROR   = 0x02,
  HAL_PCD_STATE_BUSY    = 0x03,
  HAL_PCD_STATE_TIMEOUT = 0x04
} PCD_StateTypeDef;

/* Device LPM suspend state */
typedef enum
{
  LPM_L0 = 0x00, /* on */
  LPM_L1 = 0x01, /* LPM L1 sleep */
  LPM_L2 = 0x02, /* suspend */
  LPM_L3 = 0x03, /* off */
} PCD_LPM_StateTypeDef;

/** @addtogroup Exported_types
  * @{
  */  
typedef enum 
{
  RESET = 0, 
  SET = !RESET
} FlagStatus, ITStatus;

typedef enum 
{
  DISABLE = 0, 
  ENABLE = !DISABLE
} FunctionalState;
//#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum
{
  SUCCESS = 0U,
  ERROR = !SUCCESS
} ErrorStatus;

typedef struct
{
  uint8_t   num;                  /*!< Endpoint number
                                       This parameter must be a number between Min_Data = 1 and Max_Data = 15   */

  uint8_t   is_in;                /*!< Endpoint direction
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 1    */

  uint8_t   is_stall;             /*!< Endpoint stall condition
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 1    */

#if defined (USB_OTG_FS)
//  uint8_t   is_iso_incomplete;    /*!< Endpoint isoc conditionThis parameter must be a number between Min_Data = 0 and Max_Data = 1    */
#endif /* defined (USB_OTG_FS) */

  uint8_t   type;                 /*!< Endpoint type
                                       This parameter can be any value of @ref USB_LL_EP_Type                   */

  uint8_t   data_pid_start;       /*!< Initial data PID This parameter must be a number between Min_Data = 0 and Max_Data = 1    */

#if defined (USB)
//  uint16_t  pmaadress;            /*!< PMA Address This parameter can be any value between Min_addr = 0 and Max_addr = 1K   */

//  uint16_t  pmaaddr0;             /*!< PMA Address0 This parameter can be any value between Min_addr = 0 and Max_addr = 1K   */

//  uint16_t  pmaaddr1;             /*!< PMA Address1 This parameter can be any value between Min_addr = 0 and Max_addr = 1K   */

//  uint8_t   doublebuffer;         /*!< Double buffer enable This parameter can be 0 or 1                                             */
#endif /* defined (USB) */

  uint32_t  maxpacket;            /*!< Endpoint Max packet size
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 64KB */

  uint8_t   *xfer_buff;           /*!< Pointer to transfer buffer                                               */

  uint32_t  xfer_len;             /*!< Current transfer length                                                  */

  uint32_t  xfer_count;           /*!< Partial transfer length in case of multi packet transfer                 */

#if defined (USB_OTG_FS)
//  uint8_t   even_odd_frame;       /*!< IFrame parity This parameter must be a number between Min_Data = 0 and Max_Data = 1    */

//  uint16_t  tx_fifo_num;          /*!< Transmission FIFO number This parameter must be a number between Min_Data = 1 and Max_Data = 15   */

//  uint32_t  dma_addr;             /*!< 32 bits aligned transfer buffer address                                  */

//  uint32_t  xfer_size;            /*!< requested transfer size                                                  */
#endif /* defined (USB_OTG_FS) */

#if defined (USB)
//  uint32_t  xfer_len_db;          /*!< double buffer transfer length used with bulk double buffer in            */

//  uint8_t   xfer_fill_db;         /*!< double buffer Need to Fill new buffer  used with bulk_in                 */
#endif /* defined (USB) */
} USB_EPTypeDef;

/**
  * @brief  USB Instance Initialization Structure definition
  */
typedef struct
{
  uint8_t dev_endpoints;            /*!< Device Endpoints number.
                                         This parameter depends on the used USB core.
                                         This parameter must be a number between Min_Data = 1 and Max_Data = 15 */

#if defined (USB_OTG_FS)
  uint8_t Host_channels;            /*!< Host Channels number.
                                         This parameter Depends on the used USB core.
                                         This parameter must be a number between Min_Data = 1 and Max_Data = 15 */
#endif /* defined (USB_OTG_FS) */

  uint8_t dma_enable;              /*!< USB DMA state.
                                         If DMA is not supported this parameter shall be set by default to zero */

  uint8_t speed;                   /*!< USB Core speed.
                                        This parameter can be any value of @ref PCD_Speed/HCD_Speed
                                                                                (HCD_SPEED_xxx, HCD_SPEED_xxx) */

  uint8_t ep0_mps;                 /*!< Set the Endpoint 0 Max Packet size.                                    */

  uint8_t phy_itface;              /*!< Select the used PHY interface.
                                        This parameter can be any value of @ref PCD_PHY_Module/HCD_PHY_Module  */

  uint8_t Sof_enable;              /*!< Enable or disable the output of the SOF signal.                        */

  uint8_t low_power_enable;        /*!< Enable or disable the low Power Mode.                                  */

  uint8_t lpm_enable;              /*!< Enable or disable Link Power Management.                               */

  uint8_t battery_charging_enable; /*!< Enable or disable Battery charging.                                    */

#if defined (USB_OTG_FS)
//TODO remove
//  uint8_t vbus_sensing_enable;     /*!< Enable or disable the VBUS Sensing feature.                            */

//  uint8_t use_dedicated_ep1;       /*!< Enable or disable the use of the dedicated EP1 interrupt.              */

//  uint8_t use_external_vbus;       /*!< Enable or disable the use of the external VBUS.                        */
  
#endif /* defined (USB_OTG_FS) */
} USB_CfgTypeDef;

/** @defgroup USB_LL Device Speed
  * @{
  */
#define USBD_FS_SPEED                          2U
#define USBH_FSLS_SPEED                        1U

/** @defgroup PCD_Speed PCD Speed
  * @{
  */
  //TODO replace by USBD_FS_SPEED
#define PCD_SPEED_FULL               USBD_FS_SPEED

//#if defined (USB)
typedef USB_TypeDef        PCD_TypeDef;
typedef USB_CfgTypeDef     PCD_InitTypeDef;
typedef USB_EPTypeDef      PCD_EPTypeDef;
//#endif /* defined (USB) */

typedef struct
{
  PCD_TypeDef             *Instance;   /*!< Register base address             */
  PCD_InitTypeDef         Init;        /*!< PCD required parameters           */
  __IO uint8_t            USB_Address; /*!< USB Address                       */
#if defined (USB_OTG_FS)
//TODO REMOVE IT
//  PCD_EPTypeDef           IN_ep[16];   /*!< IN endpoint parameters            */
//  PCD_EPTypeDef           OUT_ep[16];  /*!< OUT endpoint parameters           */
#endif /* defined (USB_OTG_FS) */
#if defined (USB)
//TODO IS NECESSARY?
//  PCD_EPTypeDef           IN_ep[8];    /*!< IN endpoint parameters            */
//  PCD_EPTypeDef           OUT_ep[8];   /*!< OUT endpoint parameters           */
#endif /* defined (USB) */
  HAL_LockTypeDef         Lock;        /*!< PCD peripheral status             */
  __IO PCD_StateTypeDef   State;       /*!< PCD communication state           */
  __IO  uint32_t          ErrorCode;   /*!< PCD Error code                    */
  uint32_t                Setup[12];   /*!< Setup packet buffer               */
  PCD_LPM_StateTypeDef    LPM_State;   /*!< LPM State                         */
  uint32_t                BESL;
  uint32_t                FrameNumber; /*!< Store Current Frame number        */

  void                    *pData;      /*!< Pointer to upper stack Handler */

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
//TODO Remove it
//  void (* SOFCallback)(struct __PCD_HandleTypeDef *hpcd);                              /*!< USB OTG PCD SOF callback                */
//  void (* SetupStageCallback)(struct __PCD_HandleTypeDef *hpcd);                       /*!< USB OTG PCD Setup Stage callback        */
//  void (* ResetCallback)(struct __PCD_HandleTypeDef *hpcd);                            /*!< USB OTG PCD Reset callback              */
//  void (* SuspendCallback)(struct __PCD_HandleTypeDef *hpcd);                          /*!< USB OTG PCD Suspend callback            */
//  void (* ResumeCallback)(struct __PCD_HandleTypeDef *hpcd);                           /*!< USB OTG PCD Resume callback             */
//  void (* ConnectCallback)(struct __PCD_HandleTypeDef *hpcd);                          /*!< USB OTG PCD Connect callback            */
//  void (* DisconnectCallback)(struct __PCD_HandleTypeDef *hpcd);                       /*!< USB OTG PCD Disconnect callback         */

//  void (* DataOutStageCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);      /*!< USB OTG PCD Data OUT Stage callback     */
//  void (* DataInStageCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);       /*!< USB OTG PCD Data IN Stage callback      */
//  void (* ISOOUTIncompleteCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);  /*!< USB OTG PCD ISO OUT Incomplete callback */
//  void (* ISOINIncompleteCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);   /*!< USB OTG PCD ISO IN Incomplete callback  */

//  void (* MspInitCallback)(struct __PCD_HandleTypeDef *hpcd);                          /*!< USB OTG PCD Msp Init callback           */
//  void (* MspDeInitCallback)(struct __PCD_HandleTypeDef *hpcd);                        /*!< USB OTG PCD Msp DeInit callback         */
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
} PCD_HandleTypeDef;


HAL_StatusTypeDef HAL_PCD_Init(PCD_HandleTypeDef *);

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

HAL_StatusTypeDef USB_DisableGlobalInt(USB_TypeDef *USBx);

#define __HAL_PCD_ENABLE(__HANDLE__)                       (void)USB_EnableGlobalInt ((__HANDLE__)->Instance)
#define __HAL_PCD_DISABLE(__HANDLE__)                      (void)USB_DisableGlobalInt ((__HANDLE__)->Instance)

#endif

