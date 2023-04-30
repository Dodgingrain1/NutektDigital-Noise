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
    // Voss - McCartney algorithm, this might be able to be implemented cleaner vs the or's
    // get our current counter frame and set the counter frame after processing
    float counter = s_state.counter;
    s_state.counter= s_state.counter + (float)(frames % 128);
    if (s_state.counter>127.0f){
      s_state.counter = 0.f;
    }

    for (; y <=y_e; ){
      float osc_white_total=_osc_white(); // row -1 aadded in every counter

      if (counter==0 || counter==2 || counter==4 || counter==6|| counter==8 || counter==10 || 
          counter==12 || counter==14 || counter==16 || counter==18 || counter==20 || counter==22 || 
          counter==24 || counter==26 || counter==28 || counter==30 || counter==32 || counter==34 ||
          counter==36 || counter==38 || counter==40 || counter==42 || counter==44 || counter==46 ||
          counter==48 || counter==50 || counter==52 || counter==54 || counter==56 || counter==58 ||
          counter==60 || counter==62 || counter==64 || counter==66 || counter==68 || counter==70 ||
          counter==72 || counter==74 || counter==76 || counter==78 || counter==80 || counter==82 ||
          counter==84 || counter==86 || counter==88 || counter==90 || counter==92 || counter==94 ||
          counter==96 || counter==98 || counter==100 || counter==102 || counter==104 || counter==106 ||
          counter==108 || counter==110 || counter==112 || counter==114 || counter==116 || counter==118 ||
          counter==120 || counter==122 || counter==124 || counter==126){
        s_state.row_0=_osc_white();
      }
      else if (counter==1 || counter==5 || counter==9 || counter==13 || counter==17 || 
               counter==21 || counter==25 || counter==29 || counter==33 || counter==37 ||
               counter==41 || counter==45 || counter==49 || counter==53 || counter==57 ||
               counter==61 || counter==65 || counter==69 || counter==73 || counter==77 ||
               counter==81 || counter==85 || counter==89 || counter==93 || counter==97 ||
               counter==101 || counter==105 || counter==109 || counter==113 || counter==117 ||
               counter==121 || counter==125){
        s_state.row_1=_osc_white();        
      }
      else if (counter==3 || counter == 11 || counter == 19 || counter==27 || counter==35 ||
               counter==43 || counter==51 || counter==59 || counter==67 || counter==75 || 
               counter==83 || counter==91 || counter==99 || counter==107 || counter==115 ||
               counter==123){
        s_state.row_2=_osc_white();
      }
      else if (counter==7 || counter==23 || counter==39 || counter== 55 || counter==71 ||
               counter==87 || counter==103 || counter==119){
        s_state.row_3=_osc_white();
      }
      else if (counter==15 || counter==47 || counter==79 || counter==111){
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
    // Voss - McCartney algorithm, this might be able to be implemented cleaner vs the or's
    // get our current counter frame and set the counter frame after processing

    float blueCounter = s_state.blueCounter;
    s_state.blueCounter = s_state.blueCounter + (float)(frames % 128);
    if (s_state.blueCounter > 127){
      s_state.blueCounter = 0.f;
    }

    for (; y <= y_e;){
      float osc_white_total=_osc_white(); // row -1 aadded in every counter

      if (blueCounter==0 || blueCounter==2 || blueCounter==4 || blueCounter==6|| blueCounter==8 || blueCounter==10 || 
          blueCounter==12 || blueCounter==14 || blueCounter==16 || blueCounter==18 || blueCounter==20 || blueCounter==22 || 
          blueCounter==24 || blueCounter==26 || blueCounter==28 || blueCounter==30 || blueCounter==32 || blueCounter==34 ||
          blueCounter==36 || blueCounter==38 || blueCounter==40 || blueCounter==42 || blueCounter==44 || blueCounter==46 ||
          blueCounter==48 || blueCounter==50 || blueCounter==52 || blueCounter==54 || blueCounter==56 || blueCounter==58 ||
          blueCounter==60 || blueCounter==62 || blueCounter==64 || blueCounter==66 || blueCounter==68 || blueCounter==70 ||
          blueCounter==72 || blueCounter==74 || blueCounter==76 || blueCounter==78 || blueCounter==80 || blueCounter==82 ||
          blueCounter==84 || blueCounter==86 || blueCounter==88 || blueCounter==90 || blueCounter==92 || blueCounter==94 ||
          blueCounter==96 || blueCounter==98 || blueCounter==100 || blueCounter==102 || blueCounter==104 || blueCounter==106 ||
          blueCounter==108 || blueCounter==110 || blueCounter==112 || blueCounter==114 || blueCounter==116 || blueCounter==118 ||
          blueCounter==120 || blueCounter==122 || blueCounter==124 || blueCounter==126){
        s_state.blueRow_0=_osc_white();
      }
      else if (blueCounter==1 || blueCounter==5 || blueCounter==9 || blueCounter==13 || blueCounter==17 || 
               blueCounter==21 || blueCounter==25 || blueCounter==29 || blueCounter==33 || blueCounter==37 ||
               blueCounter==41 || blueCounter==45 || blueCounter==49 || blueCounter==53 || blueCounter==57 ||
               blueCounter==61 || blueCounter==65 || blueCounter==69 || blueCounter==73 || blueCounter==77 ||
               blueCounter==81 || blueCounter==85 || blueCounter==89 || blueCounter==93 || blueCounter==97 ||
               blueCounter==101 || blueCounter==105 || blueCounter==109 || blueCounter==113 || blueCounter==117 ||
               blueCounter==121 || blueCounter==125){
        s_state.blueRow_1=_osc_white();        
      }
      else if (blueCounter==3 || blueCounter == 11 || blueCounter == 19 || blueCounter==27 || blueCounter==35 ||
               blueCounter==43 || blueCounter==51 || blueCounter==59 || blueCounter==67 || blueCounter==75 || 
               blueCounter==83 || blueCounter==91 || blueCounter==99 || blueCounter==107 || blueCounter==115 ||
               blueCounter==123){
        s_state.blueRow_2=_osc_white();
      }
      else if (blueCounter==7 || blueCounter==23 || blueCounter==39 || blueCounter== 55 || blueCounter==71 ||
               blueCounter==87 || blueCounter==103 || blueCounter==119){
        s_state.blueRow_3=_osc_white();
      }
      else if (blueCounter==15 || blueCounter==47 || blueCounter==79 || blueCounter==111){
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

