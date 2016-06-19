/*
  
  g++ -o test-001 test-001.cpp -I. `pkg-config --cflags --libs ccmpl`
  
 */

#include <iostream>
#include <elec.hpp>

#define VIEW_FILE "test.py"

#define RADIUS1 1.5
#define RADIUS2 1.0
#define RADIUS3 0.2

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cout << "Usage : " << std::endl
              << argv[0] << " generate" << std::endl
              << argv[0] << " run | ./" << VIEW_FILE << std::endl;
    return 0;
  }
  bool generate_mode = std::string(argv[1])=="generate";

  std::vector<elec::Point> E = {{
      elec::Point(-.1,0),
      { 0,-.1},
      {.1, .1},
      {.1,  0}
    }};

  auto mat = elec::material(1,1,1);

  auto left  = elec::disk(elec::Point(-RADIUS1,        0),                       RADIUS2, mat);
  auto right = elec::disk(elec::Point( RADIUS1,        0),                       RADIUS2, mat);
  auto bar   = elec::box (elec::Point(-RADIUS1, -RADIUS3), elec::Point(RADIUS1, RADIUS3), mat);
  auto group = elec::set({left,right,bar});

  elec::World world;
  auto group_idf = (world += group);
  world.build_protons(group_idf);
  world.add_electrons_random(left);

  auto display = ccmpl::layout(8.0, 4.0, {"#"}, ccmpl::RGB(1., 1., 1.));
  display.set_ratios({2.}, {1.});
  display().title   = "Test";    
  display()         = "equal";   
  display()         = world.limits(1);    
  display()        += world.plot_protons();
  display()        += world.plot_electrons();

  if(generate_mode) {
    display.make_movie_python(VIEW_FILE,true, "avconv", "", "", "", 25, "test.mp4", 300);
    //display.make_python(VIEW_FILE,true); 
    return 0;                            
  }    

  for(auto& e : E)
    for(unsigned int i=0; i<80; ++i) {
      std::cout << display("##",ccmpl::nofile() , ccmpl::nofile());
      world.move(e);
      std::cerr << i << std::endl;
    }
  std::cout << ccmpl::stop;
  
  return 0;
}
