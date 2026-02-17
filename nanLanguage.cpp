#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <fstream>
#include <cctype>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <cstring> 

// ===============================
// nanLanguage Interpreter
// - Variables: double
// - Expressions: + - * / % , parentheses, functions, variables
// - Comparisons: > < >= <= == !=
// - Logic: && || !
// - Blocks: loop ... ( ... ) and if ... ( ... )
// ===============================
class Interpreter {
private:
    std::map<std::string, double> variables;

    // -------------------------------
    // Helpers: trimming
    // -------------------------------
    static std::string ltrim(std::string s) {
        size_t i = 0;
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
        return s.substr(i);
    }
    static std::string rtrim(std::string s) {
        if (s.empty()) return s;
        size_t i = s.size();
        while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) i--;
        return s.substr(0, i);
    }
    static std::string trim(std::string s) {
        return rtrim(ltrim(std::move(s)));
    }

    // -------------------------------
    // Expression Parser (recursive descent)
    // Grammar:
    //   expr        := logical_or
    //   logical_or  := logical_and ( "||" logical_and )*
    //   logical_and := equality ( "&&" equality )*
    //   equality    := comparison ( ("=="|"!=") comparison )*
    //   comparison  := term ( (">"|"<"|">="|"<=") term )*
    //   term        := factor ( ("+"|"-") factor )*
    //   factor      := unary ( ("*"|"/"|"%") unary )*
    //   unary       := ("+"|"-"|"!") unary | primary
    //   primary     := number | identifier | function_call | "(" expr ")"
    //
    // function_call := identifier "(" [expr ("," expr)*] ")"
    // -------------------------------
    struct ExprParser {
        Interpreter& interp;
        std::string s;
        size_t pos = 0;
        std::string errPrefix;

        ExprParser(Interpreter& in, std::string input, std::string prefix)
            : interp(in), s(std::move(input)), errPrefix(std::move(prefix)) {}

        void skipWs() {
            while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) pos++;
        }

        bool matchChar(char c) {
            skipWs();
            if (pos < s.size() && s[pos] == c) {
                pos++;
                return true;
            }
            return false;
        }

        bool matchStr(const char* t) {
            skipWs();
            size_t n = std::strlen(t);
            if (pos + n <= s.size() && s.compare(pos, n, t) == 0) {
                pos += n;
                return true;
            }
            return false;
        }

        char peek() {
            skipWs();
            if (pos >= s.size()) return '\0';
            return s[pos];
        }

        [[noreturn]] void fail(const std::string& msg) {
            throw std::runtime_error(errPrefix + msg + " near: '" + s.substr(pos) + "'");
        }

        static double truthy(double v) { return (v != 0.0) ? 1.0 : 0.0; }

        std::string parseIdentifier() {
            skipWs();
            if (pos >= s.size() || !(std::isalpha(static_cast<unsigned char>(s[pos])) || s[pos] == '_'))
                return "";

            size_t start = pos++;
            while (pos < s.size()) {
                char c = s[pos];
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') pos++;
                else break;
            }
            return s.substr(start, pos - start);
        }

        double parseNumber() {
            skipWs();
            size_t start = pos;
            bool sawDigit = false;

            if (pos < s.size() && (s[pos] == '.' || std::isdigit(static_cast<unsigned char>(s[pos])))) {
                if (s[pos] == '.') pos++;
                while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
                    pos++; sawDigit = true;
                }
                if (pos < s.size() && s[pos] == '.') {
                    pos++;
                    while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
                        pos++; sawDigit = true;
                    }
                }
                // exponent
                if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
                    size_t ePos = pos++;
                    if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) pos++;
                    bool expDigit = false;
                    while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
                        pos++; expDigit = true;
                    }
                    if (!expDigit) {
                        pos = ePos; // rollback exponent
                    }
                }
            }

            if (start == pos || !sawDigit) fail("Expected number");
            return std::stod(s.substr(start, pos - start));
        }

        double callFunction(const std::string& name, const std::vector<double>& args) {
            // 1-arg
            if (name == "sqrt") { if (args.size()!=1) fail("sqrt() expects 1 arg"); return std::sqrt(args[0]); }
            if (name == "sin")  { if (args.size()!=1) fail("sin() expects 1 arg");  return std::sin(args[0]); }
            if (name == "cos")  { if (args.size()!=1) fail("cos() expects 1 arg");  return std::cos(args[0]); }
            if (name == "tan")  { if (args.size()!=1) fail("tan() expects 1 arg");  return std::tan(args[0]); }
            if (name == "abs")  { if (args.size()!=1) fail("abs() expects 1 arg");  return std::fabs(args[0]); }
            if (name == "log")  { if (args.size()!=1) fail("log() expects 1 arg");  return std::log(args[0]); }
            if (name == "exp")  { if (args.size()!=1) fail("exp() expects 1 arg");  return std::exp(args[0]); }
            if (name == "floor"){ if (args.size()!=1) fail("floor() expects 1 arg");return std::floor(args[0]); }
            if (name == "ceil") { if (args.size()!=1) fail("ceil() expects 1 arg"); return std::ceil(args[0]); }

            // 2-arg
            if (name == "pow") { if (args.size()!=2) fail("pow() expects 2 args"); return std::pow(args[0], args[1]); }
            if (name == "min") { if (args.size()!=2) fail("min() expects 2 args"); return std::fmin(args[0], args[1]); }
            if (name == "max") { if (args.size()!=2) fail("max() expects 2 args"); return std::fmax(args[0], args[1]); }

            fail("Unknown function: " + name);
        }

        double primary() {
            skipWs();

            // Parentheses
            if (matchChar('(')) {
                double v = expr();
                if (!matchChar(')')) fail("Expected ')'");
                return v;
            }

            // Identifier or function call
            if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
                std::string id = parseIdentifier();
                skipWs();

                // function call: id(...)
                if (matchChar('(')) {
                    std::vector<double> args;
                    skipWs();
                    if (!matchChar(')')) {
                        while (true) {
                            args.push_back(expr());
                            skipWs();
                            if (matchChar(')')) break;
                            if (!matchChar(',')) fail("Expected ',' or ')'");
                        }
                    }
                    return callFunction(id, args);
                }

                // variable
                if (interp.variables.count(id)) return interp.variables[id];
                fail("Unknown variable: " + id);
            }

            // Number
            if (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.') {
                return parseNumber();
            }

            fail("Expected primary expression");
        }

        double unary() {
            skipWs();
            if (matchChar('+')) return unary();
            if (matchChar('-')) return -unary();
            if (matchChar('!')) return truthy(unary()) ? 0.0 : 1.0;
            return primary();
        }

        double factor() {
            double v = unary();
            while (true) {
                skipWs();
                if (matchChar('*')) v *= unary();
                else if (matchChar('/')) v /= unary();
                else if (matchChar('%')) {
                    double rhs = unary();
                    v = std::fmod(v, rhs);
                } else break;
            }
            return v;
        }

        double term() {
            double v = factor();
            while (true) {
                skipWs();
                if (matchChar('+')) v += factor();
                else if (matchChar('-')) v -= factor();
                else break;
            }
            return v;
        }

        double comparison() {
            double v = term();
            while (true) {
                skipWs();
                if (matchStr(">=")) { double r = term(); v = (v >= r) ? 1.0 : 0.0; }
                else if (matchStr("<=")) { double r = term(); v = (v <= r) ? 1.0 : 0.0; }
                else if (matchChar('>')) { double r = term(); v = (v >  r) ? 1.0 : 0.0; }
                else if (matchChar('<')) { double r = term(); v = (v <  r) ? 1.0 : 0.0; }
                else break;
            }
            return v;
        }

        double equality() {
            double v = comparison();
            while (true) {
                skipWs();
                if (matchStr("==")) { double r = comparison(); v = (v == r) ? 1.0 : 0.0; }
                else if (matchStr("!=")) { double r = comparison(); v = (v != r) ? 1.0 : 0.0; }
                else break;
            }
            return v;
        }

        double logical_and() {
            double v = equality();
            while (true) {
                skipWs();
                if (matchStr("&&")) {
                    double r = equality();
                    v = (truthy(v) && truthy(r)) ? 1.0 : 0.0;
                } else break;
            }
            return v;
        }

        double logical_or() {
            double v = logical_and();
            while (true) {
                skipWs();
                if (matchStr("||")) {
                    double r = logical_and();
                    v = (truthy(v) || truthy(r)) ? 1.0 : 0.0;
                } else break;
            }
            return v;
        }

        double expr() { return logical_or(); }

        double eval() {
            double v = expr();
            skipWs();
            if (pos != s.size()) fail("Unexpected trailing characters");
            return v;
        }
    };

    double evalExpr(const std::string& expr, const std::string& errPrefix = "Expr error: ") {
        ExprParser p(*this, expr, errPrefix);
        return p.eval();
    }

    // Print formatting: if it's basically an integer, print like one
    static void printNumberNicely(double v) {
        double rounded = std::round(v);
        if (std::fabs(v - rounded) < 1e-9) {
            std::cout << static_cast<long long>(rounded) << "\n";
        } else {
            std::cout << std::setprecision(12) << v << "\n";
        }
    }

    // Extract block lines until a line that is exactly ")"
    static std::string readBlock(std::istringstream& stream) {
        std::string block;
        std::string blockLine;
        while (std::getline(stream, blockLine)) {
            if (trim(blockLine) == ")") break;
            block += blockLine + "\n";
        }
        return block;
    }

    // For "if ..." line: condition is everything after "if" and before the final "("
    static bool splitHeaderAndOpenParen(const std::string& line, std::string& beforeParen) {
        // We expect the block opener "(" at the end (possibly with spaces).
        std::string t = rtrim(line);
        if (t.empty()) return false;
        if (t.back() != '(') return false;
        beforeParen = rtrim(t.substr(0, t.size() - 1));
        return true;
    }

