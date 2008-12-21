/////////////////////////////////////////////////////////////////////////////

// Name:        dialog_general_options.cpp
// Author:      jean-pierre Charras
/////////////////////////////////////////////////////////////////////////////

#include "fctsys.h"
#include "common.h"
#include "pcbnew.h"

#include "id.h"

#include "dialog_generaloptions_BoardEditor_base.h"

/***********************************************************************/
class Dialog_GeneralOptions : public DialogGeneralOptionsBoardEditor_base
/***********************************************************************/
{
private:
    WinEDA_PcbFrame* m_Parent;
     wxDC* m_DC;
public:
	Dialog_GeneralOptions( WinEDA_PcbFrame* parent, wxDC* DC );
	~Dialog_GeneralOptions() {};
	void OnInitDialog( wxInitDialogEvent& event );
	void OnOkClick( wxCommandEvent& event );
	void OnCancelClick( wxCommandEvent& event );
};

/*****************************************************************/
void WinEDA_PcbFrame::OnSelectOptionToolbar( wxCommandEvent& event )
/*****************************************************************/

/* Must be called on a click on the left toolbar (options toolbar
  * Update variables according to the tools states
 */
{
    int id = event.GetId();


    switch( id )
    {
    case ID_TB_OPTIONS_DRC_OFF:
        Drc_On = m_OptionsToolBar->GetToolState( id ) ? FALSE : TRUE;
        break;

    case ID_TB_OPTIONS_SHOW_GRID:
        m_Draw_Grid = g_ShowGrid = m_OptionsToolBar->GetToolState( id );
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_RATSNEST:
        g_Show_Ratsnest = m_OptionsToolBar->GetToolState( id );
        {
        wxClientDC dc( DrawPanel );
        DrawPanel->PrepareGraphicContext( &dc );
        Ratsnest_On_Off( &dc );
        }
        break;

    case ID_TB_OPTIONS_SHOW_MODULE_RATSNEST:
        g_Show_Module_Ratsnest = m_OptionsToolBar->GetToolState( id );
        break;

    case ID_TB_OPTIONS_SELECT_UNIT_MM:
        g_UnitMetric = MILLIMETRE;

    case ID_TB_OPTIONS_SELECT_UNIT_INCH:
        if( id == ID_TB_OPTIONS_SELECT_UNIT_INCH )
            g_UnitMetric = INCHES;
        m_SelTrackWidthBox_Changed = TRUE;
        Affiche_Status_Box();    /* Reaffichage des coord curseur */
        ReCreateAuxiliaryToolbar();
        DisplayUnitsMsg();
        break;

    case ID_TB_OPTIONS_SHOW_POLAR_COORD:
        Affiche_Message( wxEmptyString );
        DisplayOpt.DisplayPolarCood = m_OptionsToolBar->GetToolState( id );
        Affiche_Status_Box();    /* Reaffichage des coord curseur */
        break;

    case ID_TB_OPTIONS_SELECT_CURSOR:
        g_CursorShape = m_OptionsToolBar->GetToolState( id );
        break;

    case ID_TB_OPTIONS_AUTO_DEL_TRACK:
        g_AutoDeleteOldTrack = m_OptionsToolBar->GetToolState( id );
        break;

    case ID_TB_OPTIONS_SHOW_ZONES:
        DisplayOpt.DisplayZonesMode = 0;
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_ZONES_DISABLE:
        DisplayOpt.DisplayZonesMode = 1;
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_ZONES_OUTLINES_ONLY:
        DisplayOpt.DisplayZonesMode = 2;
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_PADS_SKETCH:
        m_DisplayPadFill = DisplayOpt.DisplayPadFill =
                               !m_OptionsToolBar->GetToolState( id );
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_TRACKS_SKETCH:
        m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill =
                                    !m_OptionsToolBar->GetToolState( id );
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_HIGHT_CONTRAST_MODE:
        DisplayOpt.ContrastModeDisplay =
            m_OptionsToolBar->GetToolState( id );
        DrawPanel->Refresh( );
        break;

    case ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR1:
        if( m_OptionsToolBar->GetToolState( id ) )  // show aux V toolbar (Microwave tool)
            ReCreateAuxVToolbar();
        else
        {
            delete m_AuxVToolBar;
            m_AuxVToolBar = NULL;
        }
        {
            wxSizeEvent SizeEv( GetSize() );

            OnSize( SizeEv );
        }
        break;

    default:
        DisplayError( this,
            wxT( "WinEDA_PcbFrame::OnSelectOptionToolbar error \n (event not handled!)" ) );
        break;
    }

    SetToolbars();
}


Dialog_GeneralOptions::Dialog_GeneralOptions( WinEDA_PcbFrame* parent, wxDC* DC ) :
		DialogGeneralOptionsBoardEditor_base( parent )
{
    m_Parent = parent;
    m_DC = DC;
}


/*!
 * Dialog_GeneralOptions creator
 */

