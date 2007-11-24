# GenBenchmark
# Generates 256 MULACC's
# (C) 2007 Niall Douglas
# File created: 23rd Nov 2007

fh=file('benchmark.br', 'wt')
fh.write('kernel void benchmark(out float4 out1<>, float4 in1<>, float4 in2<>)\n')
fh.write('{\n')
fh.write('  out1=float4(0.0f,0.0f,0.0f,0.0f);\n')
for n in range(0, 256):
    fh.write('  out1+=in1*in2;\n')
fh.write('}\n')
fh.close()
