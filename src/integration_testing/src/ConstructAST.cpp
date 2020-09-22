#include <ConstructAST.h>
using namespace common;

Program buildTestCase() {
  // procedure main {
  //   flag = 0                     #1
  //   call computeCentroid;        #2
  //   call printResults;           #3
  // }
  // procedure readPoint {
  //   read x;                      #4
  //   read y;                      #5
  // }
  // procedure printResults {
  //   print flag;                  #6
  //   print cenX;                  #7
  //   print cenY;                  #8
  //   print normSq;                #9
  // }
  // procedure computeCentroid {
  //   count = 0;                   #10
  //   cenX = 0;                    #11
  //   cenY = 0;                    #12
  //   call readPoint;              #13
  //   while((x!=0) && (y!=0)) {    #14
  //     count = count + 1;         #15
  //     cenX = cenX + x;           #16
  //     cenY = cenY + y;           #17
  //     call readPoint;            #18
  //   }
  //   if (count == 0) then {       #19
  //     flag = 1;                  #20
  //   } else {
  //      cenX = cenX/count;        #21
  //      cenY = cenY/ count;       #22
  //   }
  //   normSq = cenX* cenX + cenY * cenY;  #23
  // }
  ///////////////////////////////////////////////
  // Constant declaration
  Const cZero{"0"};
  Const cOne{"1"};
  // Variable declaration
  Var vFlag{"flag"};
  Var vX{"x"};
  Var vY{"y"};
  Var vCenX{"cenX"};
  Var vCenY{"cenY"};
  Var vNormSq{"normSq"};
  Var vCount{"count"};
  // Procedure Names
  std::string pReadPoint{"readPoint"};
  std::string pComputeCentroid{"computeCentroid"};
  std::string pPrintResults{"printResults"};
  std::string pMain{"main"};
  // Expressions
  Expr zero = cZero;
  Expr one = cOne;
  Expr flag = vFlag;
  Expr x = vX;
  Expr y = vY;
  Expr cenX = vCenX;
  Expr cenY = vCenY;
  Expr normSq = vNormSq;
  Expr count = vCount;


  CondExpr whileCondExpr;


  StmtLst statementsMain;
  ASTBuilder(statementsMain)
      .Assign("flag", zero)
      .Call("computeCentroid")
      .Call("printResults");
  ProcDef procedureMain(pMain, statementsMain);

  StmtLst statementsReadPoint;
  ASTBuilder(statementsReadPoint)
      .Read("x")
      .Read("y");
  ProcDef procedureReadPoint(pReadPoint, statementsReadPoint);

  StmtLst statementsPrintResults;
  ASTBuilder(statementsPrintResults)
      .Print("flag")
      .Print("cenX")
      .Print("cenY")
      .Print("normSq");
  ProcDef procedurePrintResults(pPrintResults, statementsPrintResults);

  // 14
  Neq neq1;
  neq1.left = std::shared_ptr<Expr>(new Expr);
  *neq1.left = x;
  neq1.right = std::shared_ptr<Expr>(new Expr);
  *neq1.right = zero;
  Neq neq2;
  neq2.left = std::shared_ptr<Expr>(new Expr);
  *neq2.left = y;
  neq2.right = std::shared_ptr<Expr>(new Expr);
  *neq2.right = zero;
  And andExpr;
  andExpr.left = std::shared_ptr<CondExpr>(new CondExpr);
  *andExpr.left = neq1;
  andExpr.right = std::shared_ptr<CondExpr>(new CondExpr);
  *andExpr.right = neq2;
  CondExpr condExpr14 = andExpr;
  // 15
  Add plus15;
  plus15.left = std::shared_ptr<Expr>(new Expr);
  *plus15.left = count;
  plus15.right = std::shared_ptr<Expr>(new Expr);
  *plus15.right = one;
  Expr expr15 = plus15;
  // 16
  Add plus16;
  plus16.left = std::shared_ptr<Expr>(new Expr);
  *plus16.left = cenX;
  plus16.right = std::shared_ptr<Expr>(new Expr);
  *plus16.right = x;
  Expr expr16 = plus16;
  // 17
  Add plus17;
  plus17.left = std::shared_ptr<Expr>(new Expr);
  *plus17.left = cenY;
  plus17.right = std::shared_ptr<Expr>(new Expr);
  *plus17.right = y;
  Expr expr17 = plus17;
  // 18, omitted
  // 19
  Eq eq19;
  eq19.left = std::shared_ptr<Expr>(new Expr);
  *eq19.left = count;
  eq19.right = std::shared_ptr<Expr>(new Expr);
  *eq19.right = zero;
  CondExpr condExpr19 = eq19;
  // 21
  Div div21;
  div21.left = std::shared_ptr<Expr>(new Expr);
  *div21.left = cenX;
  div21.right = std::shared_ptr<Expr>(new Expr);
  *div21.right = count;
  Expr expr21 = div21;
  // 22
  Div div22;
  div22.left = std::shared_ptr<Expr>(new Expr);
  *div22.left = cenY;
  div22.right = std::shared_ptr<Expr>(new Expr);
  *div22.right = count;
  Expr expr22 = div22;
  // 23
  Mul cenXSquared;
  cenXSquared.left = std::shared_ptr<Expr>(new Expr);
  *cenXSquared.left = cenX;
  cenXSquared.right = std::shared_ptr<Expr>(new Expr);
  *cenXSquared.right = cenX;
  Expr expr23_1 = cenXSquared;
  Mul cenYSquared;
  cenYSquared.left = std::shared_ptr<Expr>(new Expr);
  *cenYSquared.left = cenY;
  cenYSquared.right = std::shared_ptr<Expr>(new Expr);
  *cenYSquared.right = cenY;
  Expr expr23_2 = cenYSquared;
  Add plus23;
  plus23.left = std::shared_ptr<Expr>(new Expr);
  *plus23.left = expr23_1;
  plus23.right = std::shared_ptr<Expr>(new Expr);
  *plus23.right = expr23_2;
  Expr expr23 = plus23;

  StmtLst statementsWhile;
  ASTBuilder(statementsWhile)
      .Assign("count", expr15)
      .Assign("cenX", expr16)
      .Assign("cenY", expr17)
      .Call("readPoint");

  StmtLst statementsIfTrue;
  ASTBuilder(statementsIfTrue)
      .Assign("flag", one);
  StmtLst statementsIfFalse;
  ASTBuilder(statementsIfFalse)
      .Assign("cenX", expr21)
      .Assign("cenY", expr22);

  StmtLst statementsComputeCentroid;
  ASTBuilder(statementsComputeCentroid)
      .Assign("count", zero)
      .Assign("cenX", zero)
      .Assign("cenY", zero)
      .Call("readPoint")
      .While(condExpr14, statementsWhile)
      .If(condExpr19, statementsIfTrue, statementsIfFalse)
      .Assign("normSq", expr23);

  ProcDef procedureComputeCentroid(pComputeCentroid, statementsComputeCentroid);
  Program prog{procedureMain,
                procedureReadPoint,
                procedurePrintResults,
                procedureComputeCentroid};
  return prog;
}

