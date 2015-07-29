/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp1x_port_dp_tx.c
*
* This contains the implementation of the HDCP port driver for DP TX
* interfaces
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* 1.01         07/23/15 Fixed link verification handling
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"

#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_NUM_INSTANCES > 0)
#include "xhdcp1x_port.h"
#include "xhdcp1x_port_dp.h"
#include "xdp.h"
#include "xdp_hw.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This reads a register from the hdcp port device
*
* @param InstancePtr  the device to read from
* @param Offset  the offset to start reading from
* @param Buf  the buffer to copy the data read
* @param BufSize  the size of the buffer
*
* @return
*   The number of bytes read
*
* @note
*   None.
*
******************************************************************************/
static int RegRead(const XHdcp1x_Port *InstancePtr, u8 Offset, u8 *Buf,
		u32 BufSize)
{
	/* Locals */
	XDptx* DpHw = InstancePtr->PhyIfPtr;
	u32 Address = 0;
	int NumRead = 0;

	/* Determine Address */
	Address = Offset;
	Address += 0x68000u;

	/* Read it */
	if (XDp_TxAuxRead(DpHw, Address, BufSize, Buf) == XST_SUCCESS) {
		NumRead = BufSize;
	}

	/* Return */
	return (NumRead);
}

/*****************************************************************************/
/**
*
* This writes a register from the hdcp port device
*
* @param InstancePtr  the device to write to
* @param Offset  the offset to start writing at
* @param Buf  the buffer containing the data to write
* @param BufSize  the size of the buffer
*
* @return
*   The number of bytes written
*
* @note
*   None.
*
******************************************************************************/
static int RegWrite(XHdcp1x_Port *InstancePtr, u8 Offset, const u8 *Buf,
		u32 BufSize)
{
	/* Locals */
	XDptx* DpHw = InstancePtr->PhyIfPtr;
	u32 Address = 0;
	int NumWritten = 0;

	/* Determine Address */
	Address = Offset;
	Address += 0x68000u;

	/* Write it */
	if (XDp_TxAuxWrite(DpHw, Address, BufSize, (u8*) Buf) == XST_SUCCESS) {
		NumWritten = BufSize;
	}

	/* Return */
	return (NumWritten);
}

/*****************************************************************************/
/**
*
* This function checks for a link integrity check failure or re-auth request
*
* @param InstancePtr  the device to process the failure
*
* @return
*   void
*
* @note
*   None
*
******************************************************************************/
static void CheckForRxStatusChange(XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;

	/* Read the Bstatus register */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1) > 0) {
		u8 ReauthMask = 0;

		/* Determine ReauthMask */
		ReauthMask  = XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE;
		ReauthMask |= XHDCP1X_PORT_BIT_BSTATUS_REAUTH_REQUEST;

		/* Check for link failure or re-authentication requested */
		if ((Value & ReauthMask) != 0) {
			/* Invoke authentication callback if set */
			if (InstancePtr->IsAuthCallbackSet) {
				(*(InstancePtr->AuthCallback))(
						InstancePtr->AuthRef);
			}
		}
	}

	return;
}

/*****************************************************************************/
/**
*
* This function enables a hdcp port device
*
* @param InstancePtr  the id of the device to enable
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxEnable(XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;
	int Status = XST_NOT_ENABLED;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Read anything to ensure that the remote end is present */
	if ((RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1)) > 0) {
		Status = XST_SUCCESS;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function disables a hdcp port device
*
* @param InstancePtr  the id of the device to disable
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxDisable(XHdcp1x_Port *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Nothing to do at this time */

	return (Status);
}

/*****************************************************************************/
/**
*
* This function initializes a hdcp port device
*
* @param InstancePtr  the device to initialize
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxInit(XHdcp1x_Port *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 Base = 0;
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Ensure that the dp video path routes through the hdcp core */
	Base = ((XDptx*) InstancePtr->PhyIfPtr)->Config.BaseAddr;
	Value  = XDptx_ReadReg(Base, XDP_TX_HDCP_ENABLE);
	Value |= XDP_TX_HDCP_ENABLE_BYPASS_DISABLE_MASK;
	XDptx_WriteReg(Base, XDP_TX_HDCP_ENABLE, Value);

	/* Disable it */
	if (XHdcp1x_PortDpTxDisable(InstancePtr) != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function confirms the presence/capability of the remote hdcp device
*
* @param InstancePtr  the device to query
*
* @return
*   Truth value
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxIsCapable(const XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;
	int IsCapable = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for hdcp capable */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1) > 0) {
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_HDCP_CAPABLE) != 0) {
			IsCapable = TRUE;
		}
	}

	return (IsCapable);
}

