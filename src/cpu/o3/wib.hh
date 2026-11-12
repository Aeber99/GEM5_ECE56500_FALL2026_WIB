#ifndef __CPU_O3_WIB_HH__
#define __CPU_O3_WIB_HH__ //by path

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "base/named.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/o3/dyn_inst_ptr.hh"
#include "params/BaseO3CPU.hh"

namespace gem5 {
namespace o3 {

class CPU;

struct wibentry {//frame
    DynInstPtr inst;
    unsigned bank;
    bool valid = false;
};

struct bitvector {//column of load miss
    std::vector<bool> bits;// one bit per WIB row
    InstSeqNum load_n = 0; //number of that load for tracking
    ThreadID tid = -1;
    bool valid = false;//allocated
    bool done = false; //load came back
    unsigned count = 0;
};

struct wibbank { //one bank
    std::vector<wibentry> entries;
    unsigned head = 0;
    unsigned tail = 0;
    unsigned count = 0;// how many used
};

struct wiboutput { //to IQ
    std::vector<DynInstPtr> insts;
};


class wib : public Named
{
  private:
    CPU *cpu;

    unsigned wibSize;// max entries
    unsigned numBanks;
    unsigned numCols;//bit vectors
    unsigned entriesPerBank;

    unsigned nextBank;// round robin
    uint64_t counter; // priority
    bool oddCycle;

    std::vector<wibbank> banks;
    std::vector<bitvector> bitvectors;
    std::vector<uint64_t> lastserved;// one per bank
    std::unordered_map<InstSeqNum, unsigned> rownum; 

  public:
    wib(CPU *_cpu, const BaseO3CPUParams &params);

    void allocate_frame(const DynInstPtr &inst); //allocate with new ROB entry 
    int allocateBitVector(InstSeqNum load_num, ThreadID tid); //load misses, allocate vector, !!! if it returns -1, vectors are full
    void load_frame_in(const DynInstPtr &inst, unsigned col); //dependent on load miss, put into WIB
    void freeLoad(InstSeqNum load_num); //load returns, column done
    wiboutput send_frame(unsigned iq_free); //send back to IQ, happens every cycle
    void ROB_exit(InstSeqNum commited);// freem frame once rob entry is commited
    void squash(InstSeqNum squashed_sn); 
};

}}
#endif
