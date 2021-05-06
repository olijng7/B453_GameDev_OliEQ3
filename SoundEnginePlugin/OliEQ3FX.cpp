/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "OliEQ3FX.h"
#include "../OliEQ3Config.h"

#include <AK/AkWwiseSDKVersion.h>

#include <cmath>
//#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265359
#endif

AK::IAkPlugin* CreateOliEQ3FX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, OliEQ3FX());
}

AK::IAkPluginParam* CreateOliEQ3FXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, OliEQ3FXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(OliEQ3FX, AkPluginTypeEffect, OliEQ3Config::CompanyID, OliEQ3Config::PluginID)

OliEQ3FX::OliEQ3FX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

OliEQ3FX::~OliEQ3FX()
{
}

AKRESULT OliEQ3FX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (OliEQ3FXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    //Basic initialization of vars
    m_previousOutput.resize(in_rFormat.GetNumChannels(), 0.0f);
    //m_previousX.resize(in_rFormat.GetNumChannels(), 0.0f);
    m_sampleRate = in_rFormat.uSampleRate;

    //EQ intermediate params initialize
    m_A = pow(10.0f, m_pParams->RTPC.fGain / 40.0f);
    m_w = (2.0f * M_PI * m_pParams->RTPC.fFrequency) / m_sampleRate;
    m_sn = sin(m_w);
    m_cs = cos(m_w);
    m_alpha = m_sn / (2.0f * m_pParams->RTPC.fQ);

    //EQ coefficients initialize
    m_b0 = 1.0f + m_alpha * m_A;
    m_b1 = -2.0f * m_cs;
    m_b2 = 1.0f - m_alpha * m_A;
    m_a0 = 1.0f + m_alpha / m_A;
    m_a1 = -2.0f * m_cs;
    m_a2 = 1 - m_alpha / m_A;

    return AK_Success;
}

AKRESULT OliEQ3FX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT OliEQ3FX::Reset()
{
    return AK_Success;
}

AKRESULT OliEQ3FX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bCanProcessObjects = false;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void OliEQ3FX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();

    AkUInt16 uFramesProcessed;
    for (AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);

        uFramesProcessed = 0;
        while (uFramesProcessed < io_pBuffer->uValidFrames)
        {
            // Execute DSP in-place here
            // y[n] =  m_previousOutput[i] (will update after execution)
            // x[n] = pBuf[uFramesProcessed] (will update after execution: becomes the processed frame)
            // y[n-1] =  m_previousOutput[i] (before execution)
            m_xn = pBuf[uFramesProcessed];
            m_ynm1 = m_previousOutput[i];
            m_yn = m_previousOutput[i] = pBuf[uFramesProcessed] = (m_b0 * m_xn + m_b1 * m_xnm1 + m_b2 * m_xnm2 - m_a1 * m_ynm1 - m_a2 * m_ynm2) / m_a0;

            // move values up
            m_xnm2 = m_xnm1;
            m_xnm1 = m_xn;
            m_ynm2 = m_ynm1;
            m_ynm1 = m_yn;

            // Recalculate coefficients
            // TODO: Optimize this/encapsulation in another class

            //EQ intermediate params
            m_A = pow(10.0f, m_pParams->RTPC.fGain / 40.0f);
            m_w = (2.0f * M_PI * m_pParams->RTPC.fFrequency) / m_sampleRate;
            m_sn = sin(m_w);
            m_cs = cos(m_w);
            m_alpha = m_sn / (2.0f * m_pParams->RTPC.fQ);

            //EQ coefficients
            m_b0 = 1.0f + m_alpha * m_A;
            m_b1 = -2.0f * m_cs;
            m_b2 = 1.0f - m_alpha * m_A;
            m_a0 = 1.0f + m_alpha / m_A;
            m_a1 = -2.0f * m_cs;
            m_a2 = 1 - m_alpha / m_A;

            ++uFramesProcessed;
        }
    }
}

AKRESULT OliEQ3FX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
