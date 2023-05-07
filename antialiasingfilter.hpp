#pragma once
/*
    BSD 3-Clause License

    Copyright (c) 2023, Christopher Brand
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//*/

/**
 * @file    antialiasingfilter.hpp
 * @brief   Generic anti-aliasing filter
 *
 * @addtogroup dsp DSP
 * @{
 *
 */

#include "dsp/biquad.hpp"

struct AntiAliasingFilter{
    float overSampleFreq = 0.f;
    float overSampleQ = 1.f;

    dsp::BiQuad overSamplingFilter1, overSamplingFilter2, overSamplingFilter3, overSamplingFilter4, overSamplingFilter5, overSamplingFilter6, overSamplingFilter7, overSamplingFilter8, overSamplingFilter9, overSamplingFilter10;

    inline __attribute__((optimize("Ofast"),always_inline))
    void init(const float overSampleFreq, const float overSampleQ){
        this->overSampleFreq = overSampleFreq; 
        this->overSampleQ = overSampleQ;
  
        overSamplingFilter1.mCoeffs.setSOLP(tan(PI* overSamplingFilter1.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter2.mCoeffs.setSOLP(tan(PI* overSamplingFilter2.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter3.mCoeffs.setSOLP(tan(PI* overSamplingFilter3.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter4.mCoeffs.setSOLP(tan(PI* overSamplingFilter4.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter5.mCoeffs.setSOLP(tan(PI* overSamplingFilter5.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter6.mCoeffs.setSOLP(tan(PI* overSamplingFilter6.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter7.mCoeffs.setSOLP(tan(PI* overSamplingFilter7.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter8.mCoeffs.setSOLP(tan(PI* overSamplingFilter8.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter9.mCoeffs.setSOLP(tan(PI* overSamplingFilter9.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
        overSamplingFilter10.mCoeffs.setSOLP(tan(PI* overSamplingFilter10.mCoeffs.wc(overSampleFreq,1.f/96000.f)), overSampleQ);
    }

    inline __attribute__((optimize("Ofast"),always_inline))
    float olddecimate (const float xn){
        return overSamplingFilter1.process_so(overSamplingFilter2.process_so(overSamplingFilter3.process_so(overSamplingFilter4.process_so(overSamplingFilter5.process_so(overSamplingFilter6.process_so(overSamplingFilter7.process_so(overSamplingFilter8.process_so(overSamplingFilter9.process_so(xn)))))))));
    }


    inline __attribute__((optimize("Ofast"),always_inline))
    void upsample (float preUpSampleBuffer[], float postUpSampleBuffer[], const float frames){    
        // need to apply a filter, is this before or after the upsample??
        // since this is a 2x there really isn't a need

        // copy over buffer
        for (int i = 0; i<frames; i++){
            postUpSampleBuffer[i * 2] = preUpSampleBuffer[i];    
        }
    }

    inline __attribute__((optimize("Ofast"),always_inline))
    void decimate (float postUpSampleBuffer[], float postDownSampleBuffer[], const float frames){
        // filter
        for (int i = 0; i<frames * 2; i++){
            postUpSampleBuffer[i]=overSamplingFilter1.process_so(
                                  overSamplingFilter2.process_so(
                                  overSamplingFilter3.process_so(
                                  overSamplingFilter4.process_so(
                                  overSamplingFilter5.process_so(
                                  overSamplingFilter6.process_so(
                                  overSamplingFilter7.process_so(
                                  overSamplingFilter8.process_so(
                                  overSamplingFilter9.process_so(
                                  overSamplingFilter10.process_so(postUpSampleBuffer[i]))))))))));            
        }

        // downsample
        for (int i = 0; i<frames; i++){
            postDownSampleBuffer[i] = postUpSampleBuffer[i * 2];
        }
    }
};

/** @} */