#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"

#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>
#include "llvm/IR/ModuleSlotTracker.h"
#include <string>

#include <unordered_map>

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "ValueNumbering"

using namespace llvm;

namespace {

    int curid = 1;// current variable alias(id)
    int expid = 1;// current expression alias(id)
    string exp = "";// set up expressions

    unordered_map< string, int > valueTable;// store expression-number
    map< Value*, int > valueName;// store value-id

    char ops[5] = "+-*/";// table for operations name
    map< unsigned int, int > opmap = { {16,2}, {14,1}, {12,0}, {19,3}, {18,3} };// map for Op_Code-index in ops

struct ValueNumbering : public FunctionPass {

    string func_name = "test";
    static char ID;
    ValueNumbering() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override
    {
        if (F.getName() != func_name) return false;

        string outName;

        outName = F.getParent()->getSourceFileName();
        outName[ outName.length() - 1 ] = 'o';
        outName += "ut";

        std::error_code EC;
        ToolOutputFile outputFile( outName, EC, sys::fs::F_None );

        for (auto& basic_block : F)
        {
            for (auto& inst : basic_block)
            {
                errs() << inst << "\n";
                errs() << "Op Code:" << inst.getOpcodeName()<<"\n"; // print opcode name
                if (inst.isBinaryOp())
                {
                    User* usr = dyn_cast<User>( &inst );
                    Value* a = inst.getOperand(0);
                    Value* b = inst.getOperand(1);
                    Value* c = dyn_cast<Value>(&inst);

                    map< Value*, int >::iterator it;
                    it = valueName.find( a );

                    if ( it == valueName.end() )
                    {
                        valueName[ a ] = curid;
                        curid++;
                    }
                    it = valueName.find( b );
                    if ( it == valueName.end() )
                    {
                        valueName[ b ] = curid;
                        curid++;
                    }
                    it = valueName.find( c );
                    if ( it == valueName.end() )
                    {
                        valueName[ c ] = curid;
                        curid++;
                    }

                    int a_id = valueName[ a ];
                    int b_id = valueName[ b ];
                    int c_id = valueName[ c ];

                    unordered_map< string, int >::iterator it2;
                    it2 = valueTable.find(  to_string(a_id) );
                    if ( it2 == valueTable.end() )
                    {
                        valueTable[ to_string(a_id) ] = expid;
                        expid++;
                    }

                    it2 = valueTable.find(  to_string(b_id) );
                    if ( it2 == valueTable.end() )
                    {
                        valueTable[ to_string(b_id) ] = expid;
                        expid++;
                    }

                    a_id = valueTable[ to_string(a_id) ];
                    b_id = valueTable[ to_string(b_id) ];

                    if ( a_id < b_id )
                        exp = to_string(a_id) + " " + to_string( inst.getOpcode() ) + " " + to_string(b_id);
                    else
                        exp = to_string(b_id) + " " + to_string( inst.getOpcode() ) + " " + to_string(a_id);

                    it2 = valueTable.find( exp );
                    if ( it2 == valueTable.end() )
                    {
                        valueTable[ exp ] = expid;
                        expid++;
                    }

                    valueTable[ to_string(c_id) ] = valueTable[ exp ];

                    exp = to_string( valueTable[ to_string(c_id) ] ) + "=" + to_string(a_id) + ops[ opmap[ inst.getOpcode() ] ] + to_string(b_id);
                    outputFile.os() << exp << "\n";
                    outputFile.keep();
                 }
             }
        }
        return false;
    }
}; // end of struct ValueNumbering
}  // end of anonymous namespace

char ValueNumbering::ID = 0;
static RegisterPass<ValueNumbering> X("ValueNumbering", "ValueNumbering Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);