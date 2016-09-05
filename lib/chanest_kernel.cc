/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <gfdm/chanest_kernel.h>
#include <iostream>
#include <volk/volk.h>
#include <string.h>

namespace gr
{
  namespace gfdm
  {

    chanest_kernel::chanest_kernel(int n_subcarriers, std::vector<gfdm_complex> preamble):
      d_n_subcarriers(n_subcarriers)
    {
      d_preamble_fft_in = (gfdm_complex*) volk_malloc(sizeof(gfdm_complex) * 2 * n_subcarriers, volk_get_alignment());
      d_preamble_fft_out = (gfdm_complex*) volk_malloc(sizeof(gfdm_complex) * 2 * n_subcarriers, volk_get_alignment());
      d_preamble_fft_plan = initialize_fft(d_preamble_fft_out, d_preamble_fft_in, 2 * n_subcarriers, true);
      memcpy(d_preamble_fft_in, &preamble[0], sizeof(gfdm_complex) * 2 *n_subcarriers);
      fftwf_execute(d_preamble_fft_plan);
      d_preamble.resize(2*n_subcarriers);
      std::cout << "known_preamble: ";
      for (int i=0;i < 2*n_subcarriers;++i){
        std::cout<<d_preamble[i];
      }
      std::cout<<std::endl;
    }

    chanest_kernel::~chanest_kernel()
    {
      volk_free(d_preamble_fft_in);
      volk_free(d_preamble_fft_out);
    }
    
    void
    chanest_kernel::get_channel_in_fdomain(gfdm_complex* channel_out, const gfdm_complex* preamble_in)
    {
      gfdm_complex* channel_tmp = (gfdm_complex*) volk_malloc(sizeof(gfdm_complex) * 2 * d_n_subcarriers, volk_get_alignment());
      memcpy(d_preamble_fft_in, preamble_in, sizeof(gfdm_complex) * 2 * d_n_subcarriers);
      fftwf_execute(d_preamble_fft_plan);
      ::volk_32fc_x2_divide_32fc(channel_tmp,d_preamble_fft_out,&d_preamble[0],2*d_n_subcarriers);
      //std::cout<<"channel_tmp: ";
      //for (int i = 0;i<2*d_n_subcarriers;++i){
      //  std::cout<<channel_tmp[i];
      //}
      //std::cout<<std::endl;
      ::volk_32f_x2_add_32f((float*) channel_out,(float*) &channel_tmp[0], (float*) &channel_tmp[d_n_subcarriers], 2*d_n_subcarriers);
      ::volk_32fc_s32fc_multiply_32fc(channel_out,channel_out,(gfdm_complex) 0.5, d_n_subcarriers);
      volk_free(channel_tmp);
    }

    void
    chanest_kernel::remove_cfo(gfdm_complex* p_out, const gfdm_complex* p_in, const float cfo, const int ninput_size)
    {
      gfdm_complex initial_phase = gfdm_complex(1.0, 0.0);
      const float cfo_incr = -1.0f * M_PI * cfo / d_n_subcarriers;
      gfdm_complex phase_increment = gfdm_complex(std::cos(cfo_incr), std::sin(cfo_incr));
      volk_32fc_s32fc_x2_rotator_32fc(p_out, p_in, phase_increment, &initial_phase, ninput_size);
    }
    
    fftwf_plan
    chanest_kernel::initialize_fft(gfdm_complex *out_buf, gfdm_complex *in_buf, const int fft_size, bool forward)
    {
      std::string filename(getenv("HOME"));
      filename += "/.gr_fftw_wisdom";
      FILE *fpr = fopen (filename.c_str(), "r");
      if (fpr != 0)
      {
        int r = fftwf_import_wisdom_from_file (fpr);
        fclose (fpr);
      }

      fftwf_plan plan = fftwf_plan_dft_1d(fft_size,
                                          reinterpret_cast<fftwf_complex *>(in_buf),
                                          reinterpret_cast<fftwf_complex *>(out_buf),
                                          forward ? FFTW_FORWARD : FFTW_BACKWARD,
                                          FFTW_MEASURE);

      FILE *fpw = fopen (filename.c_str(), "w");
      if (fpw != 0)
      {
        fftwf_export_wisdom_to_file (fpw);
        fclose (fpw);
      }
      return plan;
    }

  }
}
