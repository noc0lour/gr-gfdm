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

    chanest_kernel::chanest_kernel(int n_subcarriers, std::vector<gfdm_complex> preamble_data, std::vector<gfdm_complex> preamble_f_taps):
      d_n_subcarriers(n_subcarriers)
    {
      d_receiver_kernel = receiver_kernel_cc::sptr(new receiver_kernel_cc(2, n_subcarriers, 2, preamble_f_taps));
      d_preamble_data.resize(n_subcarriers);
      memcpy(&d_preamble_data[0],&preamble_data[0],sizeof(gfdm_complex)*n_subcarriers);
    }

    chanest_kernel::~chanest_kernel()
    {
    }
    
    void
    chanest_kernel::get_channel_in_fdomain(gfdm_complex* channel_out, const gfdm_complex* preamble_in)
    {
      gfdm_complex* received_preamble_data = (gfdm_complex*) volk_malloc(sizeof(gfdm_complex) * 2 * d_n_subcarriers, volk_get_alignment());
      d_receiver_kernel->generic_work(received_preamble_data,preamble_in);
      ::volk_32f_x2_add_32f((float*) received_preamble_data, (float*) received_preamble_data, (float*) &received_preamble_data[d_n_subcarriers],2*d_n_subcarriers);
      ::volk_32fc_s32fc_multiply_32fc(received_preamble_data, received_preamble_data, (gfdm_complex) (0.5f), d_n_subcarriers);
      ::volk_32fc_x2_divide_32fc(channel_out,received_preamble_data,&d_preamble_data[0],d_n_subcarriers);
    }

    void
    chanest_kernel::remove_cfo(gfdm_complex* p_out, const gfdm_complex* p_in, const float cfo, const int ninput_size)
    {
      gfdm_complex initial_phase = gfdm_complex(1.0, 0.0);
      const float cfo_incr = -1.0f * M_PI * cfo / d_n_subcarriers;
      gfdm_complex phase_increment = gfdm_complex(std::cos(cfo_incr), std::sin(cfo_incr));
      volk_32fc_s32fc_x2_rotator_32fc(p_out, p_in, phase_increment, &initial_phase, ninput_size);
    }
    

  }
}
