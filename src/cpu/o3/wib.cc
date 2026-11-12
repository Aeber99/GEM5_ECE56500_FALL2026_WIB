//load returns from mem
void wib::freeBitVector(InstSeqNum load_num){
    for (unsigned c = 0; c < numCols; c++) {
        if (loadValid[c] && loadNum[c] == load_num) {
            loadValid[c] = 0;
            return;
        }
    }
}

int wib::allocateBitVector(InstSeqNum load_num, ThreadID tid){
    //find free bit vector

    
    

    //no free vector
    return -1;
}





// #include "cpu/o3/wib.hh"

// #include "cpu/o3/cpu.hh"

// #include "base/logging.hh" 

// namespace gem5
// {

// namespace o3
// {

// wib::wib(CPU *_cpu, const BaseO3CPUParams &params)
//     : Named(_cpu->name() + ".wib"),
//       cpu(_cpu),
//       wibSize(params.wibSize),
//       numBanks(params.wibNumBanks),
//       numCols(params.wibNumColumns),
//       entriesPerBank(params.wibSize / params.wibNumBanks),
//       nextBank(0),
//       counter(0),
//       oddCycle(false)
// {
//     fatal_if(wibSize != params.numROBEntries, "wib size must match ROB");
//     fatal_if(wibSize % numBanks != 0, "wib size must divide by banks");

//     banks.resize(numBanks);
//     for (unsigned b = 0; b < numBanks; b++) {
//         banks[b].entries.resize(entriesPerBank);
//     }

//     bitvectors.resize(numCols);
//     for (unsigned c = 0; c < numCols; c++) {
//         bitvectors[c].bits.resize(wibSize, false);
//     }

//     lastserved.resize(numBanks, 0);
// }

// //inst space based on ROB
// void wib::allocate_frame(const DynInstPtr &inst){
//     if (inst == NULL) { return; }

//     //round robin
//     unsigned b = nextBank;
//     wibbank &bank = banks[b];

//     //bank full, dispatch stall
//     if (bank.count >= entriesPerBank) { return; }

//     //put inst at tail
//     bank.entries[bank.tail].inst = inst;
//     bank.entries[bank.tail].bank = b;
//     bank.entries[bank.tail].valid = 0; //wait on load
//     rownum[inst->seqNum] = bank.tail * numBanks + b;

//     bank.tail++;
//     if (bank.tail >= entriesPerBank) { bank.tail = 0; }
//     bank.count++;

//     //to next bank
//     nextBank++;
//     if (nextBank >= numBanks) { nextBank = 0; }
// }

// //load misses, create bit vector
// int wib::allocateBitVector(InstSeqNum load_num, ThreadID tid){
//     //find free bit vector

//     for (unsigned c = 0; c < numCols; c++) {
//         bitvector &vec = bitvectors[c];
//         if (vec.valid) { continue; } //taken by another load

//         vec.valid = true;
//         vec.done = false;
//         vec.load_n = load_num;
//         vec.tid = tid;
//         vec.count = 0;

//         //clear all bits
//         for (unsigned row = 0; row < wibSize; row++) {
//             vec.bits[row] = false;
//         }

//         return c;
//     }

//     //no free vector
//     return -1;
// }

// //send frame on a miss
// void wib::load_frame_in(const DynInstPtr &inst, unsigned col){
//     //no frame
//     if (rownum.count(inst->seqNum) == 0) { return; }

//     bitvector &vec = bitvectors[col];
//     if (!vec.valid) { return; }

//     unsigned row = rownum[inst->seqNum];
//     unsigned bank = row % numBanks;
//     unsigned frame = row / numBanks;

//     //now waiting
//     banks[bank].entries[frame].valid = 1;

//     if (!vec.bits[row]) { //set dependencies for bit vector ? will need to know dependencies, probably from regs
//         vec.bits[row] = 1;
//         vec.count++;
//     }
// }

// //load returns from mem
// void wib::freeLoad(InstSeqNum load_num) {
//     for (unsigned i = 0; i < numCols; i++) {
//         bitvector &vec = bitvectors[i];

//         if (vec.valid && vec.load_n == load_num) {//load came back
//             vec.done = true;
//             if (vec.count == 0) { vec.valid = false; } // if no one dependent on the load
//             return;
//         }
//     }

