#include "ThemeManager.h"

// ??? Dark Theme Colors ???????????????????????????????????????????
ThemeManager::Colors ThemeManager::darkColors()
{
    Colors c;
    // Surfaces
    c.base     = "#181A1F";
    c.surface     = "#21252B";
    c.surfaceAlt  = "#282C34";
    c.overlay     = "#2C313A";

    // Borders
    c.border       = "#353B45";
    c.borderStrong = "#4B5263";

    // Text
    c.textPrimary   = "#ABB2BF";
 c.textSecondary = "#828997";
    c.textMuted  = "#5C6370";

    // Accents
    c.accent       = "#528BFF";
    c.accentHover  = "#6E9EFF";
    c.accentActive = "#3D7AFF";

    // Status
    c.success = "#98C379";
    c.warning = "#E5C07B";
    c.error   = "#E06C75";
    c.info  = "#61AFEF";

    // Editor
    c.editorBg            = "#282C34";
    c.editorFg            = "#ABB2BF";
    c.editorLineHighlight = "#2C313C";
  c.editorSelection     = "#3E4451";
    c.lineNumberBg        = "#282C34";
    c.lineNumberFg      = "#4B5263";
    c.lineNumberActiveFg  = "#ABB2BF";

    // Tab
    c.tabBg       = "#21252B";
    c.tabActiveBg = "#282C34";
    c.tabHoverBg  = "#2C313A";
    c.tabFg    = "#828997";
    c.tabActiveFg = "#D7DAE0";

  // Scrollbar
    c.scrollbarBg       = "#21252B";
    c.scrollbarHandle    = "#4B5263";
  c.scrollbarHandleHover = "#636D83";

    // Toolbar
    c.toolbarBg        = "#21252B";
  c.toolbarSeparator = "#353B45";

    // StatusBar
    c.statusBarBg = "#21252B";
    c.statusBarFg = "#828997";

    // Fonts
    c.fontFamily     = "'Segoe UI', 'Helvetica Neue', sans-serif";
    c.monoFontFamily = "'Cascadia Code', 'JetBrains Mono', 'Consolas', monospace";

    return c;
}

// ??? Light Theme Colors ??????????????????????????????????????????
ThemeManager::Colors ThemeManager::lightColors()
{
    Colors c;
    c.base        = "#FFFFFF";
    c.surface     = "#F5F5F5";
    c.surfaceAlt  = "#EAEAEB";
    c.overlay     = "#FFFFFF";

    c.border       = "#D4D4D4";
    c.borderStrong = "#BCBCBC";

    c.textPrimary   = "#383A42";
    c.textSecondary = "#696C77";
    c.textMuted     = "#A0A1A7";

    c.accent       = "#4078F2";
    c.accentHover  = "#5A8EFF";
    c.accentActive = "#2E6AE6";

    c.success = "#50A14F";
    c.warning = "#C18401";
    c.error   = "#E45649";
    c.info    = "#4078F2";

    c.editorBg            = "#FAFAFA";
    c.editorFg    = "#383A42";
    c.editorLineHighlight = "#F2F2F2";
    c.editorSelection     = "#B4D5FE";
    c.lineNumberBg      = "#FAFAFA";
    c.lineNumberFg     = "#9D9D9F";
    c.lineNumberActiveFg  = "#383A42";

    c.tabBg       = "#EAEAEB";
  c.tabActiveBg = "#FAFAFA";
    c.tabHoverBg  = "#E0E0E1";
    c.tabFg       = "#696C77";
    c.tabActiveFg = "#383A42";

    c.scrollbarBg    = "#F5F5F5";
    c.scrollbarHandle      = "#C4C4C4";
    c.scrollbarHandleHover = "#A8A8A8";

    c.toolbarBg        = "#F5F5F5";
    c.toolbarSeparator = "#D4D4D4";

    c.statusBarBg = "#F5F5F5";
    c.statusBarFg = "#696C77";

    c.fontFamily     = "'Segoe UI', 'Helvetica Neue', sans-serif";
    c.monoFontFamily = "'Cascadia Code', 'JetBrains Mono', 'Consolas', monospace";

    return c;
}

