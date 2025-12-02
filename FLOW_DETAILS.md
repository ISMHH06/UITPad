# 🔄 **Flow Détaillé des Opérations - UIT-Pad**

## 📋 **Architecture Générale**

```
MainWindow (Interface + Orchestration)
    ↓
TextEditor (Gestion multi-documents)
    ↓
Document (Un fichier = un Document)
    ↓
CppSyntaxHighlighter (Coloration en temps réel)
    ↓
CppSyntaxRules (Règles de coloration)
```

**Classes transversales:**
- `SpellChecker`: Vérifie l'orthographe
- `AIAssistant`: Communication API IA
- `Settings`: Préférences utilisateur

---

## 1️⃣ **OUVRIR UN FICHIER**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Fichier → Ouvrir" ou Ctrl+O
   └─> MainWindow::onOpenFile() appelé

2. MainWindow → QFileDialog
   └─> Affiche la boîte de dialogue
   └─> Utilisateur sélectionne un fichier
   └─> Retourne le chemin du fichier (QString)

3. MainWindow → TextEditor
   └─> textEditor->openDocument(filePath)
   └─> TextEditor vérifie si le fichier est déjà ouvert

4. TextEditor → Document (nouveau)
   └─> Document* doc = new Document(filePath)
   └─> doc->loadFromFile() appelé

5. Document → QFile
   └─> Ouvre le fichier en lecture
   └─> Lit tout le contenu avec QTextStream
   └─> Stocke dans Document::content (QString)
   └─> Met à jour Document::isModified = false
   └─> Retourne true si succès

6. Document → TextEditor
   └─> Retourne le Document* créé
   └─> TextEditor ajoute à documents (QVector<Document*>)
   └─> TextEditor met à jour currentDocumentIndex

7. TextEditor → MainWindow (Signal Qt)
   └─> Émet signal: documentOpened(Document*)
   └─> MainWindow reçoit via slot: onDocumentOpened()

8. MainWindow → QTextEdit/QPlainTextEdit
   └─> Affiche le contenu: textEdit->setPlainText(doc->getContent())
   └─> Met à jour le titre de la fenêtre avec le nom du fichier

9. MainWindow → CppSyntaxHighlighter
   └─> Crée ou met à jour le highlighter
   └─> highlighter->setDocument(textEdit->document())
   └─> Le highlighter s'attache au document Qt

10. CppSyntaxHighlighter → CppSyntaxRules
    └─> Charge les règles: rules->loadRules()
    └─> Récupère les listes de mots-clés, types, opérateurs

11. CppSyntaxHighlighter → QTextDocument (automatique)
    └─> Qt appelle highlightBlock() pour chaque ligne
    └─> Applique la coloration syntaxique en temps réel

12. MainWindow → SpellChecker (optionnel)
    └─> spellChecker->setLanguage(settings->getLanguage())
    └─> spellChecker->loadDictionary()

13. MainWindow → Settings
    └─> Récupère le thème: settings->getTheme()
    └─> Applique le thème (Dark/Light) à l'interface
```

### **Diagramme de séquence:**

```
User → MainWindow → TextEditor → Document → QFile
                                      ↓
                                   Content
                                      ↓
User ← MainWindow ← TextEditor ← Document
   ↓
QTextEdit (affichage)
   ↓
CppSyntaxHighlighter → CppSyntaxRules
```

---

## 2️⃣ **MODIFIER LE TEXTE**

### **Séquence d'appels:**

```
1. Utilisateur → QTextEdit
   └─> Tape du texte dans la zone d'édition
   └─> QTextEdit émet signal: textChanged()

2. QTextEdit → MainWindow (Signal Qt)
   └─> MainWindow::onTextChanged() connecté au signal
   └─> Slot appelé automatiquement

3. MainWindow → TextEditor
   └─> textEditor->getCurrentDocument()
   └─> Récupère le Document* actif

4. MainWindow → Document
   └─> doc->setContent(textEdit->toPlainText())
   └─> Document met à jour son contenu interne
   └─> Document::isModified = true

