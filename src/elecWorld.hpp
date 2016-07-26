#pragma once

#include <vector>
#include <algorithm>
#include <iterator>

#include <elecArea.hpp>
#include <elecPoint.hpp>
#include <elecParticle.hpp>

#include <ccmpl.hpp>
#include <algorithm>

#include <iostream>

namespace elec {
  
  class World {
    std::vector<std::pair<elec::AreaRef, unsigned int>> areas;
    AreaSet all;
    Wall wall;
    std::vector<elec::Point> electrons;
    std::vector<elec::Point> protons;
    
    void noisify(Point& e) {
      Point p;
      do 
        p = shake(e,elecNOISE_RADIUS);
      while(!(all.in(p)));
      e = p;
    }

  public:

    World() : areas(), all(), wall(20), electrons(), protons() {}


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
      elec::AreaSet areaset;
      for(auto& area : areas) 
	areaset += area.first;
      auto bbox = areaset.bbox();
      elec::Point m(margin,margin);
      bbox.first  -= m;
      bbox.second += m;
      return {bbox.first.x, bbox.second.x, bbox.first.y, bbox.second.y};
    }

    ccmpl::Dots plot_protons() {
      return ccmpl::dots("c='r',lw=.5,s=10,marker='+',zorder=1", [this](std::vector<ccmpl::Point>& curve) {
	  curve.clear();
	  std::copy(this->protons.begin(), this->protons.end(), std::back_inserter(curve));
	});
    }
    ccmpl::Dots plot_electrons() {
      return ccmpl::dots("c='g',lw=.5,s=10,marker='o',zorder=2", [this](std::vector<ccmpl::Point>& curve) {
	  curve.clear();
	  std::copy(this->electrons.begin(), this->electrons.end(), std::back_inserter(curve));
	});
    }
  };
}
