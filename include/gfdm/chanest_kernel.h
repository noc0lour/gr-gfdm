/* -*- c++ -*- */
/*
 * Copyright 2016 Andrej Rode
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

#ifndef INCLUDED_GFDM_CHANEST_KERNEL_H
#define INCLUDED_GFDM_CHANEST_KERNEL_H

#include <complex>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <fftw3.h>
#include <stdexcept>
#include <gfdm/receiver_kernel_cc.h>

namespace gr {
  namespace gfdm {

    /*!
     * \brief GFDM Channel estimation
     *  This class initializes and performs all operations necessary to estiamte a GFDM channel.
     *
     */
    class  chanest_kernel
    {
    public:
      typedef std::complex<float> gfdm_complex;
      typedef boost::shared_ptr<chanest_kernel> sptr;
      void get_channel_in_fdomain(gfdm_complex* channel_out, const gfdm_complex* preamble_in);
      void remove_cfo(gfdm_complex* p_out, const gfdm_complex* p_in, const float cfo, const int ninput_size);

      chanest_kernel(int n_subcarriers, std::vector<gfdm_complex> preamble_data, std::vector<gfdm_complex> preamble_f_taps);
      ~chanest_kernel();

    private:
      int d_n_subcarriers;
      receiver_kernel_cc::sptr d_receiver_kernel;
      std::vector<gfdm_complex> d_preamble_data;
      std::vector<gfdm_complex> d_preamble_f_taps;


    };
  } /* namespace gfdm */
} /* namespace gr */

#endif /* INCLUDED_GFDM_CHANEST_KERNEL_H */



