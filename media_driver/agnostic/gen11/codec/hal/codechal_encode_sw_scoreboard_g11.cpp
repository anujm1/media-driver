/*
* Copyright (c) 2017-2018, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     codechal_encode_sw_scoreboard_g11.cpp
//! \brief    This file implements the SW scoreboard init feature for all codecs on Gen11 platform
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_sw_scoreboard_g11.h"
#include "codechal_kernel_header_g11.h"

MOS_STATUS CodechalEncodeSwScoreboardG11::InitKernelState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_kernelState)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_kernelState = MOS_New(MHW_KERNEL_STATE));
    }

    uint8_t *binary;
    auto kernelSize = m_combinedKernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuidCommon,
        &binary,
        &kernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG11(
        binary,
        ENC_SCOREBOARD,
        0,
        &currKrnHeader,
        &kernelSize));

    m_kernelState->KernelParams.iBTCount          = swScoreboardNumSurfaces;
    m_kernelState->KernelParams.iThreadCount      = m_hwInterface->GetRenderInterface()->GetHwCaps()->dwMaxThreads;
    m_kernelState->KernelParams.iCurbeLength      = m_curbeLength;
    m_kernelState->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
    m_kernelState->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
    m_kernelState->KernelParams.iIdCount          = 1;
    m_kernelState->KernelParams.iInlineDataLength = 0;
    m_kernelState->dwCurbeOffset                  = m_stateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    m_kernelState->KernelParams.pBinary           =
        binary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    m_kernelState->KernelParams.iSize             = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->CalculateSshAndBtSizesRequested(
        m_kernelState->KernelParams.iBTCount,
        &m_kernelState->dwSshSize,
        &m_kernelState->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderInterface->m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_renderInterface->m_stateHeapInterface, m_kernelState));

    return eStatus;
}
