/*
This file is part of UMAP.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/umap/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free
Software Foundation) version 2.1 dated February 1999.  This program is
distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the terms and conditions of the GNU Lesser General Public License
for more details.  You should have received a copy of the GNU Lesser General
Public License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#ifndef UMAP_APPS_MEDIAN_CALCULATION_CUBE_HPP
#define UMAP_APPS_MEDIAN_CALCULATION_CUBE_HPP

#include <iostream>
#include <vector>
#include <cassert>
#include <tuple>

#include "utility.hpp"
#include "../utility/umap_fits_file.hpp"

#define MEDIAN_CALCULATION_COLUMN_MAJOR 1
#define MEDIAN_CALCULATION_VERBOSE_OUT_OF_RANGE 0

namespace median {

template <typename _pixel_type>
class cube {
 public:

  using pixel_type = _pixel_type;

  /// -------------------------------------------------------------------------------- ///
  /// Constructor
  /// -------------------------------------------------------------------------------- ///
  cube() = default;

  cube(const size_t size_x,
    const size_t size_y,
    const size_t size_k,
    pixel_type *const image_data,
    std::vector<unsigned long> timestamp_list,
    std::vector<double> exposuretime_list,
    std::vector<double> psf_list,
    std::vector<std::vector<double>> ra_dec_list,
    std::vector<double> noise_list)
    : m_size_x(size_x),
    m_size_y(size_y),
    m_size_k(size_k),
    m_image_data(image_data),
    m_timestamp_list(std::move(timestamp_list)),
    m_exposuretime_list(std::move(exposuretime_list)),
    m_psf_list(std::move(psf_list)),
    m_ra_dec_list(std::move(ra_dec_list)),
    m_noise_list(std::move(noise_list)) {
    assert(m_size_k <= m_timestamp_list.size());
    assert(m_size_k <= m_exposuretime_list.size());
    assert(m_size_k <= m_psf_list.size());
    assert(m_size_k <= m_ra_dec_list.size());
    assert(m_size_k <= m_noise_list.size());
  }

  ~cube() = default; // Default destructor
  cube(const cube &) = default; // Default copy constructor
  cube(cube &&) noexcept = default; // Default move constructor
  cube &operator=(const cube &) = default; // Default copy assignment
  cube &operator=(cube &&) = default; // Default move assignment

  /// -------------------------------------------------------------------------------- ///
  /// Public methods
  /// -------------------------------------------------------------------------------- ///

  /// \brief Returns TRUE if the given x-y-k coordinate points at out-of-range
  bool out_of_range(const ssize_t x, const ssize_t y, const ssize_t k) const {
    return (this->u_cube.index_in_cube(x, y, k) == -1);
  }

  /// \brief Returns a pixel value of the given x-y-k coordinate
  /// A returned value can be NaN value
  pixel_type get_pixel_value(const ssize_t x, const ssize_t y, const ssize_t k) const {
    std::cout << "cube::gpv\t" << "k:" << k << " x:" << x << " y:" << y  << std::endl;
    return this->u_cube.get_pixel_value(x, y, k);
  }

  /// \brief Returns the size of cube (x, y, k) in tuple
  std::tuple<std::size_t, std::size_t, std::size_t> size() const {
    return std::make_tuple(m_size_x, m_size_y, m_size_k);
  }
  
  std::size_t cube_size() const {
    return m_size_x * m_size_y * m_size_k;
  }
  
  std::tuple<std::size_t, std::size_t, std::size_t> get_rnd_coord(std::size_t index, double x_slope, double y_slope) const {
    auto intercept = u_cube.get_rnd_coord(index);
    std::size_t k = std::get<2>(intercept);
    double time_offset = this->timestamp(k) - this->timestamp(0);
    std::size_t x = std::round(std::get<0>(intercept) - x_slope * time_offset);
    std::size_t y = std::round(std::get<1>(intercept) - y_slope * time_offset);
    return std::make_tuple(x, y, 0);
  }

  unsigned long timestamp(const size_t k) const {
    assert(k < m_timestamp_list.size());
    return m_timestamp_list[k];
  }

  double exposuretime(const size_t k) const {
    assert(k < this->m_exposuretime_list.size());
    return m_exposuretime_list[k];
  }

  double psf(const size_t k) const {
    assert(k < this->m_psf_list.size());
    return m_psf_list[k];
  }

  std::vector<double> ra_dec(const size_t k) const {
    assert(k < m_ra_dec_list.size());
    return m_ra_dec_list[k];
  }

  double noise(const size_t k) const {
    assert(k < m_noise_list.size());
    return m_noise_list[k];
  }
 private:
  /// -------------------------------------------------------------------------------- ///
  /// Private fields
  /// -------------------------------------------------------------------------------- ///
  size_t m_size_x;
  size_t m_size_y;
  size_t m_size_k;

  const utility::umap_fits_file::umap_fits_cube<pixel_type>& u_cube;

  std::vector<unsigned long> m_timestamp_list; // an array of the timestamp of each frame in HUNDRETHS OF A SECOND.
  std::vector<double> m_exposuretime_list; // an array of the exposure time of each image
  std::vector<double> m_psf_list; //an array of the psf fwhm of each image
  std::vector<std::vector<double>> m_ra_dec_list; //an array of ra/dec values for boresight of each image
  std::vector<double> m_noise_list; // an array of average background sky value (noise) for each image
};

} // namespace median

#endif //UMAP_APPS_MEDIAN_CALCULATION_CUBE_HPP
