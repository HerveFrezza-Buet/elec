

#include <iostream>
#include <elec.hpp>


#define RADIUS1 1.5
#define RADIUS2 1.0
#define RADIUS3 0.2
#define MARGIN  1.0

#define NB_STEPS        750
#define NB_SUBSTEPS       1

#define PLOT_V_MIN      -10
#define PLOT_V_MAX       10
#define PLOT_V_NB_ISO    20
#define PLOT_V_NB_X      60
#define PLOT_V_NB_Y      30

#define WIRE_RADIUS .1
#define DIPOLE_NB_PROTONS 1000

int main(int argc, char* argv[]) {
  elec::Main m(argc,argv,"example-002");
  
  auto wire = elec::wire({elec::Point(-2,0), {-2,2}, {2,2}, {2,0}},
			 WIRE_RADIUS, true, elec::metal());

  elec::World world;
  auto wire_idf = (world += wire);
  world.build_protons(wire_idf);
  world.build_electrons(wire_idf);
  world.add_dipole({0,0},WIRE_RADIUS,0,DIPOLE_NB_PROTONS);

  std::string flags;
  auto display = ccmpl::layout(8.0, 4.0, {"#"}, ccmpl::RGB(1., 1., 1.));
  display.set_ratios({2.}, {1.});

  display().title   = "Short Circuit";    
  display()         = "equal";   
  display()         = ccmpl::show_tics(false,false); 
  display()         = world.limits(MARGIN);    
  display()        += world.plot_protons();        flags += '#';
  display()        += world.plot_electrons();      flags += '#';
  display()        += world.plot_V(PLOT_V_MIN,
				   PLOT_V_MAX,
				   PLOT_V_NB_ISO,
				   PLOT_V_NB_X,
				   PLOT_V_NB_Y);   flags += '#';

  m.generate(display);

  for(unsigned int step = 0; step < NB_STEPS; ++step) {
    std::cerr << std::setw(5) << step+1 << "/" << NB_STEPS << "    \r" << std::flush;
    std::cout << display(flags, ccmpl::nofile() , ccmpl::nofile());
    for(unsigned int substep = 0; substep < NB_SUBSTEPS; ++substep)
      world.move([&world](const elec::Point& p) -> elec::Point {return world.E(p);});
  }
  std::cerr << std::endl;
  std::cout << ccmpl::stop;
  
  return 0;
}