Program buildTestCase2() {
  // procedure procA {
  //   read a;               #1
  //   a = a;                #2
  //   call procB;           #3
  //   print a;              #4
  // }
  // procedure procB {
  //   c = 10;               #5
  //   if (a > 100) {        #6
  //     b = 3;              #7
  //     a = a * b;          #8
  //   } else {
  //     while ( a <= 100) { #9
  //       a = a + c;        #10
  //       if (a < 20) {     #11
  //          c = c - b;     #12
  //       } else {
  //          c = c + b;     #13
  //       }
  //     }
  //   }
  // }
  StmtLst stmtsA;
  Const threeC("3");
  Const tenC("10");
  Const hundredC("100");
  Const twentyC("20");
  Expr three = threeC;
  Expr ten = tenC;
  Expr hundred = hundredC;
  Expr twenty = twentyC;
  Var aN{"a"};
  Var bN{"b"};
  Var cN{"c"};
  Expr a = aN;
  Expr b = bN;
  Expr c = cN;
  ASTBuilder(stmtsA).Read("a")
                      .Assign("a", a)
                      .Call("procB")
                      .Print("a");
  ProcDef procedureA("procA", stmtsA);
  Gt gtExpr;
  gtExpr.left = std::shared_ptr<Expr>(new Expr);
  gtExpr.right = std::shared_ptr<Expr>(new Expr);
  *gtExpr.left = a;
  *gtExpr.right = hundred;
  CondExpr ifCondExpr = gtExpr;
  Lte lteExpr;
  lteExpr.left = std::shared_ptr<Expr>(new Expr);
  lteExpr.right = std::shared_ptr<Expr>(new Expr);
  *lteExpr.left = a;
  *lteExpr.right = hundred;
  CondExpr innerIfCondExpr = lteExpr;
  Lt ltExpr;
  ltExpr.left = std::shared_ptr<Expr>(new Expr);
  ltExpr.right = std::shared_ptr<Expr>(new Expr);
  *ltExpr.left = a;
  *ltExpr.right = twenty;
  CondExpr whileCondExpr = ltExpr;
  StmtLst ifTrue;
  Mul timesExpr;
  timesExpr.left = std::shared_ptr<Expr>(new Expr(a));
  timesExpr.right = std::shared_ptr<Expr>(new Expr(b));
  Expr aTimesB = timesExpr;
  ASTBuilder(ifTrue).Assign("b", three)
                    .Assign("a", aTimesB);

  Add plusExpr;
  plusExpr.left = std::shared_ptr<Expr>(new Expr(a));
  plusExpr.right = std::shared_ptr<Expr>(new Expr(c));
  Expr aPlusC = plusExpr;

  Sub minusExprIfTrue;
  minusExprIfTrue.left = std::shared_ptr<Expr>(new Expr(c));
  minusExprIfTrue.right = std::shared_ptr<Expr>(new Expr(b));
  Expr cMinusB = minusExprIfTrue;

  Add plusExprIfFalse;
  plusExprIfFalse.left = std::shared_ptr<Expr>(new Expr(c));
  plusExprIfFalse.right = std::shared_ptr<Expr>(new Expr(b));
  Expr cPlusB = plusExprIfFalse;
  StmtLst innerIfTrue;
  ASTBuilder(innerIfTrue).Assign("c", cMinusB);
  StmtLst innerIfFalse;
  ASTBuilder(innerIfFalse).Assign("c", cPlusB);
  StmtLst WhileStatement;
  ASTBuilder(WhileStatement).Assign("a", aPlusC)
                            .If(innerIfCondExpr, innerIfTrue, innerIfFalse);
  StmtLst ifFalse;
  ASTBuilder(ifFalse).While(whileCondExpr, WhileStatement);

  StmtLst stmtsB;
  ASTBuilder(stmtsB).Assign("c", ten)
                    .If(ifCondExpr, ifTrue, ifFalse);
  ProcDef procedureB("procB", stmtsB);
  Program prog{procedureA, procedureB};
  return prog;
}

