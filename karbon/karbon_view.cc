/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "karbon_view.h"

#include <qdragobject.h>
#include <qiconset.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <kaction.h>
#include <kcolordrag.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <koMainWindow.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kocontexthelp.h>
#include <koUnitWidgets.h>
#include <koPageLayoutDia.h>
#include <koRuler.h>

// Commands.
#include "valigncmd.h"
#include "vcleanupcmd.h"
#include "vdeletecmd.h"
#include "vfillcmd.h"
#include "vgroupcmd.h"
#include "vstrokecmd.h"
#include "vungroupcmd.h"
#include "vzordercmd.h"

// Dialogs.
#include "vconfiguredlg.h"

// Dockers.
#include "vcolordocker.h"
#include "vdocumentdocker.h"
#include "vstrokedocker.h"
#include "vtooloptionsdocker.h"
#include "vtransformdocker.h"

// ToolBars
#include "vselecttoolbar.h"

// The rest.
#include "karbon_factory.h"
#include "karbon_part.h"
#include "karbon_view_iface.h"
#include "vselection.h"
#include "vtool.h"
#include "vtoolfactory.h"
#include "vgroup.h"
#include "vpainterfactory.h"
#include "vqpainter.h"
#include "vstrokefillpreview.h"
#include "vstatebutton.h"
#include "vcanvas.h"
#include "vtoolbox.h"

// Only for debugging.
#include <kdebug.h>


KarbonView::KarbonView( KarbonPart* p, QWidget* parent, const char* name )
		: KarbonViewBase( p, parent, name ), KXMLGUIBuilder( shell() )
{
	m_toolbox = 0L;
	m_currentTool = 0L;
	m_toolOptionsDocker = 0L;
	m_documentDocker = 0L;
	if( p->isReadWrite() )
		m_toolFactory = new VToolFactory( this );
	else
		m_toolFactory = 0L;

	setInstance( KarbonFactory::instance() );

	if( p->isReadWrite() )
		m_toolbox->setupTools();

	setAcceptDrops( true );

	setClientBuilder( this );

	if( !p->isReadWrite() )
		setXMLFile( QString::fromLatin1( "karbon_readonly.rc" ) );
	else
		setXMLFile( QString::fromLatin1( "karbon.rc" ) );

	m_dcop = 0L;
	dcopObject(); // build it

	// set up status bar message
	m_status = new KStatusBarLabel( QString::null, 0, statusBar() );
	m_status->setAlignment( AlignLeft | AlignVCenter );
	m_status->setMinimumWidth( 300 );
	addStatusBarItem( m_status, 0 );

	initActions();

	m_strokeFillPreview = 0L;
	m_ColorManager = 0L;
	m_strokeDocker = 0L;

	if( shell() )
	{
		//Create Dockers
		m_ColorManager = new VColorDocker( part(), this );
		m_strokeDocker = new VStrokeDocker( part(), this );

		m_TransformDocker = new VTransformDocker( part(), this );

		//create toolbars
		m_selectToolBar = new VSelectToolBar( this, "selecttoolbar" );
		mainWindow()->addToolBar( m_selectToolBar );
	}

	setNumberOfRecentFiles( part()->maxRecentFiles() );

	reorganizeGUI();

	// widgets:
	m_canvas = new VCanvas( this, this, p );
	connect( m_canvas, SIGNAL( contentsMoving( int, int ) ), this, SLOT( canvasContentsMoving( int, int ) ) );

	m_horizRuler = new KoRuler( this, m_canvas->viewport(), Qt::Horizontal, part()->pageLayout(), 0, part()->unit() );
	connect( m_horizRuler, SIGNAL( doubleClicked() ), this, SLOT( pageLayout() ) );
	m_horizRuler->showMousePos( true );
	m_vertRuler = new KoRuler( this, m_canvas->viewport(), Qt::Vertical, part()->pageLayout(), 0, part()->unit() );
	connect( m_vertRuler, SIGNAL( doubleClicked() ), this, SLOT( pageLayout() ) );
	m_vertRuler->showMousePos( true );
	m_horizRuler->setReadWrite( shell() );
	m_vertRuler->setReadWrite( shell() );

	m_canvas->show();
	m_horizRuler->show();
	m_vertRuler->show();

	// set up factory
	m_painterFactory = new VPainterFactory;
	m_painterFactory->setPainter( canvasWidget()->pixmap(), width(), height() );
	m_painterFactory->setEditPainter( canvasWidget()->viewport(), width(), height() );

	zoomChanged();
}

