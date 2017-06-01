/****************************************************************************

Copyright (c) 2007, Advanced Micro Devices, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of Advanced Micro Devices, Inc nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/
/*****************************************************************************
  \file BinomialOptionPricing.h
  \brief Implements Binomial Option Pricing for CPU and performs wraper for GPU.
 ****************************************************************************/
#include <iostream>
#include <cmath>
#include<cstring>

using namespace std;

// !...
#define NUMSTEP 12
/**
 * \NUMSTEP 12
 * \brief Macro specifying defaule time steps, this needs to be changed accordingly.
 */
#define RISKFREE 0.02f
/**
 * \RISKFREE 0.02f
 * \brief risk free interest rate.
 */
#define VOLATILITY 0.30f
/**
 * \VOLATILITY 0.30f
 * \brief Volatility factor for Binomial Option Pricing.
 */


// Defaults used for the struct below. They are defined in the cpp.
extern unsigned int DEFAULT_DATATYPE;
extern unsigned int DEFAULT_TIMINGSTEPS;
extern unsigned int DEFAULT_NUMSAMPLES;
extern int          DEFAULT_VERIFY;
extern int          DEFAULT_PERFORMANCE;
extern int          DEFAULT_QUIET;
extern int          DEFAULT_TIMING;

typedef struct infoRecCustom
{
    /**
     * Flag to specify whether to do CPU verification.
     * Activated with -e command line option.
     */
    int Verify;
        /**
     * Flag to specify whether to compare performance versus CPU
     * Activiated with -p command line option.
     */
    int Performance;
/**
      Flag to specify data type. 
      Activated with the -dt command line option.
      */
    int DataType;
    /**
      Flag to specify number of timing steps used to compute option pricing. 
      Activated with the -st command line option.
      */
    int TimingSteps;
    /**
      Flag to specify whether the number of samples used for simulation. 
      Activated with the -n command line option.
      */
    int NumSamples;
    /**
     * Flag to suppress all data output. 
     * Defaults to 0, modified with the -q command line option.
     */
    int Quiet;
    /**
      Flag to specify whether to print timing results or not.
      Activated with the -t command line option.
      */
    int Timing;
    
} InfoCustom;


//Function prototype declaration for gpuKernelHandler, defined in .br file...
void mainGPUHandller4(float *stockPrice, float *optionStrike, float *optionYears,
                      float *resultGPU, const float riskFactor,
                      const float volatility, const unsigned int n,
                      const unsigned int numSteps);

//Function prototype declaration for gpuKernelHandler, defined in .br file...
void mainGPUHandller8(float *stockPrice, float *optionStrike, float *optionYears,
                      float *resultGPU, const float riskFactor,
                      const float volatility, const unsigned int n,
                      const unsigned int numSteps);

//Function prototype declaration for gpuKernelHandler, defined in .br file...
void mainGPUHandller12(float *stockPrice, float *optionStrike, float *optionYears,
                      float *resultGPU, const float riskFactor,
                      const float volatility, const unsigned int n,
                      const unsigned int numSteps);

template <class T>
class BinomialOptionPricing {
public:
    //Class Constructor...
    BinomialOptionPricing(unsigned int numSteps = 4, unsigned long int numSamples = 1024 * 1024 * 4);

    //Class Destructor...
    ~BinomialOptionPricing();
    
    //Initializes all data members with valid set of values...
    void memberInitializer(void);
    
    //CPU implementation of binomial Option Pricing...
    void binomialOptionCPU(void);
    
    //GPU implementation of binomial Option Pricing...
    void binomialOptionGPU(void);
    
    //Function to compare the two outcomes of CPU computation and GPU computation...
    void compareResults(void);

    //Return the total number of samples...
    unsigned long int getNumSamples();
    
    //Return the total number of time steps...
    unsigned int getNumSteps();

    //Return the tolerance for GPU computation...
    double getTolerance();

private:
    //Utility function to generate random number in a given range...
    T randomT(const T randMax, const T randMin);

    //Returns the Maximum...
    T maxT(const T param1, const T param2);
    
    //Class member variables...
    //Stock Price...
    T *_stockPrice;
 
    //Option Strike Price...
    T *_optionStrike;

    //Time to the expiration date...
    T *_optionYears;

    //CPU computation...
    T *_resultCPU;

    //GPU computation...
    T *_resultGPU;

    //GPU tolerance...
    double _tolerance;

    //No. of time steps..
    unsigned int _numSteps;

    //No. of samples...
    unsigned long int _numSamples;
};

/** 
 * @brief 		BinomialOptionsPricing Class constructor
 * 
 * @param numSteps 	
 * @param numSamples 
 * 
 * @return 
 */
template <class T> BinomialOptionPricing<T>::BinomialOptionPricing(unsigned int numSteps, unsigned long int numSamples) {
    _numSteps = numSteps;
    _numSamples = numSamples;
    _tolerance = 0.00000001;
    
    //Allocating memory...
    _stockPrice = new T[_numSamples];
    _optionStrike = new T[_numSamples];
    _optionYears = new T[_numSamples];
    _resultCPU = new T[_numSamples];
    _resultGPU = new T[_numSamples];
}

/** 
 * @brief       class Destructor with void arguments.
 * 
 * @return 
 */
template <class T> 
BinomialOptionPricing<T>::~BinomialOptionPricing() {
    // De-allocating memory...
    delete [] _stockPrice;
    delete [] _optionStrike;
    delete [] _optionYears;
    delete [] _resultGPU;
    delete [] _resultCPU;
}

