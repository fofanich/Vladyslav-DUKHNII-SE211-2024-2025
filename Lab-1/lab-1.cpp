#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <cmath>

// Перелік типів лексем (токенів)
enum class TokenType {
    NONE,
    DELIMITER, // Роздільник (+, -, *, /, =, (, ))
    VARIABLE,  // Змінна (A-Z)
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

    std::vector<double> variables; // Масив для зберігання значень змінних A-Z

    void getToken();
    void putback();

    double parseAssignment();
    double parseAddSubtract();
    double parseMultiplyDivide();
    double parsePower();
    double parseUnary();
    double parseParentheses();
    double parseAtom();

    void handleError(const std::string& error_message);
};

Parser::Parser() : variables(26, 0.0), expr_pos(0) {}

double Parser::evaluate(const std::string& expr) {
    if (expr.empty()) {
        handleError("Немає виразу для обчислення.");
    }
    expression = expr;
    expr_pos = 0;
    getToken();
    if (current_token.type == TokenType::NONE) {
        handleError("Пустий вираз.");
    }
    return parseAssignment();
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

    if (std::string("+-*/%^=()").find(current_char) != std::string::npos) {
        current_token.type = TokenType::DELIMITER;
        current_token.value += expression[expr_pos++];
    } else if (isalpha(current_char)) {
        current_token.type = TokenType::VARIABLE;
        // Дозволяємо лише одно-літерні змінні
        current_token.value += toupper(expression[expr_pos++]);
        if (expr_pos < expression.length() && isalpha(expression[expr_pos])) {
             handleError("Невірна назва змінної (дозволено лише A-Z).");
        }
    } else if (isdigit(current_char) || current_char == '.') {
        current_token.type = TokenType::NUMBER;
        while (expr_pos < expression.length() && (isdigit(expression[expr_pos]) || expression[expr_pos] == '.')) {
            current_token.value += expression[expr_pos++];
        }
    }
}

// Рівень 1: Обробка присвоювання (=)
double Parser::parseAssignment() {
    if (current_token.type == TokenType::VARIABLE) {
        Token temp_token = current_token;
        getToken();
        if (current_token.value == "=") {
            getToken();
            double value = parseAssignment(); // Дозволяє ланцюжкові присвоєння A=B=5
            int var_index = temp_token.value[0] - 'A';
            variables[var_index] = value;
            return value;
        } else {
            putback();
            current_token = temp_token;
        }
    }
    return parseAddSubtract();
}

// Рівень 2: Додавання та віднімання
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

// Рівень 3: Множення, ділення, залишок
double Parser::parseMultiplyDivide() {
    double result = parsePower();
    while (current_token.value == "*" || current_token.value == "/" || current_token.value == "%") {
        char op = current_token.value[0];
        getToken();
        double temp = parsePower();
        if (op == '*') {
            result *= temp;
        } else if (op == '/') {
            if (temp == 0) handleError("Ділення на нуль.");
            result /= temp;
        } else if (op == '%') {
            result = fmod(result, temp);
        }
    }
    return result;
}

// Рівень 4: Піднесення до степеня
double Parser::parsePower() {
    double result = parseUnary();
    if (current_token.value == "^") {
        getToken();
        double exponent = parsePower();
        result = pow(result, exponent);
    }
    return result;
}

// Рівень 5: Унарний плюс та мінус
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

// Рівень 6: Дужки
double Parser::parseParentheses() {
    if (current_token.value == "(") {
        getToken();
        double result = parseAssignment(); // Дозволяє присвоєння всередині дужок (A=(B=5))
        if (current_token.value != ")") {
            handleError("Синтаксична помилка: очікувались закриваючі дужки ')'.");
        }
        getToken();
        return result;
    }
    return parseAtom();
}

// Рівень 7: Атом (число або змінна)
double Parser::parseAtom() {
    double result = 0.0;
    switch (current_token.type) {
        case TokenType::NUMBER:
            try {
                result = std::stod(current_token.value);
            } catch (const std::invalid_argument&) {
                handleError("Невірний формат числа: " + current_token.value);
            }
            getToken();
            break;
        case TokenType::VARIABLE:
            result = variables[current_token.value[0] - 'A'];
            getToken();
            break;
        default:
            handleError("Синтаксична помилка: несподівана лексема '" + current_token.value + "'.");
    }
    return result;
}

void Parser::handleError(const std::string& error_message) {
    throw std::runtime_error(error_message);
}

int main() {
    Parser parser;
    std::string expression;

    std::cout << "Калькулятор v3.0 з підтримкою змінних (A-Z). Введіть '.' для виходу.\n";

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
