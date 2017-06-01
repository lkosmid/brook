/****************************************************************************

Copyright (c) 2008, Advanced Micro Devices, Inc.
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
  \file  Main.cpp
  \brief Main for Binomial Options Pricing
 ****************************************************************************/

#include "Timer.h"
#include "common.h"
#include "BinomialOptionPricing.h"
#include <stdio.h>


// Default values for command line flags.
// The values for the default flags are defined in
// common.h
unsigned int DEFAULT_DATATYPE                   = 0;
unsigned int DEFAULT_TIMINGSTEPS                = 4;
unsigned int DEFAULT_NUMSAMPLES                 = 1024;

/**
* @brief      Print usage information
*/
void UsageCustom(char *name)
{
    printf("Binomial (European) Option Pricing\n");
    printf("Volatility = 0.3f and Risk Free Factor = 0.2f\n");
        printf("Inputs to algorithm: Number of time steps (-st), Number of samples (-n)\n\n");
    printf("\tUsage: %s [-h] [-e] [-s] [-n] [-q] [-t]\n", name);
    printf("   -h       Print this help menu.\n");
    printf("   -e   Verify correct output.\n");
    printf("   -p   Compare performance with CPU.\n");
    printf("   -st  Specify number of timing steps (4,8,12, default = 4).\n");
    printf("   -n   Specify the number of samples.\n");
    printf("   -q   Surpress all data output.\n");
    printf("   -t   Print out timing information.\n");
    printf("Example: %s -float 4 8192\n", name);
}


/**
* @brief      Parse command line options
*/
void ParseCommandLineCustom(int argc, char** argv, InfoCustom* info)
{
    int x;

    info->Verify = DEFAULT_VERIFY;
        info->Performance = DEFAULT_PERFORMANCE;
        info->DataType = DEFAULT_DATATYPE;
        info->TimingSteps = DEFAULT_TIMINGSTEPS;
        info->NumSamples = DEFAULT_NUMSAMPLES;
    info->Quiet = DEFAULT_QUIET;
    info->Timing = DEFAULT_TIMING;

    for (x = 1; x < argc; ++x)
    {
        switch (argv[x][1])
        {
        case 'e':
            info->Verify = 1;
            break;
        case 'p':
            info->Performance = 1;
            break;
        case 'q':
            info->Quiet = 1;
            break;
        case 't':
            info->Timing = 1;
            break;
        case 's':
            if(strcmp(argv[x],"-st") == 0)
            {
                    if (++x < argc)
                        {
                                        sscanf(argv[x], "%u", &info->TimingSteps);
                                        if( !(info->TimingSteps == 4 ||
                                                  info->TimingSteps == 8 ||
                                                  info->TimingSteps == 12) )
                                        {
                                                fprintf(stderr, "Error: Timing steps can only be 4, 8 or 12, %s",argv[x-1]);
                                                UsageCustom(argv[0]);
                                                exit(-1);
                                        }
                                }
                                else
                                {
                                        fprintf(stderr, "Error: Invalid argument, %s",argv[x-1]);
                                        UsageCustom(argv[0]);
                                        exit(-1);
                                }
                        }
            break;
        case 'n':
                    if (++x < argc)
                        {
                                        sscanf(argv[x], "%u", &info->NumSamples);
                                }
                                else
                                {
                                        fprintf(stderr, "Error: Invalid argument, %s",argv[x-1]);
                                        UsageCustom(argv[0]);
                                        exit(-1);
                                }
                        break;
        case 'h':
            UsageCustom(argv[0]);
                        exit(0);
            break;
        default:
            fprintf(stderr, "Error: Invalid command line parameter, %c\n", argv[x][1]);
            UsageCustom(argv[0]);
            exit(-1);
        }
    }
}


