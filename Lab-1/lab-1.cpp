#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <cmath>

// Перелік типів лексем (токенів) - поки без змінних
enum class TokenType {
    NONE,
    DELIMITER, // Роздільник (+, -, *, /, (, ))
    NUMBER     // Число
};

// Структура для представлення лексеми
struct Token {
    std::string value;
    TokenType type;
};

class Parser {
public:
    Parser();
    double evaluate(const std::string& expr);

private:
    std::string expression;
    size_t expr_pos;
    Token current_token;

    void getToken();
    void putback();

    double parseAddSubtract();
    double parseMultiplyDivide();
    double parseUnary();
    double parseParentheses();
    double parseAtom();

    void handleError(const std::string& error_message);
};

Parser::Parser() : expr_pos(0) {}

double Parser::evaluate(const std::string& expr) {
    if (expr.empty()) {
        handleError("Немає виразу для обчислення.");
    }
    expression = expr;
    expr_pos = 0;
    getToken();
    return parseAddSubtract();
}

void Parser::putback() {
    if (!current_token.value.empty()) {
        expr_pos -= current_token.value.length();
    }
}

void Parser::getToken() {
    current_token.type = TokenType::NONE;
    current_token.value = "";

    if (expr_pos >= expression.length()) return;

    while (expr_pos < expression.length() && isspace(expression[expr_pos])) {
        expr_pos++;
    }

    if (expr_pos >= expression.length()) return;

    char current_char = expression[expr_pos];

    if (std::string("+-*/()").find(current_char) != std::string::npos) {
        current_token.type = TokenType::DELIMITER;
        current_token.value += expression[expr_pos++];
    } else if (isdigit(current_char) || current_char == '.') {
        current_token.type = TokenType::NUMBER;
        while (expr_pos < expression.length() && (isdigit(expression[expr_pos]) || expression[expr_pos] == '.')) {
            current_token.value += expression[expr_pos++];
        }
    }
}

double Parser::parseAddSubtract() {
    double result = parseMultiplyDivide();
    while (current_token.value == "+" || current_token.value == "-") {
        char op = current_token.value[0];
        getToken();
        double temp = parseMultiplyDivide();
        if (op == '+') result += temp;
        else result -= temp;
    }
    return result;
}

double Parser::parseMultiplyDivide() {
    double result = parseUnary();
    while (current_token.value == "*" || current_token.value == "/") {
        char op = current_token.value[0];
        getToken();
        double temp = parseUnary();
        if (op == '*') {
            result *= temp;
        } else if (op == '/') {
            if (temp == 0) handleError("Ділення на нуль.");
            result /= temp;
        }
    }
    return result;
}

double Parser::parseUnary() {
    char op = 0;
    if (current_token.value == "+" || current_token.value == "-") {
        op = current_token.value[0];
        getToken();
    }
    double result = parseParentheses();
    if (op == '-') {
        result = -result;
    }
    return result;
}

double Parser::parseParentheses() {
    if (current_token.value == "(") {
        getToken();
        double result = parseAddSubtract();
        if (current_token.value != ")") {
            handleError("Незакриті дужки.");
        }
        getToken();
        return result;
    }
    return parseAtom();
}

double Parser::parseAtom() {
    double result = 0.0;
    if (current_token.type == TokenType::NUMBER) {
        try {
            result = std::stod(current_token.value);
        } catch (const std::invalid_argument&) {
            handleError("Невірний формат числа.");
        }
        getToken();
    } else {
        handleError("Синтаксична помилка (очікувалось число).");
    }
    return result;
}

void Parser::handleError(const std::string& error_message) {
    throw std::runtime_error(error_message);
}

int main() {
    Parser parser;
    std::string expression;

    std::cout << "Простий калькулятор. Введіть '.' для виходу.\n";

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, expression);
        if (expression == ".") break;
        try {
            double result = parser.evaluate(expression);
            std::cout << "Результат: " << result << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Помилка: " << e.what() << std::endl;
        }
    }
    return 0;
}