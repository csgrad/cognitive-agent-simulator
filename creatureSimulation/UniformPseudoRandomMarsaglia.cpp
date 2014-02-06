/*
$Header: /home/agerisch/work/code/Netzwerk/UPRs/RCS/UniformPseudoRandomMarsaglia.cpp,v 1.4 2002/07/22 23:52:56 agerisch Exp $

$RCSfile: UniformPseudoRandomMarsaglia.cpp,v $ $Revision: 1.4 $

$Log: UniformPseudoRandomMarsaglia.cpp,v $
Revision 1.4  2002/07/22 23:52:56  agerisch
 - renamed pow to power and j1 to i2 in method SetSeed(const unsigned
   short, const unsigned short, const unsigned short, const unsigned
   short) for better pedantic compilation.

Revision 1.3  2002/07/11 15:20:17  agerisch
 - This goes with .h file Rev. 1.3
 - added implementation of Get and Set method for full generator state:
     void GetGeneratorState(unsigned long int State[47]) const;
     void SetGeneratorState(const unsigned long int State[47]);

Revision 1.2  2002/06/21 18:20:23  agerisch
- This goes with .h file Rev. 1.2.
- Checks on the random number only if DEBUG is defined.
- The SetSeed() method with 4 arguments: the "i++;" statement has benn
  moved 2 lines down. this should have not affected the randomness of the
  sequence but only its values.

Revision 1.1  2002/03/19 22:38:38  agerisch
Initial revision

DONE
*/
#include "stdafx.h"
#include "UniformPseudoRandomMarsaglia.h"

// The combined generator produces random integers in 
// the inclusive range 0 to MAXINT, where 
// MAXINT = (2**32)-1 = 4 294 967 295. This is also the modulus of 
// the Weyl generator minus 1.
#define MAXINT        4294967295U    
// The inverse of MAXINT+1 in double precision  and in float precision
// (then a random integer divided by this value is guaranteed to be in 
// the range [0,1) of the respective type.
#define INVERSE_MAXINT_FLOAT  ( (float)(2.328306e-10))
#define INVERSE_MAXINT_DOUBLE ((double)(2.328306436538696e-10))
// The parameter k of the Weyl generator.
#define KWEYL          362436069U     
// SWBMOD is the modulus b =  2**32 - 5 of the subtract-with-borrow 
// generator 
#define SWBMOD        4294967291U   
// The lag r = 43 of the subtract-with-borrow generator (the lag is s = 22).
#define LAG            43

//[]-------------------------------------------------------------------------[]
// Print info about the UPR
void UniformPseudoRandomMarsaglia::PrintInfo(ostream &OStream) const {
  OStream 
    << "* UniformPseudoRandomMarsaglia" <<endl
    << "*    Random number generator from paper by Marsaglia, G., " <<endl
    << "*    Narasimhan, B., and Zaman, A. ( Comp Phys Comm 60 (1990) )" <<endl
    << "*    Written by Buks van Rensburg and Daniel Gruner, April 25, 1990" <<endl
    << "*    Modified by Jp Voroney April 1998" <<endl
    << "*    Implemented in a C++ class by A Gerisch January 2002" <<endl
    << "*" <<endl <<endl;

  unsigned long b = ULONG_MAX;
  OStream
    << "The value of ULONG_MAX on the system is: " << b 
    << " (must be at least 4294967295)." <<endl
    << "The result of adding 1 to this value is: " << b+1 << endl 
    <<endl; 

  OStream
    << setiosflags(ios::scientific)
    << "* Generates unsigned long integers in the inclusive range 0 to "
    <<  MAXINT << " (should be 4 294 967 295)." << endl
    << "* 1.0 - the maximum double random number is " 
    << double(1.0)-(double)(MAXINT*INVERSE_MAXINT_DOUBLE) 
    << " (should be positive, around 2.328308e-10)" << endl
    << "* 1.0 - the maximum float  random number is " 
    << float(1.0)-(float)(MAXINT*INVERSE_MAXINT_FLOAT) 
    << " (should be positive, around 1.788139e-07)." << endl
    << "* The error in INVERSE_MAXINT_DOUBLE is "
    << double(1.0)/(1.0+double(MAXINT))-INVERSE_MAXINT_DOUBLE << endl
    << "* The error in INVERSE_MAXINT_FLOAT  is  "
    << float(1.0)/(1.0+float(MAXINT))-INVERSE_MAXINT_FLOAT << endl
    << endl <<endl;
}
 
