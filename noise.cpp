/*
    BSD 3-Clause License

    Copyright (c) 2018, KORG INC., 2023 Christopher Brand
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

/*
 * File: noise.cpp
 *
 * Noise oscillator
 *
 *     Implemented:
 *      - white
 *      - pink
 *      - brown
 *      - blue
 *      - violet
 *      - grey
 *     To Implement:
 *      - cleaner implementation of pink noise (blue noise)
 */

#include "userosc.h"
#include "noise.hpp"
#include "dsp/biquad.hpp"

static State s_state;
static dsp::BiQuad brownFilter;
static dsp::BiQuad violetFilter;

static dsp::BiQuad greyLPFilter;
static dsp::BiQuad greyHP1Filter;
static dsp::BiQuad greyHP2Filter;
static dsp::BiQuad greyHP3Filter;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  s_state.flags = k_flags_none;
  s_state.noise_type = k_flag_white;
  s_state.counter=0.0f;
  s_state.blueCounter=0.f;

  brownFilter.mCoeffs.setFOLP(tan(PI* brownFilter.mCoeffs.wc(16.35f,1.f/48000.f)));
  violetFilter.mCoeffs.setFOHP(tan(PI* violetFilter.mCoeffs.wc(16744.04f,1.f/48000.f)));

  greyLPFilter.mCoeffs.setSOLP(tan(PI* greyLPFilter.mCoeffs.wc(550.f,1.f/48000.f)), 1.f);
  greyHP1Filter.mCoeffs.setSOHP(tan(PI* greyHP1Filter.mCoeffs.wc(4000.f,1.f/48000.f)), 1.f);  
  greyHP2Filter.mCoeffs.setSOHP(tan(PI* greyHP2Filter.mCoeffs.wc(4000.f,1.f/48000.f)), 1.f);  
  greyHP3Filter.mCoeffs.setSOHP(tan(PI* greyHP3Filter.mCoeffs.wc(4000.f,1.f/48000.f)), 1.f);  

  // prevents the compiler from complaining
  (void)platform;
  (void)api;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{

  const uint8_t flags = s_state.flags;
  const uint8_t osc = s_state.noise_type;

  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;

  if (osc == k_flag_white){
    for (; y <=y_e; ){
      *(y++) = f32_to_q31(_osc_white());
    }
  }
  else if (osc == k_flag_pink){
    // Voss - McCartney algorithm, this might be able to be implemented cleaner
    uint8_t counter = s_state.counter;
    s_state.counter= s_state.counter + (frames % 128);    
    if (s_state.counter>127){
      s_state.counter = 0;
    }

    for (; y <=y_e; ){
      float osc_white_total=_osc_white(); // row -1 aadded in every counter

      if (counter % 2 == 0){
        s_state.row_0=_osc_white();
      }
      else if ((counter-1) % 4 == 0){
        s_state.row_1=_osc_white();        
      }
      else if ((counter -3) % 8 == 0){
        s_state.row_2=_osc_white();
      }
      else if ((counter - 7) % 16 == 0){
        s_state.row_3=_osc_white();
      }
      else if ((counter - 15) % 32 == 0){
        s_state.row_4=_osc_white();
      }
      else if (counter==33 || counter==97){
        s_state.row_5=_osc_white();
      }
      else if (counter==63) {
        s_state.row_6=_osc_white();
      }
      osc_white_total +=s_state.row_0 + s_state.row_1 + s_state.row_2 + s_state.row_3 + s_state.row_4 + s_state.row_5 +s_state.row_6; 
      *(y++) = f32_to_q31(osc_white_total/8.f);

      counter+=1;
      if (counter>127.f){
        counter = 0.f;
      }
    }    
  }
  else if (osc == k_flag_brown){
    // 6.02db/octave low pass filter on white noise, use first order filter
    for (; y <=y_e; ){
      *(y++) = f32_to_q31(brownFilter.process_fo(_osc_white()));
    }
  }
  else if (osc == k_flag_blue){
    // we are going to use pink noise and take the difference of successive samples, aka, pink noise with a first differential operator
    // Voss - McCartney algorithm, this might be able to be implemented cleaner

    uint8_t blueCounter = s_state.counter;
    s_state.counter= s_state.counter + (frames % 128);    
    if (s_state.counter>127){
      s_state.counter = 0;
    }

    for (; y <= y_e;){
      float osc_white_total=_osc_white(); // row -1 aadded in every counter

      if (blueCounter % 2 == 0){
        s_state.blueRow_0=_osc_white();
      }
      else if ((blueCounter - 1) % 4 ==0){
        s_state.blueRow_1=_osc_white();        
      }
      else if ((blueCounter - 3) % 8 == 0){
        s_state.blueRow_2=_osc_white();
      }
      else if ((blueCounter - 7) % 16 == 0){
        s_state.blueRow_3=_osc_white();
      }
      else if ((blueCounter - 15) % 32 == 0){
        s_state.blueRow_4=_osc_white();
      }
      else if (blueCounter==33 || blueCounter==97){
        s_state.blueRow_5=_osc_white();
      }
      else if (blueCounter==63) {
        s_state.blueRow_6=_osc_white();
      }
      osc_white_total +=s_state.blueRow_0 + s_state.blueRow_1 + s_state.blueRow_2 + s_state.blueRow_3 + s_state.blueRow_4 + s_state.blueRow_5 + s_state.blueRow_6;

      *(y++) = f32_to_q31( (osc_white_total/8.f) - s_state.prev_sample);
      s_state.prev_sample = osc_white_total/8.f;

      blueCounter+=1;
      if (blueCounter>127.f){
        blueCounter = 0.f;
      }
    }
  }
  else if (osc == k_flag_violet){
   // 6.02db/octave high pass filter on white noise, use first order filter
    for (; y <=y_e; ){
      *(y++) = f32_to_q31(violetFilter.process_fo(_osc_white()));
    } 
  }
  else{
    // grey
    for (; y <=y_e; ){
      float intermediate;
      
      intermediate = greyLPFilter.process_so(_osc_white());
      intermediate += greyHP3Filter.process_so(greyHP2Filter.process_so(greyHP1Filter.process_so(_osc_white())));

      *(y++) = f32_to_q31(intermediate/2.f);
    }
  }
}

void OSC_NOTEON(const user_osc_param_t * const params)
{  
  s_state.flags = k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  // does nothing, prevents the compiler from complaining we are not using params
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{  
  switch (index) {
  case k_user_osc_param_id1:
  case k_user_osc_param_id2:    
  case k_user_osc_param_id3:
  case k_user_osc_param_id4:
  case k_user_osc_param_id5:    
  case k_user_osc_param_id6:
    break;
    
  case k_user_osc_param_shape:
    {
      // 10bit parameter  (1024 possible values?)
      // we have 6 noise types
      const float user_osc_param = param_val_to_f32(value);
      
      if (user_osc_param<=.170){
        s_state.noise_type = k_flag_white;
      }
      else if (user_osc_param>.170 && user_osc_param<=.340){
        s_state.noise_type = k_flag_pink;
      }
      else if (user_osc_param>.340 && user_osc_param<=.510){
        s_state.noise_type = k_flag_brown;
      }
      else if (user_osc_param>.510 && user_osc_param<=.680){
        s_state.noise_type = k_flag_blue;
      }
      else if (user_osc_param>.680 && user_osc_param<=.850){
        s_state.noise_type = k_flag_violet;
      }
      else{
        s_state.noise_type = k_flag_grey;
      }
      break;
    }
  case k_user_osc_param_shiftshape:
    // 10bit parameter
    break;
    
  default:
    break;
  }
}

