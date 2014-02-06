/*
$Header: /home/agerisch/work/code/Netzwerk/UPRs/RCS/UniformPseudoRandomMarsaglia.h,v 1.3 2002/07/11 15:19:58 agerisch Exp $

$RCSfile: UniformPseudoRandomMarsaglia.h,v $ $Revision: 1.3 $

$Log: UniformPseudoRandomMarsaglia.h,v $
Revision 1.3  2002/07/11 15:19:58  agerisch
 - This goes with .cpp file Rev. 1.3
 - added Get and Set method for full generator state:
     void GetGeneratorState(unsigned long int State[47]) const;
     void SetGeneratorState(const unsigned long int State[47]);

Revision 1.2  2002/06/21 18:22:42  agerisch
- This goes with .cpp file Rev. 1.2.
- added lots of detailed documentation.
- corrected a (minor) bug in the initialisation procedure. this bug
  did affect the actual random numbers corresponding to given seed
  values but not their randomness!
- added method which provides us with an array of unsigned long random
  numbers.

Revision 1.1  2002/03/19 22:38:51  agerisch
Initial revision


HISTORY:
============
1st version by Buks van Rensburg and Daniel Gruner, April 25, 1990.
2nd version by J P Voroney April 1998.
3rd version by A Gerisch January 2002, June 2002.
     - Implemented in a C++ class.
     - The setup procedure is taken from [3]

REFERENCES:
===========
 [1] G. Marsaglia, B. Narasimhan, and A. Zaman. A random number 
     generator for PC's. Comp. Phys. Comm. (60), 345-349, 1990.
 [2] G. Marsaglia. Proc. Computer Science and Statistics: 16th Symposium
     on the Interface. Elsevier, Amsterdam, (1985).
 [3] G. Marsaglia, A. Zaman,and W.W. Tsang. Towards a universal random 
     number generator. Statistics & Probability Letters (9), 35-39, 1990.

THE GENERATOR:
==============
  The uniform pseudo-random number generator implemented here is a 
  combination generator which combines a subtract-with-borrow generator 
  (these generators are related to lagged-Fibonacci generators) with a 
  Weyl generator. The combination generator returns unsigned integer 
  values in the range inclusive 0,1,2,...,2**32-1 = 4 294 967 295. 
  The period is not known but in [1] it is stated that there is 
  theoretical support for the empirical observation that combining two 
  generators will produce better results, or at least no worse, than 
  either of the component generators [2].

  THE SUBTRACT-WITH-BORROW GENERATOR:
                x(n) = x(n-s) - x(n-r) - c  mod b,
  where  s and r are the lags, b is the modulus and c the carry bit.
  c is initially 0 or 1 and after the computation of x(n) we set c = 1
  if x(n-s) - x(n-r) - c < 0, or c = 0 otherwise. 
  We use   
      r = 43,  s = 22, initial c = 0, and b = 2**32-5 = 4 294 967 291.
  For this choice of parameter values, the generator produces results 
  in the inclusive range 0,1,2,..., 2**32-6 = 4 294 967 290. Its period 
  is m-1 = b**r - b**s + 1 \approx 2**1376 \approx 10**414.

  THE WEYL GENERATOR:
               y(n) = y(n-1) - k   mod m,
  where k ist a constant and m is the modulus (k relative prime to m). 
  We use
      k = 362 436 069   and   m = 2**32 = 4 294 967 296.
  For this choice of parameters values, the generator produces results 
  in the inclusive range 0,1,2,..., 2**32-1 = 4 294 967 295.

  THE COMBINED GENERATOR:
               z(n) = y(n) - x(n)   mod m,
  where the modulus m is as for the Weyl generator.

THE INITIALISATION PROCEDURE:
=============================
  The initialisation is a two-step procedure. The user has to supply 
  four unsigned integer seed values seed1 to seed4, where only seed1, 
  seed2, seed3 % 179  and seed4 % 169 are significant. With these four 
  seed values we generate, using a small secondary random number 
  generator (described below), the 43+1 seed values required by the 
  main genrator (described above). Note that sedd1, seed2, seed3 
  must not all be one! If the user supplies only one unsigned integer 
  seed value Seed, then 
                        seed1 = Seed        mod 179
                        seed2 = seed1 + k   mod 179
			seed3 = seed2 + k   mod 179
                        seed4 = seed3 + k   mod 169
  where k = 362 436 069 as above.  
  To generate the 43+1 seed values for the main generator we will 
  generate 32*(43+1) random bit values b(1), b(2),..., b(32*(43+1)) which 
  are obtained obtained from the first 32*(43+1) random integers z(n) of
  the secondary generator by b(i) = 0 if z(i) mod 64 < 32 and b(i) = 1
  otherwise (that is b(i) is the sixth bit of z(i)). The seeds for the main
  generator are then given (in binary form)by
    b(32) b(31) ... b( 1),  b(64) b(63) ... b(33), etc.
  The secondary generator is given by a multiplicative combination of a 
  3-lag multiplicative Fibonacci generator (mod 179) and a congruential
  generator (mod 169):
       x(n) = x(n-3) * x(n-2) * x(n-1)    mod 179
       y(n) = 53 * y(n-1) +1              mod 169
       z(n) = x(n) * y(n)
  where x(1) = seed1, x(2) = seed2, x(3) = seed3, and y(1) = seed4 .
 

TESTPROGRAM:
============
You can verify the generator with the following testprogram:

>>>-------------------TestUniformPseudoMarsaglia.cpp-------------------<<<
#include <iostream>
using namespace std;
#include "UniformPseudoRandomMarsaglia.h"
int main(void) {
  UniformPseudoRandomMarsaglia UPR = UniformPseudoRandomMarsaglia(19);
  UPR.PrintInfo(cout);
  cout << setiosflags(ios::scientific) <<setprecision(56) << endl;
  cout << UPR.DrawLong() << endl;
  cout << UPR.DrawDouble() << endl;
  cout << UPR.DrawFloat() << endl;
  cout << UPR.DrawInteger(45) << endl;
  cout << UPR.DrawLong() << endl;
  cout << UPR.DrawDouble() << endl;
  cout << UPR.DrawFloat() << endl;
  cout << UPR.DrawInteger(45) << endl;
  cout << UPR.DrawLong() << endl;
  cout << UPR.DrawDouble() << endl;
  cout << UPR.DrawFloat() << endl;
  cout << UPR.DrawInteger(45) << endl;
  cout << UPR.DrawLong() << endl;
  cout << UPR.DrawDouble() << endl;
  cout << UPR.DrawFloat() << endl;
  cout << UPR.DrawInteger(45) << endl;
  return(1);
}
>>>-------------------TestUniformPseudoMarsaglia.cpp-------------------<<<

Compile this with: 
  g++ -Wall -o TestUniformPseudoMarsaglia TestUniformPseudoMarsaglia.cpp \
               UniformPseudoRandomMarsaglia.cpp
and run: TestUniformPseudoMarsaglia
The output should be:

>>>--------------Output Of TestUniformPseudoMarsaglia------------------<<<
* UniformPseudoRandomMarsaglia
*    Random number generator from paper by Marsaglia, G., 
*    Narasimhan, B., and Zaman, A. ( Comp Phys Comm 60 (1990) )
*    Written by Buks van Rensburg and Daniel Gruner, April 25, 1990
*    Modified by Jp Voroney April 1998
*    Implemented in a C++ class by A Gerisch January 2002
*

The value of ULONG_MAX on the system is: 4294967295 (must be at least 4294967295).
The result of adding 1 to this value is: 0

* Generates unsigned long integers in the inclusive range 0 to 4294967295 (should be 4 294 967 295).
* 1.0 - the maximum double random number is 2.328308e-10 (should be positive, around 2.328308e-10)
* 1.0 - the maximum float  random number is 1.788139e-07 (should be positive, around 1.788139e-07).
* The error in INVERSE_MAXINT_DOUBLE is 2.584939e-26
* The error in INVERSE_MAXINT_FLOAT  is  4.157915e-17



3307321456
9.01135945692658313355138943734345957636833190917968750000e-01
8.24859917163848876953125000000000000000000000000000000000e-01
14
2073481052
7.16672517824917920670202420296845957636833190917968750000e-01
7.78340280055999755859375000000000000000000000000000000000e-01
42
2248175988
3.04515669587999526779498182804672978818416595458984375000e-01
1.41891628503799438476562500000000000000000000000000000000e-01
24
4006605967
9.32992524001747258743932889046845957636833190917968750000e-01
4.76976096630096435546875000000000000000000000000000000000e-01
20
>>>--------------Output Of TestUniformPseudoMarsaglia------------------<<<


DONE
*/

