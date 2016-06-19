#pragma once

#include <utility>
#include <elecPoint.hpp>
#include <memory>
#include <iterator>
#include <initializer_list>

namespace elec {
  
  class Wall {
  private:

    std::vector<Point> pattern;
    std::vector<Point> extras;

  public:

    Wall(unsigned int nb_steps) {
      Point O = {0,0};
      Point M;
      if(nb_steps % 2 == 0) ++nb_steps; // the point (1,0) is thus in the pattern
      auto out = std::back_inserter(pattern);
      for(unsigned int x = 0; x < nb_steps; ++x) {
	M.x = -.5+x/(nb_steps-1.0);
	for(unsigned int y = 0; y < nb_steps; ++y) {
	  M.y = -.5+y/(nb_steps-1.0);
	  if(d2(M,O)<=.5*.5)
	    *(out++) = M;
	}
      }
      // Allows to try lateral escape...
      out      = std::back_inserter(extras);
      *(out++) = {-.5,-1/(nb_steps-1.0)};
      *(out++) = { .5,-1/(nb_steps-1.0)};

      std::sort(pattern.begin(), pattern.end(),
		[](const Point& A, const Point& B) -> bool {return d2({-.5,0.0},A) > d2({-.5,0.0},B);});
    }

    template<typename InsideFunc>
    double operator()(const Point& O, double r, const InsideFunc& inside) const {
      double d = 2*r;
      auto f = [&O,d](const Point& M) -> Point {
	return {M.x*d + O.x,
		M.y*d + O.y};
      };

      unsigned int nb_in = 0;
      for(auto& X : pattern) 
	if(inside(f(X)))
	  ++nb_in;

      return 3.14159265358979323846*r*r*nb_in/(double)pattern.size();
    }

    template<typename InsideFunc>
    Point operator()(const Point& A, const Point& B, const InsideFunc& inside) const {
      auto D = B-A;
      auto O = (A+B)*.5;
      auto f = [&O,&D](const Point& M) -> Point {
	return {M.x*D.x - M.y*D.y + O.x,
		M.x*D.y + M.y*D.x + O.y};
      };

      for(auto& X : pattern) {
	auto XX = f(X);
	if(inside(XX))
	  return XX;
      }

      for(auto& X : extras) {
	auto XX = f(X);
	if(inside(XX))
	  return XX;
      }

      return A;
    }
  };

  
  class Area {
  public:

    Area() {}
    virtual ~Area() {}

    virtual bool                   in          (const Point& pos) const = 0;
    virtual double                 mobility    (const Point& pos) const = 0;
    virtual double                 density     (const Point& pos) const = 0;
    virtual double                 max_density (const Point& pos) const = 0;
    virtual std::pair<Point,Point> bbox        ()                 const = 0;
  };

  using AreaRef = std::shared_ptr<Area>;

  class AreaSet : public Area {
  public:
    
    std::vector<AreaRef> areas;
    AreaSet() : Area(), areas() {}

    AreaSet(const std::initializer_list<AreaRef>& lst) : Area(), areas(lst) {}

    virtual ~AreaSet() {}

    void operator+=(elec::AreaRef area) {
      areas.push_back(area);
    }

    virtual bool in(const Point& pos) const override {
      for(auto& e_ptr : areas)
	if(e_ptr->in(pos)) 
	  return true;
      return false;
    }

    virtual double mobility(const Point& pos) const override  {
      double res = 0;
      for(auto& e_ptr : areas)
	res = std::max(res,e_ptr->mobility(pos));
      return res;
    }

    virtual double density(const Point& pos) const override  {
      double res = 0;
      for(auto& e_ptr : areas)
	res = std::max(res,e_ptr->density(pos));
      return res;
    }

    virtual double max_density(const Point& pos) const override  {
      double res = 0;
      for(auto& e_ptr : areas)
	res = std::max(res,e_ptr->max_density(pos));
      return res;
    }

    virtual std::pair<Point,Point> bbox() const override {
      auto iter = areas.begin();
      auto res  = (*(iter++))->bbox();
      while(iter!=areas.end()) {
	auto bb = (*(iter++))->bbox();
	res.first  = min(res.first, bb.first);
	res.second = max(res.second,bb.second);
      }
      return res;
    }
  };

  AreaRef set(const std::initializer_list<AreaRef>& lst) {
    return AreaRef(static_cast<Area*>(new AreaSet(lst)));
  }


  struct Material {
    double mobility    = 0;
    double density     = 0;
    double max_density = 0;
    Material(double mobility, double density, double max_density) 
      : mobility(mobility), density(density), max_density(max_density) {}
    Material(const Material&) = default;
  };

  inline Material metal() {return {1.0,1,100.};}
  inline Material material(double mobility, 
			   double density, 
			   double max_density) {
    return {mobility,density,max_density};
  }

  class Conductor : public Area {
  public:
    Material material;
    Conductor(const Material& mat) : Area(), material(mat) {}
    virtual ~Conductor() {}
    virtual double mobility(const Point& pos) const {
      if(in(pos)) return material.mobility;
      return 0;
    };
    virtual double density(const Point& pos) const {
      if(in(pos)) return material.density;
      return 0;
    };
    virtual double max_density(const Point& pos) const {
      if(in(pos)) return material.max_density;
      return 0;
    };
  };

  class Disk : public Conductor {
  public:
    Point O;
    double r;
    double r2;
    Disk(const Point& O, double r, const Material& mat) : Conductor(mat), O(O), r(r), r2(r*r) {}
    virtual ~Disk() {}
    virtual bool           in      (const Point& pos) const {return d2(pos,O)<=r2;}
    std::pair<Point,Point> bbox    ()                 const {return {O-Point(r,r),O+Point(r,r)};}
  };

  AreaRef disk(const Point& O, double r, const Material& mat) {
    return AreaRef(static_cast<Area*>(new Disk(O,r,mat)));
  }

  class Box : public Conductor {
  public:
    Point min,max;
    Box(const Point& min, const Point& max, const Material& mat) : Conductor(mat), min(min), max(max) {}
    virtual ~Box() {}
    virtual bool           in      (const Point& pos) const {return min <= pos && pos <= max;}
    std::pair<Point,Point> bbox    ()                 const {return {min,max};}
  };

  AreaRef box(const Point& min, const Point& max, const Material& mat) {
    return AreaRef(static_cast<Area*>(new Box(min,max,mat)));
  }

  
}