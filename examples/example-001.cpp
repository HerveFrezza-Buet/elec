

#include <iostream>
#include <elec.hpp>


#define RADIUS1 1.5
#define RADIUS2 1.0
#define RADIUS3 0.2
#define MARGIN  1.0

#define NB_STEPS        750
#define NB_SUBSTEPS       1
#define ELECTRONS_RATIO   2

#define PLOT_V_MIN      -10
#define PLOT_V_MAX        0
#define PLOT_V_NB_ISO    10
#define PLOT_V_NB_X      60
#define PLOT_V_NB_Y      30


//#define SHOW_E

int main(int argc, char* argv[]) {
  elec::Main m(argc,argv,"example-001");

  auto material = elec::material(1.0,.33,.05);

  auto left  = elec::disk(elec::Point(-RADIUS1,        0),                       RADIUS2, material);
  auto right = elec::disk(elec::Point( RADIUS1,        0),                       RADIUS2, material);
  auto bar   = elec::box (elec::Point(-RADIUS1, -RADIUS3), elec::Point(RADIUS1, RADIUS3), material);
  auto group = elec::set ({left,bar,right});

  elec::World world;
  auto group_idf = (world += group);
  world.build_protons(group_idf);
  world.add_electrons_random(left, ELECTRONS_RATIO * world.nb_protons(group_idf));

  std::string flags;
#ifdef SHOW_E
  auto display = ccmpl::layout(8.0, 8.0, {"#","#"}, ccmpl::RGB(1., 1., 1.));
  display.set_ratios({2.}, {1.,1.});
#else
  auto display = ccmpl::layout(8.0, 4.0, {"#"}, ccmpl::RGB(1., 1., 1.));
  display.set_ratios({2.}, {1.});
#endif

  display().title   = "Particles and potential";    
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
#ifdef SHOW_E
  display++;
  display().title   = "Electric field";    
  display()         = "equal";   
  display()         = ccmpl::show_tics(false,false); 
  display()         = world.limits(MARGIN);
  display()        += world.plot_E(PLOT_E_COEF,PLOT_E_NB_X,PLOT_E_NB_X, true); flags += '#';
#endif
    

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
