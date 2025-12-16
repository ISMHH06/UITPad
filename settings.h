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

class Settings : public QDialog
{
    Q_OBJECT

public:
    // --- Enumération pour les thèmes ---
    enum AppTheme {
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

    // --- NOUVEAU : Setter pour définir le thème actuel à l'ouverture ---
    void setCurrentTheme(AppTheme theme);

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

    // --- Stockage des paramètres ---
    QFont selectedFont;
    QColor selectedColor;
    AppTheme currentTheme;

    void loadDefaults();
};

#endif // SETTINGS_H
