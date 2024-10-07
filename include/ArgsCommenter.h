
#ifndef LACOMMENTER_H
#define LACOMMENTER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Rewrite/Core/Rewriter.h"

class LACommenterMatcher : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    LACommenterMatcher(clang::Rewriter &LACRewriter) : LACRewriter(LACRewriter) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult &) override;

    void onEndOfTranslationUnit() override;
private:
    clang::Rewriter LACRewriter;
    llvm::SmallSet<clang::FullSourceLoc, 8> CommentedLocations;
};

class LACommenterASTConsumer : public clang::ASTConsumer {
public:
  LACommenterASTConsumer(clang::Rewriter &R);
  
  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Finder.matchAST(Context);
  }

private:
  clang::ast_matchers::MatchFinder Finder;
  LACommenterMatcher LACHandler;
};

class LACPluginAction : public PluginASTAction {
public:
    bool ParseArgs(const CompilerInstance &, const std::vector<std::string> &) override {
        return true;
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, StringRef file) override {
        LACRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
        return std::make_unique<LACommenterASTConsumer>(LACRewriter);
    }

private:
  Rewriter LACRewriter;
};

#endif 