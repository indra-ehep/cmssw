#ifndef TowerSums_h
#define TowerSums_h

//Code from Marko Kovac
// C++
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <bitset>
#include <random>
#include <cassert>
#include <regex>


#include "Utilities.h"


using namespace std;


class TowerSums
{

public:
	
   TowerSums();
   ~TowerSums();
   
   pair<int, int> getParametersFromVhFile( const string& );
   
   vector<vector<int>> vhArchInputToArray_CE_E( const string& , int, int );
   vector<vector<vector<int>>> vhArchInputToArray_CE_H( const string& , int, int, const string& );
   
   vector<uint64_t>         readInputEnergies_CE_E( const string& );
   vector<vector<uint64_t>> readInputEnergies_CE_H( const string&, int );
   
   vector<uint64_t>         unpackInteger3m( const vector<uint64_t>& );
   vector<vector<uint64_t>> unpackInteger4m( const vector<vector<uint64_t>>& );

   vector<uint64_t> packInteger4e4m( const vector<uint64_t>& );
   
   vector<uint64_t> summation( const vector<uint64_t>&, const vector<vector<int>>&, int, bool );
   vector<uint64_t> overflowChecker( const vector<uint64_t>&, int );
   vector<uint64_t> trimming(const vector<uint64_t>&, int, int );

   vector<string> generateInputShifts( int, int );
   
   vector<uint64_t> addVectors(const vector<uint64_t>&, const vector<uint64_t>& );
   
   void writeToFile( const vector<uint64_t>& , int , int , const string& );

private:

   Utilities *utilities;

};
#endif
