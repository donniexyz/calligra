/**
 * $Id$
 *
 * Kalle Dalheimer <kalle@kde.org>
 */

#include "kchart_view.h"
#include "kchart_global.h"
#include "kchart_part.h"
#include "kchartWizard.h"
#include "kchartDataEditor.h"
#include "kchartparams.h"
#include "kchartBarConfigDialog.h"

#include <qpainter.h>
#include <kaction.h>
#include <kglobal.h>

//#include "sheetdlg.h"

KChartView::KChartView( KChartPart* part, QWidget* parent, const char* name )
    : ContainerView( part, parent, name )
{
    m_wizard = new KAction( tr("Customize with &wizard"),
			    KChartBarIcon("wizard"), 0,
			    this, SLOT( wizard() ),
			    actionCollection(), "wizard");
    m_edit = new KAction( tr("&Edit data"), KChartBarIcon("pencil"), 0,
			 this, SLOT( edit() ),
                         actionCollection(), "edit");
    m_config = new KAction( tr( "&Config" ), KChartBarIcon( "config" ), 0,
			    this, SLOT( config() ),
			    actionCollection(), "config" );
    m_loadconfig = new KAction( tr("Load config"), KChartBarIcon("loadconfig"),
				0,
				this, SLOT( loadConfig() ),
				actionCollection(), "loadconfig");
    m_saveconfig = new KAction( tr("Save config"), KChartBarIcon("saveconfig"),
				0,
				this, SLOT( saveConfig() ),
				actionCollection(), "saveconfig");
    // initialize the configuration
    //    loadConfig();

    // make sure there is always some test data
    createTempData();
}

void KChartView::paintEvent( QPaintEvent* /*ev*/ )
{
    QPainter painter;
    painter.begin( this );

    // ### TODO: Scaling

    // Let the document do the drawing
    // PENDING(kalle) Do double-buffering if we are a widget
    //    part()->paintEverything( painter, ev->rect(), FALSE, this );
    // paint everything 
    part()->paintEverything( painter, rect(), FALSE, this );


    painter.end();
}


void KChartView::createTempData()
{
    int row, col;
    KChartData *dat = ((KChartPart*)part())->data();

    // initialize some data, if there is none
    if (dat->rows() == 0) {
	cerr << "Initialize with some data!!!\n";
	dat->expand(4,4);
	for (row = 0;row < 4;row++)
	    for (col = 0;col < 4;col++) {
		//	  _widget->fillCell(row,col,row+col);
		KChartValue t; 
		t.exists= true;
		t.value.setValue((double)row+col);
		cerr << "Set cell for " << row << "," << col << "\n";
		dat->setCell(row,col,t);
	    }
	//      _dlg->exec();
    }
}


void KChartView::edit()
{
  kchartDataEditor ed;
  KChartData *dat = (( (KChartPart*)part())->data());
  ed.setData(dat);
  if (ed.exec() != QDialog::Accepted) {
    return;
  }
  ed.getData(dat);
  repaint();
}

void KChartView::wizard()
{
    qDebug("Wizard called");
    kchartWizard *wiz =
      new kchartWizard((KChartPart*)part(), this, "KChart Wizard", true);
    qDebug("Executed. Now, display it");
    wiz->exec();
    repaint();
    qDebug("Ok, executed...");
}


void KChartView::config()
{
    // open a config dialog depending on the chart type
    KChartParameters* params = ((KChartPart*)part())->params();

    switch( params->type ) {
    case KCHARTTYPE_3DBAR:
    case KCHARTTYPE_BAR: {
	KChartBarConfigDialog* d = new KChartBarConfigDialog( params, this );
	d->exec();
	delete d;
	break;
    }
    case KCHARTTYPE_LINE:
    case KCHARTTYPE_AREA:
    case KCHARTTYPE_HILOCLOSE:
    case KCHARTTYPE_COMBO_LINE_BAR:			/* aka: VOL[ume] */
    case KCHARTTYPE_COMBO_HLC_BAR:
    case KCHARTTYPE_COMBO_LINE_AREA:
    case KCHARTTYPE_COMBO_HLC_AREA:
    case KCHARTTYPE_3DHILOCLOSE:
    case KCHARTTYPE_3DCOMBO_LINE_BAR:
    case KCHARTTYPE_3DCOMBO_LINE_AREA:
    case KCHARTTYPE_3DCOMBO_HLC_BAR:
    case KCHARTTYPE_3DCOMBO_HLC_AREA:
    case KCHARTTYPE_3DAREA:
    case KCHARTTYPE_3DLINE:
    case KCHARTTYPE_3DPIE:
    case KCHARTTYPE_2DPIE:
	qDebug( "Sorry, not implemented: no config dialog for this chart type" );
	break;
    default:
	qDebug( "Unknown chart type" );
    }
}


void KChartView::saveConfig() {
    qDebug("Save config...");
    ((KChartPart*)part())->saveConfig( KGlobal::config() );
}

void KChartView::loadConfig() {
    qDebug("Load config...");
    KGlobal::config()->reparseConfiguration();
    ((KChartPart*)part())->loadConfig( KGlobal::config() );
}


#include "kchart_view.moc"
