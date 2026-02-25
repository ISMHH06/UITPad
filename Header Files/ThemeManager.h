#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include "Settings.h"

/**
 * ThemeManager - Centralized QSS theme provider for the entire application.
 * 
 * All color tokens and style rules live here so that every widget
 * can query a single source of truth instead of embedding raw colors.
 */
class ThemeManager
{
public:
    // ?? Color Tokens ??????????????????????????????????????????????
    struct Colors {
        // Surfaces
        QString base;           // deepest background
        QString surface;      // panels / sidebars
        QString surfaceAlt;  // slightly lighter surface
        QString overlay;        // popups, tooltips
        
        // Borders
        QString border;         // subtle
        QString borderStrong;   // prominent
        
        // Text
        QString textPrimary;
        QString textSecondary;
        QString textMuted;
        
        // Accents
        QString accent;
        QString accentHover;
        QString accentActive;
        
     // Status
        QString success;
    QString warning;
   QString error;
     QString info;
        
        // Editor
        QString editorBg;
    QString editorFg;
        QString editorLineHighlight;
        QString editorSelection;
        QString lineNumberBg;
        QString lineNumberFg;
        QString lineNumberActiveFg;
    
        // Tab
  QString tabBg;
        QString tabActiveBg;
        QString tabHoverBg;
        QString tabFg;
        QString tabActiveFg;
        
        // Scrollbar
     QString scrollbarBg;
 QString scrollbarHandle;
     QString scrollbarHandleHover;

        // Toolbar
        QString toolbarBg;
QString toolbarSeparator;
    
     // StatusBar
        QString statusBarBg;
      QString statusBarFg;
 
        // Font
        QString fontFamily;
QString monoFontFamily;
    };
    
    static Colors getColors(Settings::AppTheme theme);
    
    // ?? Composite Stylesheets ?????????????????????????????????????
    static QString getApplicationStyleSheet(Settings::AppTheme theme);
    static QString getEditorStyleSheet(Settings::AppTheme theme);
    static QString getOutputWindowStyleSheet(Settings::AppTheme theme);
    static QString getOutputTextStyleSheet(Settings::AppTheme theme);
    static QString getChatWidgetStyleSheet(Settings::AppTheme theme);
    static QString getToolBarStyleSheet(Settings::AppTheme theme);
    static QString getProjectExplorerStyleSheet(Settings::AppTheme theme);

private:
    static Colors darkColors();
    static Colors lightColors();
    static Colors hackerColors();
};

#endif // THEMEMANAGER_H
