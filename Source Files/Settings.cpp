#include "Settings.h"
#include <QVBoxLayout>
#include <QLabel>

Settings::Settings(QWidget* parent)
    : QDialog(parent),
    selectedFont("Arial", 12),
    selectedColor(Qt::black)
{
    setWindowTitle("Paramètres");
    setMinimumWidth(300);

    // --- Widgets ---
    chkEnableSpellChecker = new QCheckBox("Activer le correcteur orthographique");
    chkUnderlineErrors = new QCheckBox("Souligner les fautes");

    btnFont = new QPushButton("Choisir la police");
    btnColor = new QPushButton("Choisir la couleur du texte");
    btnApply = new QPushButton("Appliquer");

    // --- Mise en page ---
    QVBoxLayout* layout = new QVBoxLayout();

    layout->addWidget(chkEnableSpellChecker);
    layout->addWidget(chkUnderlineErrors);

    layout->addSpacing(10);
    layout->addWidget(btnFont);
    layout->addWidget(btnColor);

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
}

// --- Accesseurs ---
bool Settings::isSpellCheckEnabled() const { return chkEnableSpellChecker->isChecked(); }
bool Settings::underlineErrors() const { return chkUnderlineErrors->isChecked(); }
QFont Settings::getEditorFont() const { return selectedFont; }
QColor Settings::getEditorColor() const { return selectedColor; }

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
    emit settingsChanged();  // Prévenir la fenêtre principale
    close();
}