KarbonView::~KarbonView()
{
	if( shell() )
	{
		delete m_toolFactory;
		m_toolFactory = 0L;
		delete( m_ColorManager );
		delete( m_strokeDocker );
		delete( m_TransformDocker );
	}

	// widgets:
	delete( m_status );

	delete( m_painterFactory );

	delete( m_canvas );

	delete( m_dcop );
}

void
KarbonView::registerTool( VTool *tool )
{
	if( !shell() ) return;
	if( !m_toolbox )
	{
		m_toolbox = new VToolBox( (KarbonPart *)m_part, mainWindow(), "Tools" );
		connect( m_toolbox, SIGNAL( activeToolChanged( VTool * ) ), this, SLOT( slotActiveToolChanged( VTool * ) ) );
	}
	m_toolbox->registerTool( tool );
	m_currentTool = tool;
}

QWidget *
KarbonView::createContainer( QWidget *parent, int index, const QDomElement &element, int &id )
{
	if( element.attribute( "name" ) == "Tools" )
	{
		if( !m_toolbox )
			m_toolbox = new VToolBox( (KarbonPart *)m_part, mainWindow(), "Tools" );
		connect( m_toolbox, SIGNAL( activeToolChanged( VTool * ) ), this, SLOT( slotActiveToolChanged( VTool * ) ) );

		if( shell() )
		{
			m_strokeFillPreview = m_toolbox->strokeFillPreview();
			connect( m_strokeFillPreview, SIGNAL( strokeChanged( const VStroke & ) ), this, SLOT( slotStrokeChanged( const VStroke & ) ) );
			connect( m_strokeFillPreview, SIGNAL( fillChanged( const VFill & ) ), this, SLOT( slotFillChanged( const VFill & ) ) );

			connect( m_strokeFillPreview, SIGNAL( strokeSelected() ), m_ColorManager, SLOT( setStrokeDocker() ) );
			connect( m_strokeFillPreview, SIGNAL( fillSelected( ) ), m_ColorManager, SLOT( setFillDocker() ) );
			selectionChanged();

			m_documentDocker = new VDocumentDocker( this );
			mainWindow()->addDockWindow( m_documentDocker, DockRight );
			m_toolOptionsDocker = new VToolOptionsDocker( this );
			m_toolOptionsDocker->show();
			// set selectTool by default
			m_toolbox->slotPressButton( 0 );
		}

		mainWindow()->moveDockWindow( m_toolbox, Qt::DockLeft, false, 0 );
		return m_toolbox;
	}

	return KXMLGUIBuilder::createContainer( parent, index, element, id );
}

void
KarbonView::removeContainer( QWidget *container, QWidget *parent,
							 QDomElement &element, int id )
{
	if( shell() && m_toolbox )
	{
		delete m_toolbox;
		delete m_toolOptionsDocker;
		delete m_documentDocker;
		m_toolbox = 0L;
		delete m_toolFactory;
		m_toolFactory = 0L;
		m_currentTool = 0L;
		return ;
	}

	KXMLGUIBuilder::removeContainer( container, parent, element, id );
}


DCOPObject *
KarbonView::dcopObject()
{
	if( !m_dcop )
		m_dcop = new KarbonViewIface( this );

	return m_dcop;
}

void
KarbonView::updateReadWrite( bool /*rw*/ )
{}

QWidget*
KarbonView::canvas()
{
	return m_canvas;
}

void
KarbonView::resizeEvent( QResizeEvent* /*event*/ )
{
	int space = 20;
	m_horizRuler->setGeometry( space, 0, width() - space, space );
	m_vertRuler->setGeometry( 0, space, space, height() - space );
	m_canvas->setGeometry( space, space, width() - space, height() - space );
	reorganizeGUI();
}