// ??? Hacker Theme Colors ?????????????????????????????????????????
ThemeManager::Colors ThemeManager::hackerColors()
{
    Colors c;
    c.base        = "#0A0A0A";
    c.surface     = "#0D1117";
    c.surfaceAlt  = "#111921";
    c.overlay = "#161B22";

    c.border       = "#1B3A1B";
 c.borderStrong = "#238636";

    c.textPrimary   = "#33FF33";
    c.textSecondary = "#26CC26";
    c.textMuted     = "#1A8C1A";

    c.accent       = "#39D353";
    c.accentHover  = "#56D364";
    c.accentActive = "#2EA043";

    c.success = "#39D353";
    c.warning = "#D29922";
    c.error   = "#F85149";
    c.info= "#58A6FF";

    c.editorBg        = "#0D1117";
    c.editorFg     = "#33FF33";
    c.editorLineHighlight = "#111921";
    c.editorSelection   = "#1B3A1B";
    c.lineNumberBg        = "#0D1117";
    c.lineNumberFg        = "#1A8C1A";
    c.lineNumberActiveFg  = "#33FF33";

    c.tabBg   = "#0D1117";
    c.tabActiveBg = "#111921";
    c.tabHoverBg  = "#161B22";
    c.tabFg       = "#1A8C1A";
    c.tabActiveFg = "#33FF33";

    c.scrollbarBg          = "#0D1117";
    c.scrollbarHandle      = "#1B3A1B";
    c.scrollbarHandleHover = "#238636";

    c.toolbarBg        = "#0D1117";
    c.toolbarSeparator = "#1B3A1B";

    c.statusBarBg = "#0D1117";
    c.statusBarFg = "#26CC26";

    c.fontFamily     = "'Courier New', 'Consolas', monospace";
    c.monoFontFamily = "'Courier New', 'Consolas', monospace";

    return c;
}

ThemeManager::Colors ThemeManager::getColors(Settings::AppTheme theme)
{
    switch (theme) {
    case Settings::Dark:   return darkColors();
    case Settings::Hacker: return hackerColors();
    default:  return lightColors();
    }
}

// ??? Application-wide stylesheet ????????????????????????????????
QString ThemeManager::getApplicationStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(R"(
      /* ?? Global ??????????????????????????? */
        QMainWindow {
      background-color: %1;
            color: %2;
            font-family: %3;
 font-size: 13px;
        }

        /* ?? Menu Bar ????????????????????????? */
        QMenuBar {
            background-color: %4;
      color: %5;
 border: none;
            border-bottom: 1px solid %6;
            padding: 2px 0px;
        font-size: 13px;
            spacing: 0px;
        }
        QMenuBar::item {
         padding: 6px 12px;
         background: transparent;
   border-radius: 4px;
    margin: 2px 1px;
        }
        QMenuBar::item:selected {
   background-color: %7;
       color: %8;
        }

     /* ?? Menus ???????????????????????????? */
      QMenu {
            background-color: %9;
            color: %5;
   border: 1px solid %6;
            border-radius: 6px;
   padding: 4px 0px;
   }
   QMenu::item {
  padding: 6px 32px 6px 20px;
          border-radius: 4px;
            margin: 1px 4px;
        }
        QMenu::item:selected {
   background-color: %10;
color: #FFFFFF;
        }
      QMenu::separator {
    height: 1px;
          background-color: %6;
margin: 4px 12px;
        }
        QMenu::icon {
  padding-left: 8px;
        }

        /* ?? Tabs ????????????????????????????? */
        QTabWidget::pane {
            border: none;
    border-top: 1px solid %6;
  background-color: %11;
        }
        QTabBar {
   background-color: %12;
        }
     QTabBar::tab {
            background-color: %12;
      color: %13;
            padding: 8px 20px 8px 14px;
 border: none;
     border-right: 1px solid %6;
     min-width: 80px;
 font-size: 12px;
    }
        QTabBar::tab:selected {
    background-color: %14;
            color: %15;
border-top: 2px solid %10;
            border-bottom: none;
 }
   QTabBar::tab:hover:!selected {
   background-color: %16;
        }
        QTabBar::close-button {
     subcontrol-position: right;
         padding: 0px;
  margin: 4px 2px 4px 4px;
         border-radius: 3px;
         width: 16px;
 height: 16px;
        }
  QTabBar::close-button:hover {
    background-color: %7;
        }

        /* ?? Status Bar ??????????????????????? */
        QStatusBar {
background-color: %17;
     color: %18;
 border-top: 1px solid %6;
            font-size: 12px;
  padding: 0px;
   min-height: 24px;
        }
        QStatusBar::item {
            border: none;
        }
        QStatusBar QLabel {
        padding: 2px 8px;
            font-size: 12px;
         color: %18;
        }

  /* ?? Scrollbars ??????????????????????? */
   QScrollBar:vertical {
            background-color: %19;
          width: 10px;
 margin: 0;
   border: none;
        }
        QScrollBar::handle:vertical {
        background-color: %20;
min-height: 30px;
   border-radius: 5px;
        margin: 2px;
   }
QScrollBar::handle:vertical:hover {
       background-color: %21;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
        }
        QScrollBar:horizontal {
            background-color: %19;
  height: 10px;
       margin: 0;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: %20;
         min-width: 30px;
    border-radius: 5px;
   margin: 2px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: %21;
    }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
      }

        /* ?? Tooltips ????????????????????????? */
        QToolTip {
  background-color: %9;
color: %5;
      border: 1px solid %6;
            border-radius: 4px;
    padding: 4px 8px;
      font-size: 12px;
    }

        /* ?? Dock Widgets ????????????????????? */
  QDockWidget {
            titlebar-close-icon: none;
          titlebar-normal-icon: none;
       font-size: 12px;
            color: %5;
        }
   QDockWidget::title {
background-color: %4;
      padding: 6px 12px;
            border-bottom: 1px solid %6;
            font-weight: 600;
            text-align: left;
        }

        /* ?? Splitter ????????????????????????? */
        QSplitter::handle {
        background-color: %6;
        }
        QSplitter::handle:horizontal { width: 1px; }
        QSplitter::handle:vertical   { height: 1px; }

  )").arg(
        c.base,     // %1
 c.textPrimary,   // %2
        c.fontFamily,    // %3
        c.surface,       // %4
      c.textPrimary,   // %5
        c.border,        // %6
        c.surfaceAlt,    // %7
   c.textPrimary,   // %8
        c.overlay        // %9
    ).arg(
     c.accent,        // %10
        c.editorBg,      // %11
        c.tabBg,         // %12
        c.tabFg,// %13
        c.tabActiveBg,   // %14
        c.tabActiveFg,   // %15
    c.tabHoverBg,    // %16
        c.statusBarBg, // %17
     c.statusBarFg    // %18
    ).arg(
        c.scrollbarBg,           // %19
        c.scrollbarHandle,       // %20
        c.scrollbarHandleHover   // %21
    );
}

