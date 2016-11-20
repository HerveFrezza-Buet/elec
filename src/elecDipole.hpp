#pragma once

#include <cmath>
#include <elecPoint.hpp>
#include <elecParticle.hpp>

namespace elec {
  class Dipole {
  public:
    
    Point pos,neg,nneg;
    double nb;
    double r2;
    
    Dipole() = delete;
    Dipole(const Dipole&) = default;
    Dipole& operator=(const Dipole&) = default;
    
    Dipole(const Point& at, double r, double angle,
	   unsigned int nb) : nb(nb), r2(r*r) {
      double rad = angle*elecPI/180.;
      Point R = {r*std::cos(rad), r*std::sin(rad)};
      pos =  at + R;
      neg =  at - R;
      nneg = at - 1.5*R;
    }

    void transfer(Point& elec_pos) const {
      if(d2(elec_pos,pos) < r2)
	elec_pos = nneg;
    }

    template<typename Iter>
    void transfer(const Iter& begin, const Iter& end) {
      for(auto it =  begin ; it != end ; ++it)
	transfer(*it);
    }
  };
  
  inline Point E(const Dipole& dipole, const Point& at) {
    return dipole.nb*(E(dipole.pos,at)-E(dipole.neg,at));
  }
  
  inline double V(const Dipole& dipole, const Point& at) {
    return dipole.nb*(V(dipole.pos,at)-V(dipole.neg,at));
  }
  
}
