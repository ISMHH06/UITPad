#include "Settings.h"
#include <QVBoxLayout>
#include <QLabel>

Settings::Settings(QWidget* parent)
    : QDialog(parent),
    selectedFont("Arial", 12),
    selectedColor(Qt::black),
    currentTheme(Light)
{
    setWindowTitle("Paramètres");
    setMinimumWidth(300);

    // --- Widgets ---
    chkEnableSpellChecker = new QCheckBox("Activer le correcteur orthographique", this);
    chkUnderlineErrors = new QCheckBox("Souligner les fautes", this);

    // Configuration du sélecteur de thème
    comboTheme = new QComboBox(this);
    comboTheme->addItem("Mode Clair (Light)", Light);
    comboTheme->addItem("Mode Sombre (Dark)", Dark);
    comboTheme->addItem("Mode Hacker (Terminal)", Hacker);

    btnFont = new QPushButton("Choisir la police", this);
    btnColor = new QPushButton("Choisir la couleur du texte", this);
    btnApply = new QPushButton("Appliquer", this);

    // --- Mise en page ---
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("<b>Apparence :</b>", this));
    layout->addWidget(comboTheme);
    layout->addSpacing(5);
    layout->addWidget(btnFont);
    layout->addWidget(btnColor);

    layout->addSpacing(15);

    layout->addWidget(new QLabel("<b>Correction :</b>", this));
    layout->addWidget(chkEnableSpellChecker);
    layout->addWidget(chkUnderlineErrors);

    layout->addSpacing(20);
    layout->addWidget(btnApply);

    setLayout(layout);

    // --- Connexions ---
    connect(btnFont, &QPushButton::clicked, this, &Settings::onChooseFont);
    connect(btnColor, &QPushButton::clicked, this, &Settings::onChooseColor);
    connect(btnApply, &QPushButton::clicked, this, &Settings::onApply);

    loadDefaults();
}

void Settings::loadDefaults()
{
    chkEnableSpellChecker->setChecked(true);
    chkUnderlineErrors->setChecked(true);
    comboTheme->setCurrentIndex(Light);
}

// --- Accesseurs ---
bool Settings::isSpellCheckEnabled() const { return chkEnableSpellChecker->isChecked(); }
bool Settings::underlineErrors() const { return chkUnderlineErrors->isChecked(); }
QFont Settings::getEditorFont() const { return selectedFont; }
QColor Settings::getEditorColor() const { return selectedColor; }
Settings::AppTheme Settings::getSelectedTheme() const { return currentTheme; }

// --- NOUVEAU : Setter ---
void Settings::setCurrentTheme(AppTheme theme) {
    currentTheme = theme;
    // Trouve l'index correspondant dans la combobox et le sélectionne
    int index = comboTheme->findData(theme);
    if (index != -1) {
        comboTheme->setCurrentIndex(index);
    }
}

// --- Slots ---
void Settings::onChooseFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, selectedFont, this);
    if (ok)
        selectedFont = font;
}

void Settings::onChooseColor()
{
    QColor color = QColorDialog::getColor(selectedColor, this);
    if (color.isValid())
        selectedColor = color;
}

void Settings::onApply()
{
    // Sauvegarder le thème choisi
    currentTheme = static_cast<AppTheme>(comboTheme->currentData().toInt());

    emit settingsChanged();
    close();
}