void Dialog_GeneralOptions::OnInitDialog( wxInitDialogEvent& event )
{
    SetFont( *g_DialogFont );
	SetFocus();


    /* Set display options */
    m_PolarDisplay->SetSelection( DisplayOpt.DisplayPolarCood ? 1 : 0 );
    m_UnitsSelection->SetSelection( g_UnitMetric ? 1 : 0 );
    m_CursorShape->SetSelection( g_CursorShape ? 1 : 0 );

    wxString timevalue;
    timevalue << g_TimeOut / 60;
    m_SaveTime->SetValue( timevalue );
	int layer_count[] = {1,2,4,6,8,10,12,14,16};
	m_LayerNumber->SetSelection(1);
	for ( unsigned ii = 0; ii < sizeof(layer_count); ii++ )
	{
		if ( g_DesignSettings.m_CopperLayerCount == layer_count[ii] )
			continue;
		m_LayerNumber->SetSelection(ii);
		break;
	}

    m_LayerNumber->SetSelection( g_DesignSettings.m_CopperLayerCount );
    m_MaxShowLinks->SetValue( g_MaxLinksShowed );

    m_DrcOn->SetValue( Drc_On );
    m_ShowModuleRatsnest->SetValue( g_Show_Module_Ratsnest );
    m_ShowGlobalRatsnest->SetValue( g_Show_Ratsnest );
    m_TrackAutodel->SetValue( g_AutoDeleteOldTrack );
    m_Track_45_Only_Ctrl->SetValue( Track_45_Only );
    m_Segments_45_Only_Ctrl->SetValue( Segments_45_Only );
    m_AutoPANOpt->SetValue( m_Parent->DrawPanel->m_AutoPAN_Enable );
    m_Segments_45_Only_Ctrl->SetValue( Segments_45_Only );
    m_Track_DoubleSegm_Ctrl->SetValue( g_TwoSegmentTrackBuild );

    m_MagneticPadOptCtrl->SetSelection( g_MagneticPadOption );
    m_MagneticTrackOptCtrl->SetSelection( g_MagneticTrackOption );

    if( GetSizer() )
    {
        GetSizer()->SetSizeHints( this );
    }
}


/*****************************************************************/
void Dialog_GeneralOptions::OnCancelClick( wxCommandEvent& event )
/*****************************************************************/
{
    event.Skip();
}



/**************************************************************************/
void Dialog_GeneralOptions::OnOkClick( wxCommandEvent& event )
/**************************************************************************/
{
    int ii;

    DisplayOpt.DisplayPolarCood =
        (m_PolarDisplay->GetSelection() == 0) ? FALSE : TRUE;
    ii = g_UnitMetric;
    g_UnitMetric = (m_UnitsSelection->GetSelection() == 0)  ? 0 : 1;
    if( ii != g_UnitMetric )
        m_Parent->ReCreateAuxiliaryToolbar();

    g_CursorShape = m_CursorShape->GetSelection();
    g_TimeOut = 60 * m_SaveTime->GetValue();

    /* Mise a jour de la combobox d'affichage de la couche active */
	int layer_count[] = {1,2,4,6,8,10,12,14,16};
    g_DesignSettings.m_CopperLayerCount = layer_count[m_LayerNumber->GetSelection()];
    m_Parent->ReCreateLayerBox( NULL );

    g_MaxLinksShowed = m_MaxShowLinks->GetValue();
    Drc_On = m_DrcOn->GetValue();
    if( g_Show_Ratsnest != m_ShowGlobalRatsnest->GetValue() )
    {
        g_Show_Ratsnest = m_ShowGlobalRatsnest->GetValue();
        m_Parent->Ratsnest_On_Off( m_DC );
    }
    g_Show_Module_Ratsnest = m_ShowModuleRatsnest->GetValue();
    g_AutoDeleteOldTrack   = m_TrackAutodel->GetValue();
    Segments_45_Only = m_Segments_45_Only_Ctrl->GetValue();
    Track_45_Only    = m_Track_45_Only_Ctrl->GetValue();
    m_Parent->DrawPanel->m_AutoPAN_Enable = m_AutoPANOpt->GetValue();
    g_TwoSegmentTrackBuild = m_Track_DoubleSegm_Ctrl->GetValue();

    EndModal( 1 );
}


enum id_optpcb {
    ID_ACCEPT_OPT = 1000,
    ID_CANCEL_OPT
};

#include "dialog_track_options.cpp"
#include "dialog_display_options.cpp"
#include "dialog_graphic_items_options.cpp"

/*****************************************************************/
void WinEDA_PcbFrame::InstallPcbOptionsFrame( const wxPoint& pos,
                                              wxDC* DC, int id )
/*****************************************************************/
{
    switch( id )
    {
    case ID_PCB_TRACK_SIZE_SETUP:
        {
            WinEDA_PcbTracksDialog* OptionsFrame =
                new WinEDA_PcbTracksDialog( this );

            OptionsFrame->ShowModal();
            OptionsFrame->Destroy();
        }
        break;

    case ID_PCB_DRAWINGS_WIDTHS_SETUP:
        {
            WinEDA_GraphicItemsOptionsDialog* OptionsFrame =
                new WinEDA_GraphicItemsOptionsDialog( this );

            OptionsFrame->ShowModal();
            OptionsFrame->Destroy();
        }
        break;

    case ID_PCB_LOOK_SETUP:
        {
            WinEDA_DisplayOptionsDialog* OptionsFrame =
                new WinEDA_DisplayOptionsDialog( this );

            OptionsFrame->ShowModal();
            OptionsFrame->Destroy();
        }
        break;

    case ID_OPTIONS_SETUP:
        {
            Dialog_GeneralOptions* OptionsFrame =
                new Dialog_GeneralOptions( this, DC );

            OptionsFrame->ShowModal();
            OptionsFrame->Destroy();
        }
        break;
    }
}


/*******************************************************************/
void WinEDA_ModuleEditFrame::InstallOptionsFrame( const wxPoint& pos )
/*******************************************************************/
{
    WinEDA_GraphicItemsOptionsDialog OptionsFrame( this );

    OptionsFrame.ShowModal();
}