5. MainWindow → MainWindow (UI)
   └─> Met à jour le titre: "*nom_fichier" (astérisque = modifié)
   └─> Active le bouton "Sauvegarder"

6. QTextEdit → CppSyntaxHighlighter (automatique)
   └─> Qt détecte le changement de texte
   └─> Appelle highlightBlock() pour les lignes modifiées
   └─> Re-colore uniquement les lignes affectées (optimisation)

7. CppSyntaxHighlighter → CppSyntaxRules
   └─> Pour chaque mot de la ligne:
       ├─> rules->isKeyword(word) → Colorie en bleu
       ├─> rules->isDataType(word) → Colorie en violet
       ├─> rules->isOperator(word) → Colorie en rouge
       └─> rules->isString(word) → Colorie en vert

8. MainWindow → SpellChecker (en arrière-plan, optionnel)
   └─> Si vérification orthographique activée:
       └─> spellChecker->checkWord(word)
       └─> Si mot incorrect → souligne en rouge
       └─> spellChecker->suggestCorrections(word)

9. Document → TextEditor (notification)
   └─> Document émet signal: contentModified()
   └─> TextEditor met à jour sa liste interne
```

### **Points importants:**
- La coloration est **automatique** et **en temps réel**
- Seules les **lignes modifiées** sont re-colorées (performance)
- Le Document marque `isModified = true` pour savoir s'il faut sauvegarder

---

## 3️⃣ **SAUVEGARDER UN FICHIER**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Fichier → Sauvegarder" ou Ctrl+S
   └─> MainWindow::onSaveFile() appelé

2. MainWindow → TextEditor
   └─> textEditor->getCurrentDocument()
   └─> Récupère le Document* actif

3. MainWindow → Document
   └─> Vérifie: doc->isModified()
   └─> Si false → retourne (rien à sauvegarder)

4. MainWindow → Document
   └─> doc->getFilePath()
   └─> Si chemin vide → nouveau fichier (aller à étape 5)
   └─> Si chemin existe → aller à étape 6

5. MainWindow → QFileDialog (nouveau fichier)
   └─> Affiche "Enregistrer sous..."
   └─> Utilisateur choisit nom et emplacement
   └─> Retourne le chemin
   └─> doc->setFilePath(newPath)

6. MainWindow → Document
   └─> doc->saveToFile()
   └─> Document récupère le contenu actuel

7. Document → QTextEdit (via MainWindow)
   └─> doc->getContent() récupère le texte
   └─> Ou Document stocke déjà le contenu à jour

8. Document → QFile
   └─> Ouvre le fichier en écriture (QIODevice::WriteOnly)
   └─> Écrit le contenu avec QTextStream
   └─> Ferme le fichier
   └─> Retourne true si succès

9. Document → Document (état)
   └─> isModified = false
   └─> dateModification = QDateTime::currentDateTime()

10. Document → TextEditor (Signal Qt)
    └─> Émet signal: documentSaved(Document*)

11. TextEditor → MainWindow (Signal Qt)
    └─> Émet signal: documentSaved(Document*)
    └─> MainWindow::onDocumentSaved() reçoit

12. MainWindow → MainWindow (UI)
    └─> Retire l'astérisque du titre: "nom_fichier"
    └─> Désactive ou met à jour le bouton "Sauvegarder"
    └─> Affiche message: "Fichier sauvegardé" (barre d'état)

13. MainWindow → Settings (optionnel)
    └─> Sauvegarde le dernier répertoire utilisé
    └─> settings->setLastDirectory(path)
```

### **Cas spéciaux:**
- **Sauvegarder sous (Ctrl+Shift+S):** Toujours demande un nouveau nom
- **Sauvegarder tout:** Parcourt tous les documents et sauvegarde ceux modifiés

---

## 4️⃣ **FERMER UN DOCUMENT**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Fichier → Fermer" ou Ctrl+W
   └─> Ou clic sur l'onglet de fermeture
   └─> MainWindow::onCloseDocument() appelé

2. MainWindow → TextEditor
   └─> textEditor->getCurrentDocument()
   └─> Récupère le Document* à fermer

