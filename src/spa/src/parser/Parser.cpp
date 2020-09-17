#include <string>
#include <stack>
#include <unordered_map>
#include <variant>
#include "Combinator.h"
#include "AST.h"
#include "Lexer.h"
#include "Visitor.h"
#include "common/RelationId.h"
#include <SPAAssert.h>

namespace parser {

using namespace common;

std::string stripComments(const std::string& src) {
  return std::regex_replace(src, std::regex(R"((//|\\\\)(.|\r)*?(?=\r\n|\n|$))"), "");
}

bool isOp(const std::string& token) {
  return token == "+" || token == "-" || token == "*" || token == "/" || token == "%";
}

Expr exprListToTree(const std::vector<std::string>& tokens) {
  std::vector<std::string> rpn;
  std::stack<std::string> ops;
  std::unordered_map<std::string,int> prec { {"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"%", 2} };
  for (const auto& token : tokens) {
    if (isOp(token)) {
      while (!ops.empty() && isOp(ops.top()) && prec[ops.top()] >= prec[token]) {
        rpn.emplace_back(ops.top());
        ops.pop();
      }
      ops.push(token);
    } else if (token == "(") {
      ops.push(token);
    } else if (token == ")") {
      while (!ops.empty() && ops.top() != "(") {
        rpn.emplace_back(ops.top());
        ops.pop();
      }
      SPA_ASSERT(ops.top() == "(");
      ops.pop();
    } else {
      rpn.emplace_back(token);
    }
  }
  while (!ops.empty()) {
    SPA_ASSERT(ops.top() != "(");
    rpn.emplace_back(ops.top());
    ops.pop();
  }
  std::stack<Expr> expr;
  for (std::string& token : rpn) {
    if (isOp(token)) {
      Expr right = expr.top(); expr.pop();
      Expr left = expr.top(); expr.pop();
      if (token == "+") {
        expr.push(Add(left, right));
      } else if (token == "-") {
        expr.push(Sub(left, right));
      } else if (token == "*") {
        expr.push(Mul(left, right));
      } else if (token == "/") {
        expr.push(Div(left, right));
      } else if (token == "%") {
        expr.push(Mod(left, right));
      } else {
        SPA_ASSERT(false);
      }
    } else if (isdigit(token[0])) {
      expr.push(Const(token));
    } else {
      expr.push(Var(token));
    }
  }
  return expr.top();
}

// shared parsers
auto identifier_p = RegexParser("[a-zA-Z]\\w*");
auto integer_p =
  Fmap(RegexParser("\\d+"), [](auto a) {
    a.erase(0, a.find_first_not_of('0'));
    if (a.empty()) return Int{"0"};
    return Int{a};
  });
auto expr_p = [](){
  auto strToVec = [](const std::string& s){ return std::vector<std::string>{s}; };
  auto intToVec = [](const Int& i){ return std::vector<std::string>{i.value}; };
  // join :: ([string], [string]) -> [string]
  auto join = [](const std::tuple<std::vector<std::string>,std::vector<std::string>>& params){
    auto [a, b] = params;
    a.insert(a.end(), b.begin(), b.end());
    return a;
  };
  // joinMany :: [[string]] -> [string]
  auto joinMany = [](const std::vector<std::vector<std::string>>& v){
    std::vector<std::string> a;
    for (auto b : v) a.insert(a.end(), b.begin(), b.end());
    return a;
  };
  auto expr_flat_p = LazyParser<std::vector<std::string>>();
  auto bracketed_term_p =
    Fmap(Then(StringParser("("), expr_flat_p.get(), StringParser(")")),
      [](auto params){
        auto& [open, tokens, close] = params;
        tokens.insert(tokens.begin(), open);
        tokens.push_back(close);
        return tokens;
      });
  auto term_p = Alt(bracketed_term_p, Fmap(identifier_p, strToVec), Fmap(integer_p, intToVec));
  auto op_p = Fmap(Alt(StringParser("+"), StringParser("-"), StringParser("*"), StringParser("/"), StringParser("%")), strToVec);
  auto expr_string_impl = Fmap(Then(term_p, Fmap(Many(Fmap(Then(op_p, term_p), join)), joinMany)), join);
  expr_flat_p.define(expr_string_impl);
  auto expr_p = Fmap(expr_flat_p.get(), exprListToTree);
  return expr_p;
}();

ParseResult<Program> parseSimple(const std::string& src) {
  auto cond_expr_p = LazyParser<CondExpr>();
  auto bracketed_cond_expr_p = KeepRight(StringParser("("), KeepLeft(cond_expr_p.get(), StringParser(")")));
  auto gt_p = Fmap(Then(KeepLeft(expr_p, StringParser(">")), expr_p),
      [](auto params) -> RelExpr { auto [a, b] = params; return Gt(a, b); });
  auto lt_p = Fmap(Then(KeepLeft(expr_p, StringParser("<")), expr_p),
      [](auto params) -> RelExpr { auto [a, b] = params; return Lt(a, b); });
  auto gte_p = Fmap(Then(KeepLeft(expr_p, StringParser(">=")), expr_p),
      [](auto params) -> RelExpr { auto [a, b] = params; return Gte(a, b); });
  auto lte_p = Fmap(Then(KeepLeft(expr_p, StringParser("<=")), expr_p),
      [](auto params) -> RelExpr { auto [a, b] = params; return Lte(a, b); });
  auto eq_p = Fmap(Then(KeepLeft(expr_p, StringParser("==")), expr_p),
      [](auto params) -> RelExpr { auto [a, b] = params; return Eq(a, b); });
  auto neq_p = Fmap(Then(KeepLeft(expr_p, StringParser("!=")), expr_p),
      [](auto params) -> RelExpr { auto [a, b] = params; return Neq(a, b); });
  auto rel_expr_p = Fmap(Alt(gt_p, lt_p, gte_p, lte_p, eq_p, neq_p),
      [](auto a) -> CondExpr { return a; });
  auto not_p = Fmap(KeepRight(StringParser("!"), bracketed_cond_expr_p),
      [](auto a) -> CondExpr { return Not(a); });
  auto and_p = Fmap(Then(bracketed_cond_expr_p, StringParser("&&"), bracketed_cond_expr_p),
      [](auto params) -> CondExpr { return And(std::get<0>(params), std::get<2>(params)); });
  auto or_p = Fmap(Then(bracketed_cond_expr_p, StringParser("||"), bracketed_cond_expr_p),
      [](auto params) -> CondExpr { return Or(std::get<0>(params), std::get<2>(params)); });
  auto cond_expr_impl = Alt(rel_expr_p, not_p, and_p, or_p);
  cond_expr_p.define(cond_expr_impl);

  auto stmt_p = LazyParser<Stmt>();
  auto stmtLst_p = AtLeastOne(stmt_p.get());
  auto while_p =
    Fmap(Then(StringParser("while"), StringParser("("), cond_expr_p.get(), StringParser(")"),
              StringParser("{"), stmtLst_p, StringParser("}")),
         [](auto params) -> Stmt { return While(std::get<2>(params), std::get<5>(params)); });
  auto if_p =
    Fmap(Then(StringParser("if"), StringParser("("), cond_expr_p.get(), StringParser(")"),
              StringParser("then"), StringParser("{"), stmtLst_p, StringParser("}"),
              StringParser("else"), StringParser("{"), stmtLst_p, StringParser("}")),
         [](auto params) -> Stmt { return If(std::get<2>(params), std::get<6>(params), std::get<10>(params)); });
  auto assign_p = Fmap(Then(identifier_p, StringParser("="), expr_p, StringParser(";")),
      [](auto params) -> Stmt { return Assign(Var(std::get<0>(params)), std::get<2>(params)); });
  auto read_p = Fmap(KeepLeft(KeepRight(StringParser("read"), identifier_p), StringParser(";")),
      [](auto a) -> Stmt { return Read(Var(a)); });
  auto print_p = Fmap(KeepLeft(KeepRight(StringParser("print"), identifier_p), StringParser(";")),
      [](auto a) -> Stmt { return Print(Var(a)); });
  auto call_p = Fmap(KeepLeft(KeepRight(StringParser("call"), identifier_p), StringParser(";")),
      [](auto a) -> Stmt { return Call(Proc(a)); });
  auto stmt_impl = Alt(while_p, if_p, assign_p, print_p, call_p, read_p);
  stmt_p.define(stmt_impl);

  auto proc_def_p = Fmap(Then(StringParser("procedure"), identifier_p, StringParser("{"), stmtLst_p, StringParser("}")),
      [](auto params){ return ProcDef(std::get<1>(params), std::get<3>(params)); });
  auto program_p = AtLeastOne(proc_def_p);

  std::string preprocessed_src = stripComments(src);

  auto tokens = tokenizeSimple(preprocessed_src);
  return fullyMatched(program_p.parse(tokens));
}

ParseResult<Query> parseQuery(const std::string& src) {
  auto synonym_p = Fmap(identifier_p, [](auto a) -> Synonym { return a; });
  auto design_entity_p =
    Alt(StringParser("stmt"), StringParser("read"), StringParser("print"),
        StringParser("call"), StringParser("while"), StringParser("if"), StringParser("assign"),
        StringParser("variable"), StringParser("constant"), StringParser("prog_line"), StringParser("procedure"));
  auto declarations_p =
    Fmap(Then(design_entity_p, AtLeastOneSepBy(synonym_p, StringParser(",")), StringParser(";")),
      [](auto params) {
        std::vector<Declaration> ret;
        for (const auto& synonym : std::get<1>(params)) {
          ret.emplace_back(Declaration{TYPE_STRING_ID.at(std::get<0>(params)), synonym});
        }
        return ret;
      });

  auto attr_name_p =
    Alt(
      Fmap(StringParser("procName"), [](auto){ return AttrNameId::ProcName; }),
      Fmap(StringParser("varName"), [](auto){ return AttrNameId::VarName; }),
      Fmap(StringParser("value"), [](auto){ return AttrNameId::Value; }),
      Fmap(StringParser("stmt#"), [](auto){ return AttrNameId::StmtNo; })
    );
  auto attr_ref_p =
    Fmap(Then(synonym_p, Maybe(KeepRight(StringParser("."), attr_name_p))),
      [](auto params) { auto& [a, b] = params; return AttrRef(a, b); });
  auto one_attr_ref_p = Fmap(attr_ref_p, [](auto a){ return std::vector<AttrRef>{a}; });
  auto many_attr_ref_p = KeepRight(StringParser("<"), KeepLeft(AtLeastOneSepBy(attr_ref_p, StringParser(",")), StringParser(">")));
  auto tuple_p = Fmap(Alt(one_attr_ref_p, many_attr_ref_p), [](auto a) -> Result { return a; });
  auto boolean_p = Fmap(StringParser("BOOLEAN"), [](auto) -> Result { return Bool{}; });
  auto result_p = Alt(boolean_p, tuple_p);

  auto stmt_ref_p =
    Alt(
      Fmap(synonym_p, [](auto a) -> StmtRef { return a; }),
      Fmap(StringParser("_"), [](auto) -> StmtRef { return Wildcard{}; }),
      Fmap(integer_p, [](auto a) -> StmtRef { return a; })
    );
  auto ent_ref_p =
    Alt(
      Fmap(synonym_p, [](auto a) -> EntRef { return a; }),
      Fmap(StringParser("_"), [](auto) -> EntRef { return Wildcard{}; }),
      Fmap(KeepRight(StringParser("\""), KeepLeft(identifier_p, StringParser("\""))), [](auto a) -> EntRef { return Var(a); })
    );
  auto stmt_stmt_p =
    Fmap(Then(StringParser("("), stmt_ref_p, StringParser(","), stmt_ref_p, StringParser(")")),
         [](auto params) { return make_pair(std::get<1>(params), std::get<3>(params)); });
  auto stmt_ent_p =
    Fmap(Then(StringParser("("), stmt_ref_p, StringParser(","), ent_ref_p, StringParser(")")),
         [](auto params) { return make_pair(std::get<1>(params), std::get<3>(params)); });
  auto ent_ent_p =
    Fmap(Then(StringParser("("), ent_ref_p, StringParser(","), ent_ref_p, StringParser(")")),
         [](auto params) { return make_pair(std::get<1>(params), std::get<3>(params)); });
  auto follows_p =
    Fmap(KeepRight(StringParser("Follows"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::Follows, params.first, params.second }; });
  auto follows_t_p =
    Fmap(KeepRight(StringParser("Follows*"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::FollowsT, params.first, params.second }; });
  auto parent_p =
    Fmap(KeepRight(StringParser("Parent"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::Parent, params.first, params.second }; });
  auto parent_t_p =
    Fmap(KeepRight(StringParser("Parent*"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::ParentT, params.first, params.second }; });
  auto uses_s_p =
    Fmap(KeepRight(StringParser("Uses"), stmt_ent_p),
         [](auto params) -> SuchThat { return StmtEntEntRelation{ RelationId::Uses, {params.first}, params.second }; });
  auto uses_p_p =
    Fmap(KeepRight(StringParser("Uses"), ent_ent_p),
         [](auto params) -> SuchThat { return StmtEntEntRelation{ RelationId::Uses, {params.first}, params.second }; });
  auto modifies_s_p =
    Fmap(KeepRight(StringParser("Modifies"), stmt_ent_p),
         [](auto params) -> SuchThat { return StmtEntEntRelation{ RelationId::Modifies, {params.first}, params.second }; });
  auto modifies_p_p =
    Fmap(KeepRight(StringParser("Modifies"), ent_ent_p),
         [](auto params) -> SuchThat { return StmtEntEntRelation{ RelationId::Modifies, {params.first}, params.second }; });
  auto calls_p =
    Fmap(KeepRight(StringParser("Calls"), ent_ent_p),
         [](auto params) -> SuchThat { return EntEntRelation{ RelationId::Calls, params.first, params.second }; });
  auto calls_t_p =
    Fmap(KeepRight(StringParser("Calls*"), ent_ent_p),
         [](auto params) -> SuchThat { return EntEntRelation{ RelationId::CallsT, params.first, params.second }; });
  auto next_p =
    Fmap(KeepRight(StringParser("Next"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::Next, params.first, params.second }; });
  auto next_t_p =
    Fmap(KeepRight(StringParser("Next*"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::NextT, params.first, params.second }; });
  auto affects_p =
    Fmap(KeepRight(StringParser("Affects"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::Affects, params.first, params.second }; });
  auto affects_t_p =
    Fmap(KeepRight(StringParser("Affects*"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::AffectsT, params.first, params.second }; });
  auto next_bip_p =
    Fmap(KeepRight(StringParser("NextBip"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::NextBip, params.first, params.second }; });
  auto next_bip_t_p =
    Fmap(KeepRight(StringParser("NextBip*"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::NextBipT, params.first, params.second }; });
  auto affects_bip_p =
    Fmap(KeepRight(StringParser("AffectsBip"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::AffectsBip, params.first, params.second }; });
  auto affects_bip_t_p =
    Fmap(KeepRight(StringParser("AffectsBip*"), stmt_stmt_p),
         [](auto params) -> SuchThat { return StmtStmtRelation{ RelationId::AffectsBipT, params.first, params.second }; });

  auto relation_p = Fmap(Alt(follows_p, follows_t_p, parent_p, parent_t_p, uses_s_p, uses_p_p,
                             modifies_s_p, modifies_p_p, calls_p, calls_t_p, next_p, next_t_p,
                             affects_p, affects_t_p, next_bip_p, next_bip_t_p, affects_bip_p, affects_bip_t_p),
                         [](auto a) -> Clause { return a; });
  auto such_that_cl_p = KeepRight(Then(StringParser("such"), StringParser("that")), AtLeastOneSepBy(relation_p, StringParser("and")));

  auto exact_match_p = Fmap(Then(StringParser("\""), expr_p, StringParser("\"")),
                            [](auto params) -> PatternRHS { return ExactMatch{std::get<1>(params)}; });
  auto partial_match_p = Fmap(Then(StringParser("_\""), expr_p, StringParser("\"_")),
                              [](auto params) -> PatternRHS { return PartialMatch{std::get<1>(params)}; });
  auto expr_spec_p =
    Alt(
      exact_match_p,
      partial_match_p,
      Fmap(StringParser("_"), [](auto) -> PatternRHS { return Wildcard{}; })
    );
  auto binary_pattern_p =
    Fmap(Then(synonym_p, StringParser("("), ent_ref_p, StringParser(","), expr_spec_p, StringParser(")")),
         [](auto params){ return Clause{Pattern{BinaryPattern{ std::get<0>(params), std::get<2>(params), std::get<4>(params) }}}; });
  auto ternary_pattern_p =
    Fmap(Then(synonym_p, StringParser("("), ent_ref_p, StringParser(","), StringParser("_"), StringParser(","), StringParser("_"), StringParser(")")),
         [](auto params){ return Clause{Pattern{TernaryPattern{ std::get<0>(params), std::get<2>(params) }}}; });
  auto pattern_p =
    Fmap(Alt(binary_pattern_p, ternary_pattern_p),
         [](auto a) -> Clause { return a; });
  auto pattern_cl_p = KeepRight(StringParser("pattern"), AtLeastOneSepBy(pattern_p, StringParser("and")));

  auto ref_p =
    Alt(
      Fmap(KeepRight(StringParser("\""), KeepLeft(identifier_p, StringParser("\""))), [](auto a) -> Ref { return a; }),
      Fmap(integer_p, [](auto a) -> Ref { return a; }),
      Fmap(attr_ref_p, [](auto a) -> Ref { return a; }),
      Fmap(synonym_p, [](auto a) -> Ref { return AttrRef(a); })
    );
  auto with_p =
    Fmap(Then(ref_p, StringParser("="), ref_p),
         [](auto params){ return Clause{std::pair<Ref,Ref>{ std::get<0>(params), std::get<2>(params) }}; });
  auto with_cl_p = KeepRight(StringParser("with"), AtLeastOneSepBy(with_p, StringParser("and")));

  auto clauses_p = Alt(such_that_cl_p, pattern_cl_p, with_cl_p);

  auto query_p = Fmap(Then(Many(declarations_p), StringParser("Select"), result_p, Many(clauses_p)),
    [](auto params){
      Query q;
      auto r = std::get<2>(params);
      if (std::holds_alternative<Bool>(r)) {
        q.isBooleanResult = true;
        q.result = {};
      } else {
        q.isBooleanResult = false;
        q.result = std::get<std::vector<AttrRef>>(r);
      }
      for (const auto& declarations : std::get<0>(params)) {
        q.declarations.insert(q.declarations.end(), declarations.begin(), declarations.end());
      }
      for (const auto& clauses : std::get<3>(params)) {
        for (const auto& clause : clauses) {
          std::visit(visitor {
            [&](SuchThat s){ q.suchThats.emplace_back(s); },
            [&](Pattern p){ q.patterns.emplace_back(p); },
            [&](With w){ q.withs.emplace_back(w); }
          }, clause);
        }
      }
      return q;
    });

  auto tokens = tokenizeQuery(src);
  return fullyMatched(query_p.parse(tokens));
}

}
