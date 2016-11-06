#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <stdexcept>

#include <elecArea.hpp>
#include <elecPoint.hpp>
#include <elecParticle.hpp>

#include <ccmpl.hpp>

namespace elec {
  
  class World {
    std::vector<std::pair<elec::AreaRef, unsigned int>> areas;
    AreaSet all;
    Wall wall;
    std::vector<elec::Point> electrons;
    std::vector<elec::Point> protons;
    ccmpl::chart::Limits2d limits2d;
    bool limits2d_computed;
    
    void noisify(Point& e) {
      Point p;
      unsigned int nb = 0;
      do  {
        p = shake(e,
		  elecNOISE_RADIUS_MAX,
		  elecNOISE_RADIUS_MIN*elecNOISE_RADIUS_MIN,
		  elecNOISE_RADIUS_MAX*elecNOISE_RADIUS_MAX);
	++nb;
      }
      while(!(all.in(p)) && nb < elecNOISE_NB_TRIES_INSIDE);
      
      if(nb < elecNOISE_NB_TRIES_INSIDE)
	e = p;
    }

  public:

    World() : areas(), all(), wall(20), electrons(), protons(),
	      limits2d(), limits2d_computed(false) {}


    std::pair<Point,double> closest_electron_d2(const Point& p, const Point& exclude) {
      std::pair<Point,double> res = {Point(0,0),std::numeric_limits<double>::max()};
      double d;
      for(auto& e_pos : electrons) 
	if((e_pos != exclude) && ((d = d2(e_pos,p)) < res.second))
	  res = {e_pos,d};
      return res;
    }

    void move(Point& e, const Point& E) {
      auto scored = wall(e,e-E*all.mobility(e),
			 [this,e](const Point& p, std::pair<Point,double>& sc) -> bool {
			   if(this->all.in(p)) {
			     sc = this->closest_electron_d2(p,e);
			     return true;
			   }
			   else
			     return false;
			 });
	
      bool ee_found = false;
      Point ee;
      double min_d2_e = 0;
      std::pair<Point,double> closest_d2 = {Point(0,0),0};

      // Let us find the first fitting point, if any
      if(!ee_found)  {
	min_d2_e = all.min_d2(e);
	for(auto& p_sc : scored)
	  if(p_sc.second.second > min_d2_e) {
	    ee = p_sc.first;
	    ee_found = true;
	    break;
	  }
      }
     

      // No fitting point, let us move toward the best one. 
      
      if(!ee_found)  {
	closest_d2 = closest_electron_d2(e,e);
	if(scored.size() > 0) {
	  auto m = std::max_element(scored.begin(), scored.end(), 
				    [](const std::pair<Point,std::pair<Point,double>>& p1,
				       const std::pair<Point,std::pair<Point,double>>& p2) -> double {return p1.second.second < p2.second.second;});
	  if(m->second.second > closest_d2.second) {
	    auto d1 = m->second.first - m->first;
	    auto d2 = closest_d2.first - e;
	    if(d1*d2 > 0)  {// the closest is not toward the current motion
	      ee       = m->first;
	      ee_found = true;
	    }
	  }
	}
      }

      if(!ee_found) {
	ee = e;
	ee_found = true;
      }

      // Let us noisify the position
      for(unsigned i=0; i< elecNB_NOISE_TRIES; ++i) {
	auto p =  ee;
	noisify(p);
	if(all.in(p)) {
	  auto closest_p = closest_electron_d2(p,e);
	  if(closest_p.second > closest_d2.second) {
	    auto d1 = closest_p.first - p;
	    auto d2 = closest_d2.first - e;
	    if(d1*d2 > 0)  {// the closest is not toward the current motion
	      ee = p;
	      break;
	    }
	  }
	}
      }

      e = ee;
      
    }

    Point E(const Point& pos) {
      return elecELEMENTARY_CHARGE
	* (elec::E(protons.begin(), protons.end(), pos)
	   - elec::E(electrons.begin(), electrons.end(), pos));
    }

    double V(const Point& pos) {
      return elecELEMENTARY_CHARGE
	* (elec::V(protons.begin(), protons.end(), pos)
	   - elec::V(electrons.begin(), electrons.end(), pos));
    }

    template<typename Efunc>
    void move(const Efunc& E) {
      for(auto& e : electrons) move(e,E(e));
    }

    unsigned int operator+=(elec::AreaRef area) {
      unsigned int res = areas.size();
      areas.push_back({area,0});
      all += area;
      return res;
    }

    void add_electron(const Point& pos) {
      auto e = std::back_inserter(electrons);
      *(e++) = pos;
    }

    unsigned int add_protons_random(AreaRef a) {
      auto p = std::back_inserter(protons);
      return add_particles_random(a,p);
    }

