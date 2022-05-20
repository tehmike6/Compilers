#include "IntermediateCode.hpp"
#include <iomanip>
#include <algorithm>

Scopespace_T getCurrentScopespace() {
    if (scopeSpaceCounter == 1)
        return PROGRAM_VAR;
    else if (scopeSpaceCounter % 2 == 0)
        return FORMAL_ARG;
    else
        return FUNCTION_LOCAL;
}

unsigned int getCurrentScopeOffset() {
    switch (getCurrentScopespace()) {
    case PROGRAM_VAR:
        return programVarOffset;
    case FORMAL_ARG:
        return formalArgOffset;
    case FUNCTION_LOCAL:
        return functionLocalOffset;
    default:
        assert(0);
    }
}

void enterScopespace() {
    ++scopeSpaceCounter;
}

void exitScopepace() {
    assert(scopeSpaceCounter > 1);
    --scopeSpaceCounter;
}

void incCurrentScopeOffset() {
    switch (getCurrentScopespace()) {
    case PROGRAM_VAR:
        ++programVarOffset;
        break;
    case FORMAL_ARG:
        ++formalArgOffset;
        break;
    case FUNCTION_LOCAL:
        ++functionLocalOffset;
        break;
    default:
        assert(0);
    }
}

void emit(Opcode op, Expr* arg1, Expr* arg2, Expr* result, unsigned int line,
    unsigned int label) {

    Quad* newQuad = new Quad();
    newQuad->arg1 = arg1;
    newQuad->arg2 = arg2;
    newQuad->result = result;
    newQuad->line = line;
    newQuad->label = label;
    newQuad->opcode = op;

    Quads.push_back(newQuad);
}

Expr* emit_iftableitem(Expr* expr, unsigned int lineno) {
    if (expr->type != TABLE_ITEM_EXPR) {
        return expr;
    }

    Expr* newExpr = newExprType(VAR_EXPR);
    Symbol* newSymbol = newTempSymbol();
    symtable.insert(newSymbol);
    newExpr->symbol = newSymbol;
    newExpr->value = newSymbol->getId();
    emit(OP_TABLEGETELEM, expr, expr->index, newExpr, lineno, 0);

    return newExpr;
}

Expr* emit_table(Expr* arg1, Expr* arg2, unsigned int lineno) {
    Expr* newExpr = emit_iftableitem(arg1, lineno);

    Expr* newExpr2 = newExprType(TABLE_ITEM_EXPR);
    newExpr2->symbol = newExpr->symbol;
    newExpr2->index = arg2;

    return newExpr2;
}

std::string newTempName() {
    return std::string("$") + std::to_string(tempNameCounter++);
}

void resetTemp() {

    auto symList = symtable.getSymbols(currentScope);
    for (auto it = symList.begin(); it != symList.end(); it++) {
        std::string id = (*it)->getId();
        if (id[0] == '$') symtable.removeSymbol(id, currentScope);
    }

    tempNameCounter = 1;
}

Symbol* newTempSymbol() {
    std::string name = newTempName();
    Symbol* symbol = symtable.scopeLookup(name, currentScope);
    Symbol_T type = currentScope == 0 ? GLOBALVAR : LOCALVAR;
    if (symbol == NULL)
        return new Symbol(name, type, yylineno, currentScope, true);
    else
        return symbol;
}

bool isValidArithmeticOperation(Expr* e1, Expr* e2) {
    return isValidArithmeticExpr(e1) && isValidArithmeticExpr(e2);
}

bool isValidArithmeticExpr(Expr* expr) {
    return expr->type == CONST_INTEGER_EXPR ||
        expr->type == INTEGER_EXPR ||
        expr->type == CONST_REAL_EXPR ||
        expr->type == REAL_EXPR ||
        expr->type == CONST_NUMBER_EXPR ||
        expr->type == NUMBER_EXPR ||
        expr->type == VAR_EXPR ||
        expr->type == ARITHMETIC_EXPR;
}

void resetFormalArgsOffset() {
    formalArgOffset = 0;
}

void resetFunctionLocalOffset() {
    functionLocalOffset = 0;
}

