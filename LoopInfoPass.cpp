#include "llvm/ADT/Statistic.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Analysis/LoopInfo.h>
#include <vector>
#include <string>

#include <iostream>

using namespace llvm;
using namespace std;

namespace {
  class FunctionInfo : public FunctionPass {
  public:
    static char ID;
    FunctionInfo() : FunctionPass(ID) { }
    ~FunctionInfo() { }

    int global_loop_id = 0;
    map<int,map<Loop*,int>> loop_levels;
    
    // We don't modify the program, so we preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
      AU.addRequired<LoopInfoWrapperPass>();
    }

    // Do some initialization
    bool doInitialization(Module &M) override {
      errs() << "CSE521 Loop Information Pass\n"; // TODO: remove this.
      // outs() << "Name,\tArgs,\tCalls,\tBlocks,\tInsns\n";

      return false;
    }

    //find if a loop has a sub loop
    string hasSubLoops(Loop* l){
      if(l->getSubLoops().size()){
        return "true";
      }else{
        return "false";
      }
    }

    //find basic blocks(top-level)
    int getTopLevelBasicBlocksCount(Loop* l){
      int bb_count = 0;
      for(Loop::block_iterator bi = l->block_begin(); bi!=l->block_end(); bi++){
        bb_count++;
      }
      for(Loop* sl: l->getSubLoops()){
        
        for(Loop::block_iterator sbi = sl->block_begin(); sbi!=sl->block_end(); sbi++){
          bb_count--;
        }
      }
      return bb_count;
    }

    //find all instructions count
    int getInstructionsCount(Loop* l){
      int instr_count = 0;
      for(Loop::block_iterator bi = l->block_begin(); bi!=l->block_end(); bi++){
        BasicBlock *bb = *bi;
        for(Instruction &i: *bb){
          instr_count++;
        }
      }
      return instr_count;
    }

    //find all atomic instructions count
    int getAtomicOperationsCount(Loop *l){
      int atomic_instr_count = 0;
      for(Loop::block_iterator bi = l->block_begin(); bi!=l->block_end(); bi++){
        BasicBlock *bb = *bi;
        for(Instruction &i: *bb){
          if(i.isAtomic())
	    atomic_instr_count++;
        }
      }
      return atomic_instr_count;
    }

    //find all top level branch instructions count
    int getTopLevelBranchInstructionsCount(Loop *l){
      int top_branch_instr_count = 0;
      for(Loop::block_iterator bi = l->block_begin(); bi!=l->block_end(); bi++){
        BasicBlock *bb = *bi;
        for(Instruction &i: *bb){
          if(BranchInst *bri = dyn_cast<BranchInst>(&i))
	    top_branch_instr_count++;
        }
      }
      for(Loop* sl: l->getSubLoops()){
        
        for(Loop::block_iterator sbi = sl->block_begin(); sbi!=sl->block_end(); sbi++){
          BasicBlock *bb = *sbi;
          for(Instruction &i: *bb){
            if(BranchInst *bri = dyn_cast<BranchInst>(&i))
	      top_branch_instr_count--;
          }
        }
      }
      return top_branch_instr_count;
    }

    //find depth
    void findDepth(Loop* l, int d){
      vector<Loop*> children = l->getSubLoops();     
      for(vector<Loop*>::iterator sl=children.begin(); sl!=children.end(); sl++){
        Loop *lp = *sl;
	errs()<<lp<<"\n";
	map<Loop*, int> temp = {{lp,d+1}};
	loop_levels[global_loop_id++] = temp;
        findDepth(lp, d+1);
      }
      
    }
    // Print output for each function
    bool runOnFunction(Function &F) override {
      int initialIdx = global_loop_id;
      LoopInfo& loops = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
      for(LoopInfo::iterator it = loops.begin(), e=loops.end(); it != e; ++it){
        Loop *l = *it;
	map<Loop*, int> temp = {{l,0}};
	loop_levels[global_loop_id++] = temp;
	findDepth(l,0);
      }

      for(int i = initialIdx; i<global_loop_id; i++){
      	map<Loop*,int> loopDetails = loop_levels[i];
	for (auto itr = loopDetails.begin(); itr != loopDetails.end(); ++itr) { 
          errs()<<"<"<<i<<">: func="<<F.getName()
<<", depth="<<itr->second
<<", subLoops="<<hasSubLoops(itr->first)
<<", BBs="<<getTopLevelBasicBlocksCount(itr->first)
<<", instrs="<<getInstructionsCount(itr->first)
<<", atomics="<<getAtomicOperationsCount(itr->first)
<<", branches="<<getTopLevelBranchInstructionsCount(itr->first)<<"\n";
    	} 
	
      }
      
      return false;
    }
  };
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char FunctionInfo::ID = 0;
static RegisterPass<FunctionInfo> X("function-info", "CSE521: Function Information", false, false);