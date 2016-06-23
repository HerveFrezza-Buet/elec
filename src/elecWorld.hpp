#pragma once

#include <vector>
#include <iterator>

#include <elecArea.hpp>
#include <elecPoint.hpp>
#include <elecParticle.hpp>

#include <ccmpl.hpp>
#include <algorithm>


namespace elec {
  
  class World {
    std::vector<std::pair<elec::AreaRef, unsigned int>> areas;
    AreaSet all;
    Wall wall;
    std::vector<elec::Point> electrons;
    std::vector<elec::Point> protons;
    
  public:

    World() : areas(), all(), wall(20), electrons(), protons() {}

    void move(Point& e, const Point& E) {
      if(!wall(e,e-E*all.mobility(e),[this](const Point& p) -> bool {
	    double max_density = this->all.max_density(p);
	    if(max_density > 0)
	      return this->electrons_density(p,max_density) < max_density;
	    else
	      return false;
	  })) {
	Point p;
	do 
	  p = shake(e,elecNOISE_RADIUS);
	while(!(all.in(p)));
	e = p;
      }
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

    // returns 1 for elecDENSITY electrons per m2.
    double electrons_density(const Point& O, double max_density) {
      double r2 = elecDENSITY_ESTIMATION_NB/(max_density*elecPI*elecDENSITY);
      double area = wall(O,r2,[this](const Point& p) -> bool {return this->all.in(p);});
      unsigned int nb = 0;
      for(auto& e : electrons) if(d2(O,e)<r2) ++nb;
      return nb/(area*elecDENSITY);
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