public:
    // ============================================
    // Execute full script (multiple lines)
    // ============================================
    void execute(const std::string& code) {
        std::istringstream stream(code);
        std::string line;

        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.empty()) continue;

            // ignore comment lines
            if (line.rfind("comment", 0) == 0) continue;

            std::istringstream ss(line);
            std::string command;
            ss >> command;

            // =========================
            // LOOP COMMAND
            // loop i:10 (
            //   ...
            // )
            // =========================
            if (command == "loop") {
                std::string varAndCount;
                ss >> varAndCount;

                size_t colonPos = varAndCount.find(':');
                if (colonPos == std::string::npos) {
                    std::cout << "Syntax error: loop expects var:count\n";
                    continue;
                }

                std::string var = varAndCount.substr(0, colonPos);
                std::string countExpr = varAndCount.substr(colonPos + 1);

                // Expect "(" token next (or end with "(")
                std::string openParen;
                ss >> openParen;
                if (openParen != "(") {
                    std::cout << "Syntax error: expected (\n";
                    continue;
                }

                std::string block = readBlock(stream);

                int count = 0;
                try {
                    double c = evalExpr(countExpr, "Loop count error: ");
                    count = static_cast<int>(std::floor(c));
                } catch (const std::exception& e) {
                    std::cout << e.what() << "\n";
                    continue;
                }

                for (int i = 0; i < count; i++) {
                    variables[var] = static_cast<double>(i);
                    execute(block);
                }
            }

            // =========================
            // IF COMMAND
            // if <condition> (
            //   ...
            // )
            // condition is an expression that evaluates to non-zero => true
            // =========================
            else if (command == "if") {
                std::string headerBeforeParen;
                if (!splitHeaderAndOpenParen(line, headerBeforeParen)) {
                    std::cout << "Syntax error: if expects '(' at end of line\n";
                    continue;
                }

                // headerBeforeParen looks like: "if x > 5" or "if (x > 5)"
                std::string condition = trim(headerBeforeParen.substr(2)); // remove "if"
                if (!condition.empty() && condition[0] == ' ') condition.erase(0, 1);
                condition = trim(condition);

                std::string block = readBlock(stream);

                try {
                    double condVal = evalExpr(condition, "If condition error: ");
                    if (condVal != 0.0) {
                        execute(block);
                    }
                } catch (const std::exception& e) {
                    std::cout << e.what() << "\n";
                }
            }

            // =========================
            // Everything else: single-line commands
            // =========================
            else {
                runLine(line);
            }
        }
    }