//[]-----------------------------------------------------------------------[]
UniformPseudoRandomMarsaglia::UniformPseudoRandomMarsaglia 
           (const unsigned long seed) {
  SetSeed(seed);
}

//[]-----------------------------------------------------------------------[]
UniformPseudoRandomMarsaglia::
     UniformPseudoRandomMarsaglia(const unsigned short seed1, 
				  const unsigned short seed2,
				  const unsigned short seed3,
				  const unsigned short seed4)
{
  SetSeed(seed1, seed2, seed3, seed4);
}


//[]-----------------------------------------------------------------------[]
UniformPseudoRandomMarsaglia::~UniformPseudoRandomMarsaglia () {
}


  // Return maximum possible random integer of the sequence
//[]-----------------------------------------------------------------------[]
unsigned long UniformPseudoRandomMarsaglia::GetMaximumRandomInteger(void) 
  const {
  return MAXINT;
}


//[]-----------------------------------------------------------------------[]
void UniformPseudoRandomMarsaglia::SetSeed (const unsigned long seed)
{
  int i;
  unsigned long   m, mm[3];

  // Generate 3 other seeds in the inclusive range 0, 178 from the seed n
  // for the 3-lag Fibonacci generator
  mm[0] = seed % 179;
  for( i = 1; i < 3; i++ )
    {
      mm[i] = (mm[i-1] + KWEYL) % 179;
    }
  // Generate seed for the congruential generator in the inclusive 
  // range 0, 168
  m = (mm[2] + KWEYL) % 169;

  // now seed the main generator
  SetSeed((unsigned short)mm[0], (unsigned short)mm[1],
	  (unsigned short)mm[2], (unsigned short)m);
}


//[]-----------------------------------------------------------------------[]
void UniformPseudoRandomMarsaglia::SetSeed(const unsigned short seed1, 
					   const unsigned short seed2,
					   const unsigned short seed3,
					   const unsigned short seed4)
/*
 *    Initialisation routine for the random number generator.
 *    Takes 4 seeds as input, and produces the initial
 *    LAG+1 random numbers needed for the generator.
 *
 *    The vector lRan contains the last LAG random integers, while 
 *    lRan1 is a random integer generated by a Weyl scheme.  
 *    ic is the carry bit, iKounter keeps track of the coordinate
 *    of the generated integer LAG times ago; iIndex of the coordinate of
 *    the coordinate of the generated integer 22 times ago.
 *
 *   (AG) The setup procedure is taken from [G. Marsaglia, A. Zaman,
 *   and W.W. Tsang. Towards a universal random number generator.
 *   Statistics & Probability Letters (9), 35-39, 1990.]
*/
{
  int i, i1, i2;
  unsigned long   m, mm[3];
  unsigned long   bit, power;

  iKounter = 0; 
  iIndex = 21;
  ic = 0;

  mm[0] = seed1 % 179;
  mm[1] = seed2 % 179;
  mm[2] = seed3 % 179;
  m     = seed4 % 169;

  //i % 3 points to the next number to use in mm
  i = 0;

  /* generate the lRan vector */
  for( i1 = 0; i1 < LAG; i1++ )
    {
      lRan[i1] = 0;
      power = 1;
      for( i2 = 1; i2 <= 32; i2++ )
	{
	  bit = m * mm[i % 3];
	  if( (bit % 64) > 31 ) lRan[i1] += power;
	  power *= 2;
	  mm[i % 3] = (mm[0] * mm[1] * mm[2]) % 179;
	  m = (53 * m + 1) % 169;
	  i++;
	}
      if (lRan[i1] >= SWBMOD) { 
	cout 
	  << "Error: lRan[" <<i1 <<"] = " << lRan[i1]
	  << " >= SWBMOD. Choose other seed value."
	  << endl;
	return;
      }
    }
  
  /* generate lRan1 */
  lRan1 = 0;
  power = 1;
  for( i2 = 1; i2 <= 32; i2++ )
    {
      bit = m * mm[i % 3];
      if( (bit % 64) > 31 ) lRan1 += power;
      power *= 2;
      mm[i % 3] = (mm[0] * mm[1] * mm[2]) % 179;
      m = (53 * m + 1) % 169;
      i++;
    }
  if (lRan1 > MAXINT) {
    cout 
      << "Error: lRan = " << lRan1 
      << " > MAXINT. Choose other seed value."
      << endl; 
    return;
  }
}