3. MainWindow → Document
   └─> doc->isModified()
   └─> Si true → aller à étape 4 (demander confirmation)
   └─> Si false → aller à étape 6 (fermer directement)

4. MainWindow → QMessageBox
   └─> Affiche: "Le document a été modifié. Voulez-vous sauvegarder?"
   └─> Boutons: "Sauvegarder", "Ignorer", "Annuler"
   └─> Utilisateur choisit

5. MainWindow → MainWindow
   └─> Si "Sauvegarder" → Appelle onSaveFile() (voir section 3)
   └─> Si "Ignorer" → Continue à l'étape 6
   └─> Si "Annuler" → Retourne (ne ferme pas)

6. MainWindow → TextEditor
   └─> textEditor->closeDocument(doc)
   └─> TextEditor retire le Document de sa liste

7. TextEditor → Document
   └─> Supprime le Document* (delete doc)
   └─> Document destructeur appelé

8. TextEditor → TextEditor
   └─> Met à jour currentDocumentIndex
   └─> Si d'autres documents → active le suivant
   └─> Si aucun document → currentDocumentIndex = -1

9. TextEditor → MainWindow (Signal Qt)
    └─> Émet signal: documentClosed(Document*)
    └─> Ou: currentDocumentChanged(Document*)

10. MainWindow → MainWindow (UI)
    └─> Si document suivant existe:
        └─> Affiche le contenu du nouveau document actif
        └─> Met à jour le titre
    └─> Si aucun document:
        └─> Efface QTextEdit (setPlainText(""))
        └─> Titre: "UIT-Pad - Sans titre"
        └─> Désactive les menus d'édition

11. MainWindow → CppSyntaxHighlighter
    └─> Si nouveau document → Re-attache le highlighter
    └─> Si aucun document → Détache le highlighter
```

### **Cas spéciaux:**
- **Fermer tout (Ctrl+Shift+W):** Parcourt tous les documents, demande confirmation pour chacun
- **Fermer l'application:** Vérifie tous les documents modifiés avant de quitter

---

## 5️⃣ **CRÉER UN NOUVEAU DOCUMENT**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Fichier → Nouveau" ou Ctrl+N
   └─> MainWindow::onNewFile() appelé

2. MainWindow → TextEditor
   └─> textEditor->createNewDocument()
   └─> TextEditor crée un nouveau Document

3. TextEditor → Document (nouveau)
   └─> Document* doc = new Document()
   └─> Document::filePath = "" (vide = nouveau fichier)
   └─> Document::content = "" (vide)
   └─> Document::isModified = false

4. TextEditor → TextEditor
   └─> Ajoute doc à documents (QVector<Document*>)
   └─> currentDocumentIndex = documents.size() - 1

5. TextEditor → MainWindow (Signal Qt)
   └─> Émet signal: documentCreated(Document*)

6. MainWindow → MainWindow (UI)
   └─> textEdit->setPlainText("")
   └─> Titre: "UIT-Pad - Sans titre"
   └─> Met le focus sur QTextEdit

7. MainWindow → CppSyntaxHighlighter
   └─> Re-attache le highlighter au nouveau document
   └─> Le highlighter est prêt pour la coloration

8. MainWindow → Settings
    └─> Récupère le modèle par défaut (template)
    └─> Si template existe → Insère le contenu du template
```

---

## 6️⃣ **RECHERCHE/REMPLACER**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Édition → Rechercher" ou Ctrl+F
   └─> MainWindow::onFind() appelé

2. MainWindow → QDialog (boîte de recherche)
   └─> Affiche la boîte de dialogue de recherche
   └─> Utilisateur entre le texte à chercher

3. MainWindow → QTextEdit
   └─> textEdit->find(searchText, options)
   └─> QTextEdit cherche dans son contenu
   └─> Met en surbrillance la première occurrence

4. Utilisateur → MainWindow
   └─> Clic sur "Remplacer" ou Ctrl+H
   └─> MainWindow::onReplace() appelé

5. MainWindow → QTextEdit
   └─> Récupère la sélection actuelle
   └─> textEdit->textCursor().selectedText()
   └─> Si correspond à searchText → remplace

