# 📝 Changelog - Implémentation du Système Multi-Documents

## 🎯 Objectif

Implémentation d'un système de gestion de documents multiples avec interface à onglets dans l'application UITPad, permettant d'ouvrir, modifier et gérer plusieurs fichiers simultanément.

---

## 🏗️ Architecture Implémentée

### Structure en 3 Couches

```
MainWindow (Interface utilisateur + Onglets)
    ↓
TextEditor (Gestionnaire multi-documents)
    ↓
Document[] (Liste de documents individuels)
```

### Séparation des Responsabilités

- **MainWindow** : Interface utilisateur, gestion des onglets, coordination avec TextEditor
- **TextEditor** : Gestion de plusieurs documents (ouverture, fermeture, navigation, sauvegarde)
- **Document** : Représentation d'un fichier unique (contenu, état, métadonnées)

---

## 📦 Nouvelles Classes Créées

### 1. Document (Document.h / Document.cpp)

**Responsabilité** : Représente un fichier unique avec son état et ses métadonnées.

#### Fonctionnalités Implémentées :

- **Gestion du fichier** :
  - `loadFromFile()` : Charge le contenu depuis le disque
  - `saveToFile()` : Sauvegarde le contenu sur le disque
  - `getFilePath()` / `setFilePath()` : Gestion du chemin de fichier
  - `getFileName()` : Retourne uniquement le nom du fichier
  - `hasFilePath()` : Vérifie si le document a un chemin

- **Gestion du contenu** :
  - `getContent()` / `setContent()` : Accès au contenu texte
  - `appendContent()` : Ajoute du texte au contenu
  - `clearContent()` : Vide le contenu
  - `getContentLength()` : Longueur du contenu
  - `getLineCount()` : Nombre de lignes

- **État de modification** :
  - `getIsModified()` / `setIsModified()` : Gestion de l'état modifié
  - `markAsModified()` : Marque comme modifié
  - `markAsSaved()` : Marque comme sauvegardé

- **Métadonnées** :
  - `getDateCreated()` / `getDateModified()` : Dates de création/modification
  - `updateModificationDate()` : Met à jour la date de modification

- **Signaux Qt** :
  - `contentChanged()` : Émis quand le contenu change
  - `filePathChanged(QString)` : Émis quand le chemin change
  - `modificationStateChanged(bool)` : Émis quand l'état de modification change
  - `fileSaved()` : Émis après sauvegarde
  - `fileLoaded()` : Émis après chargement

### 2. TextEditor (TextEditor.h / TextEditor.cpp)

**Responsabilité** : Gestionnaire central de tous les documents ouverts.

#### Fonctionnalités Implémentées :

- **Gestion des documents** :
  - `openDocument(QString filePath)` : Ouvre un fichier (évite les doublons)
  - `createNewDocument()` : Crée un nouveau document vide
  - `closeDocument(Document* doc)` : Ferme un document
  - `closeDocument(int index)` : Ferme un document par index
  - `saveDocument(Document* doc)` : Sauvegarde un document
  - `saveDocument(int index)` : Sauvegarde un document par index
  - `saveAllDocuments()` : Sauvegarde tous les documents modifiés

- **Navigation entre documents** :
  - `switchToDocument(int index)` : Bascule vers un document par index
  - `switchToDocument(Document* doc)` : Bascule vers un document par pointeur
  - `switchToNextDocument()` : Document suivant
  - `switchToPreviousDocument()` : Document précédent

- **Accès aux documents** :
  - `getCurrentDocument()` : Retourne le document actif
  - `getDocument(int index)` : Retourne un document par index
  - `getCurrentDocumentIndex()` : Index du document actif
  - `getDocumentCount()` : Nombre de documents ouverts
  - `findDocumentIndex(QString filePath)` : Trouve l'index d'un fichier
  - `isDocumentOpen(QString filePath)` : Vérifie si un fichier est ouvert

- **Vérifications** :
  - `hasDocuments()` : Vérifie s'il y a des documents
  - `hasUnsavedDocuments()` : Vérifie s'il y a des modifications non sauvegardées

- **Signaux Qt** :
  - `documentOpened(Document*)` : Émis quand un document est ouvert
  - `documentClosed(Document*)` : Émis quand un document est fermé
  - `documentSaved(Document*)` : Émis quand un document est sauvegardé
  - `currentDocumentChanged(Document*)` : Émis quand on change de document
  - `documentModified(Document*)` : Émis quand un document est modifié

