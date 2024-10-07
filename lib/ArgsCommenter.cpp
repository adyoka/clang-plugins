#include "ArgsCommenter.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ast_matchers;


void LACommenterMatcher::run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    const FunctionDecl *CalleeDecl = Result.Nodes.getNodeAs<FunctionDecl>("callee");
    const CallExpr *Caller = Result.Nodes.getNodeAs<CallExpr>("caller");

    // no parameters - nothing to comment
    if (CalleeDecl->parameters().empty())
        return;

    Expr const *const *Args = Caller->getArgs();
    size_t numArgs = Caller->getNumArgs();

    // if the call is overloaded operator call, then skip first argument which is this pointer
    if (isa<CXXOperatorCallExpr>(Caller)) {
        Args++; // skip first arg
        numArgs--;
    }

    for (unsigned idx = 0; idx < numArgs; idx++) {
        const Expr *ArgExpr = Args[idx]->IgnoreParenCasts();

        if ( !(dyn_cast<IntegerLiteral>(ArgExpr) || dyn_cast<StringLiteral>(ArgExpr) ||
               dyn_cast<FloatingLiteral>(ArgExpr) || dyn_cast<CXXBoolLiteralExpr>(ArgExpr) ||
               dyn_cast<CharacterLiteral>(ArgExpr)) )
            continue;
        
        ParmVarDecl *ParamDecl = CalleeDecl->parameters()[idx];

        FullSourceLoc ParamLoc = Context->getFullLoc(ParamDecl->getBeginLoc());
        FullSourceLoc ArgLoc = Context->getFullLoc(ArgExpr->getBeginLoc());

        if (ParamLoc.isValid() && !ParamDecl->getDeclName().isEmpty() && CommentedLocations.insert(ArgLoc).second) {
            LACRewriter.InsertText(ArgLoc, 
                                   (Twine("/*") + ParamDecl->getDeclName().getAsString() + "=*/").str());
        }

    } 
}

void LACommenterMatcher::onEndOfTranslationUnit() {
    LACRewriter.getEditBuffer(LACRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
}


LACommenterASTConsumer::LACommenterASTConsumer(Rewriter &R) : LACHandler(R) {
    StatementMatcher CallsMatcher = 
        callExpr(
            allOf(
                callee(functionDecl( unless(isVariadic()) ).bind("callee")),
                unless(cxxMemberCallExpr(on(hasType(substTemplateTypeParmType()))) ),
                anyOf(
                    hasAnyArgument(ignoringParenCasts(cxxBoolLiteral())),
                    hasAnyArgument(ignoringParenCasts(stringLiteral())),
                    hasAnyArgument(ignoringParenCasts(integerLiteral())),
                    hasAnyArgument(ignoringParenCasts(floatLiteral())),
                    hasAnyArgument(ignoringParenCasts(characterLiteral()))
                )
            )
        ).bind("caller");

    Finder.addMatcher(CallsMatcher, &LACHandler);
}


static FrontendPluginRegistry::Add<LACPluginAction> X(/*Name=*/"args-commenter", /*Desc=*/"Literal Argument Commenter");