/*****************************************************************************/
/**
*
* This function confirms if the remote hdcp device is a repeater
*
* @param InstancePtr  the device to query
*
* @return
*   Truth value
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxIsRepeater(const XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;
	int IsRepeater = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for repeater */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1) > 0) {
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_REPEATER) != 0) {
			IsRepeater = TRUE;
		}
	}

	return (IsRepeater);
}

/*****************************************************************************/
/**
*
* This function retrieves the repeater information
*
* @param InstancePtr  the device to query
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxGetRepeaterInfo(const XHdcp1x_Port *InstancePtr, u16 *Info)
{
	u8 Value = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Info != NULL);

	/* Read the remote capabilities */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1) > 0) {

		/* Check for repeater */
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_REPEATER) != 0) {

			/* Read the remote status */
			RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
				&Value, 1);

			/* Check for ready */
			if ((Value & XHDCP1X_PORT_BIT_BSTATUS_READY) != 0) {

				u8 Buf[2];
				u16 U16Value = 0;

				/* Read the Binfo */
				RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BINFO,
					Buf, 2);

				/* Determine U16Value */
				XHDCP1X_PORT_BUF_TO_UINT(U16Value, Buf, 16);

				/* Update Info */
				*Info = (U16Value & 0x0FFFu);

			}
			else {
				Status = XST_DEVICE_BUSY;
			}
		}
		else {
			Status = XST_NO_FEATURE;
		}
	}
	else {
		Status = XST_RECV_ERROR;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function reads a register from a hdcp port device
*
* @param InstancePtr  the device to read from
* @param Offset  the offset to start reading from
* @param Buf  the buffer to copy the data read
* @param BufSize  the size of the buffer
*
* @return
*   The number of bytes read
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxRead(const XHdcp1x_Port *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Read it */
	return (RegRead(InstancePtr, Offset, Buf, BufSize));
}

/*****************************************************************************/
/**
*
* This function writes a register from a hdcp port device
*
* @param InstancePtr  the device to write to
* @param Offset  the offset to start writing to
* @param Buf  the buffer containing the data to write
* @param BufSize  the size of the buffer
*
* @return
*   The number of bytes written
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortDpTxWrite(XHdcp1x_Port *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Write it */
	return (RegWrite(InstancePtr, Offset, Buf, BufSize));
}

/*****************************************************************************/
/**
*
* This handles an interrupt generated by a hdcp port device
*
* @param InstancePtr  the device to write to
* @param IntCause  the interrupt cause bit map
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_PortDpTxIntrHandler(XHdcp1x_Port *InstancePtr, u32 IntCause)
{
	int HpdDetected = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine HpdDetected */
	if (IntCause & XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK) {
		HpdDetected = TRUE;
	}
	else if (IntCause & XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK) {
		HpdDetected = TRUE;
	}

	/* Check for HPD irq */
	if (HpdDetected) {
		CheckForRxStatusChange(InstancePtr);
	}

	return;
}

/*****************************************************************************/
/**
*
* This tables defines the adaptor for the DP TX HDCP port driver
*
******************************************************************************/
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpTxAdaptor =
{
	&XHdcp1x_PortDpTxInit,
	&XHdcp1x_PortDpTxEnable,
	&XHdcp1x_PortDpTxDisable,
	&XHdcp1x_PortDpTxRead,
	&XHdcp1x_PortDpTxWrite,
	&XHdcp1x_PortDpTxIsCapable,
	&XHdcp1x_PortDpTxIsRepeater,
	&XHdcp1x_PortDpTxGetRepeaterInfo,
	&XHdcp1x_PortDpTxIntrHandler,
};

#endif  /* defined(XPAR_XDP_TX_NUM_INSTANCES) && (XPAR_XDP_TX_NUM_INSTANCES > 0) */