Program buildTestCase3() {
  // procedure a {
  //   while( 1 == 1) { #1
  //     call b;        #2
  //   }
  // }
  // procedure b {
  //   while( 1==1) {   #3
  //     z = 1;         #4
  //   }
  // }
  // procedure c {
  //   while(1 == 1) {  #5
  //     call a;        #6
  //   }
  // }

  Const cOne{"1"};
  Expr one = cOne;
  Eq eqExpression;
  eqExpression.left = std::shared_ptr<Expr>(new Expr);
  eqExpression.right = std::shared_ptr<Expr>(new Expr);
  *eqExpression.left = one;
  *eqExpression.right = one;
  CondExpr condExpr = eqExpression;
  StmtLst statementWhileA;
  ASTBuilder(statementWhileA)
      .Call("b");
  StmtLst statementA;
  ASTBuilder(statementA)
      .While(condExpr, statementWhileA);
  ProcDef procA("a", statementA);
  StmtLst statementWhileB;
  ASTBuilder(statementWhileB)
      .Assign("z", one);
  StmtLst statementB;
  ASTBuilder(statementB)
      .While(condExpr, statementWhileB);
  ProcDef procB("b", statementB);
  StmtLst statementWhileC;
  ASTBuilder(statementWhileC)
      .Call("a");
  StmtLst statementC;
  ASTBuilder(statementC)
      .While(condExpr, statementWhileC);
  ProcDef procC("c", statementC);
  Program prog({procA, procB, procC});
  return prog;
}

