/////////////////////////////////////////////////////////////////////////////

// Name:        dialog_copper_zones.cpp
// Author:      jean-pierre Charras
// Created:     09/oct/2008
/// Licence:     GNU License
/////////////////////////////////////////////////////////////////////////////

#if defined (__GNUG__) && !defined (NO_GCC_PRAGMA)
#pragma implementation "zones.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "fctsys.h"
#include "wxstruct.h"

#include "common.h"
#include "PolyLine.h"
#include "pcbnew.h"
#include "trigo.h"
#include "autorout.h"
#include "zones.h"

#include "dialog_copper_zones.h"


/************************************************************************************************/
dialog_copper_zone::dialog_copper_zone( WinEDA_PcbFrame* parent, ZONE_SETTING* zone_setting ) :
    dialog_copper_zone_base( parent )
/************************************************************************************************/
{
    m_Parent = parent;
    m_Zone_Setting = zone_setting;
    m_NetSorting   = 1;     // 0 = alphabetic sort, 1 = pad count sort
    if( m_Parent->m_Parent->m_EDA_Config )
    {
        m_NetSorting = m_Parent->m_Parent->m_EDA_Config->Read( ZONE_NET_SORT_OPTION_KEY, 1l );
    }

    SetReturnCode( ZONE_ABORT ); // Will be changed on buttons click
}


/*****************************************************************/
void dialog_copper_zone::OnInitDialog( wxInitDialogEvent& event )
/*****************************************************************/

