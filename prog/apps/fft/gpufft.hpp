void computeFFT2d(::brook::stream *input,
             ::brook::stream *output,
             int logN,
             int logM,
             int N,
             int M,
             ::brook::stream plan,
             int do_backwards, int invert);
void generatePlan(::brook::stream plan,
                  int logN,
                  int logM);
void allocatePlan(::brook::stream *plan,
                  int logN,
                  int logM);