void
KarbonView::dragEnterEvent( QDragEnterEvent *event )
{
	event->accept( KColorDrag::canDecode( event ) );
}

void
KarbonView::dropEvent ( QDropEvent *e )
{
	//Accepts QColor - from Color Manager's KColorPatch
	QColor color;
	VColor realcolor;

	if( KColorDrag::decode( e, color ) )
	{
		float r = color.red() / 255.0;
		float g = color.green() / 255.0;
		float b = color.blue() / 255.0;

		realcolor.set( r, g, b );

		if( part() )
			part()->addCommand( new VFillCmd( &part()->document(), realcolor ), true );

		selectionChanged();
	}
}

void
KarbonView::setupPrinter( KPrinter& /*printer*/ )
{}

void
KarbonView::print( KPrinter &printer )
{
	kdDebug() << "KarbonView::print" << endl;
	VQPainter p( ( QPaintDevice * ) & printer, width(), height() );
	p.begin();
	p.setZoomFactor( 1.0 );
	QWMatrix mat;
    mat.scale( 1, -1 );
	mat.translate( 0, -part()->document().height() );
    p.setWorldMatrix( mat );

	// print the doc using QPainter at zoom level 1
	// TODO : better use eps export?
	// TODO : use real page layout stuff
	KoRect rect( 0, 0, width(), height() );

	part()->document().draw( &p, &rect );

	p.end();
}

void
KarbonView::editCut()
{
	addSelectionToClipboard();
	// remove selection
	editDeleteSelection();
}

void
KarbonView::editCopy()
{
	addSelectionToClipboard();
}

void
KarbonView::addSelectionToClipboard() const
{
	QClipboard *clipboard = QApplication::clipboard();
	VObjectListIterator itr( part()->document().selection()->objects() );
	// build a xml fragment containing the selection as karbon xml
	QDomDocument doc( "clip" );
	QDomElement elem = doc.createElement( "clip" );
	QString result;
	QTextStream ts( &result, IO_WriteOnly );
	for( ; itr.current() ; ++itr )
		itr.current()->save( elem );
	ts << elem;
	// push to clipbaord
	clipboard->setText( result.latin1() );

}

void
KarbonView::editPaste()
{
	QClipboard *clipboard = QApplication::clipboard();
	QDomDocument doc( "clip" );
	doc.setContent( clipboard->text() );
	QDomElement clip = doc.documentElement();
	// Try to parse the clipboard data
	if( clip.tagName() == "clip" )
	{
		VObjectList selection;
		// Use group to assemble the xml contents
		// TODO : maybe not clone() so much
		VGroup grp( 0L );
		grp.load( clip );
		VObjectListIterator itr( grp.objects() );
		for( ; itr.current() ; ++itr )
			selection.append( itr.current()->clone() );

		part()->document().selection()->clear();

		// Calc new selection
		VObjectListIterator itr2( selection );

		for( ; itr2.current() ; ++itr2 )
		{
			part()->insertObject( itr2.current() );
			part()->document().selection()->append( itr2.current() );
		}

		part()->repaintAllViews();
	}
}

void
KarbonView::editSelectAll()
{
	part()->document().selection()->append();

	if( part()->document().selection()->objects().count() > 0 )
		part()->repaintAllViews();

	selectionChanged();
}

void
KarbonView::editDeselectAll()
{
	if( part()->document().selection()->objects().count() > 0 )
	{
		part()->document().selection()->clear();
		part()->repaintAllViews();
	}

	selectionChanged();
}

void
KarbonView::editDeleteSelection()
{
	kdDebug() << "*********" << endl;

	if( part()->document().selection()->objects().count() > 0 )
	{
		part()->addCommand(
			new VDeleteCmd( &part()->document() ),
			true );
	}
}

void
KarbonView::editPurgeHistory()
{
	// TODO: check for history size != 0

	if( KMessageBox::warningContinueCancel( this,
			i18n( "This action cannot be undone later. Do you really want to continue?" ),
			i18n( "Purge History" ),
			i18n( "C&ontinue" ),		// TODO: is there a constant for this?
			"edit_purge_history" ) )
	{
		// Use the VCleanUp command to remove "deleted"
		// objects from all layers.
		VCleanUpCmd cmd( &part()->document() );
		cmd.execute();

		part()->clearHistory();
	}
}