/** 
 * @brief       utility function which returns total number of samples.
 * 
 * @return 
 */
template <class T> 
unsigned long int 
BinomialOptionPricing<T>::getNumSamples(){ 
    return _numSamples;
}

/** 
 * @brief 	class utility function which returns total number of time steps. 
 * 
 * @return 
 */
template <class T> 
unsigned int 
BinomialOptionPricing<T>::getNumSteps(){

    return _numSteps;
}

/** 
 * @brief       class utitility function which retuns the tolerance factor for GPU results.
 * 
 * @return 
 */
template <class T> 
double 
BinomialOptionPricing<T>::getTolerance(){
    return _tolerance;
}

/** 
 * @brief           generates random numbers.
 * 
 * @param randMax   maximum element for the given range.
 * @param randMin   minimum element for the given range.
 * 
 * @return 
 */
template <class T>
T 
BinomialOptionPricing<T>::randomT(const T randMax, const T randMin) {
    T result;
    result =(T)rand()/(T)RAND_MAX;

    return ((1.0f - result) * randMin + result *randMax);
}

/** 
 * @brief           returns the maximum number between two given numbers.
 * 
 * @param param1    the first number.
 * @param param2    the second number.
 * 
 * @return 
 */
template <class T>
T
BinomialOptionPricing<T>::maxT(const T param1, const T param2) {
    if(param1 >= param2)
    {

        return param1;
    }
    else
    {

        return param2;
    }
}

/** 
 * @brief       initializes class member variables.
 * 
 * @return 
 */
template <class T>
void
BinomialOptionPricing<T>::memberInitializer() {
    unsigned int i;
    for(i = 0; i < _numSamples; i++)
    {
        _stockPrice[i] = randomT(30.0, 5.0);
        _optionStrike[i] = randomT(100.0, 1.0);
        _optionYears[i] = randomT(10.0, 0.25);
        _resultCPU[i] = -1.0;
        _resultGPU[i] = -1.0;
    }
}

/** 
 * @brief       cpu implementation of the binomial option pricing.
 * 
 * @return 
 */
template <class T>
void 
BinomialOptionPricing<T>::binomialOptionCPU() {
    static T stepsArray[NUMSTEP + 1];
    unsigned int i = 0;
    
    //Iterate for all samples...
    for(i = 0; i < _numSamples; i++)
    {
        const T s = _stockPrice[i];
        const T x = _optionStrike[i];
        const T t = _optionYears[i];

        const T dt = t * (1.0f / _numSamples);
        const T vsdt = VOLATILITY * sqrtf(dt);
        const T rdt = RISKFREE * dt;

        const T r = expf(rdt);
        const T rInv = 1.0f / r;

        const T u = expf(vsdt);
        const T d = 1.0f / u;
        const T pu = (r - d)/(u - d);
        const T pd = 1.0f - pu;
        const T puByr = pu * rInv;
        const T pdByr = pd * rInv;

        // Compute values at expiration date:
        // Call option value at period end is v(t) = s(t) - x
        // If s(t) is greater than x, or zero otherwise...
        // The computation is similar for put options...
        for(unsigned int j = 0; j <= _numSteps; j++)
        {
            T price = s * expf(vsdt * (2.0f * j - _numSteps));
            stepsArray[j] = maxT(price - x, 0.0f);
        }
        
        //walk backwards up on the binomial tree of depth _numSteps...
        //Reduce the price step by step...
        for(unsigned int j = _numSteps; j > 0; j--)
        {
            for(unsigned int k = 0; k <= j - 1; k++)
            {
                stepsArray[k] = puByr * stepsArray[k+1] + pdByr * stepsArray[k];
            }
        }
        
        //Copy the root to result...
        _resultCPU[i] = stepsArray[0];
    }
}

/** 
 * @brief       invokes corresponding gpu kernel handlers based on number of time steps.
 * 
 * @return 
 */
template <class T>
void
BinomialOptionPricing<T>::binomialOptionGPU(){
    
    //Invoke appropriate gpuKernelHandler according to number of time steps for GPU computation...
    switch(_numSteps)
    {
    case 12: mainGPUHandller12(_stockPrice, _optionStrike, _optionYears, _resultGPU, RISKFREE, VOLATILITY, _numSamples, _numSteps);
        break;
    case 8: mainGPUHandller8(_stockPrice, _optionStrike, _optionYears, _resultGPU, RISKFREE, VOLATILITY, _numSamples, _numSteps);
        break;
    case 4: mainGPUHandller4(_stockPrice, _optionStrike, _optionYears, _resultGPU, RISKFREE, VOLATILITY, _numSamples, _numSteps);
        break;
    default:
        cout<<"\n\nWhoops! Time steps only can on be 4, 8 or 12...\n";
    }
}

/** 
 * @brief       compares between cpu and gpu results for regression test.
 * 
 * @return 
 */
template <class T>
void
BinomialOptionPricing<T>::compareResults()
{
    unsigned int i = 0;
    unsigned int flag = 0;
    
    while(i < _numSamples)
    {
        if(fabs((double)_resultCPU[i] - (double)_resultGPU[i]) <= _tolerance)
        {
            i++;
        }
        else
        {
            _tolerance *= 10;
            i = 0;
            flag++;
            if(flag == 8)
            {
                cout<<"Failed!\n\n";
                break;
            }
        }
    }
    if(flag != 8)
    {
        cout<<"Passed!\n\n";
    }
}