    void add_protons_random(AreaRef a,  unsigned int nb) {
      auto p = std::back_inserter(protons);
      add_particles_random(a,nb,p);
    }

    unsigned int add_electrons_random(AreaRef a) {
      auto e = std::back_inserter(electrons);
      return add_particles_random(a,e);
    }

    void add_electrons_random(AreaRef a,  unsigned int nb) {
      auto e = std::back_inserter(electrons);
      add_particles_random(a,nb,e);
    }

    void build_protons(unsigned int idf) {
      auto& area = areas[idf];
      auto  p    = std::back_inserter(protons);
      area.second = elec::add_particles_random(area.first,p);
    }

    void build_electrons(unsigned int idf) {
      auto& area = areas[idf];
      auto  e    = std::back_inserter(electrons);
      elec::add_particles_random(area.first,area.second,e);
    }

    unsigned int nb_protons(unsigned int idf) {
      return areas[idf].second;
    }

    void build(unsigned int idf) {
      build_protons(idf);
      build_electrons(idf);
    }

    void build_protons() {
      for(auto& area : areas) 
	if(area.second == 0) {
	  auto p = std::back_inserter(protons);
	  area.second = elec::add_particles_random(area.first,p);
	}
    }

    void build() {
      for(auto& area : areas) 
	if(area.second == 0) {
	  auto p = std::back_inserter(protons);
	  auto e = std::back_inserter(electrons);
	  area.second = elec::add_particles_random(area.first,p);
	  elec::add_particles_random(area.first,area.second,e);
	}
    }

    ccmpl::chart::Limits2d limits(double margin) {
      if(!limits2d_computed) {
	elec::AreaSet areaset;
	for(auto& area : areas) 
	  areaset += area.first;
	auto bbox = areaset.bbox();
	elec::Point m(margin,margin);
	bbox.first  -= m;
	bbox.second += m;
	limits2d = {bbox.first.x, bbox.second.x, bbox.first.y, bbox.second.y};
	limits2d_computed = true;
      }
      return limits2d;
    }

    ccmpl::Dots plot_protons() {
      return ccmpl::dots("c='r',lw=.5,s=10,marker='+',zorder=3", [this](std::vector<ccmpl::Point>& curve) {
	  curve.clear();
	  std::copy(this->protons.begin(), this->protons.end(), std::back_inserter(curve));
	});
    }
    
    ccmpl::Dots plot_electrons() {
      return ccmpl::dots("c='g',lw=.5,s=10,marker='o',zorder=5", [this](std::vector<ccmpl::Point>& curve) {
	  curve.clear();
	  std::copy(this->electrons.begin(), this->electrons.end(), std::back_inserter(curve));
	});
    }
    
    ccmpl::Contours plot_V(double vmin, double vmax, unsigned int nb_contours,
			   unsigned int nb_X, unsigned int nb_Y) {
      if(!limits2d_computed)
	throw std::runtime_error("plot_V requires the limits to be computed");
      return ccmpl::contours("zorder=1", 0,
			     [this, vmin, vmax, nb_X, nb_Y, nb_contours](std::vector<double>& z,
									 double& xmin, double& xmax, unsigned int& nb_x,
									 double& ymin, double& ymax, unsigned int& nb_y,
									 double& zmin, double& zmax, unsigned int& nb_z) {
			       z.clear();
			       xmin = this->limits2d.xmin;
			       xmax = this->limits2d.xmax;
			       nb_x = nb_X;
			       ymin = this->limits2d.ymin;
			       ymax = this->limits2d.ymax;
			       nb_y = nb_Y;
			       zmin = vmin;
			       zmax = vmax;
			       nb_z = nb_contours;
			       auto outz = std::back_inserter(z);
			       for(auto y : ccmpl::range(ymin, ymax, nb_y))
				 for(auto x : ccmpl::range(xmin, xmax, nb_x))
				   *(outz++) = V(Point(x,y));
			     });
    }
    
    ccmpl::Vectors plot_E(double coef, unsigned int nb_X, unsigned int nb_Y) {
      if(!limits2d_computed)
	throw std::runtime_error("plot_E requires the limits to be computed");
      return ccmpl::vectors("zorder=1,color='blue',pivot='tail',scale=1.0",
			    [this, coef, nb_X, nb_Y](std::vector<std::pair<ccmpl::Point,ccmpl::Point>>& vectors) {
			      vectors.clear();
			      auto outv = std::back_inserter(vectors);
			      for(auto y : ccmpl::range(this->limits2d.ymin, this->limits2d.ymax, nb_Y))
				for(auto x : ccmpl::range(this->limits2d.xmin, this->limits2d.xmax, nb_X)) {
				  auto p = Point(x,y);
				  if(!(this->all.in(p)))
				    *(outv++) = {Point(x,y),E(Point(x,y))*coef};
				}
			    });
    }
  };
}
