/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef InsertCitationDialog_H
#define InsertCitationDialog_H

#include "ui_InsertCitationDialog.h"
#include <KoTextEditor.h>
#include <QTextBlock>


class TextTool;
class KoStyleManager;
class KoInlineCite;
class BibliographyDb;

class InsertCitationDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode {
        Default,
        DB
    };

    explicit InsertCitationDialog(KoTextEditor *editor ,QWidget *parent = 0);
    explicit InsertCitationDialog(BibliographyDb *db ,QWidget *parent = 0);
    void init();
    KoInlineCite *toCite();                 //returns cite with values filled in form
    void fillValuesFrom(KoInlineCite *cite);        //fills form with values in cite
    QMap<QString, KoInlineCite*> citations();

public slots:
    void insert();
    void selectionChangedFromExistingCites();
    void selectionChangedFromDatabaseCites();

private:
    Ui::InsertCitationDialog dialog;
    bool m_blockSignals;
    KoTextEditor *m_editor;
    BibliographyDb *m_table;
    BibliographyDb *m_biblio;
    QMap<QString, KoInlineCite*> m_cites;
    QMap<QString, KoInlineCite*> m_records;
    Mode m_mode;
};

#endif // CITATIONBIBLIOGRAPHYDIALOG_H