6. MainWindow → Document
   └─> doc->setContent(textEdit->toPlainText())
   └─> doc->isModified = true

7. QTextEdit → CppSyntaxHighlighter
   └─> Re-colore la ligne modifiée automatiquement
```

---

## 7️⃣ **ANNULER/RÉTABLIR (Undo/Redo)**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Édition → Annuler" ou Ctrl+Z
   └─> MainWindow::onUndo() appelé

2. MainWindow → QTextEdit
   └─> textEdit->undo()
   └─> QTextEdit gère automatiquement la pile d'annulation
   └─> Restaure l'état précédent

3. QTextEdit → MainWindow (Signal Qt)
   └─> Émet signal: textChanged()
   └─> MainWindow::onTextChanged() appelé

4. MainWindow → Document
   └─> doc->setContent(textEdit->toPlainText())
   └─> Met à jour le contenu du Document

5. QTextEdit → CppSyntaxHighlighter
   └─> Re-colore les lignes modifiées

6. MainWindow → MainWindow (UI)
   └─> Met à jour l'état des boutons Undo/Redo
   └─> textEdit->isUndoAvailable()
   └─> textEdit->isRedoAvailable()
```

**Note:** QTextEdit gère automatiquement la pile Undo/Redo. Pas besoin de classe séparée.

---

## 8️⃣ **UTILISER L'ASSISTANT IA**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Sélectionne du texte dans QTextEdit
   └─> Clic sur "IA → Analyser le code" ou raccourci clavier

2. MainWindow → TextEditor
   └─> textEditor->getCurrentDocument()
   └─> Récupère le Document* actif

3. MainWindow → Document
   └─> doc->getContent()
   └─> Ou récupère la sélection: textEdit->textCursor().selectedText()

4. MainWindow → AIAssistant
   └─> aiAssistant->analyzeCode(selectedText)
   └─> Ou: aiAssistant->suggestImprovements(code)

5. AIAssistant → Settings
   └─> Récupère la clé API: settings->getApiKey()
   └─> Récupère l'URL de l'API: settings->getApiUrl()

6. AIAssistant → QNetworkAccessManager
   └─> Crée une requête HTTP POST
   └─> Construit le JSON avec le code et la requête
   └─> Envoie la requête à l'API

7. AIAssistant → API Externe (OpenAI, Claude, etc.)
   └─> Attend la réponse
   └─> Reçoit le JSON de réponse

8. API Externe → AIAssistant
   └─> Retourne les suggestions/analyses

9. AIAssistant → QNetworkReply
   └─> Lit la réponse JSON
   └─> Parse le contenu

10. AIAssistant → MainWindow (Signal Qt)
    └─> Émet signal: analysisComplete(QString result)
    └─> Ou: suggestionsReady(QStringList suggestions)

11. MainWindow → QDialog
    └─> Affiche les résultats dans une boîte de dialogue
    └─> Ou dans un panneau latéral
    └─> Utilisateur peut copier/appliquer les suggestions
```

---

## 9️⃣ **CHANGER DE THÈME (Dark/Light)**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Clic sur "Paramètres → Thème → Sombre"
   └─> MainWindow::onThemeChanged() appelé

2. MainWindow → Settings
   └─> settings->setTheme("dark")
   └─> Settings sauvegarde dans QSettings

3. Settings → QSettings
   └─> QSettings::setValue("theme", "dark")
   └─> Sauvegarde dans le registre/fichier de config

4. Settings → MainWindow (Signal Qt)
   └─> Émet signal: themeChanged(QString theme)

5. MainWindow → MainWindow (UI)
   └─> Applique la feuille de style (QSS)
   └─> setStyleSheet(darkThemeStylesheet)
   └─> Met à jour tous les widgets

6. MainWindow → CppSyntaxHighlighter
    └─> Met à jour les couleurs de syntaxe
    └─> highlighter->setTheme(theme)
    └─> Re-colore tout le document avec les nouvelles couleurs
```

---

## 🔟 **VÉRIFICATION ORTHOGRAPHIQUE**

