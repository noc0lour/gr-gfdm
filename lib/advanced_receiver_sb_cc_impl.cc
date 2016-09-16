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
#include "advanced_receiver_sb_cc_impl.h"
#include <volk/volk.h>
#include <pmt/pmt.h>

namespace gr {
  namespace gfdm {

    advanced_receiver_sb_cc::sptr
    advanced_receiver_sb_cc::make(int n_timeslots, int n_subcarriers, int overlap, int ic_iter, std::vector< gr_complex > frequency_taps, gr::digital::constellation_sptr constellation)
    {
      return gnuradio::get_initial_sptr
        (new advanced_receiver_sb_cc_impl(n_timeslots, n_subcarriers, overlap, ic_iter, frequency_taps, constellation));
    }

    /*
     * The private constructor
     */
    advanced_receiver_sb_cc_impl::advanced_receiver_sb_cc_impl(int n_timeslots, int n_subcarriers, int overlap, int ic_iter, std::vector< gr_complex > frequency_taps, gr::digital::constellation_sptr constellation)
      : gr::sync_block("advanced_receiver_sb_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_n_timeslots(n_timeslots),
      d_n_subcarriers(n_subcarriers),
      d_constellation(constellation),
      d_ic_iter(ic_iter)
    {
      d_kernel = receiver_kernel_cc::sptr(new receiver_kernel_cc(n_timeslots, n_subcarriers, overlap, frequency_taps));
      set_output_multiple(d_kernel->block_size());
      
      //Initialize vector of vectors for temporary subcarrier data
      d_sc_fdomain.resize(d_n_subcarriers);
      for (std::vector< std::vector<gr_complex> >::iterator it = d_sc_fdomain.begin(); it != d_sc_fdomain.end(); ++it)
      {
        it->resize(d_n_timeslots);
      }
      d_sc_symbols.resize(d_n_subcarriers);
      for (std::vector< std::vector<gr_complex> >::iterator it = d_sc_symbols.begin(); it != d_sc_symbols.end(); ++it)
      {
        it->resize(d_n_timeslots);
      }
      d_ic_syms.resize(d_n_subcarriers);
      for (std::vector< std::vector<gr_complex> >::iterator it = d_ic_syms.begin(); it != d_ic_syms.end(); ++it)
      {
        it->resize(d_n_timeslots);
      }

      d_freq_block = (gr_complex *) volk_malloc(sizeof(gr_complex) * d_kernel->block_size(), volk_get_alignment());
      d_ic_time_buffer = (gr_complex *) volk_malloc(sizeof(gr_complex) * d_kernel->block_size(), volk_get_alignment());
      d_ic_freq_buffer = (gr_complex *) volk_malloc(sizeof(gr_complex) * d_kernel->block_size(), volk_get_alignment());

    }

    /*
     * Our virtual destructor.
     */
    advanced_receiver_sb_cc_impl::~advanced_receiver_sb_cc_impl()
    {
      volk_free(d_ic_freq_buffer);
      volk_free(d_freq_block);
      volk_free(d_ic_time_buffer);
    }

    void advanced_receiver_sb_cc_impl::check_if_equal(const std::vector<std::vector<gr_complex> > &first,
                        const std::vector<std::vector<gr_complex> > &second)
    {
      std::cout << "START check_if_equal M=" << d_n_timeslots << ", K=" << d_n_subcarriers << std::endl;
      for (int k = 0; k < d_n_subcarriers; ++k) {
        for (int m = 0; m < d_n_timeslots; ++m) {
          bool is_equal = std::abs(first[k][m] - second[k][m]) < 0.00001;
          if(!is_equal){
            std::cout << k << "\t" << m << "\tf: " << first[k][m] << "\ts: " << second[k][m] << "\tequal: " << (is_equal ? "True" : "False") << std::endl;
          }
        }
      }
      std::cout << "END   check_if_equal M=" << d_n_timeslots << ", K=" << d_n_subcarriers << std::endl;
    }

    void
    advanced_receiver_sb_cc_impl::demodulate_block_ic_vector(gr_complex *p_out, const gr_complex *p_in)
    {
      d_kernel->filter_superposition(d_sc_fdomain, p_in);
      d_kernel->demodulate_subcarrier(d_sc_symbols, d_sc_fdomain);

      for (int j = 0; j < d_ic_iter; ++j) {
        map_sc_symbols(d_sc_symbols);
        d_kernel->remove_sc_interference(d_sc_symbols, d_sc_fdomain);
        d_kernel->demodulate_subcarrier(d_sc_symbols, d_sc_symbols);
      }
      d_kernel->serialize_output(p_out, d_sc_symbols);
    }

    int
    advanced_receiver_sb_cc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      std::vector<tag_t> channel_tag;
      std::vector<gr_complex> channel(d_n_timeslots);

      const int n_blocks = noutput_items / d_kernel->block_size();

      for (int i = 0; i < n_blocks; ++i) {
      get_tags_in_window(channel_tag,0,i*d_kernel->block_size(),(i+1)*d_kernel->block_size(),pmt::string_to_symbol("channel_taps"));
        if (channel_tag.size() > 0)
        {
          channel = pmt::c32vector_elements(channel_tag.begin()->value);
          demodulate_block_ic_array(out, in, &channel[0]);
        }else{
          demodulate_block_ic_array(out, in);
        }

        in += d_kernel->block_size();
        out += d_kernel->block_size();
      }
      return noutput_items;
    }

