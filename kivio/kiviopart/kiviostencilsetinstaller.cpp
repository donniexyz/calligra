/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2004 Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* Based heavily on code from the icon theme installer
 * Copyright (C) 2000 Antonio Larrosa <larrosa@kde.org>
 */
 
#include "kiviostencilsetinstaller.h"

#include <qframe.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qstringlist.h>

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <karchive.h>
#include <kguiitem.h>

namespace Kivio {
  StencilSetInstaller::StencilSetInstaller(QWidget *parent, const char *name)
    : KDialogBase(KDialogBase::Plain, i18n("Install Stencil Set"), Ok|Cancel, Ok, parent, name)
  {
    QFrame* page = plainPage();
    QVBoxLayout* l = new QVBoxLayout(page);
    l->setAutoAdd(true);
    
    m_url = new KURLRequester(page);
    
    setButtonOK(KGuiItem(i18n("&Install"), "button_ok"));
    resize(400, 10);
  }
  
  StencilSetInstaller::~StencilSetInstaller()
  {
  }

  void StencilSetInstaller::install(const QString& urlString)
  {
    KURL url(urlString);
    
    if(url.isEmpty()) return;
    
    QString tmpFile;
    
    if(!KIO::NetAccess::download(url, tmpFile, this)) {
      KMessageBox::error(this, i18n("Could not find the stencil set archive %1!").arg(url.prettyURL()));
      return;
    }
    
    KTar archive(urlString);
    archive.open(IO_ReadOnly);
    const KArchiveDirectory* rootDir = archive.directory();
    
    QStringList dirs = checkDirs(rootDir);
    
    if(dirs.isEmpty()) {
      KMessageBox::error(this, i18n("The file isn't a valid stencil set archive!"));
      archive.close();
      KIO::NetAccess::removeTempFile(tmpFile);
      return;
    }
    
    if(installStencilSets(rootDir, dirs)) {
      KMessageBox::information(this, i18n("The stencil set archive installed without errors!"));
    } else {
      KMessageBox::error(this, i18n("The entire archive could not be installed successfully!"));
    }
  
    archive.close();
    KIO::NetAccess::removeTempFile(tmpFile);
  }

  QStringList StencilSetInstaller::checkDirs(const KArchiveDirectory* rootDir)
  {
    QStringList dirs;
  
    const KArchiveEntry* possibleDir = 0;
    const KArchiveDirectory* subDir = 0;
    
    QStringList entries = rootDir->entries();
    
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
      possibleDir = rootDir->entry(*it);
      
      if (possibleDir->isDirectory()) {
        subDir = dynamic_cast<const KArchiveDirectory*>( possibleDir );
        
        if (subDir && subDir->entry("desc")) {
          dirs.append(subDir->name());
        }
      }
    }
  
    return dirs;
  }

  bool StencilSetInstaller::installStencilSets(const KArchiveDirectory* rootDir, const QStringList& dirs)
  {
    QString installDir = locateLocal("data", "kivio/stencils");
    KStandardDirs::makeDir(installDir);
    const KArchiveDirectory* currentDir = 0;
    bool ok = true;
  
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
      currentDir = dynamic_cast<const KArchiveDirectory*>(rootDir->entry(*it));
      
      if(!currentDir) {
        ok = false;
        continue;
      }
      
      currentDir->copyTo(installDir + "/" + currentDir->name());
    }
    
    return ok;
  }

  void StencilSetInstaller::slotOk()
  {
    install(m_url->url());
    accept();
  }
};

#include "kiviostencilsetinstaller.moc"
