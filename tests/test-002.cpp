
#include <iostream>
#include <elec.hpp>


#define RADIUS1 3
#define RADIUS2 1.0
#define RADIUS3 0.2
#define RADIUS4 1.2
#define RADIUS5 (RADIUS2*2.5)

#define OFFSET .3
#define RADIUS6 (RADIUS3+OFFSET)

#define HIGH_MIN_DIST  .2 
#define LOW_DENSITY    .3
#define LOW_MOBILITY   .3


#define MODULE_UP
// #define MODULE_MIDDLE
// #define MODULE_DOWN

int main(int argc, char* argv[]) {
  elec::Main m(argc,argv,"test-002");

  auto lball  = elec::disk({-RADIUS1, 0}, RADIUS2, elec::metal());
  auto lbar   = elec::box ({-RADIUS1, -RADIUS3}, {-RADIUS4, RADIUS3}, elec::metal());
  auto lbag   = elec::set({lball,lbar});

  auto r_mat1  = elec::box({-RADIUS4, -RADIUS6}, {RADIUS4, RADIUS6}, elec::material(           1, LOW_DENSITY, HIGH_MIN_DIST     ));
  auto r_mat2  = elec::box({-RADIUS4, -RADIUS6}, {RADIUS4, RADIUS6}, elec::material(LOW_MOBILITY,           1, elecMETAL_MIN_DIST));
  auto r_mat3  = elec::box({-RADIUS4, -RADIUS6}, {RADIUS4, RADIUS6}, elec::material(LOW_MOBILITY, LOW_DENSITY, HIGH_MIN_DIST     ));

  elec::World world;

  auto rbag   = elec::hflip(lbag,0);

#ifdef MODULE_MIDDLE
  auto lball_idf  = (world += lball);
  world                    += lbar;
  world                    += rbag;
  world                    += r_mat2;
#endif
#ifdef MODULE_UP
  auto ulball_idf = (world += elec::translate(lball,  {0,  RADIUS5}));
  world                    += elec::translate(lbar,   {0,  RADIUS5});
  world                    += elec::translate(rbag,   {0,  RADIUS5});
  world                    += elec::translate(r_mat1, {0,  RADIUS5});
#endif		
#ifdef MODULE_DOWN    
  auto blball_idf = (world += elec::translate(lball,  {0, -RADIUS5}));
  world                    += elec::translate(lbar,   {0, -RADIUS5});
  world                    += elec::translate(rbag,   {0, -RADIUS5});
  world                    += elec::translate(r_mat3, {0, -RADIUS5});
#endif



  world.build_protons();
#ifdef MODULE_UP
  world.build_electrons(ulball_idf);
#endif
#ifdef MODULE_MIDDLE   
  world.build_electrons( lball_idf);
#endif
#ifdef MODULE_DOWN    
  world.build_electrons(blball_idf);
#endif

  auto display = ccmpl::layout(16.0, 8.0, {"#"}, ccmpl::RGB(1., 1., 1.));

  display.set_ratios({2.}, {1.});
  display().title   = "Test";    
  display()         = "equal";   
  display()         = world.limits(1);    
  display()        += world.plot_protons();
  display()        += world.plot_electrons();

  m.generate(display);

  for(unsigned int i=0; i<1000; ++i) {
      std::cout << display("##",ccmpl::nofile() , ccmpl::nofile());
      world.move([](const elec::Point&) -> elec::Point {return {-.1,0};});
      std::cerr << i << "     \r" << std::flush;
    }
  std::cerr << std::endl;
  std::cout << ccmpl::stop;
  
  return 0;
}