//[]-----------------------------------------------------------------------[]
unsigned long UniformPseudoRandomMarsaglia::DrawLong (void) {
  unsigned long result;

  /////////////////////////////////////////////////////////////////
  /////  IF CHANGES ARE MADE TO THE FOLLOWING BLOCK THEN ALSO  ////
  /////  COPY THIS BLOCK AGAIN TO THE METHOD DrawLong() WITH   ////
  /////  ARGUMENTS const unsigned int count AND unsigned long  ////
  /////  randomVector[].                                       ////
  /////////////////////////////////////////////////////////////////
  // The lagged Fibonacci subtract with borrow generator:
  // (iKounter points to x(n-43).iIndex  points to x(n-22))
  // we now form x(n) and store it on place iKounter.
  if (lRan[iIndex] < lRan[iKounter] + ic)
    {
      // the braces (SWBMOD - lRan[iKounter]) are important to avoid
      // overflow (hopefully they are not optimised away...)
      lRan[iKounter] = lRan[iIndex] + (SWBMOD - lRan[iKounter]) - ic;
      ic = 1;
#ifdef DEBUG
      // if the following test succeeds then something went wrong 
      // (should not happen normally!): 
      if (lRan[iKounter]>(SWBMOD -1)) 
      	cout << "Error in DrawLong (Case 1)." <<endl;
#endif
    }
  else
    {
      lRan[iKounter] = lRan[iIndex] - lRan[iKounter] - ic;
      ic = 0;
#ifdef DEBUG
      // if the following test succeeds then something went wrong 
      // (should not happen normally!): 
      if (lRan[iKounter]>(MAXINTFIB -1)) 
      	cout << "Error in DrawLong (Case 2)." <<endl;
#endif
    }

  // The Weyl generator:
  if (lRan1 >= KWEYL)
    lRan1 -= KWEYL;
  else
    lRan1 = MAXINT - ( (KWEYL - lRan1) - 1);

  // Now combine both by subtraction modulo 2**32 to obtain the result:
  if (lRan1 >= lRan[iKounter])
    result = lRan1 - lRan[iKounter];
  else
    result = MAXINT - ( (lRan[iKounter] - lRan1) - 1);
#ifdef DEBUG
  // if the following test succeeds then something went wrong 
  // (should not happen normally!): 
  if (result > MAXINT) 
    cout << "Error in DrawLong (Case 3)." <<endl; 
#endif

  // update iKounter and iIndex
  iKounter++; if (iKounter>(LAG-1)) iKounter = 0;
  iIndex++;   if (iIndex  >(LAG-1)) iIndex   = 0;
  /////////////////////////////////////////////////////////////////
  //////                  END OF BLOCK                       //////
  /////////////////////////////////////////////////////////////////
  return result;
}