void
KarbonView::selectionAlignHorizontalLeft()
{
	part()->addCommand(
		new VAlignCmd( &part()->document(), VAlignCmd::ALIGN_HORIZONTAL_LEFT ), true );
}
void
KarbonView::selectionAlignHorizontalCenter()
{
	part()->addCommand(
		new VAlignCmd( &part()->document(), VAlignCmd::ALIGN_HORIZONTAL_CENTER ), true );
}

void
KarbonView::selectionAlignHorizontalRight()
{
	part()->addCommand(
		new VAlignCmd( &part()->document(), VAlignCmd::ALIGN_HORIZONTAL_RIGHT ), true );
}

void
KarbonView::selectionAlignVerticalTop()
{
	part()->addCommand(
		new VAlignCmd( &part()->document(), VAlignCmd::ALIGN_VERTICAL_TOP ), true );
}

void
KarbonView::selectionAlignVerticalCenter()
{
	part()->addCommand(
		new VAlignCmd( &part()->document(), VAlignCmd::ALIGN_VERTICAL_CENTER ), true );
}

void
KarbonView::selectionAlignVerticalBottom()
{
	part()->addCommand(
		new VAlignCmd( &part()->document(), VAlignCmd::ALIGN_VERTICAL_BOTTOM ), true );
}

void
KarbonView::selectionBringToFront()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::bringToFront ), true );
}

void
KarbonView::selectionMoveUp()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::up ), true );
}

void
KarbonView::selectionMoveDown()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::down ), true );
}

void
KarbonView::selectionSendToBack()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::sendToBack ), true );
}

void
KarbonView::groupSelection()
{
	part()->addCommand( new VGroupCmd( &part()->document() ), true );
	selectionChanged();
}

void
KarbonView::ungroupSelection()
{
	part()->addCommand( new VUnGroupCmd( &part()->document() ), true );
	selectionChanged();
}

// TODO: remove this one someday:
void
KarbonView::dummyForTesting()
{
	kdDebug() << "KarbonView::dummyForTesting()" << endl;
}

void
KarbonView::objectTrafoTranslate()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Translate );
		m_TransformDocker->show();
	}
}

void
KarbonView::objectTrafoScale()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Scale );
		m_TransformDocker->show();
	}
}

void
KarbonView::objectTrafoRotate()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Rotate );
		m_TransformDocker->show();
	}
}

void
KarbonView::objectTrafoShear()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Shear );
		m_TransformDocker->show();
	}
}

void
KarbonView::slotActiveToolChanged( VTool *tool )
{
	if( m_currentTool )
		m_currentTool->deactivate();

	m_currentTool = tool;

	m_currentTool->activateAll();

	m_canvas->repaintAll();
}

void
KarbonView::viewModeChanged()
{
	canvasWidget()->pixmap()->fill();

	if( m_viewAction->currentItem() == 1 )
		m_painterFactory->setWireframePainter( canvasWidget()->pixmap(), width(), height() );
	else
		m_painterFactory->setPainter( canvasWidget()->pixmap(), width(), height() );

	m_canvas->repaintAll();
}

void
KarbonView::setZoomAt( double zoom, const KoPoint &p )
{
	QString zoomText = QString( "%1%" ).arg( zoom * 100.0, 0, 'f', 2 );
	QStringList stl = m_zoomAction->items();
	if( stl.first() == "    25%" )
	{
		stl.prepend( zoomText.latin1() );
		m_zoomAction->setItems( stl );
		m_zoomAction->setCurrentItem( 0 );
	}
	else
	{
		m_zoomAction->setCurrentItem( 0 );
		m_zoomAction->changeItem( m_zoomAction->currentItem(), zoomText.latin1() );
	}
	zoomChanged( p );
}

