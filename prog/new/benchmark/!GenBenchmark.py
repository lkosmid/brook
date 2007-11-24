# GenBenchmark
# Generates 256 MULACC's
# (C) 2007 Niall Douglas
# File created: 23rd Nov 2007

fh=file('benchmark.br', 'wt')
fh.write('kernel void benchmark(out float4 output<>, float4 input<>)\n')
fh.write('{\n')
fh.write('  output=float4(0.0f,0.0f,0.0f,0.0f);\n')
for n in range(0, 256):
    fh.write('  output+=input*output;\n')
fh.write('}\n')
fh.close()