### **Séquence d'appels:**

```
1. Utilisateur → MainWindow
   └─> Tape du texte dans QTextEdit
   └─> QTextEdit émet signal: textChanged()

2. MainWindow → SpellChecker (en arrière-plan)
   └─> Si vérification activée:
       └─> spellChecker->checkDocument(textEdit->toPlainText())

3. SpellChecker → Settings
   └─> Récupère la langue: settings->getLanguage()
   └─> Charge le dictionnaire correspondant

4. SpellChecker → QFile
   └─> Ouvre le fichier dictionnaire (ex: "fr_FR.txt")
   └─> Charge tous les mots dans un QSet<QString>

5. SpellChecker → SpellChecker
   └─> Parse le texte en mots
   └─> Pour chaque mot:
       └─> Vérifie: dictionary.contains(word.toLowerCase())
       └─> Si absent → mot incorrect

6. SpellChecker → MainWindow (Signal Qt)
   └─> Émet signal: misspelledWordFound(int position, QString word)

7. MainWindow → QTextEdit
   └─> Applique un formatage (soulignement rouge)
   └─> QTextCharFormat avec underline style
   └─> textCursor.setCharFormat(format)

8. Utilisateur → MainWindow
   └─> Clic droit sur le mot souligné
   └─> MainWindow::onWordRightClick() appelé

9. MainWindow → SpellChecker
   └─> spellChecker->suggestCorrections(word)
   └─> SpellChecker utilise un algorithme (Levenshtein, etc.)
   └─> Retourne une liste de suggestions

10. MainWindow → QMenu (contextuel)
    └─> Affiche un menu avec les suggestions
    └─> Utilisateur choisit une correction

11. MainWindow → QTextEdit
    └─> Remplace le mot par la suggestion choisie
    └─> textEdit->textCursor().insertText(correction)
```

---

## 📊 **Résumé des Flux Principaux**

### **Flux d'ouverture:**
```
User → MainWindow → TextEditor → Document → QFile
                                      ↓
                                   Content
                                      ↓
User ← MainWindow ← TextEditor ← Document
   ↓
QTextEdit → CppSyntaxHighlighter → CppSyntaxRules
```

### **Flux de modification:**
```
User → QTextEdit → MainWindow → Document
                            ↓
                    isModified = true
                            ↓
              CppSyntaxHighlighter (auto)
                            ↓
              SpellChecker (optionnel)
```

### **Flux de sauvegarde:**
```
User → MainWindow → Document → QFile
                            ↓
                    isModified = false
                            ↓
                    UI mise à jour
```

### **Flux IA:**
```
User → MainWindow → AIAssistant → QNetworkAccessManager → API
                                                              ↓
User ← MainWindow ← AIAssistant ← QNetworkReply ← API
```

---

## 🔗 **Connexions Qt (Signaux/Slots)**

### **Signaux principaux:**

```cpp
// TextEditor émet:
- documentOpened(Document*)
- documentClosed(Document*)
- documentSaved(Document*)
- currentDocumentChanged(Document*)

// Document émet:
- contentModified()
- fileSaved()

// Settings émet:
- themeChanged(QString)
- languageChanged(QString)
- settingsChanged()

// AIAssistant émet:
- analysisComplete(QString)
- errorOccurred(QString)

// SpellChecker émet:
- misspelledWordFound(int, QString)
- suggestionsReady(QStringList)
```

### **Slots dans MainWindow:**

```cpp
- onDocumentOpened(Document*)
- onDocumentClosed(Document*)
- onTextChanged()
- onThemeChanged(QString)
- onAnalysisComplete(QString)
```

---

## 💡 **Points Clés à Retenir**

1. **TextEditor** est le gestionnaire de documents (multi-documents)
2. **Document** représente UN fichier (contenu + métadonnées)
3. **CppSyntaxHighlighter** s'attache au QTextDocument et colore automatiquement
4. **Settings** centralise toutes les préférences et les sauvegarde
5. **Signaux Qt** permettent la communication asynchrone entre classes
6. **QTextEdit** gère automatiquement Undo/Redo
7. **SpellChecker** et **AIAssistant** fonctionnent en arrière-plan
8. Toute modification passe par **Document** pour garder la cohérence