void restoreCurrentScopeOffset(unsigned int offset) {
    switch (getCurrentScopespace()) {
    case PROGRAM_VAR:
        programVarOffset = offset;
        break;
    case FORMAL_ARG:
        formalArgOffset = offset;
        break;
    case FUNCTION_LOCAL:
        functionLocalOffset = offset;
        break;
    default: assert(0);
    }
}

std::string opcodeToString(Opcode opcode) {
    switch (opcode) {
    case OP_ASSIGN: return "assign";
    case OP_ADD: return "add";
    case OP_SUB: return "sub";
    case OP_MUL: return "mul";
    case OP_DIV: return "div";
    case OP_MOD: return "mod";
    case OP_UMINUS: return "uminus";
    case OP_AND: return "and";
    case OP_OR: return "or";
    case OP_NOT: return "not";
    case OP_IF_EQ: return "if_eq";
    case OP_IF_NOTEQ: return "if_noteq";
    case OP_IF_LESSEQ: return "if_lesseq";
    case OP_IF_GREATEQ: return "if_greatereq";
    case OP_IF_LESS: return "if_less";
    case OP_IF_GREATER: return "if_greater";
    case OP_JUMP: return "jump";
    case OP_CALL: return "call";
    case OP_PARAM: return "param";
    case OP_RET: return "return";
    case OP_GETRETVAL: return "getretval";
    case OP_FUNCSTART: return "funcstart";
    case OP_FUNCEND: return "funcend";
    case OP_TABLECREATE: return "tablecreate";
    case OP_TABLEGETELEM: return "tablegetelem";
    case OP_TABLESETELEM: return "tablesetelem";
    default: return "UNKNOWN";
    }
}

// Extend for other cases
Expr* symbolToExpr(Symbol* symbol) {
    Expr* newExpr = new Expr;

    switch (symbol->getType()) {
    case GLOBALVAR: {
        newExpr->symbol = symbol;
        newExpr->value = symbol->getId();
        newExpr->type = VAR_EXPR;
        break;
    };
    case LOCALVAR: {
        newExpr->symbol = symbol;
        newExpr->value = symbol->getId();
        newExpr->type = VAR_EXPR;
        break;
    };
    case FORMAL_ARG: {
        newExpr->type = VAR_EXPR;
        newExpr->symbol = symbol;
        newExpr->value = symbol->getId();
        break;
    };
    case LIBRARYFUNC: {
        newExpr->type = LIBRARYFUNCTION_EXPR;
        newExpr->symbol = symbol;
        newExpr->value = symbol->getId();
        break;
    };
    case USERFUNC: {
        newExpr->type = USERFUNCTION_EXPR;
        newExpr->symbol = symbol;
        newExpr->value = symbol->getId();
        break;
    }
    case SYMERROR:
        break;
    default: break;
    }

    return newExpr;
}

Expr* changeType(Expr* expr, Expr_T type) {
    expr->type = type;
    return expr;
}

Expr* changeValue(Expr* expr, std::variant<std::string, int, double, bool> value) {
    expr->value = value;
    return expr;
}

Expr* newExprType(Expr_T type) {
    Expr* newExpr = new Expr;

    newExpr->type = type;

    return newExpr;
}

Expr* newNilExpr() {
    Expr* newExpr = new Expr;

    newExpr->type = NIL_EXPR;

    return newExpr;
}

Expr* newBoolExpr(bool value) {
    Expr* newExpr = new Expr;

    newExpr->type = CONST_BOOLEAN_EXPR;
    newExpr->value = value;

    return newExpr;
}

Expr* newStringExpr(std::string value) {
    Expr* newExpr = new Expr;

    newExpr->type = CONST_STRING_EXPR;
    newExpr->value = value;

    return newExpr;
}

Expr* newIntegerExpr(int value) {
    Expr* newExpr = new Expr;

    newExpr->type = CONST_INTEGER_EXPR;
    newExpr->value = value;

    return newExpr;
}