/**
* @brief      main
*/
int
main(int argc, char **argv)
{
    double gpuElapsedTime;
        InfoCustom cmd;
        unsigned int retval = 0;

        /////////////////////////////////////////////////////////////////////////
    // Setup the timers
        // 0 = GPU Total Time
        // 1 = CPU Total Time
        /////////////////////////////////////////////////////////////////////////
    Setup(0);
        Setup(1);

        ParseCommandLineCustom(argc, argv, &cmd);

    // Adjust sample size to avoid address translation
    unsigned int temp = (unsigned int)sqrt((float)cmd.NumSamples);
    temp = temp * temp * 4;
    printf("Adjusting sample size to avoid address translation\n");
    printf("Adjusted Sample Size = \t%u\n", temp);

    //Create an object of class BinomialOptionPricing
        BinomialOptionPricing <float>bop(cmd.TimingSteps, temp);

        /////////////////////////////////////////////////////////////////////////
        // Initialize _stockPrice, _optionStrike, _optionYears with a valid
        // set of random values
        /////////////////////////////////////////////////////////////////////////
    bop.memberInitializer();

        /////////////////////////////////////////////////////////////////////////
    // GPU implementation of Binomial Option Pricing
        /////////////////////////////////////////////////////////////////////////
    //Record GPU Total time
    Start(0);
    bop.binomialOptionGPU();
    Stop(0);
        gpuElapsedTime = GetElapsedTime(0);

        /////////////////////////////////////////////////////////////////////////
        // Print results if requested
        /////////////////////////////////////////////////////////////////////////
        if( !cmd.Quiet )
        {
                printf("\n");
        printf("Volatality Factor:\t\t%3.2f\n", VOLATILITY);
        printf("Riskfree Factor:\t\t%3.2f\n", RISKFREE);
                printf( "%-16s%-16s%-16s%-16s%-16s\n","Sample Size","Time Steps",
                                "GPU Total Time","Options/Sec", "Tolerance");
                printf( "%-16d%-16d%-16lf%-16lf%-16lf", bop.getNumSamples(), bop.getNumSteps(), gpuElapsedTime,
                                double(bop.getNumSamples())/(gpuElapsedTime), bop.getTolerance());
                printf("\n\n");
        }

        /////////////////////////////////////////////////////////////////////////
        // Verify against CPU implementation if requested
        /////////////////////////////////////////////////////////////////////////
        if( cmd.Verify )
        {
                printf("-e Verify correct output.\n");
                printf("Performing Binonmial Option Pricing on CPU ... ");

                //Invoke CPU implementation of Binomial Option Pricing
                bop.binomialOptionCPU();

                printf("Done\n");

                bop.compareResults();
        }

        /////////////////////////////////////////////////////////////////////////
        // Compare performance with CPU if requested
        /////////////////////////////////////////////////////////////////////////
    if (cmd.Performance)
    {
                double cpu_time = 0.0;

                printf("-p Compare performance with CPU.\n");

                // Record CPU Total Time
                Start(1);
                //Invoke CPU implementation of Binomial Option Pricing
                bop.binomialOptionCPU();
                Stop(1);

                // Print CPU timing and speedup
                cpu_time = GetElapsedTime(1);
                printf("Volatality Factor:\t\t%3.2f\n", VOLATILITY);
        printf("Riskfree Factor:\t\t%3.2f\n", RISKFREE);
                printf( "%-16s%-16s%-16s%-16s%-16s%-16s%-16s%-16s\n","Sample Size","Time Steps",
                                "GPU Time","GPU Options/S", "CPU Time", "CPU Options/S", "Speedup", "Tolerance");
                printf( "%-16d%-16d%-16lf%-16lf%-16lf%-16lf%-16lf%-16lf", bop.getNumSamples(), bop.getNumSteps(),
                                gpuElapsedTime, double(bop.getNumSamples())/(gpuElapsedTime), cpu_time,
                                double(bop.getNumSamples())/(cpu_time), cpu_time/gpuElapsedTime, bop.getTolerance());
                printf("\n\n");
        }

    if (!cmd.Verify)
    {
        printf("\nPress enter to exit...\n");
        getchar();
    }
        return retval;
}
