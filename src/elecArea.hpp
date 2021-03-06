#pragma once

#include <utility>
#include <elecPoint.hpp>
#include <memory>
#include <limits>
#include <iterator>
#include <initializer_list>

namespace elec {
  
  class Wall {
  private:

    std::vector<Point> pattern;

  public:

    Wall(unsigned int nb_steps) {
      Point O = {0,0};
      Point M;
      if(nb_steps % 2 == 0) ++nb_steps; // nb_steps is odd.
      unsigned int half_step = nb_steps/2;
      double coef = 1/(nb_steps-1.0);
      auto out = std::back_inserter(pattern);
      for(unsigned int x = 0; x < nb_steps; ++x) {
	M.x = -.5+x*coef;
	for(unsigned int y = 0; y < nb_steps; ++y) {
	  if(y != half_step || x != 0) { // let us avoid the leftmost point (i.e. no motion). 
	    M.y = -.5+y*coef;
	    if(d2(M,O)<=.5*.5)
	      *(out++) = M;
	  }
	}
      }
            
      std::sort(pattern.begin(), pattern.end(),
		[](const Point& A, const Point& B) -> bool {return d2({-.5,0.0},A) > d2({-.5,0.0},B);});
    }

    

    /* Tries to move... Return each motion with a (point,score) pair.
       std::pair<Point,double> sc; if(score(x,sc) register x;*/
    template<typename ScoreFunc>
    std::vector<std::pair<Point,std::pair<Point,double>>> operator()(const Point& A, const Point& BB, const ScoreFunc& score) const {
      double norm_dd_2 = d2(A,BB);
      Point B;
      if(norm_dd_2 < elecMAX_VARIATION*elecMAX_VARIATION)
	B = BB;
      else
	B = A+(*(BB-A))*elecMAX_VARIATION;
	
      std::vector<std::pair<Point,std::pair<Point,double>>> res;
      auto out = std::back_inserter(res);
      std::pair<Point,double> score_value;

      auto D = B-A;
      auto O = (A+B)*.5;
      auto f = [&O,&D](const Point& M) -> Point {
	return {M.x*D.x - M.y*D.y + O.x,
		M.x*D.y + M.y*D.x + O.y};
      };

      for(auto& X : pattern) {
	auto XX = f(X);
	if(score(XX,score_value))
	  *(out++) = {XX,score_value};
      }

      return res;
    }
  };

  
  class Area {
  public:

    Area() {}
    virtual ~Area() {}

    virtual bool                   in          (const Point& pos) const = 0;
    virtual double                 mobility    (const Point& pos) const = 0;
    virtual double                 density     (const Point& pos) const = 0;
    virtual double                 min_d2      (const Point& pos) const = 0;
    virtual std::pair<Point,Point> bbox        ()                 const = 0;
  };

  using AreaRef = std::shared_ptr<Area>;

  class Translate : public Area {
  public:
    
    AreaRef content;
    Point t;
    Point forward (const Point& p) const {return p+t;}
    Point backward(const Point& p) const {return p-t;}
    
    Translate(AreaRef a, const Point& t) : Area(), content(a), t(t) {}
    virtual ~Translate() {}

    virtual bool                   in          (const Point& pos) const override {return content->in         (backward(pos));}
    virtual double                 mobility    (const Point& pos) const override {return content->mobility   (backward(pos));}
    virtual double                 density     (const Point& pos) const override {return content->density    (backward(pos));}
    virtual double                 min_d2      (const Point& pos) const override {return content->min_d2     (backward(pos));}
    virtual std::pair<Point,Point> bbox        ()                 const override {
      auto bb = content->bbox();
      return {forward(bb.first),forward(bb.second)};
    }
  };

  AreaRef translate(AreaRef a, const Point& t) {
    return AreaRef(static_cast<Area*>(new Translate(a,t)));
  }

  class Hflip : public Area {
  public:
    
    AreaRef content;
    double xx;
    Point forward (const Point& p) const {return {xx - p.x, p.y};}
    Point backward(const Point& p) const {return {xx - p.x, p.y};}
    
    Hflip(AreaRef a, double x) : Area(), content(a), xx(2*x) {}
    virtual ~Hflip() {}

    virtual bool                   in          (const Point& pos) const override {return content->in         (backward(pos));}
    virtual double                 mobility    (const Point& pos) const override {return content->mobility   (backward(pos));}
    virtual double                 density     (const Point& pos) const override {return content->density    (backward(pos));}
    virtual double                 min_d2      (const Point& pos) const override {return content->min_d2     (backward(pos));}
    virtual std::pair<Point,Point> bbox        ()                 const override {
      auto bb   = content->bbox();
      auto fmin = forward(bb.first);
      auto fmax = forward(bb.second);
      return {Point(fmax.x,fmin.y),Point(fmin.x,fmax.y)};
    }
  };

  AreaRef hflip(AreaRef a, double x) {
    return AreaRef(static_cast<Area*>(new Hflip(a,x)));
  }



  class Vflip : public Area {
  public:
    
    AreaRef content;
    double yy;
    Point forward (const Point& p) const {return {p.x, yy - p.y};}
    Point backward(const Point& p) const {return {p.x, yy - p.y};}
    
