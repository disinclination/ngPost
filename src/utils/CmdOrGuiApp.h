//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/disinclination/ngPost
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>
//
//========================================================================

#ifndef CMDORGUIAPP_H
#define CMDORGUIAPP_H
#include <QString>

class MainWindow;
class QCoreApplication;

class CmdOrGuiApp
{
protected:
    enum class AppMode : bool {CMD = 0, HMI = 1}; //!< supposed to be CMD but a simple HMI has been added

    QCoreApplication  *_app;  //!< Application instance (either a QCoreApplication or a QApplication in HMI mode)
#ifdef __USE_HMI__
    const AppMode      _mode; //!< CMD or HMI (for Windowser...)
    MainWindow        *_hmi;  //!< potential HMI
#endif

public:
    explicit CmdOrGuiApp(int &argc, char *argv[]);
    virtual ~CmdOrGuiApp();

    virtual const char *appName() = 0;

    virtual bool parseCommandLine(int argc, char *argv[]) = 0;
    virtual void checkForNewVersion();

    int startEventLoop(); //!< to start in CMD

#ifdef __USE_HMI__
    virtual int startHMI();
    void hideOrShowGUI();
#endif
    inline bool useHMI() const;



    inline static QString escapeXML(const char *str);
    inline static QString escapeXML(const QString &str);
    inline static QString xml2txt(const char *str);
    inline static QString xml2txt(const QString &str);
};

bool CmdOrGuiApp::useHMI() const {
#ifdef __USE_HMI__
    return _mode == AppMode::HMI;
#else
    return false;
#endif
}

QString CmdOrGuiApp::escapeXML(const char *str)
{
    QString escaped(str);
    escaped.replace('&',  "&amp;");
    escaped.replace('<',  "&lt;");
    escaped.replace('>',  "&gt;");
    escaped.replace('"',  "&quot;");
    escaped.replace('\'', "&apos;");
    return escaped;
}

QString CmdOrGuiApp::escapeXML(const QString &str)
{
    QString escaped(str);
    escaped.replace('&',  "&amp;");
    escaped.replace('<',  "&lt;");
    escaped.replace('>',  "&gt;");
    escaped.replace('"',  "&quot;");
    escaped.replace('\'', "&apos;");
    return escaped;
}

QString CmdOrGuiApp::xml2txt(const char *str)
{
    QString escaped(str);
    escaped.replace("&amp;",  "&");
    escaped.replace("&lt;",   "<");
    escaped.replace("&gt;",   ">");
    escaped.replace("&quot;", "\"");
    escaped.replace("&apos;", "'");
    return escaped;
}

QString CmdOrGuiApp::xml2txt(const QString &str)
{
    QString escaped(str);
    escaped.replace("&amp;",  "&");
    escaped.replace("&lt;",   "<");
    escaped.replace("&gt;",   ">");
    escaped.replace("&quot;", "\"");
    escaped.replace("&apos;", "'");
    return escaped;
}

#endif // CMDORGUIAPP_H
