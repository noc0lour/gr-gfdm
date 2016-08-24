/* -*- c++ -*- */
/* 
 * Copyright 2016 Andrej Rode.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "channel_estimator_cc_impl.h"

namespace gr {
  namespace gfdm {

    channel_estimator_cc::sptr
    channel_estimator_cc::make(int timeslots, int subcarriers, int cp_len, std::vector<gr_complex> preamble)
    {
      return gnuradio::get_initial_sptr
        (new channel_estimator_cc_impl(timeslots, subcarriers, cp_len, preamble));
    }

    /*
     * The private constructor
     */
    channel_estimator_cc_impl::channel_estimator_cc_impl(int n_timeslots, int n_subcarriers, int cp_len, std::vector<gr_complex> preamble)
      : gr::block("channel_estimator_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_n_timeslots(n_timeslots),
      d_n_subcarriers(n_subcarriers),
      d_cp_len(cp_len)
    {
      set_output_multiple(d_n_timeslots*d_n_subcarriers);
     


    }

    /*
     * Our virtual destructor.
     */
    channel_estimator_cc_impl::~channel_estimator_cc_impl()
    {
    }

    void
    channel_estimator_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = noutput_items + 2*d_n_subcarriers + d_cp_len;
    }

    int
    channel_estimator_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];


      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


  } /* namespace gfdm */
} /* namespace gr */

