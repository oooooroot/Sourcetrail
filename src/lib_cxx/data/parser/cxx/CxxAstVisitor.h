#ifndef CXX_AST_VISITOR_H
#define CXX_AST_VISITOR_H

#include <memory>

#include <clang/AST/RecursiveASTVisitor.h>

#include "data/parser/cxx/CxxContext.h"
#include "utility/messaging/MessageInterruptTasksCounter.h"
#include "utility/Cache.h"

class ParserClient;
struct ParseLocation;
class FileRegister;

class CxxAstVisitorComponent;
class CxxAstVisitorComponentContext;
class CxxAstVisitorComponentDeclRefKind;
class CxxAstVisitorComponentTypeRefKind;
class CxxAstVisitorComponentIndexer;

// methods are called in this order:
//	TraverseDecl()
//	`-	TraverseFunctionDecl()
// 		|-	WalkUpFromFunctionDecl()
// 		|	|-	WalkUpFromNamedDecl()
// 		|	|	|-	WalkUpFromDecl()
//	 	|	|	|	`-	VisitDecl()
// 		|	|	`-	VisitNamedDecl()
// 		|	`-	VisitFunctionDecl()
// 		`-	TraverseChildNodes()

class CxxAstVisitor: public clang::RecursiveASTVisitor<CxxAstVisitor>
{
public:
	CxxAstVisitor(clang::ASTContext* astContext, clang::Preprocessor* preprocessor, ParserClient* client, FileRegister* fileRegister);
	virtual ~CxxAstVisitor();

	template <typename T>
	std::shared_ptr<T> getComponent();

	std::shared_ptr<DeclNameCache> getDeclNameCache();
	std::shared_ptr<TypeNameCache> getTypeNameCache();

	// Indexing entry point
    void indexDecl(clang::Decl *d);

	// Visitor options
	virtual bool shouldVisitTemplateInstantiations() const;
	virtual bool shouldVisitImplicitCode() const;

	bool checkIgnoresTypeLoc(const clang::TypeLoc& tl) const;

	// Traversal methods. These specify how to traverse the AST and record context info.
	virtual bool TraverseDecl(clang::Decl *d);
	virtual bool TraverseQualifiedTypeLoc(clang::QualifiedTypeLoc tl);
	virtual bool TraverseTypeLoc(clang::TypeLoc tl);
	virtual bool TraverseType(clang::QualType t);
	virtual bool TraverseStmt(clang::Stmt *stmt);

	virtual bool TraverseCXXRecordDecl(clang::CXXRecordDecl* d);
	bool traverseCXXBaseSpecifier(const clang::CXXBaseSpecifier& d);
	virtual bool TraverseTemplateTypeParmDecl(clang::TemplateTypeParmDecl* d);
	virtual bool TraverseTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl* d);
	virtual bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc loc);
	virtual bool TraverseConstructorInitializer(clang::CXXCtorInitializer* init);
	virtual bool TraverseCallExpr(clang::CallExpr* s);
	virtual bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr* s);
	virtual bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr* s);
	virtual bool TraverseCXXConstructExpr(clang::CXXConstructExpr* s);
	virtual bool TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr* s);
	virtual bool TraverseLambdaExpr(clang::LambdaExpr* s);
	virtual bool TraverseFunctionDecl(clang::FunctionDecl* d);
	virtual bool TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *d);
	virtual bool TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl* d);
	virtual bool TraverseDeclRefExpr(clang::DeclRefExpr* s);
	virtual bool TraverseTemplateSpecializationTypeLoc(clang::TemplateSpecializationTypeLoc loc);
	virtual bool TraverseUnresolvedLookupExpr(clang::UnresolvedLookupExpr* s);
	virtual bool TraverseTemplateArgumentLoc(const clang::TemplateArgumentLoc& loc);
	virtual bool TraverseLambdaCapture(clang::LambdaExpr* lambdaExpr, const clang::LambdaCapture* capture);
	virtual bool TraverseBinComma(clang::BinaryOperator* s);