#### Caractéristiques Techniques :

- **Stockage** : `QVector<Document*>` pour la liste des documents
- **Index courant** : `int currentDocumentIndex` (-1 si aucun document)
- **Détection des doublons** : Vérifie si un fichier est déjà ouvert avant de l'ouvrir à nouveau
- **Gestion mémoire** : Suppression automatique des documents dans le destructeur

---

## 🔄 Modifications de MainWindow

### Changements Structurels

#### Avant :
- Un seul `QPlainTextEdit` pour afficher le contenu
- Gestion directe des fichiers avec `QFile` et `QTextStream`
- Pas de gestion multi-documents

#### Après :
- `QTabWidget` pour afficher plusieurs documents en onglets
- `TextEditor*` pour gérer les documents
- Chaque onglet a son propre `QPlainTextEdit`
- Synchronisation automatique entre onglets et documents

### Nouvelles Fonctionnalités

#### 1. Système d'Onglets

- **Création d'onglets** : Chaque document ouvert crée un nouvel onglet
- **Titres dynamiques** : Le nom du fichier s'affiche dans l'onglet avec un astérisque (*) si modifié
- **Onglets déplaçables** : Possibilité de réorganiser les onglets par glisser-déposer
- **Fermeture par onglet** : Bouton X sur chaque onglet pour fermer le document

#### 2. Gestion des Documents

- **Menu Fichier enrichi** :
  - "Nouveau" : Crée un nouveau document
  - "Ouvrir" : Ouvre un fichier (détecte les doublons)
  - "Sauvegarder" : Sauvegarde le document actif
  - "Sauvegarder sous..." : Sauvegarde avec un nouveau nom
  - "Fermer" : Ferme le document actif (demande confirmation si modifié)
  - "Quitter" : Ferme l'application

#### 3. Synchronisation Automatique

- **Changement d'onglet** → Bascule automatiquement vers le document correspondant dans TextEditor
- **Modification du texte** → Met à jour le Document et le titre de l'onglet
- **Fermeture d'onglet** → Demande confirmation si le document est modifié

### Méthodes Ajoutées

- `getCurrentTextEdit()` : Retourne le QPlainTextEdit de l'onglet actif
- `getTextEditForDocument(Document*)` : Trouve le QPlainTextEdit d'un document
- `updateTabTitle(Document*)` : Met à jour le titre d'un onglet
- `onTabChanged(int index)` : Gère le changement d'onglet
- `onTabCloseRequested(int index)` : Gère la fermeture d'onglet

### Connexions Signaux/Slots

```cpp
// TextEditor → MainWindow
connect(textEditor, &TextEditor::documentOpened, ...)
connect(textEditor, &TextEditor::documentClosed, ...)
connect(textEditor, &TextEditor::documentSaved, ...)
connect(textEditor, &TextEditor::currentDocumentChanged, ...)

// QTabWidget → MainWindow
connect(tabWidget, &QTabWidget::currentChanged, ...)
connect(tabWidget, &QTabWidget::tabCloseRequested, ...)

// QPlainTextEdit (par onglet) → MainWindow
connect(textEdit, &QPlainTextEdit::textChanged, ...)
```

---

## 🔧 Modifications Techniques

### Configuration du Projet (UITPad.vcxproj)

#### Fichiers marqués pour MOC (Meta-Object Compiler) :

- `Document.h` → `<QtMoc Include="Header Files\Document.h" />`
- `TextEditor.h` → `<QtMoc Include="Header Files\TextEditor.h" />`
- `MainWindow.h` → `<QtMoc Include="Header Files\MainWindow.h" />`

**Raison** : Ces classes utilisent `Q_OBJECT` et des signaux/slots, nécessitant le traitement par MOC.

### Gestion de la Mémoire

- **Document** : Supprimé automatiquement par Qt (parent/child)
- **TextEditor** : Supprime tous les documents dans son destructeur
- **QPlainTextEdit** : Supprimé automatiquement quand l'onglet est fermé

### Stockage des Données

- **Association Document ↔ QPlainTextEdit** : Utilisation de `QObject::setProperty()` pour stocker le pointeur Document dans chaque QPlainTextEdit
- **Recherche de documents** : Parcours des onglets pour trouver le QPlainTextEdit associé à un Document

---

## ✨ Fonctionnalités Utilisateur

### 1. Ouverture Multiple de Fichiers