---

## 📝 **Exemple de Code Qt - Connexions Signaux/Slots**

### **Dans MainWindow::MainWindow():**

```cpp
// Connexion TextEditor → MainWindow
connect(textEditor, &TextEditor::documentOpened,
        this, &MainWindow::onDocumentOpened);
        
connect(textEditor, &TextEditor::documentClosed,
        this, &MainWindow::onDocumentClosed);

// Connexion QTextEdit → MainWindow
connect(textEdit, &QTextEdit::textChanged,
        this, &MainWindow::onTextChanged);

// Connexion Settings → MainWindow
connect(settings, &Settings::themeChanged,
        this, &MainWindow::onThemeChanged);

// Connexion AIAssistant → MainWindow
connect(aiAssistant, &AIAssistant::analysisComplete,
        this, &MainWindow::onAnalysisComplete);

// Connexion SpellChecker → MainWindow
connect(spellChecker, &SpellChecker::misspelledWordFound,
        this, &MainWindow::onMisspelledWordFound);
```

### **Dans TextEditor::openDocument():**

```cpp
Document* TextEditor::openDocument(const QString& filePath) {
    Document* doc = new Document(filePath);
    if (doc->loadFromFile()) {
        documents.append(doc);
        currentDocumentIndex = documents.size() - 1;
        emit documentOpened(doc);  // Émet le signal
        return doc;
    }
    delete doc;
    return nullptr;
}
```

---

## 🎯 **Diagramme de Classes Simplifié**

```
┌─────────────┐
│ MainWindow  │
│             │
│ - textEdit  │──┐
│ - textEditor│  │
│ - highlighter│ │
│ - settings  │  │
└─────────────┘  │
       │         │
       │         │
       ▼         │
┌─────────────┐  │
│ TextEditor  │  │
│             │  │
│ - documents │  │
│ - currentIdx│  │
└─────────────┘  │
       │         │
       │         │
       ▼         │
┌─────────────┐  │
│  Document   │  │
│             │  │
│ - filePath  │  │
│ - content   │  │
│ - isModified│  │
└─────────────┘  │
                 │
                 │
                 ▼
         ┌───────────────┐
         │   QTextEdit   │
         │               │
         │ - document()  │
         └───────────────┘
                 │
                 │
                 ▼
    ┌────────────────────────┐
    │ CppSyntaxHighlighter   │
    │                        │
    │ - highlightBlock()     │
    └────────────────────────┘
                 │
                 │
                 ▼
    ┌────────────────────────┐
    │   CppSyntaxRules       │
    │                        │
    │ - isKeyword()          │
    │ - isDataType()         │
    └────────────────────────┘
```

---

## 🔄 **Cycle de Vie d'un Document**

```
1. Création
   └─> Document* doc = new Document()
   └─> Ajouté à TextEditor::documents

2. Ouverture/Chargement
   └─> doc->loadFromFile()
   └─> Contenu chargé depuis le disque

3. Modification
   └─> doc->setContent(newContent)
   └─> isModified = true

4. Sauvegarde
   └─> doc->saveToFile()
   └─> isModified = false

5. Fermeture
   └─> Vérification isModified
   └─> Demande confirmation si nécessaire
   └─> delete doc
```

---

## 📚 **Références Qt Utilisées**

- **QTextEdit / QPlainTextEdit**: Zone d'édition de texte
- **QTextDocument**: Document Qt pour le contenu
- **QSyntaxHighlighter**: Base pour la coloration syntaxique
- **QFile / QTextStream**: Lecture/écriture de fichiers
- **QSettings**: Sauvegarde des préférences
- **QNetworkAccessManager**: Requêtes HTTP pour l'IA
- **QFileDialog**: Boîtes de dialogue de fichiers
- **QMessageBox**: Boîtes de dialogue de confirmation
- **Signaux/Slots**: Communication entre objets Qt

---

**Document créé pour le projet UIT-Pad - Éditeur de Texte Graphique**