// ??? Editor text-edit stylesheet ?????????????????????????????????
QString ThemeManager::getEditorStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(
        "QPlainTextEdit {"
   "  color: %1;"
        "  background-color: %2;"
    "  selection-background-color: %3;"
        "  selection-color: %4;"
        "  border: none;"
        "  font-family: %5;"
        "  font-size: 14px;"
        "}"
  ).arg(c.editorFg, c.editorBg, c.editorSelection,
          theme == Settings::Light ? "#FFFFFF" : c.editorFg,
   c.monoFontFamily);
}

// ??? Toolbar stylesheet ??????????????????????????????????????????
QString ThemeManager::getToolBarStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(R"(
QToolBar {
background-color: %1;
   border: none;
            border-bottom: 1px solid %2;
            padding: 2px 4px;
            spacing: 2px;
        }
        QToolBar::separator {
   width: 1px;
    background-color: %2;
 margin: 4px 6px;
        }
        QToolButton {
        background: transparent;
   color: %3;
            border: none;
         border-radius: 4px;
            padding: 5px 8px;
            font-size: 12px;
         font-weight: 500;
        }
        QToolButton:hover {
    background-color: %4;
      }
        QToolButton:pressed {
         background-color: %5;
      }
    )").arg(c.toolbarBg, c.toolbarSeparator, c.textSecondary,
      c.surfaceAlt, c.border);
}

// ??? Output window stylesheet ????????????????????????????????????
QString ThemeManager::getOutputWindowStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(R"(
        QDockWidget {
     background-color: %1;
      }
        QDockWidget::title {
            background-color: %2;
  color: %3;
            padding: 5px 10px;
            border-bottom: 1px solid %4;
     font-weight: 600;
            font-size: 11px;
        }
        QPushButton {
       background-color: %2;
    color: %5;
    border: 1px solid %4;
       padding: 3px 12px;
            border-radius: 3px;
       font-size: 11px;
       font-weight: 500;
  }
        QPushButton:hover {
            background-color: %6;
            border-color: %7;
        }
        QTabWidget::pane {
            border: none;
          border-top: 1px solid %4;
            background-color: %1;
    }
     QTabBar::tab {
      background-color: %2;
       color: %5;
       padding: 5px 16px;
  border: none;
         border-right: 1px solid %4;
      font-size: 11px;
        }
        QTabBar::tab:selected {
            background-color: %1;
     color: %3;
    border-top: 2px solid %8;
      }
        QTabBar::tab:hover:!selected {
            background-color: %6;
        }
    )").arg(c.base, c.surface, c.textPrimary, c.border,
            c.textSecondary, c.surfaceAlt, c.borderStrong, c.accent);
}

