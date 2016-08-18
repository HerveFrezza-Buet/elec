#pragma once

/*   This file is part of rl-lib
 *
 *   Copyright (C) 2010,  Herve FREZZA-BUET
 *
 *   Author : Herve Frezza-Buet
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : Herve.Frezza-Buet@supelec.fr
 *
 */

#include <vector>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <ccmpl.hpp>

namespace elec {
  class Point {
  public:
    double x,y;
    
    Point() : x(0), y(0) {}
    Point(const Point& cp) : x(cp.x), y(cp.y) {}
    Point(double xx, double yy) : x(xx), y(yy) {}
    Point& operator=(const Point& cp) {
      x = cp.x;
      y = cp.y;
      return *this;
    }

    Point& operator=(double val) {
      x = val;
      y = val;
      return *this;
    }
  
    bool operator==(const Point& p) const {
      return x == p.x && y == p.y;
    }
  
    bool operator<(const Point& p) const {
      return x < p.x && y < p.y;
    }

    bool operator>(const Point& p) const {
      return x > p.x && y > p.y;
    }
  
    bool operator<=(const Point& p) const {
      return x <= p.x && y <= p.y;
    }

    bool operator>=(const Point& p) const {
      return x >= p.x && y >= p.y;
    }
  
    bool operator!=(const Point& p) const {
      return x != p.x || y != p.y;
    }

    Point operator+(const Point& p) const {
      return {x+p.x, y+p.y};
    }
  
    Point operator-() const {
      return {-x,-y};
    }

    Point operator+() const {
      return {x,y};
    }

    Point operator-(const Point& p) const {
      return {x-p.x, y-p.y};
    }
  
    double operator*(const Point& p) const {
      return x*p.x + y*p.y;
    }

    Point operator&(const Point& p) const {
      return {x*p.x, y*p.y};
    }

    /**
     * Unitary vector.
     */
    Point operator*() const {
      const Point& me = *this;
      return me/std::sqrt(me*me);
    }

    Point operator*(double a) const {
      return {x*a, y*a};
    }

    Point operator/(double a) const {
      return (*this)*(1/a);
    }

    Point& operator+=(const Point& p) {
      x += p.x;
      y += p.y;
      return *this;
    }

    Point& operator-=(const Point& p) {
      x -= p.x;
      y -= p.y;
      return *this;
    }
  
    Point& operator*=(double a) {
      x *= a;
      y *= a;
      return *this;
    }
  
    Point& operator/=(double a) {
      (*this)*=(1/a);
      return *this;
    }

    operator ccmpl::Point () const {
      return ccmpl::Point(x,y);
    }
  };

  inline elec::Point operator*(double a, const elec::Point p) {
    return p*a;
  }

  inline std::ostream& operator<<(std::ostream& os, 
				  const Point& p) {
    os << '(' << p.x << ", " << p.y << ')';
    return os;
  }

  inline std::ostream& operator<<(std::ostream& os, 
				  const std::pair<Point,Point> p) {
    os << "(" << p.first << "," << p.second << ")";
    return os;
  }

  inline Point uniform(const Point& A, const Point& B) {
    Point d = {std::rand()/(RAND_MAX+1.0),
	       std::rand()/(RAND_MAX+1.0)};
    return A + (d & (B-A));
  }
  
  inline double d2(const Point& A, const Point& B) {
    Point tmp = B-A;
    return tmp*tmp;
  }

  inline double d(const Point& A, const Point& B) {
    return std::sqrt(d2(A,B));
  }

  inline Point min(const Point& A, const Point& B) {
    return {std::min(A.x,B.x),std::min(A.y,B.y)};
  }

  inline Point max(const Point& A, const Point& B) {
    return {std::max(A.x,B.x),std::max(A.y,B.y)};
  }
  
  inline Point shake(const Point& A, double radius_max, double radius2_min, double radius2_max) {
    Point p;
    Point   R = {radius_max,radius_max};
    double d_2;
    do {
      p  = uniform(A-R,A+R);
      d_2 = d2(p,A);
    }
    while(d_2>radius2_max || d_2<radius2_min);
    return p;
  }

}
