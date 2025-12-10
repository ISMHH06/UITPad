#include "CppSyntaxRules.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

CppSyntaxRules::CppSyntaxRules() {
    // Essayer de charger depuis les ressources, sinon utiliser les règles par défaut
    if (!loadFromResources()) {
        initializeDefaultRules();
    }
}

bool CppSyntaxRules::loadFromResources() {
    // Charger depuis les fichiers de ressources Qt (qrc)
    bool success = true;
    success &= loadKeywordsFromFile(":/syntax/cpp_keywords.txt");
    success &= loadDataTypesFromFile(":/syntax/cpp_datatypes.txt");
    success &= loadPreprocessorsFromFile(":/syntax/cpp_preprocessors.txt");
    success &= loadOperatorsFromFile(":/syntax/cpp_operators.txt");
    return success;
}

bool CppSyntaxRules::loadFromFiles() {
    // Charger depuis des fichiers externes (pour personnalisation)
    bool success = true;
    success &= loadKeywordsFromFile("syntax/cpp_keywords.txt");
    success &= loadDataTypesFromFile("syntax/cpp_datatypes.txt");
    success &= loadPreprocessorsFromFile("syntax/cpp_preprocessors.txt");
    success &= loadOperatorsFromFile("syntax/cpp_operators.txt");
    return success;
}

QSet<QString> CppSyntaxRules::loadWordsFromFile(const QString& filePath) {
    QSet<QString> words;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Impossible d'ouvrir:" << filePath;
        return words;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        // Ignorer les lignes vides et commentaires
        if (!line.isEmpty() && !line.startsWith("#") && !line.startsWith("//")) {
            words.insert(line);
        }
    }

    file.close();
    return words;
}

bool CppSyntaxRules::loadKeywordsFromFile(const QString& filePath) {
    keywords = loadWordsFromFile(filePath);
    return !keywords.isEmpty();
}

bool CppSyntaxRules::loadDataTypesFromFile(const QString& filePath) {
    dataTypes = loadWordsFromFile(filePath);
    return !dataTypes.isEmpty();
}

bool CppSyntaxRules::loadPreprocessorsFromFile(const QString& filePath) {
    preprocessors = loadWordsFromFile(filePath);
    return !preprocessors.isEmpty();
}

bool CppSyntaxRules::loadOperatorsFromFile(const QString& filePath) {
    operators = loadWordsFromFile(filePath);
    return !operators.isEmpty();
}

void CppSyntaxRules::initializeDefaultRules() {
    // ============ MOTS-CLÉS C++ COMPLETS ============
    keywords = {
        // Contrôle de flux
        "if", "else", "switch", "case", "default",
        "while", "do", "for", "break", "continue",
        "goto", "return",

        // Exceptions
        "try", "catch", "throw", "noexcept",

        // Classes et structures
        "class", "struct", "union", "enum",
        "public", "private", "protected",
        "virtual", "override", "final",

        // Modificateurs
        "static", "const", "constexpr", "mutable",
        "extern", "inline", "explicit", "volatile",
        "thread_local", "register",

        // Templates
        "template", "typename", "decltype",

        // Namespaces
        "namespace", "using",

        // Opérateurs spéciaux
        "new", "delete", "sizeof", "alignof",
        "typeid", "this", "operator",

        // Casting
        "const_cast", "dynamic_cast",
        "reinterpret_cast", "static_cast",

        // Types spéciaux
        "auto", "nullptr", "true", "false",

        // Héritage et amis
        "friend", "typedef",

        // C++11/14/17/20
        "consteval", "constinit", "concept",
        "requires", "co_await", "co_return", "co_yield",
        "export", "module", "import",

        // Autres
        "asm", "static_assert"
    };

    // ============ TYPES DE DONNÉES COMPLETS ============
    dataTypes = {
        // Types fondamentaux
        "void", "bool",
        "char", "wchar_t", "char8_t", "char16_t", "char32_t",
        "short", "int", "long", "signed", "unsigned",
        "float", "double",

        // Types C++11
        "long long",

        // Types standard
        "size_t", "ptrdiff_t", "nullptr_t",

        // Types entiers fixes (C++11)
        "int8_t", "int16_t", "int32_t", "int64_t",
        "uint8_t", "uint16_t", "uint32_t", "uint64_t",
        "intptr_t", "uintptr_t",
        "int_fast8_t", "int_fast16_t", "int_fast32_t", "int_fast64_t",
        "uint_fast8_t", "uint_fast16_t", "uint_fast32_t", "uint_fast64_t",
        "int_least8_t", "int_least16_t", "int_least32_t", "int_least64_t",
        "uint_least8_t", "uint_least16_t", "uint_least32_t", "uint_least64_t",
        "intmax_t", "uintmax_t",

        // STL Types communs
        "string", "wstring",
        "vector", "list", "deque", "array",
        "set", "multiset", "map", "multimap",
        "unordered_set", "unordered_map",
        "stack", "queue", "priority_queue",
        "pair", "tuple",
        "shared_ptr", "unique_ptr", "weak_ptr",
        "optional", "variant", "any",
        "function", "bind",
        "thread", "mutex", "atomic",
        "ifstream", "ofstream", "fstream",
        "stringstream", "ostringstream", "istringstream",

        // Qt Types
        "QString", "QByteArray", "QChar",
        "QList", "QVector", "QSet", "QMap", "QHash",
        "QVariant", "QObject", "QWidget",
        "QFile", "QDir", "QTextStream"
    };

    // ============ PRÉPROCESSEUR COMPLET ============
    preprocessors = {
        "#include", "#define", "#undef",
        "#ifdef", "#ifndef", "#if", "#else", "#elif", "#endif",
        "#pragma", "#error", "#warning", "#line"
    };

    // ============ OPÉRATEURS COMPLETS ============
    operators = {
        // Arithmétiques
        "+", "-", "*", "/", "%",
        "++", "--",

        // Comparaison
        "==", "!=", "<", ">", "<=", ">=",
        "<=>", // C++20 spaceship operator

        // Logiques
        "&&", "||", "!",

        // Binaires
        "&", "|", "^", "~", "<<", ">>",

        // Affectation
        "=", "+=", "-=", "*=", "/=", "%=",
        "&=", "|=", "^=", "<<=", ">>=",

        // Autres
        "?", ":", "::", ".", "->", ".*", "->*",
        ",", "..."
    };
}

