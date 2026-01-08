#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QColor>
#include <QFont>
#include <QFontDialog>
#include <QColorDialog>
#include <QComboBox>
#include <QLineEdit>

class Settings : public QDialog
{
    Q_OBJECT

public:
    // --- Énumération pour les thèmes ---
    enum AppTheme {
        System,    // NOUVEAU : Détecte automatiquement le thème du système
        Light,
        Dark,
        Hacker
    };

    explicit Settings(QWidget* parent = nullptr);

    // --- Accesseurs (Getters) ---
    bool isSpellCheckEnabled() const;
    bool underlineErrors() const;
    QFont getEditorFont() const;
    QColor getEditorColor() const;
    AppTheme getSelectedTheme() const;
    QString getDeepSeekApiKey() const;
    QString getAiModel() const;

    static QString deepSeekApiKey();
    static void setDeepSeekApiKey(const QString& apiKey);
    static QString aiModel();
    static void setAiModel(const QString& model);

    // --- NOUVEAU : Setter pour définir le thème actuel à l'ouverture ---
    void setCurrentTheme(AppTheme theme);

    // NOUVEAU : Détecte si Windows est en mode sombre
    static bool isSystemDarkMode();

signals:
    void settingsChanged();

private slots:
    void onChooseFont();
    void onChooseColor();
    void onApply();

private:
    // --- Widgets ---
    QCheckBox* chkEnableSpellChecker;
    QCheckBox* chkUnderlineErrors;
    QPushButton* btnFont;
    QPushButton* btnColor;
    QPushButton* btnApply;
    QComboBox* comboTheme;
    QLineEdit* editDeepSeekApiKey;
    QLineEdit* editAiModel;

    // --- Stockage des paramètres ---
    QFont selectedFont;
    QColor selectedColor;
    AppTheme currentTheme;
    QString deepSeekApiKeyValue;
    QString aiModelValue;

    void loadDefaults();
    void loadFromSettings();
    void saveToSettings() const;
};

#endif // SETTINGS_H