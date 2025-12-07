#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QColor>
#include <QFont>
#include <QFontDialog>
#include <QColorDialog>

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget* parent = nullptr);

    // --- Accesseurs (Getters) ---
    bool isSpellCheckEnabled() const;
    bool underlineErrors() const;
    QFont getEditorFont() const;
    QColor getEditorColor() const;

signals:
    void settingsChanged();   // Signal envoyé lorsque l’utilisateur clique sur "Appliquer"

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

    // --- Stockage des paramètres ---
    QFont selectedFont;
    QColor selectedColor;

    void loadDefaults(); // Charge les valeurs par défaut
};

#endif // SETTINGS_H
