#pragma once

#include <cmath>
#include <cstdlib>
#include <utility>
#include <limits>
#include <algorithm>

#include <elecParams.hpp>
#include <elecPoint.hpp>
#include <elecArea.hpp>

namespace elec {

  bool proba(double p) {
    return std::rand()/(1.0+RAND_MAX) < p;
  }

  // inline Point E(const Point& p, const Point& at) {
  //   if(p == at)
  //     return {0.,0.};
  //   return (*(at-p))/std::max(d2(p,at),elecMIN_E_RADIUS*elecMIN_E_RADIUS);
  // }

  // inline double V(const Point& p, const Point& at) {
  //   if(p == at)
  //     return 0;
  //   return 1/std::max(d(p,at),elecMIN_E_RADIUS);
  // }
  
  inline Point E(const Point& p, const Point& at) {
    double r2 = d2(p,at);
    if(r2 < elecMIN_E_RADIUS*elecMIN_E_RADIUS)
      return {0.,0.};
    else
      return (*(at-p))/r2;
  }

  inline double V(const Point& p, const Point& at) {
    double r = d(p,at);
    if(r < elecMIN_E_RADIUS)
      return 0;
    return 1/r;
  }

  template<typename Iter>
  Point E(const Iter& begin,
	  const Iter& end,
	  const Point& at) {
    Point e = {0,0};
    for(auto it = begin; it != end; ++it)
      e += E(*it,at);
    return e;
  }

  template<typename Iter>
  double V(const Iter& begin,
	  const Iter& end,
	  const Point& at) {
    double v = 0;
    for(auto it = begin; it != end; ++it)
      v += V(*it,at);
    return v;
  }

  template<typename OutputIterator>
  unsigned int add_particles_random(AreaRef a, OutputIterator& out) {
    Point min,max;
    std::tie(min,max) = a->bbox();
    unsigned int nb_elems = 0;
    unsigned int n = (unsigned int)(elecDENSITY*(max.x-min.x)*(max.y-min.y)+.5);
    for(unsigned int i = 0; i < n; ++i) {
      auto p = elec::uniform(min,max);
      if(elec::proba(a->density(p))) {
	*(out++) = p;
	++nb_elems;
      }
    }
    return nb_elems;
  }

  template<typename OutputIterator>
  void add_particles_random(AreaRef a,  unsigned int nb, OutputIterator& out) {
    Point min,max;
    std::tie(min,max) = a->bbox();
    for(unsigned int i = 0; i < nb; ++i) {
      Point p;
      do
	p = elec::uniform(min,max); 
      while(!elec::proba(a->density(p)));
      *(out++) = p;
    }
  }

}
