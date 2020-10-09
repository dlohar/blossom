#include <string.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <math.h>
#include <vector>
#include <algorithm>
#include <locale>
#include <cxxabi.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constant.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Demangle/Demangle.h"

using namespace llvm;
using namespace std;

/*These are the command line flags for the pass, adding these flags allows the user to select
the time the program should be run for, the seed, the configuration file and optionally the programming
language of the input program.*/
cl::opt<unsigned int> seed_arg("seed", cl::desc("Specify random seed"), cl::value_desc("seed"));
cl::opt<int> time_arg("time", cl::desc("Specify time to run testing"), cl::value_desc("time(m)"));
cl::opt<int> mutation_arg("mutations", cl::desc("Specify number of mutations"), cl::value_desc("#mutations"));
cl::opt<string> file_arg("config", cl::desc("Specify config file"), cl::value_desc("filename"));
cl::opt<string> lang_arg("lang", cl::desc("Specify programming language"), cl::value_desc("language"));
map<string,string> properties;
Value* changeTracker;//keeps track if any ranges change so it can be decided whether to mutate variables
Value* infTracker;

/*Returns if the function is a kernel function from its name*/
bool isValid(string name) {
	return regex_match(name, regex("numerical_kernel|numerical_kernel([1-9]([0-9])*)"));
}

/*This struct is used for passing data from the function loop to printing the program run information*/
struct ARGUMENT_DATA {
	Constant* MAX_VAR;
	Constant* MIN_VAR;
	Constant* NAME;
	Constant* ARG;
	Constant* INDEX;
	bool IS_FP;
};

namespace {
/*This function takes a variable name and whether that variable is tracking a maximum
or minimum value and creates the corresponding floating point global variable in the 
program and returns the global variables value as a constant*/
Constant* CreateFloatingPointTracker(Module &M, StringRef GlobalVarName, bool max_min) {
	auto &CTX = M.getContext();
	Constant *NewGlobalVar = M.getOrInsertGlobal(GlobalVarName, Type::getX86_FP80Ty(CTX));
	GlobalVariable *NewGV = M.getNamedGlobal(GlobalVarName);
	NewGV->setLinkage(GlobalValue::InternalLinkage);//allow initialization to non zero
	NewGV->setAlignment(MaybeAlign(16));
	NewGV->setInitializer(llvm::ConstantFP::get(Type::getX86_FP80Ty(CTX), max_min ? -1.0/0.0 : 1.0/0.0));
	return NewGlobalVar;
}

/*This function takes a variable name and whether that variable is tracking a maximum
or minimum value and creates the corresponding integer type global variable of the specified 
length in the program and returns the global variables value as a constant*/
Constant* CreateIntegerTracker(Module &M, StringRef GlobalVarName, bool max_min, unsigned int width) {
	auto &CTX = M.getContext();
	Constant *NewGlobalVar = M.getOrInsertGlobal(GlobalVarName, Type::getIntNTy(CTX, width));
	GlobalVariable *NewGV = M.getNamedGlobal(GlobalVarName);
	NewGV->setLinkage(GlobalValue::InternalLinkage);//allow initialization to non zero
	NewGV->setAlignment(MaybeAlign(width));
	NewGV->setInitializer(llvm::ConstantInt::get(CTX,  max_min ? APInt::getSignedMinValue(width) : APInt::getSignedMaxValue(width)));
	return NewGlobalVar;
}

/*This struct represents the actual pass itself*/
struct GuidedBlackBoxTester : public ModulePass {
	static char ID; // Pass identification, replacement for typeid

	GuidedBlackBoxTester() : ModulePass(ID) {}

	/*This function takes the current program argument and the IRBuilder of the current function as well as relevant other information and inserts code
	to track the minimum and maximum value of the argument by creating two global variables and the approriate if statements to maintain the min and
	max variables*/
	void injectIntCode(Value* compareValue, string funcName, int argNum, string index, Module &M, vector<ARGUMENT_DATA *> &CallCounterMap, Instruction* splitInstruction, IRBuilder<> &Builder){
		auto &CTX = M.getContext();
		unsigned int width = compareValue->getType()->getIntegerBitWidth();
		string combName = funcName + "$$$" + std::to_string(argNum) + "$$$" + index;
		//Create max tracker
		std::string MaxCounterName = "@MaxCounterFor_" + combName;
		Constant *MaxVar = CreateIntegerTracker(M, MaxCounterName, true, width);
		//Create min tracker
		std::string MinCounterName = "@MinCounterFor_" + combName;
		Constant *MinVar = CreateIntegerTracker(M, MinCounterName, false, width);
		
		//Add new data to vector, so all appropriate information can be printed at the end
		CallCounterMap.push_back(new ARGUMENT_DATA{MaxVar, MinVar, Builder.CreateGlobalStringPtr(funcName), ConstantInt::get(CTX, APInt(32, argNum+1)),
			Builder.CreateGlobalStringPtr(index.size()==0 ? "0" : index), false});

		//Load old max value
		LoadInst *oldValue = Builder.CreateLoad(MaxVar);
		//If new value is greater than old max value update global variable
		Value* compare = Builder.CreateICmp(CmpInst::Predicate::ICMP_SGT,compareValue,oldValue);
		Instruction* ifMaxStatement = SplitBlockAndInsertIfThen(compare, splitInstruction, false, 0, 0, 0, 0);

		IRBuilder<> maxIncreaseBuilder(ifMaxStatement);
		maxIncreaseBuilder.CreateStore(ConstantInt::get(Type::getInt8Ty(M.getContext()), 1), changeTracker);
		maxIncreaseBuilder.CreateStore(compareValue, MaxVar);

		//Same as above but for min value
		IRBuilder<> tempBuilder(splitInstruction->getParent(), splitInstruction->getParent()->getFirstInsertionPt());
		oldValue = tempBuilder.CreateLoad(MinVar);
		compare = tempBuilder.CreateICmp(CmpInst::Predicate::ICMP_SLT,compareValue,oldValue);
		Instruction* ifMinStatement = SplitBlockAndInsertIfThen(compare, splitInstruction, false, 0, 0, 0, 0);

		IRBuilder<> minIncreaseBuilder(ifMinStatement);
		minIncreaseBuilder.CreateStore(ConstantInt::get(Type::getInt8Ty(M.getContext()), 1), changeTracker);
		minIncreaseBuilder.CreateStore(compareValue, MinVar);
	}

