/*
  
  g++ -o test-001 test-001.cpp -I. `pkg-config --cflags --libs ccmpl`
  
 */

#include <iostream>
#include <elec.hpp>


#define RADIUS1 2
#define RADIUS2 1.0
#define RADIUS3 0.2
#define RADIUS4 .75
#define RADIUS5 (RADIUS2*2.5)

int main(int argc, char* argv[]) {
  elec::Main m(argc,argv,"test-002");

  auto ball  = elec::disk({0, 0}, RADIUS2, elec::metal());
  auto lbar  = elec::box ({-RADIUS1, -RADIUS3}, {-RADIUS4, RADIUS3}, elec::metal());
  auto rbar  = elec::box ({ RADIUS4, -RADIUS3}, { RADIUS1, RADIUS3}, elec::metal());
  
  auto lball = elec::translate(ball, {-RADIUS1, 0});
  auto rball = elec::translate(ball, {-RADIUS1, 0});

  auto lbag   = elec::set({lball,lbar});

  elec::World world;


  auto lball_idf  = (world += lball);
  world                    += lbar;
  
  auto ulball_idf = (world += elec::translate(lball, {0,  RADIUS5}));
  world                    += elec::translate(lbar,  {0,  RADIUS5});
		     
  auto blball_idf = (world += elec::translate(lball, {0, -RADIUS5}));
  world                    += elec::translate(lbar,  {0, -RADIUS5});

  world += vflip(lbar,0);


  world.build_protons();
  world.build_electrons(ulball_idf);
  world.build_electrons( lball_idf);
  world.build_electrons(blball_idf);

  auto display = ccmpl::layout(16.0, 8.0, {"#"}, ccmpl::RGB(1., 1., 1.));

  display.set_ratios({2.}, {1.});
  display().title   = "Test";    
  display()         = "equal";   
  display()         = world.limits(1);    
  display()        += world.plot_protons();
  display()        += world.plot_electrons();

  m.generate(display);

  for(unsigned int i=0; i<200; ++i) {
      std::cout << display("##",ccmpl::nofile() , ccmpl::nofile());
      world.move([](const elec::Point&) -> elec::Point {return {-.1,0};});
    }
  std::cout << ccmpl::stop;
  
  return 0;
}