void
KarbonView::zoomChanged( const KoPoint &p )
{
	double centerX;
	double centerY;
	double zoomFactor;
	bool bOK;
	if( !p.isNull() )
	{
		kdDebug() << "p.x(): " << p.x() << endl;
		kdDebug() << "p.y(): " << p.y() << endl;
		centerX = ( p.x() * zoom() ) / double( m_canvas->contentsWidth() );
		centerY = 1 - ( p.y() * zoom() ) / double( m_canvas->contentsHeight() );
		zoomFactor = m_zoomAction->currentText().toDouble( &bOK ) / 100.0;
	}
	else if( m_zoomAction->currentText() == "  Width" )
	{
		centerX = 0.5;
		centerY = double( m_canvas->contentsY() + m_canvas->visibleHeight() / 2 ) / double( m_canvas->contentsHeight() );
		zoomFactor = zoom() * double( m_canvas->visibleWidth() ) / double( m_canvas->contentsWidth() );
	}
	else if( m_zoomAction->currentText() == "Whole page" )
	{
		centerX = 0.5;
		centerY = 0.5;
		double zoomFactorX = zoom() * double( m_canvas->visibleWidth() ) / double( m_canvas->contentsWidth() );
		double zoomFactorY = zoom() * double( m_canvas->visibleHeight() ) / double( m_canvas->contentsHeight() );
		zoomFactor = kMin( zoomFactorX, zoomFactorY );
	}
	else
	{
		centerX = double( m_canvas->contentsX() + m_canvas->visibleWidth() / 2 ) / double( m_canvas->contentsWidth() );
		centerY = double( m_canvas->contentsY() + m_canvas->visibleHeight() / 2 ) / double( m_canvas->contentsHeight() );
		zoomFactor = m_zoomAction->currentText().toDouble( &bOK ) / 100.0;
	}
	kdDebug() << "centerX : " << centerX << endl;
	kdDebug() << "centerY : " << centerY << endl;
	//kdDebug() << "zoomFactor : " << zoomFactor << endl;


	// above 2000% probably doesnt make sense... (Rob)
	if( zoomFactor > 20 )
	{
		zoomFactor = 20;
		m_zoomAction->changeItem( m_zoomAction->currentItem(), " 2000%" );
	}

	KoView::setZoom( zoomFactor );
	m_horizRuler->setZoom( zoomFactor );
	m_vertRuler->setZoom( zoomFactor );

	m_canvas->viewport()->setUpdatesEnabled( false );

	m_canvas->resizeContents( int( ( part()->pageLayout().ptWidth + 40 ) * zoomFactor ),
							  int( ( part()->pageLayout().ptHeight + 80 ) * zoomFactor ) );

	VPainter *painter = painterFactory()->editpainter();
	painter->setZoomFactor( zoomFactor );

	m_canvas->setViewport( centerX, centerY );
	m_canvas->repaintAll();
	m_canvas->viewport()->setUpdatesEnabled( true );

	m_canvas->viewport()->setFocus();

	emit zoomChanged( zoomFactor );
}

void
KarbonView::slotStrokeChanged( const VStroke &c )
{
	part()->document().selection()->setStroke( c );
	selectionChanged();
}

void
KarbonView::slotFillChanged( const VFill &f )
{
	part()->document().selection()->setFill( f );
	selectionChanged();
}

void
KarbonView::setLineWidth()
{
	setLineWidth( m_setLineWidth->value() );
	selectionChanged();
}

//necessary for dcop call !
void
KarbonView::setLineWidth( double val )
{
	part()->addCommand( new VStrokeCmd( &part()->document(), val ), true );
}

void
KarbonView::viewColorManager()
{
	if( m_ColorManager->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_ColorManager, DockRight );
		m_ColorManager->show();
	}
}

void
KarbonView::viewToolOptions()
{
	if( m_toolOptionsDocker->isVisible() == false )
	{
		m_toolOptionsDocker->show();
	}
}

void
KarbonView::viewStrokeDocker()
{
	if( m_strokeDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_strokeDocker, DockRight );
		m_strokeDocker->show();
	}
}