// Initialise all dialog options and values in wxTextCtrl
{
    BOARD* board = m_Parent->m_Pcb;

    SetFont( *g_DialogFont );

    SetFocus();     // Required under wxGTK if we want to demiss the dialog with the ESC key

    wxString msg = _( "Zone clearance value:" ) + ReturnUnitSymbol( g_UnitMetric );
    m_ClearanceValueTitle->SetLabel( msg );

    msg = _( "Grid :" ) + ReturnUnitSymbol( g_UnitMetric );
    m_GridCtrl->SetLabel( msg );

    msg = ReturnStringFromValue( g_UnitMetric,
                                 m_Zone_Setting->m_ZoneClearance,
                                 m_Parent->m_InternalUnits );
    m_ZoneClearanceCtrl->SetValue( msg );

    if( g_Zone_45_Only )
        m_OrientEdgesOpt->SetSelection( 1 );

    static const int GridList[4] = { 25, 50, 100, 250 };
    int selection = 0;

    int grid_routing = m_Zone_Setting->m_GridFillValue;

    for( unsigned ii = 0; ii < 4; ii++ )
    {
        msg = ReturnStringFromValue( g_UnitMetric,
                                     GridList[ii],
                                     m_Parent->m_InternalUnits );
        m_GridCtrl->SetString( ii, msg );
        if( grid_routing == GridList[ii] )
            selection = ii;
    }

    if( grid_routing == 0 )  // No Grid: fill with polygons
        selection = 4;

    m_GridCtrl->SetSelection( selection );

    msg = ReturnStringFromValue( g_UnitMetric,
                                 m_Zone_Setting->m_ZoneClearance,
                                 m_Parent->m_InternalUnits );
    m_ZoneClearanceCtrl->SetValue( msg );

    switch( m_Zone_Setting->m_Zone_Pad_Options )
    {
    case PAD_NOT_IN_ZONE:           // Pads are not covered
        m_PadInZoneOpt->SetSelection( 2 );
        break;

    case THERMAL_PAD:               // Use thermal relief for pads
        m_PadInZoneOpt->SetSelection( 1 );
        break;

    case PAD_IN_ZONE:               // pads are covered by copper
        m_PadInZoneOpt->SetSelection( 0 );
        break;
    }

    if( m_Zone_Setting->m_FilledAreasShowMode == 1 )
        m_ShowFilledAreasInSketchOpt->SetValue( true );


    if( m_Zone_Setting->m_Zone_Pad_Options != THERMAL_PAD )
    {
        m_AntipadSizeValue->Enable( false );
        m_CopperWidthValue->Enable( false );
    }
    else
    {
        m_AntipadSizeValue->Enable( true );
        m_CopperWidthValue->Enable( true );
    }

    AddUnitSymbol( *m_AntipadSizeText, g_UnitMetric );
    AddUnitSymbol( *m_CopperBridgeWidthText, g_UnitMetric );
    PutValueInLocalUnits( *m_AntipadSizeValue,
                          m_Zone_Setting->m_ThermalReliefGapValue,
                          PCB_INTERNAL_UNIT );
    PutValueInLocalUnits( *m_CopperWidthValue,
                          m_Zone_Setting->m_ThermalReliefCopperBridgeValue,
                          PCB_INTERNAL_UNIT );

    switch( m_Zone_Setting->m_Zone_HatchingStyle )
    {
    case CPolyLine::NO_HATCH:
        m_OutlineAppearanceCtrl->SetSelection( 0 );
        break;

    case CPolyLine::DIAGONAL_EDGE:
        m_OutlineAppearanceCtrl->SetSelection( 1 );
        break;

    case CPolyLine::DIAGONAL_FULL:
        m_OutlineAppearanceCtrl->SetSelection( 2 );
        break;
    }

    m_ArcApproximationOpt->SetSelection( m_Zone_Setting->m_ArcToSegmentsCount == 32 ? 1 : 0 );

    /* build copper layers list */
    int layer_cnt = board->GetCopperLayerCount();
    for( int ii = 0; ii < board->GetCopperLayerCount(); ii++ )
    {
        int layer_number = COPPER_LAYER_N;

        if( layer_cnt <= 1 || ii < layer_cnt - 1 )
            layer_number = ii;
        else if( ii == layer_cnt - 1 )
            layer_number = LAYER_CMP_N;

        m_LayerId[ii] = layer_number;

        msg = board->GetLayerName( layer_number ).Trim();
        m_LayerSelectionCtrl->InsertItems( 1, &msg, ii );

        if( m_Zone_Setting->m_CurrentZone_Layer == layer_number )
            m_LayerSelectionCtrl->SetSelection( ii );
    }

    m_NetSortingOption->SetSelection( m_NetSorting );

    wxString      NetNameFilter;
    if( m_Parent->m_Parent->m_EDA_Config )
    {
        NetNameFilter =
            m_Parent->m_Parent->m_EDA_Config->Read( ZONE_NET_FILTER_STRING_KEY, wxT( "N_0*" ) );
    }

    m_NetNameFilter->SetValue( NetNameFilter );
    wxArrayString ListNetName;
    m_Parent->m_Pcb->ReturnSortedNetnamesList(
        ListNetName,
        m_NetSorting ==
        0 ? BOARD::ALPHA_SORT : BOARD::PAD_CNT_SORT );

    if( m_NetSorting != 0 )
    {
        wxString Filter = m_NetNameFilter->GetValue();
        for( unsigned ii = 0; ii < ListNetName.GetCount(); ii++ )
        {
            if( ListNetName[ii].Matches( Filter.GetData() ) )
            {
                ListNetName.RemoveAt( ii );
                ii--;
            }
        }
    }

    m_ListNetNameSelection->InsertItems( ListNetName, 0 );

    // Select net:
    int net_select = m_Zone_Setting->m_NetcodeSelection;

    if( net_select > 0 )
    {
        EQUIPOT* equipot = m_Parent->m_Pcb->FindNet( net_select );
        if( equipot )  // Search net in list and select it
        {
            for( unsigned ii = 0; ii < ListNetName.GetCount(); ii++ )
            {
                if( ListNetName[ii] == equipot->m_Netname )
                {
                    m_ListNetNameSelection->SetSelection( ii );
                    m_ListNetNameSelection->EnsureVisible( ii );
                    break;
                }
            }
        }
    }
    if( GetSizer() )
    {
        GetSizer()->SetSizeHints( this );
    }
    Center();
}


/********************************************************************/
void dialog_copper_zone::OnButtonCancelClick( wxCommandEvent& event )
/********************************************************************/
{
    EndModal( ZONE_ABORT );
}


/********************************************************************************************/
bool dialog_copper_zone::AcceptOptions( bool aPromptForErrors, bool aUseExportableSetupOnly )
/********************************************************************************************/

