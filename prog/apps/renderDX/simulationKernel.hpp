// simulationKernel.hpp
#pragma once

void simulationKernel(
  const ::brook::stream& input,
  const ::brook::stream& inputs,
  const float4 control,
  ::brook::stream& result );

void smoothKernel(
  const ::brook::stream& input,
  const ::brook::stream& inputs,
  ::brook::stream& result );

void normalGenerationKernel(
  const ::brook::stream& inputs,
  ::brook::stream& result );