void
KarbonView::initActions()
{
	// edit ----->
	KStdAction::cut( this,
					 SLOT( editCut() ), actionCollection(), "edit_cut" );
	KStdAction::copy( this,
					  SLOT( editCopy() ), actionCollection(), "edit_copy" );
	KStdAction::paste( this,
					   SLOT( editPaste() ), actionCollection(), "edit_paste" );
	KStdAction::selectAll( this,
						   SLOT( editSelectAll() ), actionCollection(), "edit_select_all" );
	new KAction(
		i18n( "&Deselect All" ), QKeySequence( "Ctrl+D" ), this,
		SLOT( editDeselectAll() ), actionCollection(), "edit_deselect_all" );
	new KAction(
		i18n( "D&elete" ), "editdelete", QKeySequence( "Shift+Del" ), this,
		SLOT( editDeleteSelection() ), actionCollection(), "edit_delete" );
	new KAction(
		i18n( "&History" ), 0, 0, this,
		SLOT( editPurgeHistory() ), actionCollection(), "edit_purge_history" );
	// edit <-----

	// object ----->
	new KAction(
		i18n( "Bring to &Front" ), 0, QKeySequence( "Shift+PgUp" ), this,
		SLOT( selectionBringToFront() ), actionCollection(), "object_move_totop" );
	new KAction(
		i18n( "&Raise" ), 0, QKeySequence( "Ctrl+PgUp" ), this,
		SLOT( selectionMoveUp() ), actionCollection(), "object_move_up" );
	new KAction(
		i18n( "&Lower" ), 0, QKeySequence( "Ctrl+PgDown" ), this,
		SLOT( selectionMoveDown() ), actionCollection(), "object_move_down" );
	new KAction(
		i18n( "Send to &Back" ), 0, QKeySequence( "Shift+PgDown" ), this,
		SLOT( selectionSendToBack() ), actionCollection(), "object_move_tobottom" );

	new KAction(
		i18n( "Align left" ), "aoleft", 0, this,
		SLOT( selectionAlignHorizontalLeft() ), actionCollection(), "object_align_horizontal_left" );
	new KAction(
		i18n( "Align center (Horizontal)" ), "aocenterh", 0, this,
		SLOT( selectionAlignHorizontalCenter() ), actionCollection(), "object_align_horizontal_center" );
	new KAction(
		i18n( "Align right" ), "aoright", 0, this,
		SLOT( selectionAlignHorizontalRight() ), actionCollection(), "object_align_horizontal_right" );
	new KAction(
		i18n( "Align top" ), "aotop", 0, this,
		SLOT( selectionAlignVerticalTop() ), actionCollection(), "object_align_vertical_top" );
	new KAction(
		i18n( "Align center (Vertical)" ), "aocenterv", 0, this,
		SLOT( selectionAlignVerticalCenter() ), actionCollection(), "object_align_vertical_center" );
	new KAction(
		i18n( "Align bottom" ), "aobottom", 0, this,
		SLOT( selectionAlignVerticalBottom() ), actionCollection(), "object_align_vertical_bottom" );

	m_showRulerAction = new KToggleAction( i18n( "Show Rulers" ), 0, this, SLOT( showRuler() ), actionCollection(), "view_show_ruler" );
	m_showRulerAction->setToolTip( i18n( "Shows or hides rulers." ) );
	m_showRulerAction->setChecked( true );
	m_groupObjects = new KAction(
						 i18n( "&Group Objects" ), "14_group", QKeySequence( "Ctrl+G" ), this,
						 SLOT( groupSelection() ), actionCollection(), "selection_group" );
	m_ungroupObjects = new KAction(
						   i18n( "&Ungroup Objects" ), "14_ungroup", QKeySequence( "Ctrl+U" ), this,
						   SLOT( ungroupSelection() ), actionCollection(), "selection_ungroup" );
	new KAction(
		i18n( "&Translate" ), "14_translate", 0, this,
		SLOT( objectTrafoTranslate() ), actionCollection(), "object_trafo_translate" );
	new KAction(
		i18n( "&Rotate" ), "14_rotate", 0, this,
		SLOT( objectTrafoRotate() ), actionCollection(), "object_trafo_rotate" );
	new KAction(
		i18n( "S&hear" ), "14_shear", 0, this,
		SLOT( objectTrafoShear() ), actionCollection(), "object_trafo_shear" );
	// object <-----

	// view ----->
	m_viewAction = new KSelectAction(
					   i18n( "View &Mode" ), 0, this,
					   SLOT( viewModeChanged() ), actionCollection(), "view_mode" );

	m_zoomAction = new KSelectAction(
					   i18n( "&Zoom" ), 0, this,
					   SLOT( zoomChanged() ), actionCollection(), "view_zoom" );

	QStringList mstl;
	mstl << i18n( "Normal" ) << i18n( "Wireframe" );
	m_viewAction->setItems( mstl );
	m_viewAction->setCurrentItem( 0 );
	m_viewAction->setEditable( false );

	QStringList stl;
	stl
	<< i18n( "    25%" )
	<< i18n( "    50%" )
	<< i18n( "   100%" )
	<< i18n( "   200%" )
	<< i18n( "   300%" )
	<< i18n( "   400%" )
	<< i18n( "   800%" )
	<< i18n( "Whole page" )
	<< i18n( "  Width" );

	m_zoomAction->setItems( stl );
	m_zoomAction->setEditable( true );
	m_zoomAction->setCurrentItem( 2 );

	new KAction(
		i18n( "&Color Manager" ), "colorman", 0, this,
		SLOT( viewColorManager() ), actionCollection(), "view_color_manager" );
	new KAction(
		i18n( "&Tool Options" ), "tooloptions", 0, this,
		SLOT( viewToolOptions() ), actionCollection(), "view_tool_options" );
	new KAction(
		i18n( "&Stroke" ), "strokedocker", 0, this,
		SLOT( viewStrokeDocker() ), actionCollection(), "view_stroke_docker" );
	// view <-----

	// line width
	m_setLineWidth = new KoUnitDoubleSpinComboBox( this, 0.0, 1000.0, 0.5, 1.0, KoUnit::U_PT, 1 );
	new KWidgetAction( m_setLineWidth, i18n( "Set Line Width" ), 0, this, SLOT( setLineWidth() ), actionCollection(), "setLineWidth" );
	m_setLineWidth->insertItem( 0.5 );
	m_setLineWidth->insertItem( 1.0 );
	m_setLineWidth->insertItem( 2.0 );
	m_setLineWidth->insertItem( 5.0 );
	m_setLineWidth->insertItem( 10.0 );
	m_setLineWidth->insertItem( 20.0 );
	connect( m_setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( setLineWidth() ) );

	m_configureAction = new KAction(
							i18n( "Configure Karbon..." ), "configure", 0, this,
							SLOT( configure() ), actionCollection(), "configure" );

	new KAction( i18n( "Page &Layout..." ), 0, this, SLOT( pageLayout() ), actionCollection(), "page_layout" );
	m_contextHelpAction = new KoContextHelpAction( actionCollection(), this );
}