// ============ DÉTECTION AUTOMATIQUE DE CODE C++ ============
bool CppSyntaxRules::isCppCode(const QString& text) {
    if (text.trimmed().isEmpty()) {
        return false;
    }

    int cppIndicators = 0;

    // 1. Directives préprocesseur
    if (text.contains(QRegularExpression("^\\s*#include\\s*[<\"]", QRegularExpression::MultilineOption))) {
        cppIndicators += 3;
    }
    if (text.contains(QRegularExpression("^\\s*#define", QRegularExpression::MultilineOption))) {
        cppIndicators += 2;
    }

    // 2. Mots-clés C++ fréquents
    QStringList commonKeywords = { "int main", "void", "class ", "namespace ", "using namespace",
                                   "std::", "return ", "if (", "for (", "while (" };
    for (const QString& keyword : commonKeywords) {
        if (text.contains(keyword)) {
            cppIndicators++;
        }
    }

    // 3. Syntaxe C++ typique
    if (text.contains(QRegularExpression("\\w+\\s*\\([^)]*\\)\\s*\\{"))) { // fonction { }
        cppIndicators += 2;
    }
    if (text.contains(QRegularExpression("\\w+\\s+\\w+\\s*="))) { // déclaration avec =
        cppIndicators++;
    }
    if (text.contains("::")) { // scope resolution
        cppIndicators++;
    }
    if (text.contains("->")) { // pointeur
        cppIndicators++;
    }

    // 4. Commentaires C++
    if (text.contains("//") || text.contains("/*")) {
        cppIndicators++;
    }

    // 5. Points-virgules (très fréquents en C++)
    int semicolonCount = text.count(';');
    if (semicolonCount >= 2) {
        cppIndicators += qMin(semicolonCount / 2, 3);
    }

    // 6. Accolades
    int braceCount = text.count('{') + text.count('}');
    if (braceCount >= 2) {
        cppIndicators += qMin(braceCount / 2, 2);
    }

    // Si au moins 5 indicateurs, c'est probablement du code C++
    return cppIndicators >= 5;
}

bool CppSyntaxRules::isKeyword(const QString& word) const {
    return keywords.contains(word);
}

bool CppSyntaxRules::isDataType(const QString& word) const {
    return dataTypes.contains(word);
}

bool CppSyntaxRules::isPreprocessor(const QString& word) const {
    return preprocessors.contains(word);
}

bool CppSyntaxRules::isOperator(const QString& word) const {
    return operators.contains(word);
}