Program buildTestCase4() {
// procedure procedure {
//   call y; #1
// }
// procedure x {
//   while(1 == 2) { #2
//     call y; #3
//   }
// }
// procedure y {
//   if (1 ==2) { #4
//     call z; #5
//   } else {
//     call t; #6
//   }
// }

// procedure z {
//   while(1 == 2) { #7
//     if (1 == 2) { #8
//       while(1 == 2) { #9
//         z = x  + t; #10
//       }
//     } else {
//       x = (y + x); # 11
//     }
//   }
// }
// procedure t{
//   t = z; #12
// }
  Const cOne{"1"};
  Const cTwo{"2"};
  Var nX{"x"};
  Var nY{"y"};
  Var nZ{"z"};
  Var nT{"t"};
  Expr one = cOne;
  Expr two = cTwo;
  Expr x = nX;
  Expr y = nY;
  Expr z = nZ;
  Expr t = nT;

  Eq eqExpression;
  eqExpression.left = std::shared_ptr<Expr>(new Expr);
  eqExpression.right = std::shared_ptr<Expr>(new Expr);
  *eqExpression.left = one;
  *eqExpression.right = two;
  CondExpr condExpr;
  condExpr = eqExpression;
  Add plus10;
  plus10.left = std::shared_ptr<Expr>(new Expr);
  *plus10.left = x;
  plus10.right = std::shared_ptr<Expr>(new Expr);
  *plus10.right = t;
  Expr expr10 = plus10;

  Add plus11;
  plus11.left = std::shared_ptr<Expr>(new Expr);
  *plus11.left = y;
  plus11.right = std::shared_ptr<Expr>(new Expr);
  *plus11.right = x;
  Expr expr11 = plus11;

  StmtLst statementProcedure;
  ASTBuilder(statementProcedure).Call("y");
  ProcDef procProcedure("procedure",statementProcedure);

  StmtLst statementWhileX;
  ASTBuilder(statementWhileX).Call("y");
  StmtLst statementX;
  ASTBuilder(statementX).While(condExpr, statementWhileX);
  ProcDef procX("x", statementX);

  StmtLst statementIfTrueY;
  ASTBuilder(statementIfTrueY).Call("z");
  StmtLst statementIfFalseY;
  ASTBuilder(statementIfFalseY).Call("t");
  StmtLst statementY;
  ASTBuilder(statementY).If(condExpr, statementIfTrueY, statementIfFalseY);
  ProcDef procY("y", statementY);

  StmtLst statementWhileInnerZ;
  ASTBuilder(statementWhileInnerZ).Assign("z", expr10);
  StmtLst statementIfTrueZ;
  ASTBuilder(statementIfTrueZ).While(condExpr, statementWhileInnerZ);
  StmtLst statementIfFalseZ;
  ASTBuilder(statementIfFalseZ).Assign("x", expr11);
  StmtLst statementWhileOuterZ;
  ASTBuilder(statementWhileOuterZ).If(condExpr, statementIfTrueZ,statementIfFalseZ);
  StmtLst statementZ;
  ASTBuilder(statementZ).While(condExpr, statementWhileOuterZ);
  ProcDef procZ("z", statementZ);

  StmtLst statementT;
  ASTBuilder(statementT).Assign("t", z);
  ProcDef procT("t", statementT);
  Program prog{procProcedure, procX, procY, procZ, procT};
  return prog;
}

Program buildTestCase5() {
  // proc A {
  //   call B;            #1
  //   call C;            #2
  // }

  // proc B {
  //   varB = varB + 1;   #3
  //   while(condVarB < 2) {  #4
  //     call D;          #5
  //    }
  // }

  // proc C {
  //   call D;            #6
  //   call E;            #7
  // }

  // proc D {
  //   call F;            #8
  // }

  // proc E {
  //   call F;            #9
  // }

  // proc F {
  //   varF = varF + 1;   #10
  // }
  Const cOne{"1"};
  Const cTwo{"2"};
  Var nVarB{"varB"};
  Var nVarF{"varF"};
  Var nCondVarB{"condVarB"};
  Expr one = cOne;
  Expr two = cTwo;
  Expr varB = nVarB;
  Expr varF = nVarF;
  Expr condVarB = nCondVarB;

  Lte lteExpression;
  lteExpression.left = std::shared_ptr<Expr>(new Expr);
  lteExpression.right = std::shared_ptr<Expr>(new Expr);
  *lteExpression.left = condVarB;
  *lteExpression.right = two;
  CondExpr condExpr;
  condExpr = lteExpression;

  Add varBPlusOne;
  varBPlusOne.left = std::shared_ptr<Expr>(new Expr);
  *varBPlusOne.left = varB;
  varBPlusOne.right = std::shared_ptr<Expr>(new Expr);
  *varBPlusOne.right = one;
  Expr exprVarB = varBPlusOne;

  Add varFPlusOne;
  varFPlusOne.left = std::shared_ptr<Expr>(new Expr);
  *varFPlusOne.left = varF;
  varFPlusOne.right = std::shared_ptr<Expr>(new Expr);
  *varFPlusOne.right = one;
  Expr exprVarF = varFPlusOne;

  StmtLst procAStmtList;
  ASTBuilder(procAStmtList).Call("B").Call("C");
  ProcDef procA("A", procAStmtList);

  StmtLst statementWhile;
  ASTBuilder(statementWhile).Call("D");
  StmtLst procBStmtList;
  ASTBuilder(procBStmtList).Assign("varB", exprVarB).While(condExpr, statementWhile);
  ProcDef procB("B", procBStmtList);

  StmtLst procCStmtList;
  ASTBuilder(procCStmtList).Call("D").Call("E");
  ProcDef procC("C", procCStmtList);

  StmtLst procDStmtList;
  ASTBuilder(procDStmtList).Call("F");
  ProcDef procD("D", procDStmtList);

  StmtLst procEStmtList;
  ASTBuilder(procEStmtList).Call("F");
  ProcDef procE("E", procEStmtList);

  StmtLst procFStmtList;
  ASTBuilder(procFStmtList).Assign("varF", exprVarF);
  ProcDef procF("F", procFStmtList);

  Program prog{procA, procB, procC, procD, procE, procF};
  return prog;
}