#if !defined(UniformPseudoRandomMarsaglia_h__INCLUDED_)
#define UniformPseudoRandomMarsaglia_h__INCLUDED_

#include <math.h>
#include <limits.h>
#include <iostream>
#include <iomanip>
using namespace std;


class UniformPseudoRandomMarsaglia {
 private:
  unsigned long int lRan[43];
  unsigned long int lRan1, ic; 
  // always points to x(n-43) in lRan.
  unsigned short int iKounter;
  // always points to x(n-22) in lRan.
  unsigned short int iIndex;

 public:
  // Constructor.
  UniformPseudoRandomMarsaglia(const unsigned long seed = 1);
  // Note that seed1, seed2, seed3 must not all be one!  
  UniformPseudoRandomMarsaglia(const unsigned short seed1, 
			       const unsigned short seed2,
			       const unsigned short seed3,
			       const unsigned short seed4);

  // Destructor.
  ~UniformPseudoRandomMarsaglia();

  // Get and Set method for full generator state:
  void GetGeneratorState(unsigned long int State[47]) const;
  void SetGeneratorState(const unsigned long int State[47]);


  // Return maximum possible random integer of the sequence
  unsigned long GetMaximumRandomInteger(void) const;

  // Set the seed of the generator.
  // Compute 4 new seeds from Seed and call the second version of SetSeed
  // to compute from those the initial random numbers of the generator.
  // Only seed % 179 is significant.
  void SetSeed(const unsigned long seed = 1);
  // Compute from the seeds the initial random numbers of the generator.
  // Only seed1, seed2, seed3 % 179  and seed4 % 169 are significant.
  // Note that seed1, seed2, seed3 must not all be one!  
  void SetSeed(const unsigned short seed1, 
	       const unsigned short seed2,
	       const unsigned short seed3,
	       const unsigned short seed4);

  // Draw an unsigned long random integer from the full range of 
  // possible values.
  unsigned long DrawLong(void);
  // Draw count unsigned long random integers from the full range of 
  // possible values and store them in randomVector[0] through
  // randomVector[count-1].
  void DrawLong(const unsigned int count, unsigned long randomVector[]);

  // Draw a double valued pseudo-random number from the interval $[0,1)$ 
  // with uniform probability using the full range of possible random
  // integers.
  double DrawDouble(void);

  // Draw a float valued pseudo-random number from the interval $[0,1)$ 
  // with uniform probability using the full range of possible random
  // integers.
  float DrawFloat(void);

  // Returns a uniform random draw from 1...n. If n=0 then return 0.
  unsigned int DrawInteger(const unsigned int n); 

  // Print info about the UPR
  void PrintInfo(ostream &OStream) const;
};


#endif // !defined(UniformPseudoRandomMarsaglia_h__INCLUDED_)