Expr* newDoubleExpr(double value) {
    Expr* newExpr = new Expr;

    newExpr->type = CONST_REAL_EXPR;
    newExpr->value = value;

    return newExpr;
}

bool isTempSymbol(Symbol* symbol) {
    return symbol->getId().rfind("$", 0) == 0;
}

bool isFunctionExpr(Expr* expr) {
    return expr->type == USERFUNCTION_EXPR || expr->type == LIBRARYFUNCTION_EXPR;
}

bool areExprTypesEq(Expr* expr1, Expr* expr2) {
    return expr1->type == expr2->type;
}

bool areExprBoolTypes(Expr* expr1, Expr* expr2) {
    return (expr1->type == BOOLEAN_EXPR || expr1->type == CONST_BOOLEAN_EXPR) &&
        (expr2->type == BOOLEAN_EXPR || expr2->type == CONST_BOOLEAN_EXPR);
}

void patchlabel(unsigned int quadNo, unsigned int label) {
    std::cerr << "Quad num: " << quadNo << std::endl;
    std::cerr << "Label: " << label << std::endl;
    Quads[quadNo]->label = label;
}

unsigned int nextQuadLabel() {
    return Quads.size() + 1;
}


Expr* make_call(Expr* lvalue, std::list<Expr*> revElist, unsigned int line) {
    Expr* called_func = emit_iftableitem(lvalue, line);
    while (!revElist.empty()) {
        emit(OP_PARAM, revElist.back(), NULL, NULL, line, 0);
        revElist.pop_back();
    }
    emit(OP_CALL, called_func, NULL, NULL, line, 0);

    Expr* result = new Expr();
    result->symbol = newTempSymbol();
    result->type = VAR_EXPR;
    emit(OP_GETRETVAL, NULL, NULL, result, line, 0);
    return result;
}

Expr* member_item(Expr* lv, std::string name, int line) {
    lv = emit_iftableitem(lv, line); // Emit code if r-value use of table item
    Expr* ti = newExprType(TABLE_ITEM_EXPR); // Make a new expression
    ti->symbol = lv->symbol;
    ti->index = newExprType(CONST_STRING_EXPR); // Const string index
    return ti;
}

std::list<Expr*> reverseElist(Expr* expr) {
    std::list<Expr*> list = std::list<Expr*>();
    std::stack<Expr*> stack = std::stack<Expr*>();

    while (expr) {
        stack.push(expr);
        expr = expr->next;
    }

    while (!stack.empty()) {
        list.push_front(stack.top());
        stack.pop();
    }

    return list;
}

std::string fixPrecision(std::string num) {
    std::string fixedNum = "";
    int size = num.length() - 1;
    bool flag = false;

    for (int i = 0; i <= size; i++) {
        if (num[size - i] != '0') flag = true;
        if (!flag) continue;
        fixedNum += num[size - i];
    }

    std::reverse(fixedNum.begin(), fixedNum.end());
    return fixedNum;
}

int betweenFor() {
    emit(OP_JUMP, NULL, NULL, NULL, yylineno, 0);
    return nextQuadLabel();
}

std::string exprValueToString(Expr* expr) {
    switch (expr->type) {
    case CONST_NUMBER_EXPR:
        if (expr->value.index() == 1)
            return std::to_string(std::get<int>(expr->value));
        if (expr->value.index() == 2)
            return fixPrecision(std::to_string(std::get<double>(expr->value)));
    case NUMBER_EXPR:
        if (expr->value.index() == 1)
            return std::to_string(std::get<int>(expr->value));
        if (expr->value.index() == 2)
            return fixPrecision(std::to_string(std::get<double>(expr->value)));
    case CONST_INTEGER_EXPR:
    case INTEGER_EXPR:
        return std::to_string(std::get<int>(expr->value));
    case CONST_REAL_EXPR:
    case REAL_EXPR:
        return fixPrecision(std::to_string(std::get<double>(expr->value)));
    case CONST_BOOLEAN_EXPR:
        return std::get<bool>(expr->value) ? "true" : "false";
    case CONST_STRING_EXPR:
    case STRING_EXPR:
        return std::get<std::string>(expr->value);
    case NIL_EXPR:
        return "NIL";
    case USERFUNCTION_EXPR:
    case LIBRARYFUNCTION_EXPR:
    case VAR_EXPR:
    case ASSIGN_EXPR:
    case ARITHMETIC_EXPR:
    case BOOLEAN_EXPR:
    case NEW_TABLE_EXPR:
    case TABLE_ITEM_EXPR:
        return expr->symbol->getId();
    default:
        return "UKNOWN";
    }
}

