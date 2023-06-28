#ifndef IDX_HPP
#define IDX_HPP

inline int idx(const int ix, const int iy, const int iz, const int nx, const int ny, const int nz, const int ng=0) {
  // input index of (0, 0, 0) corresponds to first *interior* point. Ghost points are accessed with (-1, -1, -1).
  return (iz+ng)*(nx+2*ng)*(ny+2*ng) + (iy+ng)*(nx+2*ng) + (ix+ng);
}

#endif  // IDX_HPP