#define OPERATOR(NAME) virtual bool TraverseBin##NAME##Assign(clang::CompoundAssignOperator *s) { return TraverseAssignCommon(s); }
	OPERATOR(Mul) OPERATOR(Div) OPERATOR(Rem) OPERATOR(Add) OPERATOR(Sub)
	OPERATOR(Shl) OPERATOR(Shr) OPERATOR(And) OPERATOR(Or)  OPERATOR(Xor)
#undef OPERATOR


	void traverseDeclContextHelper(clang::DeclContext* d);
	bool TraverseCallCommon(clang::CallExpr* s);
	bool TraverseAssignCommon(clang::BinaryOperator* s);

	// Visitor methods. These actually record stuff and store it in the database.
	virtual bool VisitCastExpr(clang::CastExpr* s);
	virtual bool VisitUnaryAddrOf(clang::UnaryOperator* s);
	virtual bool VisitUnaryDeref(clang::UnaryOperator* s);
	virtual bool VisitDeclStmt(clang::DeclStmt* s);
	virtual bool VisitReturnStmt(clang::ReturnStmt* s);
	virtual bool VisitInitListExpr(clang::InitListExpr* s);


	virtual bool VisitTagDecl(clang::TagDecl* d);
	virtual bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* d);
	virtual bool VisitFunctionDecl(clang::FunctionDecl* d);
	virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl* d);
	virtual bool VisitVarDecl(clang::VarDecl* d);
	virtual bool VisitFieldDecl(clang::FieldDecl* d);
	virtual bool VisitTypedefDecl(clang::TypedefDecl* d);
	virtual bool VisitTypeAliasDecl(clang::TypeAliasDecl* d);
	virtual bool VisitNamespaceDecl(clang::NamespaceDecl* d);
	virtual bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* d);
	virtual bool VisitEnumConstantDecl(clang::EnumConstantDecl* d);
	virtual bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* d);
	virtual bool VisitUsingDecl(clang::UsingDecl* d);
	virtual bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* d);
	virtual bool VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl* d);
	virtual bool VisitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl* d);

	virtual bool VisitTypeLoc(clang::TypeLoc tl);

	virtual bool VisitDeclRefExpr(clang::DeclRefExpr* s);
	virtual bool VisitMemberExpr(clang::MemberExpr* s);
	virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr* s);
	virtual bool VisitLambdaExpr(clang::LambdaExpr* s);
	virtual bool VisitConstructorInitializer(clang::CXXCtorInitializer* init);

	ParseLocation getParseLocationOfTagDeclBody(clang::TagDecl* decl) const;
	ParseLocation getParseLocationOfFunctionBody(const clang::FunctionDecl* decl) const;
	ParseLocation getParseLocation(const clang::SourceLocation& loc) const;
	ParseLocation getParseLocation(const clang::SourceRange& sourceRange) const;

private:
	typedef clang::RecursiveASTVisitor<CxxAstVisitor> Base;

	clang::ASTContext* m_astContext;
	clang::Preprocessor* m_preprocessor;
	ParserClient* m_client;
	FileRegister* m_fileRegister;

	MessageInterruptTasksCounter m_interruptCounter;

	std::vector<std::shared_ptr<CxxAstVisitorComponent>> m_components;
	std::shared_ptr<CxxAstVisitorComponentContext> m_contextComponent;
	std::shared_ptr<CxxAstVisitorComponentDeclRefKind> m_declRefKindComponent;
	std::shared_ptr<CxxAstVisitorComponentTypeRefKind> m_typeRefKindComponent;
	std::shared_ptr<CxxAstVisitorComponentIndexer> m_indexerComponent;

	std::shared_ptr<DeclNameCache> m_declNameCache;
	std::shared_ptr<TypeNameCache> m_typeNameCache;
};

template <>
std::shared_ptr<CxxAstVisitorComponentContext> CxxAstVisitor::getComponent();

template <>
std::shared_ptr<CxxAstVisitorComponentTypeRefKind> CxxAstVisitor::getComponent();

template <>
std::shared_ptr<CxxAstVisitorComponentDeclRefKind> CxxAstVisitor::getComponent();

template <>
std::shared_ptr<CxxAstVisitorComponentIndexer> CxxAstVisitor::getComponent();

#endif // CXX_AST_VISITOR_H