private:
    // ============================================
    // Execute one single line of code
    // ============================================
    void runLine(const std::string& line) {
        std::string t = trim(line);
        if (t.empty()) return;
        if (t.rfind("comment", 0) == 0) return;

        std::istringstream ss(t);
        std::string command;
        ss >> command;

        // =========================
        // PRINT
        // print "Hello"
        // print x
        // print (sqrt(16) + 2)
        // =========================
        if (command == "print") {
            std::string rest;
            std::getline(ss, rest);
            rest = trim(rest);

            // quoted string
            if (rest.size() >= 2 && rest.front() == '"' && rest.back() == '"') {
                std::cout << rest.substr(1, rest.size() - 2) << "\n";
                return;
            }

            // If it's exactly a variable name, print variable (nice)
            if (!rest.empty() && variables.count(rest)) {
                printNumberNicely(variables[rest]);
                return;
            }

            // Otherwise, treat as expression
            try {
                double v = evalExpr(rest, "Print expr error: ");
                printNumberNicely(v);
            } catch (const std::exception& e) {
                // fallback: print as text
                std::cout << rest << "\n";
            }
        }

        // =========================
        // SET
        // set x = 5 + 3*2
        // set y = sqrt(16)
        // set z = (pow(3,2) + 1)
        // =========================
        else if (command == "set") {
            std::string var;
            ss >> var;
            if (var.empty()) {
                std::cout << "Syntax error: set needs a variable name\n";
                return;
            }

            std::string token;
            ss >> token;

            // allow: set x = <expr>
            if (token != "=") {
                // allow: set x <expr> (no equals)
                // in that case token is first chunk of expr
                std::string rest;
                std::getline(ss, rest);
                std::string expr = token + rest;
                expr = trim(expr);

                try {
                    variables[var] = evalExpr(expr, "Set expr error: ");
                } catch (const std::exception& e) {
                    std::cout << e.what() << "\n";
                }
                return;
            }

            std::string expr;
            std::getline(ss, expr);
            expr = trim(expr);

            if (expr.empty()) {
                std::cout << "Syntax error: set needs an expression\n";
                return;
            }

            try {
                variables[var] = evalExpr(expr, "Set expr error: ");
            } catch (const std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }

        // =========================
        // ADD
        // add x 5
        // add x (pow(2,3) + 1)
        // =========================
        else if (command == "add") {
            std::string var;
            ss >> var;
            if (var.empty()) {
                std::cout << "Syntax error: add needs a variable\n";
                return;
            }
            if (!variables.count(var)) {
                std::cout << "Error: variable '" << var << "' not found\n";
                return;
            }

            std::string expr;
            std::getline(ss, expr);
            expr = trim(expr);
            if (expr.empty()) {
                std::cout << "Syntax error: add needs a value/expression\n";
                return;
            }

            try {
                variables[var] += evalExpr(expr, "Add expr error: ");
            } catch (const std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }

        // =========================
        // UNKNOWN
        // =========================
        else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }
};

// ============================================
// MAIN
// ============================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: nanLanguage <filename.txt>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file.\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Interpreter interpreter;
    interpreter.execute(buffer.str());
    return 0;
}
