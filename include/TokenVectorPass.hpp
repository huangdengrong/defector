//
// Created by Mingkai Chen on 2017-04-10.
//

#ifndef DEFECTOR_TOKENVECTORPASS_HPP
#define DEFECTOR_TOKENVECTORPASS_HPP


#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include <unordered_set>
#include <unordered_map>


using BUG_REPO = std::unordered_map<std::string, std::unordered_set<uint64_t> >;


namespace tokenprofiling
{


struct TOKEN_REPO
{
	std::vector<std::string> tokens_;
	std::vector<size_t> usage_;
	std::unordered_map<std::string, uint64_t> token_map_;

	void infrequent (std::unordered_set<uint64_t>& infreq, size_t min_uses)
	{
		for (uint64_t i = 0, n = usage_.size(); i < n; i++)
		{
			if (usage_[i] < min_uses)
			{
				infreq.emplace(i);
			}
		}
	}
};


struct TOK_INFO
{
	std::string modname_;
	bool buggy_;
	std::vector<uint64_t> vecs_;

	void prune (std::unordered_set<uint64_t>& infrequent_toks)
	{
		std::remove_if(vecs_.begin(), vecs_.end(),
		[&infrequent_toks](uint64_t ti)
		{
			return infrequent_toks.end() != infrequent_toks.find(ti);
		});
	}

	void print (std::ofstream& o)
	{
		if (1 < vecs_.size() && o.good())
		// prune tokens of size 1
		{
			// out put ttv
			o << modname_ << "," << buggy_;
			for (uint64_t tk : vecs_)
			{
				o << "," << tk;
			}
			o << std::endl;
		}
	}
};


using VEC_INFO = std::vector<TOK_INFO>;


struct TokenVectorPass : public llvm::ModulePass
{
	static char ID;

	BUG_REPO bugs_;

	uint64_t last_token = 0;

	TOKEN_REPO* tok_repo_;

	VEC_INFO token_vectors_;

	std::unordered_map<std::string, uint64_t> func_ids_;

	std::unordered_map<uint64_t, std::vector<uint64_t> > tok_vector_;

	std::unordered_set<std::string> methods_;

	TokenVectorPass(TOKEN_REPO* repo) : llvm::ModulePass(ID), tok_repo_(repo) {}

	TokenVectorPass(BUG_REPO& bugs, TOKEN_REPO* repo) :
		llvm::ModulePass(ID), bugs_(bugs), tok_repo_(repo) {}

	void getAnalysisUsage(llvm::AnalysisUsage& au) const override
	{
		au.addRequired<llvm::DominatorTreeWrapperPass>();
		au.addRequired<llvm::LoopInfoWrapperPass>();
	}

	void methodClean (std::string& method_label) {}

	bool runOnModule(llvm::Module& m) override;

	void storeToken(uint64_t func_id, std::string label);

	uint64_t setFuncId(const std::string& func_name);

	void tokenSetup (llvm::Module& module, std::unordered_set<uint64_t>& err_funcs,
		std::unordered_map<std::string, std::unordered_set<uint64_t> >& immediate_dependents,
		std::unordered_set<std::string>& structs);
};


} // namespace tokenprofiling

#endif //DEFECTOR_TOKENVECTORPASS_HPP