/** Function dialog_copper_zone::AcceptOptions(
 * @return false if incorrect options, true if Ok.
 * @param aPromptForErrors = true to prompt user on incorrectparams
 * @param aUseExportableSetupOnly = true to use exportable parametres only (used to export this setup to other zones)
 */
{
    switch( m_PadInZoneOpt->GetSelection() )
    {
    case 2:
        m_Zone_Setting->m_Zone_Pad_Options = PAD_NOT_IN_ZONE;       // Pads are not covered
        break;

    case 1:
        m_Zone_Setting->m_Zone_Pad_Options = THERMAL_PAD;           // Use thermal relief for pads
        break;

    case 0:
        m_Zone_Setting->m_Zone_Pad_Options = PAD_IN_ZONE;           // pads are covered by copper
        break;
    }

    switch( m_OutlineAppearanceCtrl->GetSelection() )
    {
    case 0:
        m_Zone_Setting->m_Zone_HatchingStyle = CPolyLine::NO_HATCH;
        break;

    case 1:
        m_Zone_Setting->m_Zone_HatchingStyle = CPolyLine::DIAGONAL_EDGE;
        break;

    case 2:
        m_Zone_Setting->m_Zone_HatchingStyle = CPolyLine::DIAGONAL_FULL;
        break;
    }

    m_Zone_Setting->m_ArcToSegmentsCount = m_ArcApproximationOpt->GetSelection() == 1 ? 32 : 16;

    if( m_Parent->m_Parent->m_EDA_Config )
    {
        m_Parent->m_Parent->m_EDA_Config->Write( ZONE_NET_OUTLINES_HATCH_OPTION_KEY,
                                                 (long) m_Zone_Setting->m_Zone_HatchingStyle );
    }

    switch( m_GridCtrl->GetSelection() )
    {
    case 0:
        m_Zone_Setting->m_GridFillValue = 25;
        break;

    case 1:
        m_Zone_Setting->m_GridFillValue = 50;
        break;

    default:
    case 2:
        m_Zone_Setting->m_GridFillValue = 100;
        break;

    case 3:
        m_Zone_Setting->m_GridFillValue = 250;
        break;

    case 4:
        m_Zone_Setting->m_GridFillValue = 0;
#if 0   // I hope this feature works fine ( JP Charras)
        DisplayInfo( this, wxT(
                        "You are using No grid for filling zones\nThis is currently in development and for tests only.\n Do not use for production" ) );
#endif
        break;
    }

    wxString txtvalue = m_ZoneClearanceCtrl->GetValue();
    m_Zone_Setting->m_ZoneClearance =
        ReturnValueFromString( g_UnitMetric, txtvalue, m_Parent->m_InternalUnits );
    if( m_OrientEdgesOpt->GetSelection() == 0 )
        g_Zone_45_Only = FALSE;
    else
        g_Zone_45_Only = TRUE;

    m_Zone_Setting->m_FilledAreasShowMode = m_ShowFilledAreasInSketchOpt->IsChecked() ? 1 : 0;

    m_Zone_Setting->m_ThermalReliefGapValue = ReturnValueFromTextCtrl( *m_AntipadSizeValue,
                                                                       PCB_INTERNAL_UNIT );
    m_Zone_Setting->m_ThermalReliefCopperBridgeValue = ReturnValueFromTextCtrl(
        *m_CopperWidthValue,
        PCB_INTERNAL_UNIT );

    m_Parent->m_Parent->m_EDA_Config->Write( ZONE_THERMAL_RELIEF_GAP_STRING_KEY,
                                             (long) m_Zone_Setting->m_ThermalReliefGapValue );
    m_Parent->m_Parent->m_EDA_Config->Write(
        ZONE_THERMAL_RELIEF_COPPER_WIDTH_STRING_KEY,
        (long) m_Zone_Setting->
        m_ThermalReliefCopperBridgeValue );

    // If we use only exportable to others zones parameters, exit here:
    if( aUseExportableSetupOnly )
        return true;

    /* Get the layer selection for this zone */
    int ii = m_LayerSelectionCtrl->GetSelection();
    if( ii < 0 && aPromptForErrors )
    {
        DisplayError( this, _( "Error : you must choose a layer" ) );
        return false;
    }


    m_Zone_Setting->m_CurrentZone_Layer = m_LayerId[ii];


    /* Get the net name selection for this zone */
    ii = m_ListNetNameSelection->GetSelection();
    if( ii < 0 && aPromptForErrors )
    {
        DisplayError( this, _( "Error : you must choose a net name" ) );
        return false;
    }

    wxString net_name = m_ListNetNameSelection->GetString( ii );

    /* Search net_code for this net */
    EQUIPOT* net;
    g_Zone_Default_Setting.m_NetcodeSelection = 0;
    for( net = m_Parent->m_Pcb->m_Equipots;   net;  net = net->Next() )
    {
        if( net->m_Netname == net_name )
        {
            g_Zone_Default_Setting.m_NetcodeSelection = net->GetNet();
            break;
        }
    }

    return true;
}


