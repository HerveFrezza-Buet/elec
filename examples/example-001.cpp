

#include <iostream>
#include <elec.hpp>


#define RADIUS1 1.5
#define RADIUS2 1.0
#define RADIUS3 0.2

#define NB_STEPS     1000
#define NB_SUBSTEPS     2
#define NB_ELECTRONS 1500

int main(int argc, char* argv[]) {
  elec::Main m(argc,argv,"example-001");

  auto left  = elec::disk(elec::Point(-RADIUS1,        0),                       RADIUS2, elec::metal());
  auto right = elec::disk(elec::Point( RADIUS1,        0),                       RADIUS2, elec::metal());
  auto bar   = elec::box (elec::Point(-RADIUS1, -RADIUS3), elec::Point(RADIUS1, RADIUS3), elec::metal());
  auto group = elec::set ({left,bar,right});

  elec::World world;
  auto group_idf = (world += group);
  world.build_protons(group_idf);
  world.add_electrons_random(left, NB_ELECTRONS);

  auto display = ccmpl::layout(8.0, 4.0, {"#"}, ccmpl::RGB(1., 1., 1.));

  display.set_ratios({2.}, {1.});
  display().title   = "Example 001";    
  display()         = "equal";   
  display()         = world.limits(1);    
  display()        += world.plot_protons();
  display()        += world.plot_electrons();

  m.generate(display);

  for(unsigned int step = 0; step < NB_STEPS; ++step) {
    std::cerr << std::setw(5) << step+1 << "/" << NB_STEPS << "    \r" << std::flush;
    std::cout << display("##",ccmpl::nofile() , ccmpl::nofile());
    for(unsigned int substep = 0; substep < NB_SUBSTEPS; ++substep)
      world.move([&world](const elec::Point& p) -> elec::Point {return world.E(p);});
  }
  std::cerr << std::endl;
  std::cout << ccmpl::stop;
  
  return 0;
}
