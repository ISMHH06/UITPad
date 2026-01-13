#include "Settings.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QFont>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

Settings::Settings(QWidget* parent)
    : QDialog(parent),
    selectedFont("Arial", 12),
    selectedColor(Qt::black),
    currentTheme(System)  // Par défaut : Mode Système
{
    setWindowTitle("Paramètres");
    setMinimumWidth(300);

    // --- Widgets ---
    chkEnableSpellChecker = new QCheckBox("Activer le correcteur orthographique", this);
    chkUnderlineErrors = new QCheckBox("Souligner les fautes", this);

    // Configuration du sélecteur de thème
    comboTheme = new QComboBox(this);
    comboTheme->addItem("Système (Auto)", System);      // NOUVEAU
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
    loadFromSettings();
}

void Settings::loadDefaults()
{
    chkEnableSpellChecker->setChecked(true);
    chkUnderlineErrors->setChecked(true);
    comboTheme->setCurrentIndex(comboTheme->findData(System));  // Par défaut : Système
}

void Settings::loadFromSettings()
{
    QSettings s;

    // Spell
    chkEnableSpellChecker->setChecked(s.value("spell/enabled", chkEnableSpellChecker->isChecked()).toBool());
    chkUnderlineErrors->setChecked(s.value("spell/underlineErrors", chkUnderlineErrors->isChecked()).toBool());

    // UI theme
    currentTheme = static_cast<AppTheme>(s.value("ui/theme", static_cast<int>(currentTheme)).toInt());
    int themeIndex = comboTheme->findData(currentTheme);
    if (themeIndex != -1) {
        comboTheme->setCurrentIndex(themeIndex);
    }

    // Font / Color
    selectedFont = qvariant_cast<QFont>(s.value("ui/font", selectedFont));
    selectedColor = qvariant_cast<QColor>(s.value("ui/textColor", selectedColor));
}

void Settings::saveToSettings() const
{
    QSettings s;
    s.setValue("spell/enabled", chkEnableSpellChecker->isChecked());
    s.setValue("spell/underlineErrors", chkUnderlineErrors->isChecked());
    s.setValue("ui/theme", static_cast<int>(currentTheme));
    s.setValue("ui/font", selectedFont);
    s.setValue("ui/textColor", selectedColor);
}

// --- NOUVEAU : Détecte si Windows est en mode sombre ---
bool Settings::isSystemDarkMode()
{
#ifdef Q_OS_WIN
    // Lire la clé de registre Windows pour le thème
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat
    );

    // AppsUseLightTheme: 0 = Dark, 1 = Light
    int value = settings.value("AppsUseLightTheme", 1).toInt();

    // DEBUG: Print the value
    qDebug() << "Windows theme registry value:" << value << "(0=Dark, 1=Light)";

    return (value == 0);  // 0 = Dark mode
#else
    // Sur Linux/Mac, on peut vérifier différemment ou retourner false par défaut
    qDebug() << "Not Windows, defaulting to light mode";
    return false;
#endif
}

// --- Accesseurs ---
bool Settings::isSpellCheckEnabled() const { return chkEnableSpellChecker->isChecked(); }
bool Settings::underlineErrors() const { return chkUnderlineErrors->isChecked(); }
QFont Settings::getEditorFont() const { return selectedFont; }
QColor Settings::getEditorColor() const { return selectedColor; }
Settings::AppTheme Settings::getSelectedTheme() const { return currentTheme; }

// --- Setter ---
void Settings::setCurrentTheme(AppTheme theme) {
    currentTheme = theme;
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

    saveToSettings();

    emit settingsChanged();
    close();
}