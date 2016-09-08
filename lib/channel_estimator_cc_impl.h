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

#ifndef INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_IMPL_H
#define INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_IMPL_H

#include <gfdm/channel_estimator_cc.h>

namespace gr {
  namespace gfdm {

    class channel_estimator_cc_impl : public channel_estimator_cc
    {
     private:
       std::vector<gr_complex> d_known_preamble;
       std::string d_gfdm_sync_tag_key;


     public:
      channel_estimator_cc_impl(int subcarriers, std::vector<gr_complex> preamble);
      ~channel_estimator_cc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_CHANNEL_ESTIMATOR_CC_IMPL_H */

