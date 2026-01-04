//Code from Marko Kovac
// Include classes
#include "../../interface/TPGStage1Emulation/Utilities.h"


//=================
Utilities::Utilities()
{
   

}
//=================



//===========
Utilities::~Utilities()
{
}
//===========



//==============================================================================
vector<uint64_t> Utilities::unpack5e3mToInt( const vector<uint64_t>& inputData )
{
   
   vector<uint64_t> output;
   
   for (auto num : inputData)
   {
      // Make sure the input number is an 8-bit number
      if (num >= 0x100)
      {
         continue; // Skip invalid numbers
      }

      // Extract exponent (e) and mantissa (m)
      int e = (num >> 3) & 0x1F;  // First 5 bits as exponent
      uint64_t m = num & 0x07;    // Last 3 bits as mantissa

      // Calculate the unpacked integer
      uint64_t unpackedValue;
      
      if (e == 0)
      {
         unpackedValue = m;
      }
      else if (e == 1)
      {
         unpackedValue = 8 + m;
      }
      else
      {
         unpackedValue = (16 + 2 * m + 1) << (e - 2);
      }

//      cout << "unpackedValue = " << unpackedValue << ", ";

      // Add the unpacked value to the result vector
      output.push_back(unpackedValue);
   }      
   return output;
}
//==============================================================================



//==============================================================================================
vector<vector<uint64_t>> Utilities::unpack5e4mToInt( const vector<vector<uint64_t>>& inputData )
{

   vector<vector<uint64_t>> output(inputData.size(), vector<uint64_t>(inputData[0].size(), 0));
   
   // Iterate over each element in the 2D vector
   for (size_t i = 0; i < inputData.size(); ++i)
   {
      for (size_t j = 0; j < inputData[i].size(); ++j)
      {
         uint64_t num = inputData[i][j];
         
         // Ensure the input is a 9-bit number
//         assert(num < 0x200);
         
         // Make sure the input number is an 9-bit number
         if (num >= 0x200)
         {
            continue; // Skip invalid numbers
         }

         // Extract exponent (e) and mantissa (m)
         int e = (num >> 4) & 0x1F; // First 5 bits as exponent
         uint64_t m = num & 0x0F;   // Last 4 bits as mantissa

         // Apply the unpacking logic
         if (e == 0)
         {
            output[i][j] = m;
         }
         else if (e == 1)
         {
            output[i][j] = 16 + m;
         }
         else
         {
            output[i][j] = static_cast<uint64_t>(32 + 2 * m + 1) << (e - 2);
         }
      }
   }

   return output;
}
//==============================================================================================



//====================================================================
vector<uint64_t> Utilities::pack4e4mFromInt( const vector<uint64_t>& inputData )
{
   
   vector<uint64_t> output;
   
   for (auto energy : inputData)
   {
      // Make sure input is a 19-bit number (energy < 0x80000)
      assert(energy < 0x80000);
      
      // If energy is less than 16, return it directly
      if (energy < 16)
      {
         output.push_back(energy);
         continue;
      }

      int e{1};
      // Shift the energy value until it's less than 32
      while (energy >= 32)
      {
         e += 1;
         energy >>= 1;
      }

      // Format: 4 bits exponent (eee), 4 bits mantissa (mmm)
      output.push_back(static_cast<uint64_t>(16 * (e - 1) + energy));
   }      
      
   return output;
}
//====================================================================



//==========================================================================================
vector<uint64_t> Utilities::addVectors(const vector<uint64_t>& a, const vector<uint64_t>& b)
{
   assert(a.size() == b.size());
   
   vector<uint64_t> result(a.size());
    
   for (size_t i = 0; i < a.size(); ++i)
   {
      result[i] = a[i] + b[i];
   }
   return result;
}
//==========================================================================================



//===========================================================================
 void Utilities::printArchMatrix_CE_E(const vector<vector<int>>& archMatrix )
{
   cout << "Architecture:" << endl;
   
   cout << "[";
   
   for (size_t i = 0; i < archMatrix.size(); ++i)
   {
      cout << "[ ";
      for (size_t j = 0; j < archMatrix[i].size(); ++j)
      {
         cout << archMatrix[i][j];
         if (j < archMatrix[i].size() - 1)
         {
            cout << ".  ";
         }

         // Add a newline every 18 elements, but not after the last element
         if ((j + 1) % 18 == 0 && j + 1 < archMatrix[i].size())
         {
            cout << "\n  "; // Align with the inner list
            cout << " ";
         }
      }
      cout << "]" << endl << " ";
   }
   cout << "]" << endl;
}
//===========================================================================



//===================================================================================
 void Utilities::printArchMatrix_CE_H(const vector<vector<vector<int>>>& archMatrix )
{
   cout << "Architecture:" << endl;
   
   cout << "[";   
   for (const auto& slice : archMatrix)
   {
      cout << "[";
      
      for (size_t i = 0; i < slice.size(); ++i)
      {
         cout << "[ ";

         for (size_t j = 0; j < slice[i].size(); ++j)
         {
            cout << slice[i][j];
            
            if (j < slice[i].size() - 1)
            {
               cout << ".  ";
            }
            else
            {
               cout << ".";
            }
         }
         
         if (i < slice.size() - 1)
         {
            cout << "]" << endl << "  ";
         }
         else
         {
            cout << "]";
         }
       
         
      }
      cout << "]" << endl << endl;
   }
   cout << "]" << endl;

}
//===================================================================================