// }

// //even-odd banks stuff
// //bank that went longest ago goes first
// //no space in IQ -> stamp stays -> stays highest priority
// wiboutput wib::send_frame(unsigned iq_free){
//     wiboutput out;

//     unsigned odd = oddCycle ? 1 : 0;
//     oddCycle = !oddCycle;

//     bool loadback = 0;
//     for (unsigned i = 0; i < numCols; i++) {
//         if (bitvectors[i].valid && bitvectors[i].done) {
//             loadback = 1;
//             break;
//         }
//     }
//     if (!loadback) { return out; }

//     //got load back
//     for (unsigned n = 0; n < numBanks / 2; n++) {
//         //IQ full
//         if (out.insts.size() >= iq_free) {
//             break;
//         }

//         //priority pick
//         int bank = -1;
//         for (unsigned b = odd; b < numBanks; b += 2) {
//             if (bank < 0 || lastserved[b] < lastserved[bank]) {
//                 bank = b;
//             }
//         }

//         //oldest frame w load back
//         int frame = -1;
//         int col = -1;
//         unsigned s = banks[bank].head;

//         for (unsigned k = 0; k < banks[bank].count; k++) {
//             if (banks[bank].entries[s].valid) {
//                 unsigned row = s * numBanks + bank;
//                 for (unsigned c = 0; c < numCols; c++) {
//                     if (bitvectors[c].valid && bitvectors[c].done && bitvectors[c].bits[row]) {
//                         frame = s;//found itttt
//                         col = c;
//                         break;
//                     }
//                 }
//             }
//             if (frame >= 0) {break;}
//             s++;
//             if (s >= banks[bank].entries.size()) {s = 0;}
//         }

//         //nothing to send
//         if (frame < 0) {
//             lastserved[bank] = ++counter;
//             continue;
//         }

//         //send it
//         out.insts.push_back(banks[bank].entries[frame].inst);
//         banks[bank].entries[frame].valid = 0;

//         //clear bit
//         unsigned row = frame * numBanks + bank;
//         bitvectors[col].bits[row] = 0;
//         bitvectors[col].count--;
//         if (bitvectors[col].count == 0) { bitvectors[col].valid = 0; } //column free

//         //lowest
//         lastserved[bank] = ++counter;
//     }

//     return out;
// }


// void wib::ROB_exit(InstSeqNum commited)
// {
//     unsigned row = rownum[commited];
//     unsigned b = row % numBanks;
//     unsigned frame = row / numBanks;

//     wibbank &bank = banks[b];

//     if (frame != bank.head) { return; } //just oldest in bank since its in order

//     bank.entries[frame].inst = NULL;
//     bank.entries[frame].valid = 0;
//     rownum.erase(commited);

//     bank.head++;
//     if (bank.head == entriesPerBank) { bank.head = 0; }
//     bank.count--;
// }


// void wib::squash(InstSeqNum squashed_sn) //from IEW
// {
//     //in each bank if seqnum > mispredicted, remove the inst
//     for (unsigned b = 0; b < numBanks; b++) {
//         wibbank &bank = banks[b];

//         while (bank.count > 0) {
//             //last used slot one before tail
//             unsigned slot;
//             if (bank.tail == 0) { slot = entriesPerBank - 1; }
//             else { slot = bank.tail - 1; }

//             wibentry &entry = bank.entries[slot];
//             if (entry.inst->seqNum <= squashed_sn) { break; } //older, keep

//             //clear
//             if (bitvectors[c].bits[row]) {
//                 bitvectors[c].bits[row] = 0;
//                 bitvectors[c].count--;
//                 if (bitvectors[c].done && bitvectors[c].count == 0) { 
//                     bitvectors[c].valid = 0; }
//             }

//             //free slot, move tail back
//             rownum.erase(entry.inst->seqNum);
//             entry.inst = NULL;
//             entry.valid = false;
//             bank.tail = slot;
//             bank.count--;

//             //update for after squash
//             if (nextBank == 0) { nextBank = numBanks - 1; }
//             else { nextBank--; }
//         }
//     }
// }

// }
// }