	/*This function is the same as the previous one except it is for floating point types instead*/
	void injectFloatCode(Value* compareValue, string funcName, int argNum, string index, Module &M, vector<ARGUMENT_DATA *> &CallCounterMap, Instruction* splitInstruction,
		IRBuilder<> &Builder){
		auto &CTX = M.getContext();
		Value* cmpInf = Builder.CreateFCmpOEQ(compareValue, ConstantFP::getInfinity(compareValue->getType(), true));
		cmpInf = Builder.CreateOr(cmpInf, Builder.CreateFCmpOEQ(compareValue, ConstantFP::getInfinity(compareValue->getType(), false)));
		Builder.CreateStore(Builder.CreateOr(Builder.CreateLoad(infTracker), cmpInf), infTracker);

		Value* promotedType = Builder.CreateFPCast(compareValue, Type::getX86_FP80Ty(CTX));
		string combName = funcName + "$$$" + std::to_string(argNum) + "$$$" + index;
		std::string MaxCounterName = "@MaxCounterFor_" + combName;
		Constant *MaxVar = CreateFloatingPointTracker(M, MaxCounterName, true);
		std::string MinCounterName = "@MinCounterFor_" + combName;
		Constant *MinVar = CreateFloatingPointTracker(M, MinCounterName, false);
		CallCounterMap.push_back(new ARGUMENT_DATA{MaxVar, MinVar, Builder.CreateGlobalStringPtr(funcName), ConstantInt::get(CTX, APInt(32, argNum+1)),
			Builder.CreateGlobalStringPtr(index.size() == 0 ? "0" : index), true});
		LoadInst *oldValue = Builder.CreateLoad(MaxVar);
		Value* compare = Builder.CreateFCmp(CmpInst::Predicate::FCMP_OGT,promotedType,oldValue);
		Instruction* ifMaxStatement = SplitBlockAndInsertIfThen(compare, splitInstruction, false, 0, 0, 0, 0);

		IRBuilder<> maxIncreaseBuilder(ifMaxStatement);
		maxIncreaseBuilder.CreateStore(ConstantInt::get(Type::getInt8Ty(M.getContext()), 1), changeTracker);
		maxIncreaseBuilder.CreateStore(promotedType, MaxVar);

		IRBuilder<> tempBuilder(splitInstruction->getParent(), splitInstruction->getParent()->getFirstInsertionPt());
		oldValue = tempBuilder.CreateLoad(MinVar);
		compare = tempBuilder.CreateFCmp(CmpInst::Predicate::FCMP_OLT,promotedType,oldValue);
		Instruction* ifMinStatement = SplitBlockAndInsertIfThen(compare, splitInstruction, false, 0, 0, 0, 0);

		IRBuilder<> minIncreaseBuilder(ifMinStatement);
		minIncreaseBuilder.CreateStore(ConstantInt::get(Type::getInt8Ty(M.getContext()), 1), changeTracker);
		minIncreaseBuilder.CreateStore(promotedType, MinVar);
	}

