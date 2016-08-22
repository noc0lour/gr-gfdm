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


#ifndef INCLUDED_GFDM_SYNC_CC_H
#define INCLUDED_GFDM_SYNC_CC_H

#include <gfdm/api.h>
#include <gnuradio/block.h>
#include <gfdm/preamble_generator.h>

namespace gr {
  namespace gfdm {

    /*!
     * \brief GFDM - Framesynchronization based on modified Schmidl & Cox Preamble
     * \ingroup gfdm
     *  
     */
    class GFDM_API sync_cc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<sync_cc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of gfdm::sync_cc.
       * \param n_subcarriers number of subcarriers in GFDM frame and synchronization symbol
       * \param cp_length length of cyclic prefix in number of samples
       * \param frame_len resulting frame length (timeslots*subcarriers + cp_len + 2*subcarriers)
       * \param preamble a vector of length (2*subcarriers) containing the preamble of the sender
       * \param thr_acorr a float value [0,1] representing minimal autocorrelation energy to detect a frame
       * \param gfdm_tag_key a string which represents the sync tag name
       */
      static sptr make(int n_subcarriers, int cp_length, int frame_len, std::vector<gr_complex> preamble, float thr_acorr,
                       const std::string &gfdm_tag_key = "gfdm_block");
    };

  } // namespace gfdm
} // namespace gr

#endif /* INCLUDED_GFDM_SYNC_CC_H */