/***************************************************************************/
void dialog_copper_zone::OnNetSortingOptionSelected( wxCommandEvent& event )
/***************************************************************************/
{
    wxArrayString ListNetName;

    m_NetSorting = m_NetSortingOption->GetSelection();
    m_Parent->m_Pcb->ReturnSortedNetnamesList(
        ListNetName,
        m_NetSorting ==
        0 ? BOARD::ALPHA_SORT : BOARD::PAD_CNT_SORT );
    if( m_NetSorting != 0 )
    {
        wxString Filter = m_NetNameFilter->GetValue();
        for( unsigned ii = 0; ii < ListNetName.GetCount(); ii++ )
        {
            if( ListNetName[ii].Matches( Filter.GetData() ) )
            {
                ListNetName.RemoveAt( ii );
                ii--;
            }
        }
    }
    m_ListNetNameSelection->Clear();
    m_ListNetNameSelection->InsertItems( ListNetName, 0 );
    if( m_Parent->m_Parent->m_EDA_Config )
    {
        m_Parent->m_Parent->m_EDA_Config->Write( ZONE_NET_SORT_OPTION_KEY, (long) m_NetSorting );
        m_Parent->m_Parent->m_EDA_Config->Write( ZONE_NET_FILTER_STRING_KEY,
                                                m_NetNameFilter->GetValue() );
    }

    // Select and isplay current zone net name in listbox:
    int net_select = m_Zone_Setting->m_NetcodeSelection;
    if( net_select > 0 )
    {
        EQUIPOT* equipot = m_Parent->m_Pcb->FindNet( net_select );
        if( equipot )  // Search net in list and select it
        {
            for( unsigned ii = 0; ii < ListNetName.GetCount(); ii++ )
            {
                if( ListNetName[ii] == equipot->m_Netname )
                {
                    m_ListNetNameSelection->SetSelection( ii );
                    m_ListNetNameSelection->EnsureVisible( ii );
                    break;
                }
            }
        }
    }
}


/*****************************************************************/
void dialog_copper_zone::OnButtonOkClick( wxCommandEvent& event )
/*****************************************************************/
{
    if( AcceptOptions( true ) )
        EndModal( ZONE_OK );
}


/******************************************************************************/
void dialog_copper_zone::ExportSetupToOtherCopperZones( wxCommandEvent& event )
/******************************************************************************/
{
    if( !AcceptOptions( true, true ) )
        return;

    // Export settings ( but layer ) to others zones:
    BOARD* pcb = m_Parent->m_Pcb;
    for( int ii = 0; ii < pcb->GetAreaCount(); ii++ )
    {
        ZONE_CONTAINER* zone = pcb->GetArea( ii );
        int             zone_layer = zone->GetLayer();
        m_Zone_Setting->ExportSetting( *zone );
        zone->SetLayer( zone_layer );
        m_Parent->GetScreen()->SetModify();
    }
}


/******************************************************************/
void dialog_copper_zone::OnPadsInZoneClick( wxCommandEvent& event )
/******************************************************************/
{
    switch( m_PadInZoneOpt->GetSelection() )
    {
    default:
        m_AntipadSizeValue->Enable( false );
        m_CopperWidthValue->Enable( false );
        break;

    case 1:
        m_AntipadSizeValue->Enable( true );
        m_CopperWidthValue->Enable( true );
        break;
    }
}