Program buildTestCase6() {
    StmtLst stmtLst;
    ASTBuilder(stmtLst).Print("x");
    ProcDef proc("a", stmtLst);
    return {proc};
}

Program buildTestCase7() {
// procedure main {                      #
//     read x;                               #1
//     while (i < 100) {                     #2
//         if (j < 200) {                    #3
//             y = y + 1;                    #4
//         } else {                          #
//             if (k < 300) {                #5
//                 d = x + y + z;            #6
//             } else {                      #
//                 if (m < 400) {            #7
//                     d = x + y + z;        #8
//                 } else {                  #
//                     d = x + y + z;        #9
//                 }                         #
//                 d = x + y + z;            #10
//             }                             #
//             d = x + y + z;                #11
//         }                                 #
//         d = x + y + z;                    #12
//     }                                     #
//     d = x + y + z;                        #13
// }                                         #
  Const c1{"1"};
  Const c100{"100"};
  Const c200{"200"};
  Const c300{"300"};
  Const c400{"400"};
  Var nD{"d"};
  Var nX{"x"};
  Var nY{"y"};
  Var nZ{"z"};
  Var nI{"i"};
  Var nJ{"j"};
  Var nK{"k"};
  Var nM{"m"};
  Expr one = c1;
  Expr one00 = c100;
  Expr two00 = c200;
  Expr three00 = c300;
  Expr four00 = c400;
  Expr d = nD;
  Expr x = nX;
  Expr y = nY;
  Expr z = nZ;
  Expr i = nI;
  Expr j = nJ;
  Expr k = nK;
  Expr m = nM;

  Lte lteExpression1;
  lteExpression1.left = std::shared_ptr<Expr>(new Expr);
  lteExpression1.right = std::shared_ptr<Expr>(new Expr);
  *lteExpression1.left = i;
  *lteExpression1.right = one00;
  CondExpr condExpr1;
  condExpr1 = lteExpression1;

  Lte lteExpression2;
  lteExpression2.left = std::shared_ptr<Expr>(new Expr);
  lteExpression2.right = std::shared_ptr<Expr>(new Expr);
  *lteExpression2.left = j;
  *lteExpression2.right = two00;
  CondExpr condExpr2;
  condExpr2 = lteExpression2;

  Lte lteExpression3;
  lteExpression3.left = std::shared_ptr<Expr>(new Expr);
  lteExpression3.right = std::shared_ptr<Expr>(new Expr);
  *lteExpression3.left = k;
  *lteExpression3.right = three00;
  CondExpr condExpr3;
  condExpr3 = lteExpression3;

  Lte lteExpression4;
  lteExpression4.left = std::shared_ptr<Expr>(new Expr);
  lteExpression4.right = std::shared_ptr<Expr>(new Expr);
  *lteExpression4.left = m;
  *lteExpression4.right = four00;
  CondExpr condExpr4;
  condExpr4 = lteExpression4;

  // create y + 1
  Add yPlusOne;
  yPlusOne.left = std::shared_ptr<Expr>(new Expr); *yPlusOne.left = y;
  yPlusOne.right = std::shared_ptr<Expr>(new Expr); *yPlusOne.right = one;
  Expr yPlusOneExpr = yPlusOne;

  // create x + y + z
  Add plus1;
  plus1.left = std::shared_ptr<Expr>(new Expr); *plus1.left = x;
  plus1.right = std::shared_ptr<Expr>(new Expr); *plus1.right = y;
  Expr plus1Expr = plus1;
  Add sum1;
  sum1.left = std::shared_ptr<Expr>(new Expr); *sum1.left = plus1Expr;
  sum1.right = std::shared_ptr<Expr>(new Expr); *sum1.right = z;
  Expr sum1Expr = sum1;

  StmtLst ifStmtListTrue1;
  ASTBuilder(ifStmtListTrue1).Assign("d", sum1Expr);
  StmtLst ifStmtListFalse1;
  ASTBuilder(ifStmtListFalse1).Assign("d", sum1Expr); // qns: im reusing the same stmt, i think its ok

  StmtLst elseStmtList2; // inner most if
  ASTBuilder(elseStmtList2).If(condExpr4, ifStmtListTrue1, ifStmtListFalse1).Assign("d", sum1Expr);

  StmtLst ifStmtList2;
  ASTBuilder(ifStmtList2).Assign("d", sum1Expr);

  StmtLst elseStmtList3; // 2nd inner most else stmt list
  ASTBuilder(elseStmtList3).If(condExpr3, ifStmtList2, elseStmtList2).Assign("d", sum1Expr);

  StmtLst ifStmtList3;
  ASTBuilder(ifStmtList3).Assign("y", yPlusOneExpr);

  StmtLst whileStmtList;
  ASTBuilder(whileStmtList).If(condExpr2, ifStmtList3, elseStmtList3).Assign("d", sum1Expr);

  StmtLst mainStmtList;
  ASTBuilder(mainStmtList).Read("x").While(condExpr1, whileStmtList).Assign("d", sum1Expr);

  ProcDef procMain("main", mainStmtList);

  Program prog{procMain};
  return prog;
}