void
KarbonView::paintEverything( QPainter& /*p*/, const QRect& /*rect*/,
							 bool /*transparent*/ )
{
	kdDebug() << "view->paintEverything()" << endl;
}

bool
KarbonView::mouseEvent( QMouseEvent* event, const KoPoint &p )
{
	if( event->type() == QEvent::MouseMove )
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );
		m_horizRuler->setMousePos( mouseEvent->pos().x(), mouseEvent->pos().y() );
		m_vertRuler->setMousePos( mouseEvent->pos().x(), mouseEvent->pos().y() );
	}
	if( m_currentTool )
		return m_currentTool->mouseEvent( event, p );
	else
		return false;
}

bool
KarbonView::keyEvent( QEvent* event )
{
	if( m_currentTool )
		return m_currentTool->keyEvent( event );
	else
		return false;
}

void
KarbonView::reorganizeGUI()
{
	if( statusBar() )
	{
		if( part()->showStatusBar() )
			statusBar()->show();
		else
			statusBar()->hide();
	}
}

void
KarbonView::setNumberOfRecentFiles( int number )
{
	if( shell() )	// 0L when embedded into konq !
		shell()->setMaxRecentItems( number );
}

void
KarbonView::showRuler()
{
	int space = 20;
	if( m_showRulerAction->isChecked() )
	{
		m_horizRuler->show();
		m_vertRuler->show();
		m_horizRuler->setGeometry( space, 0, width() - space, space );
		m_vertRuler->setGeometry( 0, space, space, height() - space );
		m_canvas->setGeometry( space, space, width() - space, height() - space );
	}
	else
	{
		m_horizRuler->hide();
		m_vertRuler->hide();
		//m_horizRuler->setGeometry( space, 0, width() - space, space );
		//m_vertRuler->setGeometry( 0, space, space, height() - space );
		m_canvas->setGeometry( 0, 0, width(), height() );
	}
}