    Vflip(AreaRef a, double y) : Area(), content(a), yy(2*y) {}
    virtual ~Vflip() {}

    virtual bool                   in          (const Point& pos) const override {return content->in         (backward(pos));}
    virtual double                 mobility    (const Point& pos) const override {return content->mobility   (backward(pos));}
    virtual double                 density     (const Point& pos) const override {return content->density    (backward(pos));}
    virtual double                 min_d2      (const Point& pos) const override {return content->min_d2     (backward(pos));}
    virtual std::pair<Point,Point> bbox        ()                 const override {
      auto bb   = content->bbox();
      auto fmin = forward(bb.first);
      auto fmax = forward(bb.second);
      return {Point(fmin.x,fmax.y),Point(fmax.x,fmin.y)};
    }
  };

  AreaRef vflip(AreaRef a, double y) {
    return AreaRef(static_cast<Area*>(new Vflip(a,y)));
  }




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

    virtual double min_d2(const Point& pos) const override  {
      double res = std::numeric_limits<double>::max();
      for(auto& e_ptr : areas)
	res = std::min(res,e_ptr->min_d2(pos));
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
    double min_d2      = 0;
    /**
       @param mobility
       @param density in [0,1]. 
     */
    Material(double mobility, double density, double min_d) 
      : mobility(mobility), density(density), min_d2(min_d*min_d) {}
    Material(const Material&) = default;
  };

  inline Material metal() {return {1.0,1,elecMETAL_MIN_DIST};}
  inline Material material(double mobility, 
			   double density, 
			   double min_d2) {
    return {mobility,density,min_d2};
  }

  class Conductor : public Area {
  public:
    Material material;
    Conductor(const Material& mat) : Area(), material(mat) {}
    virtual ~Conductor() {}
    virtual double mobility(const Point& pos) const override {
      if(in(pos)) return material.mobility;
      return 0;
    };
    virtual double density(const Point& pos) const override {
      if(in(pos)) return material.density;
      return 0;
    };
    virtual double min_d2(const Point& pos) const override {
      if(in(pos)) return material.min_d2;
      return std::numeric_limits<double>::max();
    };
  };

  class Disk : public Conductor {
  public:
    Point O;
    double r;
    double r2;
    Disk(const Point& O, double r, const Material& mat) : Conductor(mat), O(O), r(r), r2(r*r) {}
    virtual ~Disk() {}
    virtual bool                   in      (const Point& pos) const override {return d2(pos,O)<=r2;}
    virtual std::pair<Point,Point> bbox    ()                 const override {return {O-Point(r,r),O+Point(r,r)};}
  };

  AreaRef disk(const Point& O, double r, const Material& mat) {
    return AreaRef(static_cast<Area*>(new Disk(O,r,mat)));
  }

  class Box : public Conductor {
  public:
    Point min,max;
    Box(const Point& min, const Point& max, const Material& mat) : Conductor(mat), min(min), max(max) {}
    virtual ~Box() {}
    virtual bool                   in      (const Point& pos) const override {return min <= pos && pos <= max;}
    virtual std::pair<Point,Point> bbox    ()                 const override {return {min,max};}
  };

  AreaRef box(const Point& min, const Point& max, const Material& mat) {
    return AreaRef(static_cast<Area*>(new Box(min,max,mat)));
  }

  class Wire : public Conductor {
  private:
    Point min,max;
    double r2;
  public:
    double r;
    std::vector<Point> vertices;
    Wire(const std::vector<Point>& vertices, double r, bool loop, const Material& mat)
      : Conductor(mat), r2(r*r), r(r), vertices(vertices) {
      min = *(vertices.cbegin());
      max = min;
      for(auto& pt : vertices) {
	if(pt.x < min.x) min.x = pt.x;
	else if(pt.x > max.x) max.x = pt.x;
	if(pt.y < min.y) min.y = pt.y;
	else if(pt.y > max.y) max.y = pt.y;
      }
      min -= {r,r};
      max += {r,r};
      
      if(loop)
	this->vertices.push_back(*(vertices.cbegin()));
    }
    virtual ~Wire() {}
    
    virtual bool in(const Point& pos) const override {
      auto ita = vertices.begin();
      double r2 = r*r;
      for(auto itb = ita+1; itb != vertices.end(); ita = itb++) {
	Point   A = *ita;
	Point   B = *itb;
	Point   u = *(B-A);
	double l2 = d2(A,B);
	double  l = sqrt(l2);
	double  dd;

	double lambda = u*(pos-A);
	if(lambda < 0)
	  dd = d2(A,pos);
	else if(lambda > l)
	  dd = d2(B,pos);
	else
	  dd = d2(A+u*lambda, pos);
	if(dd < r2)
	  return true;
      }
      return false;
    }
    
    virtual std::pair<Point,Point> bbox() const override {return {min,max};}
  };

  AreaRef wire(const std::vector<Point>& vertices, double r, bool loop, const Material& mat) {
    return AreaRef(static_cast<Area*>(new Wire(vertices,r,loop,mat)));
  }

  
}
