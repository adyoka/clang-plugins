#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

// ----------------------------------
// RecursiveASTVisitor
// ----------------------------------

class DeclCounterASTVisitor : public RecursiveASTVisitor<DeclCounterASTVisitor> {
public:
    explicit DeclCounterASTVisitor(ASTContext *Ctx) : Context(Ctx) {}

    bool VisitCXXRecordDecl(CXXRecordDecl *Decl) {
        FullSourceLoc FullLoc = Context->getFullLoc(Decl->getBeginLoc());

        if (!FullLoc.isValid()) {
            return true;
        }

        if (FullLoc.isMacroID()) {
            FullLoc = FullLoc.getExpansionLoc();
        }

        SourceManager &SrcMgr = Context->getSourceManager();
        OptionalFileEntryRef EntryRef = SrcMgr.getFileEntryRefForID(SrcMgr.getFileID(FullLoc));
        DeclCounterMap[EntryRef->getName()]++;

        return true;
    }

    llvm::StringMap<unsigned> getDeclCounterMap() { return DeclCounterMap; }

private:
    ASTContext *Context;
    // map from string Decl name to unsigned count of decls in every file
    llvm::StringMap<unsigned> DeclCounterMap;
};

// ----------------------------------
// ASTConsumer for the action
// ----------------------------------


class DeclCounterASTConsumer : public clang::ASTConsumer {
public:
    explicit DeclCounterASTConsumer(ASTContext *Context) : Visitor(Context) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());

        auto DeclMap = Visitor.getDeclCounterMap();
        if (DeclMap.empty()) {
            llvm::outs() << "(clang-plugins)  No declarations found.\n";
            return;
        }

        for (auto &Element : DeclMap) {
            llvm::outs() << "(clang-plugins)   file: " << Element.first() << "\n";
            llvm::outs() << "(clang-plugins)  count: " << Element.second << "\n";
        }
    }    
private:
    DeclCounterASTVisitor Visitor;
};

// ----------------------------------
// FrontendAction for the plugin
// ----------------------------------

class DeclCounterAction : public clang::PluginASTAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, 
                                                        llvm::StringRef InFile) override {
        return std::unique_ptr<clang::ASTConsumer>(
            std::make_unique<DeclCounterASTConsumer>(&Compiler.getASTContext())
        );   
    }

    bool ParseArgs(const CompilerInstance &Compiler, 
                   const std::vector<std::string> &args) override {
        return true;
    }
};

// ----------------------------------
// Registering the plugin
// ----------------------------------

static FrontendPluginRegistry::Add<DeclCounterAction> 
    X("decl-count", "Plugin to count declarations in input files");