Program buildTestCase8() {
// we want to calculate that
// 1 -> 9 -> 5 -> 6 -> 8 -> 10
// procedure main {
//     a = 1;                         #1
//     while (i < 100) {              #2
//         if (i < 100) {             #3 // firstIfStmt
//             if (i < 100) {         #4
//                 c = b;             #5 // list 1
//             } else {
//                 d = c;             #6 // list 2
//             }
//         } else {
//             if (i < 100) {         #7 // secondIfStmt
//                 e = d;             #8 // list 3
//             } else {
//                 b = a;             #9 // list 4
//             }
//         }
//     }
//     f = e;                         #10
// }

  Const c1{"1"};; Expr one = c1;
  Const c100{"100"}; Expr one00 = c100;
  Var nA{"a"}; Expr a = nA;
  Var nB{"b"}; Expr b = nB;
  Var nC{"c"}; Expr c = nC;
  Var nD{"d"}; Expr d = nD;
  Var nE{"e"}; Expr e = nE;
  Var nF{"f"}; Expr f = nF;
  Var nI{"i"}; Expr i = nI;

  StmtLst stmtList1;
  ASTBuilder(stmtList1).Assign("c", b);
  StmtLst stmtList2;
  ASTBuilder(stmtList2).Assign("d", c);
  StmtLst stmtList3;
  ASTBuilder(stmtList3).Assign("e", d);
  StmtLst stmtList4;
  ASTBuilder(stmtList4).Assign("b", a);

  Lt ltExpression;
  ltExpression.left = std::shared_ptr<Expr>(new Expr); *ltExpression.left = i;
  ltExpression.right = std::shared_ptr<Expr>(new Expr); *ltExpression.right = one00;
  CondExpr condExpr;
  condExpr = ltExpression;

  StmtLst firstIfStmt;
  ASTBuilder(firstIfStmt).If(condExpr, stmtList1, stmtList2);
  StmtLst secondIfStmt;
  ASTBuilder(secondIfStmt).If(condExpr, stmtList3, stmtList4);

  StmtLst outerIfStmt;
  ASTBuilder(outerIfStmt).If(condExpr, firstIfStmt, secondIfStmt);

  StmtLst mainStmtList;
  ASTBuilder(mainStmtList).Assign("a", one).While(condExpr, outerIfStmt).Assign("f", e);

  ProcDef procMain("main", mainStmtList);
  Program prog{procMain};
  return prog;
}