- Ouvrir plusieurs fichiers via "Fichier → Ouvrir"
- Chaque fichier s'ouvre dans un nouvel onglet
- Détection automatique des doublons (si un fichier est déjà ouvert, bascule vers l'onglet existant)

### 2. Navigation Entre Documents

- **Clic sur onglet** : Bascule vers le document
- **Glisser-déposer** : Réorganise les onglets
- **Raccourcis clavier** : Ctrl+Tab pour naviguer (à implémenter si nécessaire)

### 3. Indicateurs Visuels

- **Astérisque (*)** dans le titre de l'onglet si le document est modifié
- **Titre de la fenêtre** : Affiche le nom du fichier actif avec astérisque si modifié
- **Bouton de fermeture** : X visible sur chaque onglet

### 4. Gestion des Modifications

- **Confirmation avant fermeture** : Si un document est modifié, demande confirmation
- **Options** : Sauvegarder, Ignorer, Annuler
- **Sauvegarde automatique** : Possibilité d'étendre avec auto-save (non implémenté)

---

## 🐛 Corrections Apportées

### Problèmes Résolus

1. **Erreurs MOC** : Configuration correcte des fichiers pour le Meta-Object Compiler
2. **Include guards** : Remplacement de `#pragma once` par des guards classiques pour compatibilité MOC
3. **Références obsolètes** : Suppression de `textArea` remplacé par le système d'onglets
4. **Syntaxe Qt 6** : Correction de `QMenu::addAction()` pour Qt 6

### Optimisations

- **Détection des doublons** : Utilise le chemin canonique pour éviter les doublons
- **Gestion mémoire** : Suppression automatique des objets Qt
- **Synchronisation** : Utilisation de `blockSignals()` pour éviter les boucles infinies

---

## 📋 Fichiers Modifiés

### Nouveaux Fichiers
- `Header Files/Document.h`
- `Source Files/Document.cpp`
- `Header Files/TextEditor.h`
- `Source Files/TextEditor.cpp`

### Fichiers Modifiés
- `Header Files/MainWindow.h`
- `Source Files/MainWindow.cpp`
- `UITPad.vcxproj` (ajout de `<QtMoc>` pour Document, TextEditor, MainWindow)
- `UITPad.vcxproj.filters` (ajout des fichiers dans la section QtMoc)

---

## 🚀 Prochaines Étapes Possibles

### Améliorations Futures

1. **Raccourcis clavier** :
   - Ctrl+Tab : Document suivant
   - Ctrl+Shift+Tab : Document précédent
   - Ctrl+W : Fermer l'onglet actif

2. **Menu contextuel onglets** :
   - Clic droit sur onglet → Fermer, Fermer les autres, Fermer tout

3. **Historique des fichiers** :
   - Menu "Fichiers récents" avec les derniers fichiers ouverts

4. **Sauvegarde automatique** :
   - Auto-save périodique des documents modifiés

5. **Recherche globale** :
   - Rechercher dans tous les documents ouverts

6. **Onglets détachables** :
   - Possibilité de détacher un onglet dans une fenêtre séparée

---

## 📝 Notes Techniques

### Dépendances Qt

- `QtCore` : QObject, QString, QVector, QDateTime
- `QtGui` : QPlainTextEdit, QTabWidget
- `QtWidgets` : QMainWindow, QMenuBar, QFileDialog, QMessageBox

### Compatibilité

- **Qt Version** : Testé avec Qt 6.10.1
- **Compilateur** : MSVC 2022 (v143)
- **Plateforme** : Windows x64

### Performance

- **Chargement** : Chargement paresseux possible (non implémenté)
- **Mémoire** : Chaque document garde son contenu en mémoire
- **UI** : Mise à jour uniquement de l'onglet actif

---

## ✅ Tests Recommandés

1. **Ouverture multiple** : Ouvrir 5-10 fichiers différents
2. **Navigation** : Basculer entre les onglets
3. **Modification** : Modifier plusieurs documents et vérifier les astérisques
4. **Sauvegarde** : Sauvegarder chaque document individuellement
5. **Fermeture** : Fermer des documents modifiés (vérifier les confirmations)
6. **Doublons** : Essayer d'ouvrir le même fichier deux fois
7. **Réorganisation** : Glisser-déposer les onglets

---

**Date de création** : 2 décembre 2025  
**Auteur** : Assistant IA  
**Version** : 1.0