QString ThemeManager::getOutputTextStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(
   "QPlainTextEdit {"
        "  background-color: %1;"
      "  color: %2;"
        "  font-family: %3;"
        "  font-size: 11px;"
        "  border: none;"
        "  padding: 4px;"
        "}"
    ).arg(c.base, c.textPrimary, c.monoFontFamily);
}

// ??? Chat widget stylesheet ?????????????????????????????????????
QString ThemeManager::getChatWidgetStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(R"(
        QDockWidget {
       background: %1;
      }
   QDockWidget::title {
       background-color: %2;
        color: %3;
        padding: 6px 12px;
border-bottom: 1px solid %4;
 font-weight: 600;
   font-size: 12px;
        }
        QWidget#chatMainWidget {
            background: %1;
   }
     QScrollArea {
            background: %1;
            border: none;
        }
      QScrollArea > QWidget > QWidget {
            background: %1;
        }
        QWidget#chatContainer {
            background: %1;
   }
        QLineEdit {
     background: %5;
    color: %3;
            border: 1px solid %4;
 border-radius: 6px;
      padding: 8px 12px;
            font-size: 13px;
   }
        QLineEdit:focus {
            border-color: %6;
   }
        QPushButton {
       background: %6;
            color: #FFFFFF;
            border: none;
border-radius: 6px;
 padding: 8px 16px;
            font-weight: 600;
            font-size: 12px;
        }
        QPushButton:hover {
            background: %7;
        }
        QPushButton#clearBtn {
            background: %2;
   color: %8;
        border: 1px solid %4;
   }
        QPushButton#clearBtn:hover {
            background: %5;
        }
    )").arg(c.base, c.surface, c.textPrimary, c.border,
   c.surfaceAlt, c.accent, c.accentHover, c.textSecondary);
}

// ??? Project explorer stylesheet ?????????????????????????????????
QString ThemeManager::getProjectExplorerStyleSheet(Settings::AppTheme theme)
{
    Colors c = getColors(theme);
    return QString(R"(
     QDockWidget {
      background-color: %1;
        }
        QDockWidget::title {
   background-color: %2;
 color: %3;
     padding: 6px 12px;
         border-bottom: 1px solid %4;
  font-weight: 600;
      font-size: 11px;
     text-transform: uppercase;
   letter-spacing: 1px;
        }
        QTreeView {
background-color: %1;
   color: %5;
 border: none;
  font-size: 12px;
   outline: none;
        }
    QTreeView::item {
       padding: 3px 4px;
       border-radius: 3px;
  margin: 0px 4px;
     }
      QTreeView::item:hover {
 background-color: %6;
  }
        QTreeView::item:selected {
background-color: %7;
       color: %3;
   }
     QTreeView::branch {
  background-color: %1;
        }
        QHeaderView::section {
   background-color: %2;
       color: %8;
     border: none;
    padding: 4px 8px;
 font-size: 11px;
   font-weight: 600;
  }
        QWidget#explorerPlaceholder {
   background-color: %1;
        }
        QPushButton#openFolderBtn {
            background-color: %9;
       color: #FFFFFF;
       border: none;
    border-radius: 4px;
      padding: 8px 16px;
 font-size: 12px;
    font-weight: 600;
      }
        QPushButton#openFolderBtn:hover {
      background-color: %10;
}
        QWidget#sectionHeader {
       background-color: %2;
     border-bottom: 1px solid %4;
        }
        QWidget#sectionHeader:hover {
   background-color: %6;
        }
      QWidget#sectionTitle {
            color: %3;
        }
        QListWidget#openEditorsList {
     background-color: %1;
  border: none;
    outline: none;
        }
        QListWidget#openEditorsList::item {
   padding: 1px 4px;
  border-radius: 3px;
 margin: 0px 4px;
   }
     QListWidget#openEditorsList::item:hover {
  background-color: %6;
        }
        QListWidget#openEditorsList::item:selected {
            background-color: %7;
        }
        QWidget#openEditorItem {
            background: transparent;
        }
        QWidget#openEditorItem QLabel {
 color: %5;
          background: transparent;
        border: none;
        }
      QWidget#openEditorItem QPushButton#editorCloseBtn {
            color: %8;
            background: transparent;
         border: none;
     border-radius: 3px;
       font-size: 14px;
            font-weight: bold;
            padding: 0px;
        }
QWidget#openEditorItem QPushButton#editorCloseBtn:hover {
   color: %3;
            background-color: %6;
    }
      QWidget#openEditorsContainer {
  background-color: %1;
        }
 QWidget#folderContainer {
            background-color: %1;
        }
    )").arg(c.base, c.surface, c.textPrimary, c.border,
    c.textSecondary, c.surfaceAlt, c.accent + "33", c.textMuted,
  c.accent, c.accentHover);
}