    void advanced_receiver_sb_cc_impl::demodulate_block_ic_array(gr_complex *p_out, const gr_complex *p_in)
    {
      d_kernel->fft_filter_downsample(d_freq_block, p_in);
      d_kernel->transform_subcarriers_to_td(p_out, d_freq_block);


      for (int j = 0; j < d_ic_iter; ++j) {
        map_symbols_to_constellation_points(d_ic_time_buffer, p_out);
        d_kernel->cancel_sc_interference(d_ic_freq_buffer, d_ic_time_buffer, d_freq_block);
        d_kernel->transform_subcarriers_to_td(p_out, d_ic_freq_buffer);
      }
    }

    void advanced_receiver_sb_cc_impl::demodulate_block_ic_array(gr_complex *p_out, const gr_complex *p_in, const gr_complex *channel_taps)
    {
      d_kernel->fft_filter_downsample(d_freq_block, p_in);
      d_kernel->transform_subcarriers_to_td(p_out, d_freq_block);
      d_kernel->equalize_channel(p_out,p_in,channel_taps);


      for (int j = 0; j < d_ic_iter; ++j) {
        map_symbols_to_constellation_points(d_ic_time_buffer, p_out);
        d_kernel->cancel_sc_interference(d_ic_freq_buffer, d_ic_time_buffer, d_freq_block);
        d_kernel->transform_subcarriers_to_td(p_out, d_ic_freq_buffer);
      }
    }

    void advanced_receiver_sb_cc_impl::map_symbols_to_constellation_points(gr_complex *p_out, const gr_complex *p_in)
    {
      unsigned int symbol_tmp = 0;
      std::vector<gr_complex> const_points = d_constellation->points();
      for (int i = 0; i < d_kernel->block_size(); ++i) {
        symbol_tmp = d_constellation->decision_maker(p_in++);
        *p_out++ = const_points[symbol_tmp];
      }
    }
    
    void
    advanced_receiver_sb_cc_impl::map_sc_symbols( std::vector< std::vector<gr_complex> > &sc_symbols)
    {
      unsigned int symbol_tmp = 0;
      std::vector<gr_complex> const_points = d_constellation->points();
      for (int k=0;k<d_n_subcarriers;k++)
      {
        for (int m=0;m<d_n_timeslots;m++)
        {
          symbol_tmp =d_constellation->decision_maker(&sc_symbols[k][m]);
          sc_symbols[k][m] = const_points[symbol_tmp];
        }
      }
    }

  } /* namespace gfdm */
} /* namespace gr */