void
KarbonView::configure()
{
	VConfigureDlg dialog( this );
	dialog.exec();
}

void
KarbonView::pageLayout()
{
	KoHeadFoot hf;
	KoPageLayout layout = part()->pageLayout();
	KoUnit::Unit unit = part()->unit();
	if( KoPageLayoutDia::pageLayout( layout, hf, FORMAT_AND_BORDERS, unit ) )
	{
		part()->setPageLayout( layout, unit );
		m_horizRuler->setPageLayout( layout );
		m_horizRuler->setUnit( unit );
		m_vertRuler->setPageLayout( layout );
		m_vertRuler->setUnit( unit );
		m_canvas->resizeContents( int( ( part()->pageLayout().ptWidth + 40 ) * zoom() ),
								  int( ( part()->pageLayout().ptHeight + 80 ) * zoom() ) );
		part()->repaintAllViews();
	}
}

void
KarbonView::canvasContentsMoving( int x, int y )
{
	m_horizRuler->setOffset( x - 20, 0 );
	m_vertRuler->setOffset( 0, y - 20 );
}

void
KarbonView::selectionChanged()
{
	int count = part()->document().selection()->objects().count();

	if( count > 0 )
	{
		VGroup *group = dynamic_cast<VGroup *>( part()->document().selection()->objects().getFirst() );
		m_groupObjects->setEnabled( count > 1 );
		m_ungroupObjects->setEnabled( group && ( count == 1 ) );
		VObject *obj = part()->document().selection()->objects().getFirst();

		if( count == 1 )
		{
			m_strokeFillPreview->update( *obj->stroke(), *obj->fill() );
			m_strokeDocker->setStroke( *( obj->stroke() ) );
		}
		else
		{
			VStroke stroke;
			stroke.setType( VStroke::none );
			VFill fill;
			m_strokeFillPreview->update( stroke, fill );
		}

		part()->document().selection()->setStroke( *obj->stroke() );
		part()->document().selection()->setFill( *obj->fill() );
		m_setLineWidth->setEnabled( true );
		m_setLineWidth->updateValue( obj->stroke()->lineWidth() );

		VColor *c = new VColor( m_ColorManager->isStrokeDocker() ? obj->stroke()->color() : obj->fill()->color() );
		m_ColorManager->setColor( c );
	}
	else
	{
		m_strokeFillPreview->update( *( part()->document().selection()->stroke() ),
									 *( part()->document().selection()->fill() ) );
		m_setLineWidth->setEnabled( false );
		m_groupObjects->setEnabled( false );
		m_ungroupObjects->setEnabled( false );
	}

	emit selectionChange();
}
void
KarbonView::setCursor( const QCursor &c )
{
	m_canvas->setCursor( c );
}

void
KarbonView::repaintAll( const KoRect &r )
{
	m_canvas->repaintAll( r );
}

void
KarbonView::repaintAll( bool repaint )
{
	m_canvas->repaintAll( repaint );
}
void
KarbonView::setPos( const KoPoint& p )
{
	m_canvas->setPos( p );
}

void
KarbonView::setViewportRect( const KoRect &rect )
{
	m_canvas->setViewportRect( rect );
}

void
KarbonView::setUnit( KoUnit::Unit /*_unit*/ )
{
	if( m_currentTool )
		m_currentTool->refreshUnit();
}

#include "karbon_view.moc"

