#ifndef Utilities_h
#define Utilities_h

//Code from Marko Kovac
// C++
#include <iostream>
#include <vector>
#include <cassert>


using namespace std;


class Utilities
{

public:
	
   Utilities();
   ~Utilities();
   
   vector<uint64_t>         unpack5e3mToInt( const vector<uint64_t>& );
   vector<vector<uint64_t>> unpack5e4mToInt( const vector<vector<uint64_t>>& );

   vector<uint64_t> pack4e4mFromInt( const vector<uint64_t>& );
   
   vector<uint64_t> addVectors(const vector<uint64_t>&, const vector<uint64_t>& );
   
   static void printArchMatrix_CE_E(const vector<vector<int>>& );
   static void printArchMatrix_CE_H(const vector<vector<vector<int>>>& );


private:


};
#endif