std::string quadTabs(std::string value) {
    if (value.length() < 8) {
        return "\t\t";
    }

    return "\t";
}

void printQuadsInFile(char* argv) {
    std::ofstream myfile;
    myfile.open(argv, std::ios_base::app);

    myfile << std::endl;
    myfile << "quad#\topcode\t\tresult\t\targ1\t\targ2\t\tlabel" << std::endl;
    myfile << "=============================================================================" << std::endl;
    int counter = 1;
    for (auto it = Quads.begin(); it != Quads.end(); ++it) {
        Quad* quad = *it;
        std::string opcode = opcodeToString(quad->opcode);
        myfile << counter++ << ':';
        myfile << '\t' << opcode << quadTabs(opcode);

        if (quad->result != NULL) myfile << exprValueToString(quad->result) << "\t\t";
        else myfile << "" << "\t\t";
        if (quad->arg1 != NULL) {
            if (quad->arg1->type == CONST_NUMBER_EXPR || quad->arg1->type == CONST_BOOLEAN_EXPR || quad->arg1->type == CONST_STRING_EXPR) {
                myfile << exprValueToString(quad->arg1) << "\t\t";
            }
            else {
                myfile << exprValueToString(quad->arg1) << "\t\t";
            }
        }
        else myfile << "" << "\t\t";
        if (quad->arg2 != NULL) {
            if (quad->arg2->type == CONST_NUMBER_EXPR || quad->arg2->type == CONST_BOOLEAN_EXPR || quad->arg2->type == CONST_STRING_EXPR)
                myfile << exprValueToString(quad->arg2) << "\t\t";
            else
                myfile << exprValueToString(quad->arg2) << "\t\t";
        }
        else myfile << "" << "\t\t";
        if (quad->label != 0) {
            myfile << quad->label;
        }

        myfile << std::endl;
    }

    myfile << std::endl;

    myfile.close();
}

void printQuads() {
    std::cout << std::endl;
    std::cout << "quad#\topcode\t\tresult\t\targ1\t\targ2\t\tlabel" << std::endl;
    std::cout << "=============================================================================" << std::endl;
    int counter = 1;
    for (auto it = Quads.begin(); it != Quads.end(); ++it) {
        Quad* quad = *it;
        std::string opcode = opcodeToString(quad->opcode);
        std::cout << counter++ << ':';
        std::cout << '\t' << opcode << quadTabs(opcode);

        if (quad->result != NULL) std::cout << exprValueToString(quad->result) << "\t\t";
        else std::cout << "" << "\t\t";
        if (quad->arg1 != NULL) {
            if (quad->arg1->type == CONST_NUMBER_EXPR || quad->arg1->type == CONST_BOOLEAN_EXPR || quad->arg1->type == CONST_STRING_EXPR) {
                std::cout << exprValueToString(quad->arg1) << "\t\t";
            }
            else {
                std::cout << exprValueToString(quad->arg1) << "\t\t";
            }
        }
        else std::cout << "" << "\t\t";
        if (quad->arg2 != NULL) {
            if (quad->arg2->type == CONST_NUMBER_EXPR || quad->arg2->type == CONST_BOOLEAN_EXPR || quad->arg2->type == CONST_STRING_EXPR)
                std::cout << exprValueToString(quad->arg2) << "\t\t";
            else
                std::cout << exprValueToString(quad->arg2) << "\t\t";
        }
        else std::cout << "" << "\t\t";
        if (quad->label != 0) {
            std::cout << quad->label;
        }

        std::cout << std::endl;
    }
    std::cout << counter++ << ':' << std::endl;

    std::cout << std::endl;
}