//[]-----------------------------------------------------------------------[]
void UniformPseudoRandomMarsaglia::DrawLong(const unsigned int count, 
					    unsigned long randomVector[])
{
  unsigned long result;
  for (unsigned int i=0; i<count; i++) {
  /////////////////////////////////////////////////////////////////
  /////  IF CHANGES ARE MADE TO THE FOLLOWING BLOCK THEN ALSO  ////
  /////  COPY THIS BLOCK AGAIN TO THE METHOD DrawLong() WITH   ////
  /////  ARGUMENTS const unsigned int count AND unsigned long  ////
  /////  randomVector[].                                       ////
  /////////////////////////////////////////////////////////////////
  // The lagged Fibonacci subtract with borrow generator:
  // (iKounter points to x(n-43).iIndex  points to x(n-22))
  // we now form x(n) and store it on place iKounter.
  if (lRan[iIndex] < lRan[iKounter] + ic)
    {
      // the braces (SWBMOD - lRan[iKounter]) are important to avoid
      // overflow (hopefully they are not optimised away...)
      lRan[iKounter] = lRan[iIndex] + (SWBMOD - lRan[iKounter]) - ic;
      ic = 1;
#ifdef DEBUG
      // if the following test succeeds then something went wrong 
      // (should not happen normally!): 
      if (lRan[iKounter]>(SWBMOD -1)) 
      	cout << "Error in DrawLong (Case 1)." <<endl;
#endif
    }
  else
    {
      lRan[iKounter] = lRan[iIndex] - lRan[iKounter] - ic;
      ic = 0;
#ifdef DEBUG
      // if the following test succeeds then something went wrong 
      // (should not happen normally!): 
      if (lRan[iKounter]>(MAXINTFIB -1)) 
      	cout << "Error in DrawLong (Case 2)." <<endl;
#endif
    }

  // The Weyl generator:
  if (lRan1 >= KWEYL)
    lRan1 -= KWEYL;
  else
    lRan1 = MAXINT - ( (KWEYL - lRan1) - 1);

  // Now combine both by subtraction modulo 2**32 to obtain the result:
  if (lRan1 >= lRan[iKounter])
    result = lRan1 - lRan[iKounter];
  else
    result = MAXINT - ( (lRan[iKounter] - lRan1) - 1);
#ifdef DEBUG
  // if the following test succeeds then something went wrong 
  // (should not happen normally!): 
  if (result > MAXINT) 
    cout << "Error in DrawLong (Case 3)." <<endl; 
#endif

  // update iKounter and iIndex
  iKounter++; if (iKounter>(LAG-1)) iKounter = 0;
  iIndex++;   if (iIndex  >(LAG-1)) iIndex   = 0;
  /////////////////////////////////////////////////////////////////
  //////                  END OF BLOCK                       //////
  /////////////////////////////////////////////////////////////////

    randomVector[i] = result;
  }
}

//[]-----------------------------------------------------------------------[]
double UniformPseudoRandomMarsaglia::DrawDouble(void)
{
  double result = (double)(DrawLong()*INVERSE_MAXINT_DOUBLE);
  if (result >= 1.0) 
    cout << "Error DrawDouble." << endl;
  return result;
}

//[]-----------------------------------------------------------------------[]
float UniformPseudoRandomMarsaglia::DrawFloat(void)
{
  float result = (float)(DrawLong()*INVERSE_MAXINT_FLOAT);
  if (result >= 1.0) 
    cout << "Error DrawFloat." << endl;
  return result;
}

//[]-----------------------------------------------------------------------[]
unsigned int UniformPseudoRandomMarsaglia::DrawInteger(const unsigned int n) 
{
  if (n == 0) return 0;
  return (unsigned int)(  (DrawLong() % n) + 1 );
}


//[]-----------------------------------------------------------------------[]
void UniformPseudoRandomMarsaglia::GetGeneratorState(unsigned long int 
						     State[LAG+4]) const
{
  for (unsigned int i=0; i < LAG; i++)
    State[i] = lRan[i];
  State[LAG] = lRan1;
  State[LAG+1] = ic;
  State[LAG+2] = (unsigned long int)iKounter;
  State[LAG+3] = (unsigned long int)iIndex;
}

//[]-----------------------------------------------------------------------[]
void UniformPseudoRandomMarsaglia::SetGeneratorState(const unsigned long int 
						     State[LAG+4])
{
  for (unsigned int i=0; i < LAG; i++)
    lRan[i] = State[i];
  lRan1    = State[LAG];
  ic       = State[LAG+1];
  iKounter = (unsigned short int)State[LAG+2];
  iIndex   = (unsigned short int)State[LAG+3];
}
