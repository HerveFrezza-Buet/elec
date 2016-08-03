

#include <iostream>
#include <elec.hpp>


#define RADIUS1  2
#define RADIUS2 .5
#define RADIUS3  2


int main(int argc, char* argv[]) {
  elec::Main m(argc,argv,"test-003");

  auto mat  = elec::material( 1, 1, .2);

  auto bar = elec::box(elec::Point(-RADIUS1,         -RADIUS2), elec::Point( RADIUS1, RADIUS2), mat);
  auto src = elec::box(elec::Point(-RADIUS1-RADIUS2, -RADIUS2), elec::Point(-RADIUS1, RADIUS2), mat);
  auto dsk = elec::disk({RADIUS1+RADIUS3, 0}, RADIUS3, elec::metal());

  elec::World world;
  world += bar;
  world += dsk;
  auto src_idf = (world += src);
  world.build_protons();
  world.add_electrons_random(bar);
  world.add_electrons_random(src,20*world.nb_protons(src_idf));

  auto display = ccmpl::layout(16.0, 9.0, {"#"}, ccmpl::RGB(1., 1., 1.));

  display.set_ratios({3.}, {1.});
  display().title   = "Test";    
  display()         = "equal";    
  display()         = world.limits(1);    
  display()        += world.plot_protons();
  display()        += world.plot_electrons();

  m.generate(display);

  for(unsigned int i=0; i<500; ++i) {
    std::cout << display("##",ccmpl::nofile() , ccmpl::nofile());
    world.move([](const elec::Point&) -> elec::Point {return {-.1,0};});
    std::cerr << i << "    \r" << std::flush;
  }
  std::cerr << std::endl;
  std::cout << ccmpl::stop;
  
  return 0;
}
