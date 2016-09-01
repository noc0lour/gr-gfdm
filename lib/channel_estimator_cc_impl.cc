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
    channel_estimator_cc::make(int timeslots, int subcarriers, int cp_len, std::vector<gr_complex> preamble, const std::string& gfdm_block_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new channel_estimator_cc_impl(timeslots, subcarriers, cp_len, preamble, gfdm_block_tag_key));
    }

    /*
     * The private constructor
     */
    channel_estimator_cc_impl::channel_estimator_cc_impl(int n_timeslots, int n_subcarriers, int cp_len, std::vector<gr_complex> preamble, const std::string& gfdm_block_tag_key)
      : gr::block("channel_estimator_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_n_timeslots(n_timeslots),
      d_n_subcarriers(n_subcarriers),
      d_cp_len(cp_len),
      d_gfdm_block_tag_key(gfdm_block_tag_key),
      d_block_len(n_subcarriers*n_timeslots)
    {
      set_output_multiple(n_timeslots*n_subcarriers);
      set_tag_propagation_policy(TPP_DONT);
      d_kernel = new chanest_kernel(n_subcarriers, preamble);
      d_cfo = 0.0f;
      d_frame_len = d_block_len + cp_len + 2*n_subcarriers;
    }

    /*
     * Our virtual destructor.
     */
    channel_estimator_cc_impl::~channel_estimator_cc_impl()
    {
      delete d_kernel;
    }

    void
    channel_estimator_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      int blocks = noutput_items/(d_n_subcarriers*d_n_timeslots);
      for (std::vector<int>::iterator it = ninput_items_required.begin(); it != ninput_items_required.end(); ++it)
      {
        *it = blocks*d_frame_len;
      }
    }

    int
    channel_estimator_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      std::vector< tag_t > cfo_tag(1);
      gr_complex* frame_tmp = (gr_complex*) volk_malloc(sizeof(gr_complex)*d_frame_len,volk_get_alignment());
      std::vector<gr_complex> channel_taps(d_n_subcarriers);
      
      const int n_blocks = noutput_items / d_block_len;
      for (int i = 0; i < n_blocks; ++i)
      {
        get_tags_in_window(cfo_tag,0,i*d_frame_len,(i+1)*d_frame_len,pmt::string_to_symbol("cfo"));
        if (cfo_tag.size() > 0)
        {
          d_cfo = pmt::to_float(cfo_tag.begin()->value);
        }
        d_kernel->remove_cfo(frame_tmp, in, d_cfo, d_frame_len);
        d_kernel->get_channel_in_fdomain(&channel_taps[0],frame_tmp);
        produce_output(out,frame_tmp,&channel_taps[0], i);
        in += d_frame_len;
        out += d_block_len;
      }
      consume_each (n_blocks*d_frame_len);
      return n_blocks*d_block_len;
    }

    void
    channel_estimator_cc_impl::produce_output(gr_complex* out, const gr_complex* frame_in, const gr_complex* channel_taps, const int block_number)
    {
      frame_in += d_frame_len - d_block_len;
      pmt::pmt_t channel_pmt =  pmt::init_c32vector(d_n_subcarriers, channel_taps);
      memcpy(out, frame_in, sizeof(gr_complex)*d_block_len);
      add_item_tag(0, nitems_written(0)+block_number*d_block_len,
          pmt::string_to_symbol("channel_taps"),
          channel_pmt);
      add_item_tag(0, nitems_written(0)+block_number*d_block_len,
          pmt::string_to_symbol(d_gfdm_block_tag_key),
          pmt::from_long(d_block_len));
    }


  } /* namespace gfdm */
} /* namespace gr */