	/*
		This function recursively takes each argument from a function and creates a min and max value for each argument.  If the argument is a 
		composite type such as an array or pointer the process is instead done recursively to store the min and max of each individual array element
		with the index representing the argument_index where index is the position of the sub element in the element.
	*/
	bool processTypeIf(Value* compareValue, string funcName, int argNum, string index, Module &M, vector<ARGUMENT_DATA *> &CallCounterMap, Instruction* splitInstruction, IRBuilder<> &Builder){

		if(compareValue->getType()->isFloatingPointTy()) {
			injectFloatCode(compareValue, funcName, argNum, index, M, CallCounterMap, splitInstruction, Builder);
			return true;
		}
		else if(compareValue->getType()->isIntegerTy()) {
			injectIntCode(compareValue, funcName, argNum, index, M, CallCounterMap, splitInstruction, Builder);
			return true;
		}
		else if(compareValue->getType()->isPointerTy()){
			string constant_name = funcName+"_"+to_string(argNum+1)+"_"+(index.size()>0 ? index+"_" : "")+"size";
			unsigned int length;

			try {
				string value = properties.at(constant_name);
				length = stoi(value);
			}
			catch (const std::out_of_range& oor) {
				length = 0;
  			}

			if(compareValue->getType()->getPointerElementType()->isArrayTy()){
				if(compareValue->getType()->getPointerElementType()->getArrayNumElements() == 0 && lang_arg.compare("rust") == 0){
					Value* transform = Builder.CreateBitCast(compareValue, PointerType::getUnqual(Type::getDoubleTy(M.getContext())));
					compareValue = transform;
				}
			}

			for(unsigned int j = 0; j < length; j++) {
				std::vector<Value*> index_vector;
				index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), j));
				auto arrayedValue = Builder.CreateGEP(compareValue, index_vector, "tmp");
				processTypeIf(Builder.CreateLoad(arrayedValue), funcName, argNum, index+(index.size()>0 ? "_" : "")+std::to_string(j), M, CallCounterMap, splitInstruction, Builder);
			}
		}
		else if(compareValue->getType()->isArrayTy()){
			unsigned int arrayLength = compareValue->getType()->getArrayNumElements();

			for(unsigned int j = 0; j < arrayLength; j++) {
				processTypeIf(Builder.CreateExtractValue(compareValue, j), funcName, argNum, index+(index.size()>0 ? "_" : "")+std::to_string(j), M, CallCounterMap, splitInstruction, Builder);
			}
		}

		return false;
	}

	/*This function takes a type and a min and max value and returns a random floating point value between
	the two values
	*/
	Value* getRandomFloatingValue(Module &M, IRBuilder<> &Builder, Type* type, Value* minVal, Value* maxVal) {
		auto &CTX = M.getContext();
		long max = 0x7FFF;
		Value* randStart = getRandomIntegralValue(M, Builder, Type::getInt32Ty(CTX), 0, max);
		Value* promotedRandom = Builder.CreateUIToFP(randStart, Type::getX86_FP80Ty(CTX));
		Value* promotedDenominator = Builder.CreateUIToFP(llvm::ConstantInt::get(Type::getInt32Ty(CTX), max), Type::getX86_FP80Ty(CTX));
		Value* quotient = Builder.CreateFDiv(promotedRandom, promotedDenominator);
		Value* minPromoted = Builder.CreateFPExt(minVal, Type::getX86_FP80Ty(CTX));
		Value* maxPromoted = Builder.CreateFPExt(maxVal, Type::getX86_FP80Ty(CTX));
		Value* range = Builder.CreateFSub(maxPromoted, minPromoted);
		Value* scaled = Builder.CreateFMul(range, quotient);
		scaled = Builder.CreateFAdd(scaled, minPromoted);
		return Builder.CreateFPTrunc(scaled, type);
	}

	//This function returns a random integral value within a range ensuring that random values can occupy all 64 bits instead of rand_max
	//by using repeated uses of the rand function and shifting bits
	Value* getRandomIntegralValue(Module &M, IRBuilder<> &Builder, Type* type, int64_t minVal, int64_t maxVal) {
		auto &CTX = M.getContext();
		FunctionType *RandTy = FunctionType::get(IntegerType::getInt32Ty(CTX), false);
		FunctionCallee Rand = M.getOrInsertFunction("rand", RandTy);
		Value* randNumber = Builder.CreateCall(Rand,{});
		Value* extended = Builder.CreateSExtOrBitCast(randNumber, Type::getInt64Ty(CTX));
		extended = Builder.CreateShl(extended, 15);
		extended = Builder.CreateXor(extended, Builder.CreateSExtOrBitCast(Builder.CreateCall(Rand,{}), Type::getInt64Ty(CTX)));
		extended = Builder.CreateShl(extended, 15);
		extended = Builder.CreateXor(extended, Builder.CreateSExtOrBitCast(Builder.CreateCall(Rand,{}), Type::getInt64Ty(CTX)));
		extended = Builder.CreateShl(extended, 15);
		extended = Builder.CreateXor(extended, Builder.CreateSExtOrBitCast(Builder.CreateCall(Rand,{}), Type::getInt64Ty(CTX)));
		extended = Builder.CreateShl(extended, 15);
		extended = Builder.CreateXor(extended, Builder.CreateSExtOrBitCast(Builder.CreateCall(Rand,{}), Type::getInt64Ty(CTX)));
		extended = Builder.CreateAnd(extended, 0x7FFFFFFFFFFFFFFF);
		long div = maxVal-minVal+1;
		extended = Builder.CreateSRem(extended, ConstantInt::get(Type::getInt64Ty(CTX), div, true));
		extended = Builder.CreateAdd(extended, ConstantInt::get(Type::getInt64Ty(CTX), minVal, true));
		return Builder.CreateSExtOrTrunc(extended, type);
	}

	/*generates values for each argument in main function
	for numeric types an interval is read from the properties file and a 
	random number is chosen within the range of that file, otherwise for
	array and pointer types an empty structure is made and then each sub element
	is recursively filled in, if oldCopy is null the values are random, otherwise
	a fuzzed value is generated using old copy as a reference*/
	Value* genValue(Module &M, Type* param, string index, IRBuilder<> &Builder, Value* oldCopy){
		auto &CTX = M.getContext();
		Type *mallocArgTy = Type::getInt64Ty(CTX);
		FunctionType *MallocTy = FunctionType::get(PointerType::getUnqual(Type::getInt8Ty(CTX)), mallocArgTy, false);
		FunctionCallee Malloc = M.getOrInsertFunction("malloc", MallocTy);
		
		PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
		FunctionType *PrintfTy = FunctionType::get(IntegerType::getInt32Ty(CTX), PrintfArgTy, true);
		FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
		Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
		PrintfF->setDoesNotThrow();
		PrintfF->addParamAttr(0, Attribute::NoCapture);
		PrintfF->addParamAttr(0, Attribute::ReadOnly);
		
		if(param->isIntegerTy()) {
			string constant_min_name = "main_"+index+"_min";//read values from property file
			string constant_max_name = "main_"+index+"_max";
			int64_t minVal;
			int64_t maxVal;
			
			try {
				minVal = stoll(properties.at(constant_min_name));
				maxVal = stoll(properties.at(constant_max_name));
			}
			catch (const std::out_of_range& oor) {
				errs()<<"Error: Missing min or max attribute on main program argument\n" <<index<<"\n";
				exit(0);
  			}

			if(oldCopy != 0) {//this is where integer fuzzing occurs
				int64_t fuzz = (maxVal-minVal)/10000;//fuzz is 1/10000 of range

				if(fuzz == 0)//let fuzz always be at least 1
					fuzz = 1;

				oldCopy = Builder.CreateLoad(oldCopy);
				Value* fuzzedRandom = getRandomIntegralValue(M, Builder, param, -fuzz, fuzz);//choose a value between [oldValue-fuzz, oldValue+fuzz]
				fuzzedRandom = Builder.CreateAdd(oldCopy, fuzzedRandom);
				Value* maxConstant = ConstantInt::get(param, maxVal, true);
				Value* minConstant = ConstantInt::get(param, minVal, true);
				//remaining code ensures fuzzed value is still in valid program range
				Value* maxCompare = Builder.CreateSExtOrTrunc(Builder.CreateICmpSGT(fuzzedRandom, maxConstant), param);
				maxCompare = Builder.CreateAnd(maxCompare, ConstantInt::get(param, 1, true));
				fuzzedRandom = Builder.CreateAdd(fuzzedRandom, Builder.CreateMul(maxCompare, Builder.CreateSub(maxConstant, fuzzedRandom)));
				Value* minCompare = Builder.CreateSExtOrTrunc(Builder.CreateICmpSLT(fuzzedRandom, minConstant), param);
				minCompare = Builder.CreateAnd(minCompare, ConstantInt::get(param, 1, true));
				fuzzedRandom = Builder.CreateAdd(fuzzedRandom, Builder.CreateMul(minCompare, Builder.CreateSub(minConstant, fuzzedRandom)));
				return fuzzedRandom;
			}
			
			return getRandomIntegralValue(M, Builder, param, minVal, maxVal);
		}
		else if(param->isFloatingPointTy()) {
			string constant_min_name = "main_"+index+"_min";
			string constant_max_name = "main_"+index+"_max";
			long double minVal;
			long double maxVal;
			
			try {
				minVal = stold(properties.at(constant_min_name));
				maxVal = stold(properties.at(constant_max_name));
			}
			catch (const std::out_of_range& oor) {
				errs()<<"Error: Missing min or max attribute on main program argument\n" <<index<<"\n";
				exit(0);
  			}

			if(oldCopy != 0) {//this is where floating point fuzzing occurs
				long double fuzz = (maxVal-minVal)/10000.0;//fuzz is 1/10000 of range
				oldCopy = Builder.CreateLoad(oldCopy);
				Value* fuzzedRandom = getRandomFloatingValue(M, Builder, param, ConstantFP::get(param, fuzz), ConstantFP::get(param, -fuzz));
				fuzzedRandom = Builder.CreateFAdd(oldCopy, fuzzedRandom);//choose a value between [oldValue-fuzz, oldValue+fuzz]
				Value* maxConstant = ConstantFP::get(param, maxVal);
				Value* minConstant = ConstantFP::get(param, minVal);
				//remaining code ensures fuzzed value is still in valid program range
				Value* greaterThanMax = Builder.CreateFCmpOGT(fuzzedRandom, maxConstant);
				greaterThanMax = Builder.CreateAnd(greaterThanMax, ConstantInt::get(greaterThanMax->getType(), 1, true));
				greaterThanMax = Builder.CreateSIToFP(greaterThanMax, param);
				greaterThanMax = Builder.CreateFMul(greaterThanMax, greaterThanMax);
				Value* inverse = Builder.CreateFSub(ConstantFP::get(param, 1), greaterThanMax);
				fuzzedRandom = Builder.CreateFAdd(Builder.CreateFMul(inverse, fuzzedRandom), Builder.CreateFMul(greaterThanMax, maxConstant));
				
				Value* lessThanMin = Builder.CreateFCmpOLT(fuzzedRandom, minConstant);
				lessThanMin = Builder.CreateAnd(lessThanMin, ConstantInt::get(lessThanMin->getType(), 1, true));
				lessThanMin = Builder.CreateSIToFP(lessThanMin, param);
				lessThanMin = Builder.CreateFMul(lessThanMin, lessThanMin);
				inverse = Builder.CreateFSub(ConstantFP::get(param, 1), lessThanMin);
				fuzzedRandom = Builder.CreateFAdd(Builder.CreateFMul(inverse, fuzzedRandom), Builder.CreateFMul(lessThanMin, minConstant));
				
				return fuzzedRandom;
			}

			return getRandomFloatingValue(M, Builder, param, ConstantFP::get(Type::getX86_FP80Ty(CTX), minVal), ConstantFP::get(Type::getX86_FP80Ty(CTX), maxVal));
		}//recursively continue for composite types
		else if(param->isPointerTy()) {
			if(oldCopy!=0){
				oldCopy = Builder.CreateLoad(oldCopy);
			}

			string constant_size_name = "main_"+index+"_size";//if it's a pointer type get size from property file
			int size = 0;

			try {
				string value = properties.at(constant_size_name);
				size = stoi(value);
			}
			catch (const std::out_of_range& oor) {
				errs()<<"Error: Missing size attribute on main program argument\n"<<constant_size_name<<"\n";
				exit(0);
  			}

			
			if(param->getPointerElementType()->isArrayTy()){
				if(lang_arg.compare("rust") == 0 && param->getPointerElementType()->getArrayNumElements() == 0){
					Value* sizePtr = 
			Builder.CreateGEP(param, ConstantPointerNull::get(PointerType::getUnqual(param->getPointerElementType()->getArrayElementType())), ConstantInt::get(Type::getInt64Ty(CTX), 1));
					Value* sizeCast = Builder.CreatePtrToInt(sizePtr, Type::getInt64Ty(CTX));
					Value* allocate = Builder.CreateCall(Malloc, {Builder.CreateMul(sizeCast, ConstantInt::get(Type::getInt64Ty(CTX), size))});
					Value* MainKernelFunctionArg = Builder.CreateBitCast(allocate, PointerType::getUnqual(param->getPointerElementType()->getArrayElementType()));
//Value* MainKernelFunctionArg = Builder.CreateAlloca(param->getPointerElementType()->getArrayElementType(), ConstantInt::get(Type::getInt32Ty(CTX), size));
					Value* oldBitcastedArray = oldCopy == 0 ? 0 : Builder.CreateBitCast(oldCopy, PointerType::getUnqual(param->getPointerElementType()->getArrayElementType()));

					for(int ind = 0; ind<size; ind++) {						
						Builder.CreateStore(genValue(M, param->getPointerElementType()->getArrayElementType(), index+"_"+to_string(ind), Builder, oldCopy == 0 ? 0 : Builder.CreateGEP(oldBitcastedArray, ConstantInt::get(Type::getInt32Ty(CTX), ind))), 
						Builder.CreateGEP(MainKernelFunctionArg, ConstantInt::get(Type::getInt32Ty(CTX), ind)));
					}

					return Builder.CreateBitCast(MainKernelFunctionArg, PointerType::getUnqual(param->getPointerElementType()));
				}
			}

			Value* sizePtr = 
			Builder.CreateGEP(param, ConstantPointerNull::get(PointerType::getUnqual(param->getPointerElementType())), ConstantInt::get(Type::getInt64Ty(CTX), 1));
			Value* sizeCast = Builder.CreatePtrToInt(sizePtr, Type::getInt64Ty(CTX));
			Value* allocate = Builder.CreateCall(Malloc, {Builder.CreateMul(sizeCast, ConstantInt::get(Type::getInt64Ty(CTX), size))});
			Value* MainKernelFunctionArg = Builder.CreateBitCast(allocate, param);
			
			//Value* MainKernelFunctionArg = Builder.CreateAlloca(param->getPointerElementType(), ConstantInt::get(Type::getInt32Ty(CTX), size));

			for(int ind = 0; ind<size; ind++) {
				Builder.CreateStore(genValue(M, param->getPointerElementType(), index+"_"+to_string(ind), Builder, 
				oldCopy == 0 ? 0 : Builder.CreateGEP(oldCopy, ConstantInt::get(Type::getInt32Ty(CTX), ind))), 
				Builder.CreateGEP(MainKernelFunctionArg, ConstantInt::get(Type::getInt32Ty(CTX), ind)));
			}

			return MainKernelFunctionArg;
		}
		else if(param->isArrayTy()) {
			int size = param->getArrayNumElements();
			Value* sizePtr = 
			Builder.CreateGEP(param, ConstantPointerNull::get(PointerType::getUnqual(param)), ConstantInt::get(Type::getInt64Ty(CTX), 1));
			Value* sizeCast = Builder.CreatePtrToInt(sizePtr, Type::getInt64Ty(CTX));
			Value* allocate = Builder.CreateCall(Malloc, {sizeCast});
			Value* MainKernelFunctionArg = Builder.CreateBitCast(allocate, PointerType::getUnqual(param));

			//Value* MainKernelFunctionArg = Builder.CreateAlloca(param);
			Value* PointerCast = Builder.CreateBitCast(MainKernelFunctionArg, PointerType::getUnqual(param->getArrayElementType()));
			Value* PointerOldCast = oldCopy == 0 ? 0 : Builder.CreateBitCast(oldCopy, PointerType::getUnqual(param->getArrayElementType()));

			for(int ind = 0; ind<size; ind++) {
				Builder.CreateStore(genValue(M, param->getArrayElementType(), index+"_"+to_string(ind), Builder,
					oldCopy == 0 ? 0 : Builder.CreateGEP(PointerOldCast,  ConstantInt::get(Type::getInt32Ty(CTX), ind))), 
				Builder.CreateGEP(PointerCast, ConstantInt::get(Type::getInt32Ty(CTX), ind)));		
			}

			return Builder.CreateLoad(MainKernelFunctionArg);
		}
		else {
			errs()<<"Error: Unknown Type Encountered\n";
			exit(0);
		}
	}

	/*
	For C++ this function corrects function names and removes namespace information,
	this ensures that function names in the property file match their expected name and
	in addition that kernel functions are correctly identified*/
	std::string demangle(const std::string &Name) {
		int status;
		char* demangled_name = NULL;
		char* cstr_name = new char[Name.length() + 1];
		strcpy(cstr_name, Name.c_str());
		demangled_name = __cxxabiv1::__cxa_demangle(cstr_name, NULL, NULL, &status);
      
		if (status != 0) {
			return Name;
		}

		std::string s(demangled_name);
		s = s.substr(s.find_last_of(':')+1);
		s = s.substr(0, s.find("("));
		return s;
	}

	/*
		prints a value (useful for debugging)
	*/
	void printValue(Module &M, Type* param, IRBuilder<> &Builder, Value* oldCopy, string index){
		auto &CTX = M.getContext();
		
		PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
		FunctionType *PrintfTy = FunctionType::get(IntegerType::getInt32Ty(CTX), PrintfArgTy, true);
		FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
		Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
		PrintfF->setDoesNotThrow();
		PrintfF->addParamAttr(0, Attribute::NoCapture);
		PrintfF->addParamAttr(0, Attribute::ReadOnly);
		
		if(param->isIntegerTy()) {
			oldCopy = Builder.CreateLoad(oldCopy);
			Builder.CreateCall(Printf, { Builder.CreatePointerCast(Builder.CreateGlobalString(index+" %ld\n"), PrintfArgTy), Builder.CreateSExtOrTrunc(oldCopy, 
Type::getInt64Ty(CTX))});
		}
		else if(param->isFloatingPointTy()) {
			oldCopy = Builder.CreateLoad(oldCopy);
			Builder.CreateCall(Printf, { Builder.CreatePointerCast(Builder.CreateGlobalString(index+" %Lf\n"), PrintfArgTy), Builder.CreateFPExt(oldCopy, Type::getX86_FP80Ty(CTX))});
		}
		//recursively continue for composite types
		else if(param->isPointerTy()) {
			oldCopy = Builder.CreateLoad(oldCopy);
			string constant_size_name = "main_"+index+"_size";//if it's a pointer type get size from property file
			int size = 0;

			try {
				string value = properties.at(constant_size_name);
				size = stoi(value);
			}
			catch (const std::out_of_range& oor) {
				errs()<<"Error: Missing size attribute on main program argument\n"<<constant_size_name<<"\n";
				exit(0);
  			}

			if(param->getPointerElementType()->isArrayTy()){
				if(lang_arg.compare("rust") == 0 && param->getPointerElementType()->getArrayNumElements() == 0){
					Value* oldBitcastedArray = Builder.CreateBitCast(oldCopy, PointerType::getUnqual(param->getPointerElementType()->getArrayElementType()));

					for(int ind = 0; ind<size; ind++) {						
						printValue(M, param->getPointerElementType()->getArrayElementType(), Builder, Builder.CreateGEP(oldBitcastedArray, ConstantInt::get(Type::getInt32Ty(CTX), ind)), index+"_"+to_string(ind));
					}

					return;
				}
			}

			for(int ind = 0; ind<size; ind++) {
				printValue(M, param->getPointerElementType(), Builder, 
				Builder.CreateGEP(oldCopy, ConstantInt::get(Type::getInt32Ty(CTX), ind)), index+"_"+to_string(ind));
			}
		}
		else if(param->isArrayTy()) {
			int size = param->getArrayNumElements();
			Value* PointerOldCast = Builder.CreateBitCast(oldCopy, PointerType::getUnqual(param->getArrayElementType()));

			for(int ind = 0; ind<size; ind++) {
				printValue(M, param->getArrayElementType(), Builder,
					Builder.CreateGEP(PointerOldCast,  ConstantInt::get(Type::getInt32Ty(CTX), ind)), index+"_"+to_string(ind));
			}
		}
		else {
			errs()<<"Error: Unknown Type Encountered\n";
			exit(0);
		}
	}

	/*
		prints all the values of a struct (useful for debugging)
	*/
	void printStruct(Module &M, IRBuilder<>* builder, StructType* struct_type, Value* oldCopy){
		auto &CTX = M.getContext();
		
		for(unsigned int i = 0; i < struct_type->getNumElements()-1; i++) {
			Type* param = struct_type->getElementType(i);
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), i));
			Value* position = builder->CreateGEP(oldCopy, index_vector);
			printValue(M, param, *builder, position, std::to_string(i+1));
		}
	}

	/*
		this function creates a new struct instance for the queue, if old copy is null then a
		completely random value is chosen in the range, otherwise oldCopy is used as a reference
		for fuzzing
	*/
	Value* allocateStruct(Module &M, IRBuilder<>* builder, StructType* struct_type, Value* oldCopy){
		auto &CTX = M.getContext();
		Type *mallocArgTy = Type::getInt64Ty(CTX);
		FunctionType *MallocTy = FunctionType::get(PointerType::getUnqual(Type::getInt8Ty(CTX)), mallocArgTy, false);
		FunctionCallee Malloc = M.getOrInsertFunction("malloc", MallocTy);
		Value* sizePtr = 
		builder->CreateGEP(struct_type, ConstantPointerNull::get(PointerType::getUnqual(struct_type)), ConstantInt::get(Type::getInt64Ty(CTX), 1));
		Value* sizeCast = builder->CreatePtrToInt(sizePtr, Type::getInt64Ty(CTX));
		Value* allocate = builder->CreateCall(Malloc, {sizeCast});
		Value* mallocedStruct = builder->CreateBitCast(allocate, PointerType::getUnqual(struct_type));
		
		for(unsigned int i = 0; i < struct_type->getNumElements()-1; i++) {
			Type* param = struct_type->getElementType(i);
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), i));
			Value* position = builder->CreateGEP(mallocedStruct, index_vector);
			Value* position_old = oldCopy == 0 ? 0 : builder->CreateGEP(oldCopy, index_vector);
			Value* genValued = genValue(M, param, std::to_string(i+1), *builder, position_old);
			builder->CreateStore(genValued, position);
		}

		{
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), struct_type->getNumElements()-1));
			Value* position = builder->CreateGEP(mallocedStruct, index_vector);
			builder->CreateStore(ConstantPointerNull::get(PointerType::getUnqual(struct_type)), position);
		}

		return mallocedStruct;
	}

	/*frees any malloced memory for an individual element, since gen value uses malloc free must also be 
	called or else the program quickly runs out of memory*/
	void freeElement(Value* element, Module &M, IRBuilder<>* builder, string index){
		element = builder->CreateLoad(element);
		auto &CTX = M.getContext();

		if(element->getType()->isPointerTy()) {
			string constant_size_name = "main_"+index+"_size";
			int size = 0;

			try {
				string value = properties.at(constant_size_name);
				size = stoi(value);
			}
			catch (const std::out_of_range& oor) {
				return;
  			}

			for(int i = 0; i < size; i++) {
				Value* position = builder->CreateGEP(element,  ConstantInt::get(Type::getInt32Ty(CTX), i));
				freeElement(position, M, builder, index + "_" + to_string(i));
			}

			Type *freeArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
			FunctionType *FreeTy = FunctionType::get(Type::getVoidTy(CTX), freeArgTy, false);
			FunctionCallee Free = M.getOrInsertFunction("free", FreeTy);
			builder->CreateCall(Free, {builder->CreateBitCast(element, freeArgTy)});
		}
	}

	/*calls free element on each member of a struct so that any memory holding the struct is returned 
	when an element is removed from the queue*/
	void freeStruct(Value* struct_val, Module &M, IRBuilder<>* builder){
		auto &CTX = M.getContext();
		
		for(unsigned int i = 0; i < struct_val->getType()->getPointerElementType()->getStructNumElements()-1; i++) {
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), i));
			Value* position = builder->CreateGEP(struct_val, index_vector);
			freeElement(position, M, builder, to_string(i+1));
		}
		

		Type *freeArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
		FunctionType *FreeTy = FunctionType::get(Type::getVoidTy(CTX), freeArgTy, false);
		FunctionCallee Free = M.getOrInsertFunction("free", FreeTy);
		builder->CreateCall(Free, {builder->CreateBitCast(struct_val, freeArgTy)});
	}

	//The main function of the pass
	bool runOnModule(Module &M) override {
		//First start by reading the configuration file into a properties map and exiting if an error occurs
		ifstream myfile (file_arg);
  		string line;

		if (myfile.is_open()) {
    			while (getline (myfile,line)) {
				line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
     				size_t pos = line.find(":");
				
				if(pos == string::npos)
					continue;

				string key = line.substr(0,pos);
				string val = line.substr(pos+1);
				properties[key] = val;
    			}
    			
			myfile.close();
  		}
		else {
			errs() << "Error: Unable to open config file\n"; 
			return false;
		}

		//store unmangled rust names
		vector<string> unmangledRustNames;
		
		if(lang_arg.compare("rust")==0){
			ofstream mangleFile;
			mangleFile.open("mangled_names.list", ios::out | ios::trunc | ios::binary);

			if(!mangleFile.is_open()){
				errs() << "Error: File IO Error\n"; 
				return false;
			}

			for (auto &F : M) {
				mangleFile<<F.getName().str()<<"\n";
			}

			mangleFile.close();
			system("rustfilt -i mangled_names.list -o unmangled_names.list");//run rustfilt tool to demangle rust names
			ifstream unmangleFile("unmangled_names.list");

			if(!unmangleFile.is_open()){
				errs() << "Error: File IO Error\n"; 
				return false;
			}

			while (getline (unmangleFile,line)) {
				line = line.substr(line.find_last_of(":")+1);//remove namespace information
				unmangledRustNames.push_back(line);
    			}

			unmangleFile.close();
			remove("mangled_names.list");//clean up
			remove("unmangled_names.list");
		}

		bool Instrumented = false;//keep track if any kernel functions exist
		vector<ARGUMENT_DATA *> CallCounterMap;

		auto &CTX = M.getContext();

		Function* mainFunction = 0;//find original program main so it can be called by black box tester
		int i = 0;

		changeTracker = CreateIntegerTracker(M, "new*var*val", true, 8);
		infTracker = CreateIntegerTracker(M, "new*inf*track", true, 1);

		for (auto &F : M) {
			if (F.isDeclaration())//skip declarations
				continue;

			string name = F.getName();

			if(lang_arg.compare("c++") == 0)//if language is c++ names must be demangled
				name = demangle(name);

			if(lang_arg.compare("rust") == 0)//if language is rust names must be demangled
				name = unmangledRustNames.at(i++);

			if(name.compare("original_program_main") == 0)//if the function is the main function store it
				mainFunction = &F;

			// Check that function is actually a kernel function, otherwise continue iterating through all functions
			if(!isValid(name))
				continue;

			//Cannot track var args functions
			if(F.isVarArg())
				continue;

			errs()<<"Instrumented function : " << name << "\n";
			Instrumented = true;//at least one kernel function was found
			// End kernel function checking
			string funcName = name;
			Instruction* splitInstruction = &F.getEntryBlock().getInstList().back();//find instruction that should follow instrumenting code
			IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());

			for(unsigned int i = 0; i < F.getFunctionType()->getNumParams(); i++) {
				processTypeIf(F.getArg(i), funcName, i, "", M, CallCounterMap, splitInstruction, Builder);//track min and max of each variable
			}
		}

		//Stop here if there are no kernel functions
		if (false == Instrumented)
			return Instrumented;

		//Stop if no main function exists
		if(mainFunction == 0){
			errs()<<"No main function exists\n";
			return true;
		}

		/*
		The next long block of code adds instructions in the destructor of the program to print the min and max value for each variable
		when the program finished executing.  This is done by creating a function that prints each variable and then calling that function 
		in the programs DTORS
		*/
		PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));
		FunctionType *PrintfTy = FunctionType::get(IntegerType::getInt32Ty(CTX), PrintfArgTy, true);
		FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
		Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
		PrintfF->setDoesNotThrow();
		PrintfF->addParamAttr(0, Attribute::NoCapture);
		PrintfF->addParamAttr(0, Attribute::ReadOnly);
		llvm::Constant *ResultFormatStrInt = llvm::ConstantDataArray::getString(CTX, "%-18s %-6d %-6s %-21lld %lld\n");
		llvm::Constant *ResultFormatStrFloat = llvm::ConstantDataArray::getString(CTX, "%-18s %-6d %-6s %-21Le %Le\n");
		Constant *ResultFormatStrVarInt = M.getOrInsertGlobal("ResultFormatStrIRInt", ResultFormatStrInt->getType());
		dyn_cast<GlobalVariable>(ResultFormatStrVarInt)->setInitializer(ResultFormatStrInt);
		Constant *ResultFormatStrVarFloat = M.getOrInsertGlobal("ResultFormatStrIRFloat", ResultFormatStrFloat->getType());
		dyn_cast<GlobalVariable>(ResultFormatStrVarFloat)->setInitializer(ResultFormatStrFloat);

		std::string out = "\n";
		out += "===========================================================================\n";
		out += "                     Guided Blackbox Analysis Results\n";
		out += "===========================================================================\n";
		out += "FUNCTION           ARG    INDEX  MIN                   MAX\n";
		out += "---------------------------------------------------------------------------\n";

		llvm::Constant *ResultHeaderStr = llvm::ConstantDataArray::getString(CTX, out.c_str());
		Constant *ResultHeaderStrVar = M.getOrInsertGlobal("ResultHeaderStrIR", ResultHeaderStr->getType());
		dyn_cast<GlobalVariable>(ResultHeaderStrVar)->setInitializer(ResultHeaderStr);
		FunctionType *PrintfWrapperTy = FunctionType::get(llvm::Type::getVoidTy(CTX), {}, false);
		Function *PrintfWrapperF = dyn_cast<Function>(M.getOrInsertFunction("@print_results", PrintfWrapperTy).getCallee());
		llvm::BasicBlock *RetBlock = llvm::BasicBlock::Create(CTX, "enter", PrintfWrapperF);
		IRBuilder<>* Builder = new IRBuilder<>(RetBlock);
		llvm::Value *ResultHeaderStrPtr = Builder->CreatePointerCast(ResultHeaderStrVar, PrintfArgTy);
		llvm::Value *ResultFormatStrPtrInt = Builder->CreatePointerCast(ResultFormatStrVarInt, PrintfArgTy);
		llvm::Value *ResultFormatStrPtrFloat = Builder->CreatePointerCast(ResultFormatStrVarFloat, PrintfArgTy);
		Builder->CreateCall(Printf, {ResultHeaderStrPtr});
		LoadInst *LoadCounterMax;
		LoadInst *LoadCounterMin;
		
		int unique = 0;
		IRBuilder<>* currentIteration = Builder;

		for (auto &item : CallCounterMap) {
			LoadCounterMax = currentIteration->CreateLoad(item->MAX_VAR);
			LoadCounterMin = currentIteration->CreateLoad(item->MIN_VAR);
			bool isFp = item->IS_FP;

			if(isFp){
				llvm::BasicBlock* notCalledBlock = llvm::BasicBlock::Create(CTX, "not_called"+to_string(unique), PrintfWrapperF);
				llvm::BasicBlock* calledBlock = llvm::BasicBlock::Create(CTX, "called"+to_string(unique), PrintfWrapperF);
				IRBuilder<> notCalledBuilder(notCalledBlock);
				IRBuilder<> calledBuilder(calledBlock);
				Value* notCalled = currentIteration->CreateFCmpOGT(LoadCounterMin, LoadCounterMax);
				currentIteration->CreateCondBr(notCalled, notCalledBlock, calledBlock);
				calledBuilder.CreateCall(Printf, {ResultFormatStrPtrFloat, item->NAME, item->ARG, item->INDEX, 
						LoadCounterMin, LoadCounterMax});
				
				notCalledBuilder.CreateCall(Printf, { notCalledBuilder.CreatePointerCast(notCalledBuilder.CreateGlobalString("%-18s %-6d %-6s Not Reachable\n"), PrintfArgTy), item->NAME, item->ARG, item->INDEX});

				llvm::BasicBlock* endingBlock = llvm::BasicBlock::Create(CTX, "ending"+to_string(unique), PrintfWrapperF);
				calledBuilder.CreateBr(endingBlock);
				notCalledBuilder.CreateBr(endingBlock);
				currentIteration = new IRBuilder<>(endingBlock);
			}
			else{
				llvm::BasicBlock* notCalledBlock = llvm::BasicBlock::Create(CTX, "not_called"+to_string(unique), PrintfWrapperF);
				llvm::BasicBlock* calledBlock = llvm::BasicBlock::Create(CTX, "called"+to_string(unique), PrintfWrapperF);
				IRBuilder<> notCalledBuilder(notCalledBlock);
				IRBuilder<> calledBuilder(calledBlock);
				Value* notCalled = currentIteration->CreateICmpSGT(LoadCounterMin, LoadCounterMax);
				currentIteration->CreateCondBr(notCalled, notCalledBlock, calledBlock);
				calledBuilder.CreateCall(
						Printf, {ResultFormatStrPtrInt, item->NAME, item->ARG, item->INDEX, calledBuilder.CreateSExtOrBitCast(LoadCounterMin,
						Type::getInt64Ty(CTX)), calledBuilder.CreateSExtOrBitCast(LoadCounterMax, Type::getInt64Ty(CTX))});
				
				notCalledBuilder.CreateCall(Printf, { notCalledBuilder.CreatePointerCast(notCalledBuilder.CreateGlobalString("%-18s %-6d %-6s Not Reachable\n"), PrintfArgTy), item->NAME, item->ARG, item->INDEX});

				llvm::BasicBlock* endingBlock = llvm::BasicBlock::Create(CTX, "ending"+to_string(unique), PrintfWrapperF);
				calledBuilder.CreateBr(endingBlock);
				notCalledBuilder.CreateBr(endingBlock);
				currentIteration = new IRBuilder<>(endingBlock);
			}

			unique++;
		}
		
		currentIteration->CreateRetVoid();
		appendToGlobalDtors(M, PrintfWrapperF, 0);
		//End print function

		//Get program run time in seconds
		int time = 60*time_arg;
		PointerType *TimeArgTy = PointerType::getUnqual(Type::getInt64Ty(CTX));
		FunctionType *TimeTy = FunctionType::get(IntegerType::getInt64Ty(CTX), TimeArgTy, false);
		FunctionCallee Time = M.getOrInsertFunction("time", TimeTy);
		FunctionType *MainWrapperTy = FunctionType::get(llvm::Type::getInt32Ty(CTX), {}, false);
		Function *Initializer = dyn_cast<Function>(M.getOrInsertFunction("main", MainWrapperTy).getCallee());
		llvm::BasicBlock *StartBlock = llvm::BasicBlock::Create(CTX, "enter", Initializer);
		IRBuilder<> InitializerBuilder(StartBlock);
		Type *SrandArgTy = Type::getInt32Ty(CTX);
		FunctionType *SrandTy = FunctionType::get(Type::getVoidTy(CTX), SrandArgTy, false);
		//set random seed to user specified seed
		FunctionCallee Srand = M.getOrInsertFunction("srand", SrandTy);
		InitializerBuilder.CreateCall(Srand, {ConstantInt::get(Type::getInt32Ty(CTX), seed_arg)});
		FunctionCallee MainProgram = M.getOrInsertFunction(mainFunction->getName(), mainFunction->getFunctionType());
		Value* stackSize = InitializerBuilder.CreateAlloca(Type::getInt32Ty(CTX));
		InitializerBuilder.CreateStore(ConstantInt::get(Type::getInt32Ty(CTX), 0), stackSize);
		Value* timeBegin = InitializerBuilder.CreateAlloca(Type::getInt64Ty(CTX));
		//set initial time to current clock time
		InitializerBuilder.CreateStore(InitializerBuilder.CreateCall(Time, {Constant::getNullValue(TimeArgTy)}), timeBegin);
		vector<Type*> structVector;

		for(unsigned int i = 0; i < MainProgram.getFunctionType()->getNumParams(); i++) {
			structVector.push_back(MainProgram.getFunctionType()->getParamType(i));
		}

		/*create a struct representing the inputs to the main function as a node in a linked list (queue)
		structure is like

		struct Arguments{
			Arg1,
			Arg2,
			...
			Argn,
			Arguments* nextPointer (points to next node in list)
		}*/


		StructType* argStruct = StructType::create(CTX);
		structVector.push_back(PointerType::getUnqual(argStruct));
		argStruct->setBody(structVector);

		//Create a queue with a linked list
		//head ptr is the head of the list and tail ptr is the tail
		Value* head_ptr = InitializerBuilder.CreateAlloca(PointerType::getUnqual(argStruct));
		Value* tail_ptr = InitializerBuilder.CreateAlloca(PointerType::getUnqual(argStruct));
		Value* temp_ptr = InitializerBuilder.CreateAlloca(PointerType::getUnqual(argStruct));
		//initially head and tail are null
		InitializerBuilder.CreateStore(ConstantPointerNull::get(PointerType::getUnqual(argStruct)), head_ptr);
		InitializerBuilder.CreateStore(ConstantPointerNull::get(PointerType::getUnqual(argStruct)), tail_ptr);

		//loop the next set of instructions
		llvm::BasicBlock *TopLoop = llvm::BasicBlock::Create(CTX, "top_loop", Initializer);
		InitializerBuilder.CreateBr(TopLoop);
		IRBuilder<> TopLoopBuilder(TopLoop);
		TopLoopBuilder.CreateStore(ConstantInt::get(Type::getInt8Ty(M.getContext()), 0), changeTracker);
		TopLoopBuilder.CreateStore(ConstantInt::get(Type::getInt1Ty(M.getContext()), 0), infTracker);

		llvm::BasicBlock *BottomLoop = llvm::BasicBlock::Create(CTX, "bottom_loop", Initializer);
		llvm::BasicBlock *UpdateNodes = llvm::BasicBlock::Create(CTX, "update_nodes", Initializer);
		Value* diffVal = TopLoopBuilder.CreatePtrDiff(TopLoopBuilder.CreateLoad(head_ptr), ConstantPointerNull::get(PointerType::getUnqual(argStruct)));
		Value* shouldUpdate = TopLoopBuilder.CreateICmp(CmpInst::Predicate::ICMP_EQ,diffVal,ConstantInt::get(Type::getInt64Ty(CTX), 0, true));
		TopLoopBuilder.CreateCondBr(shouldUpdate, UpdateNodes, BottomLoop);

		//If the queue is empty then add a new random node to it
		IRBuilder<> UpdateNodeBuilder(UpdateNodes);
		Value* mallocedStruct = allocateStruct(M, &UpdateNodeBuilder, argStruct, 0);
		//queue size grows by one
		UpdateNodeBuilder.CreateStore(UpdateNodeBuilder.CreateAdd(UpdateNodeBuilder.CreateLoad(stackSize), ConstantInt::get(Type::getInt32Ty(CTX), 1)), stackSize);
		//update queue pointers to add new element
		UpdateNodeBuilder.CreateStore(mallocedStruct, head_ptr);
		UpdateNodeBuilder.CreateStore(mallocedStruct, tail_ptr);
		UpdateNodeBuilder.CreateBr(BottomLoop);
		IRBuilder<> BottomLoopBuilder(BottomLoop);
		
		Value* newStruct = BottomLoopBuilder.CreateLoad(head_ptr);
		vector<Value*> argVector;

		for(unsigned int i = 0; i < MainProgram.getFunctionType()->getNumParams(); i++) {
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), i));
			Value* position = BottomLoopBuilder.CreateGEP(newStruct, index_vector);
			argVector.push_back(BottomLoopBuilder.CreateLoad(position));//generate new values for each argument
		}

		//Call main function with generated arguments
		BottomLoopBuilder.CreateCall(MainProgram,argVector);

		//If any range was updated branch to mutations otherwise continue to top of loop
		llvm::BasicBlock *IfChangeBlock = llvm::BasicBlock::Create(CTX, "change_block", Initializer);
		IRBuilder<> IfChangeBuilder(IfChangeBlock);
		llvm::BasicBlock *VeryBottomBlock = llvm::BasicBlock::Create(CTX, "very_bottom", Initializer);
		IRBuilder<> VeryBottomBuilder(VeryBottomBlock);
		
		//Mutate
		/*
		If a range was updated then mutations are added to the queue
		The number of mutations is determined by the mutation argument
		The actual fuzzing is done in gen value function
		Queue is updated to hold new mutations
		*/
		for(int mutations = 0; mutations < mutation_arg; mutations++) {
			IfChangeBuilder.CreateStore(IfChangeBuilder.CreateAdd(IfChangeBuilder.CreateLoad(stackSize), ConstantInt::get(Type::getInt32Ty(CTX), 1)), stackSize);
			Value* mutateStruct = allocateStruct(M, &IfChangeBuilder, argStruct, IfChangeBuilder.CreateLoad(head_ptr));
		
			{
				std::vector<Value*> index_vector;
				index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
				index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), MainProgram.getFunctionType()->getNumParams()));
				Value* position = IfChangeBuilder.CreateGEP(IfChangeBuilder.CreateLoad(tail_ptr), index_vector);
				IfChangeBuilder.CreateStore(mutateStruct, position);
				IfChangeBuilder.CreateStore(mutateStruct, tail_ptr);
			}
		}

		//Add one random node (this helps prevent getting stuck in local mins/maxes since new values will be at least ocassionally explored)
		Value* mutateStruct = allocateStruct(M, &IfChangeBuilder, argStruct, 0);
		{
			IfChangeBuilder.CreateStore(IfChangeBuilder.CreateAdd(IfChangeBuilder.CreateLoad(stackSize), ConstantInt::get(Type::getInt32Ty(CTX), 1)), stackSize);
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), MainProgram.getFunctionType()->getNumParams()));
			Value* position = IfChangeBuilder.CreateGEP(IfChangeBuilder.CreateLoad(tail_ptr), index_vector);
			IfChangeBuilder.CreateStore(mutateStruct, position);
			IfChangeBuilder.CreateStore(mutateStruct, tail_ptr);
		}

		//If an infinity occurs print the inputs that caused the infinity
		llvm::BasicBlock *IfInfinityBlock = llvm::BasicBlock::Create(CTX, "infinity_block", Initializer);
		IRBuilder<> infinityBuilder(IfInfinityBlock);
		
		llvm::Value *PrintInfinityString = infinityBuilder.CreatePointerCast(infinityBuilder.CreateGlobalString("Infinity Encountered with values\n"), PrintfArgTy);
		infinityBuilder.CreateCall(Printf, {PrintInfinityString});
		printStruct(M, &infinityBuilder, argStruct, infinityBuilder.CreateLoad(head_ptr));
		infinityBuilder.CreateBr(VeryBottomBlock);

		Value* ifInfinity = IfChangeBuilder.CreateLoad(infTracker);
		IfChangeBuilder.CreateCondBr(ifInfinity, IfInfinityBlock, VeryBottomBlock);
		Value* shouldUpdateIf = BottomLoopBuilder.CreateICmp(CmpInst::Predicate::ICMP_EQ,BottomLoopBuilder.CreateLoad(changeTracker) ,ConstantInt::get(Type::getInt8Ty(CTX), 1, true));
		//Add mutations if and only if a range was updated and additionally the stack size is less than 10000
		shouldUpdateIf = BottomLoopBuilder.CreateAnd(shouldUpdateIf, BottomLoopBuilder.CreateICmpULE(BottomLoopBuilder.CreateLoad(stackSize), ConstantInt::get(Type::getInt32Ty(CTX), 10000)));
		BottomLoopBuilder.CreateCondBr(shouldUpdateIf, IfChangeBlock, VeryBottomBlock);
		//~//

		//subtract current time from start time to find total time
		Value* timeElapsed = VeryBottomBuilder.CreateSub(VeryBottomBuilder.CreateCall(Time, {Constant::getNullValue(TimeArgTy)}),VeryBottomBuilder.CreateLoad(timeBegin));
		//if time is greater than user specified time end program and print results otherwise continue looping
		Value* compare = VeryBottomBuilder.CreateICmp(CmpInst::Predicate::ICMP_SGE,timeElapsed,ConstantInt::get(Type::getInt64Ty(CTX), time, true));
		llvm::BasicBlock *EndBlock = llvm::BasicBlock::Create(CTX, "end", Initializer);

		//update head pointer (remove and make the next element the head)
		{
			VeryBottomBuilder.CreateStore(VeryBottomBuilder.CreateLoad(head_ptr), temp_ptr);
			std::vector<Value*> index_vector;
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), 0));
			index_vector.push_back(ConstantInt::get(Type::getInt32Ty(M.getContext()), MainProgram.getFunctionType()->getNumParams()));
			Value* position = VeryBottomBuilder.CreateGEP(newStruct, index_vector);
			VeryBottomBuilder.CreateStore(VeryBottomBuilder.CreateLoad(position), head_ptr);
			freeStruct(VeryBottomBuilder.CreateLoad(temp_ptr), M, &VeryBottomBuilder);
		}

		//stack size decreases since an element is removed
		VeryBottomBuilder.CreateStore(VeryBottomBuilder.CreateSub(VeryBottomBuilder.CreateLoad(stackSize), ConstantInt::get(Type::getInt32Ty(CTX), 1)), stackSize);

		VeryBottomBuilder.CreateCondBr(compare, EndBlock, TopLoop);
		
		IRBuilder<> EndBuilder(EndBlock);
		EndBuilder.CreateRet(ConstantInt::get(Type::getInt32Ty(CTX), 0));
		return true;
	}
};
}

//These variables correctly register the pass with an ID and name
char GuidedBlackBoxTester::ID = 0;
static RegisterPass<GuidedBlackBoxTester> X("guided-blackbox", "Guided Blackbox Testing Pass");

