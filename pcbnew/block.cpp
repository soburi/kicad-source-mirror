/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file pcbnew/block.cpp
 */


#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <block_commande.h>
#include <pcb_edit_frame.h>
#include <trigo.h>

#include <class_board.h>
#include <class_track.h>
#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <class_pcb_target.h>
#include <class_module.h>
#include <class_dimension.h>
#include <class_zone.h>

#include <dialog_block_options.h>

#include <pcbnew.h>
#include <protos.h>

#include <connectivity_data.h>

#define BLOCK_OUTLINE_COLOR YELLOW

/**
 * Function drawPickedItems
 * draws items currently selected in a block
 * @param aPanel = Current draw panel
 * @param aDC = Current device context
 * @param aOffset = Drawing offset
 **/
static void drawPickedItems( EDA_DRAW_PANEL* aPanel, wxDC* aDC, wxPoint aOffset );

/**
 * Function drawMovingBlock
 * handles drawing of a moving block
 * @param aPanel = Current draw panel
 * @param aDC = Current device context
 * @param aPosition The cursor position in logical (drawing) units.
 * @param aErase = Erase block at current position
 **/
static void drawMovingBlock( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                             bool aErase );


static DIALOG_BLOCK_OPTIONS::OPTIONS blockOpts;


static bool InstallBlockCmdFrame( PCB_BASE_FRAME* parent, const wxString& title )
{
    wxPoint oldpos = parent->GetCrossHairPosition();

    parent->GetCanvas()->SetIgnoreMouseEvents( true );
    DIALOG_BLOCK_OPTIONS * dlg = new DIALOG_BLOCK_OPTIONS( parent, blockOpts, true, title );

    int cmd = dlg->ShowModal();
    dlg->Destroy();

    parent->SetCrossHairPosition( oldpos );
    parent->GetCanvas()->MoveCursorToCrossHair();
    parent->GetCanvas()->SetIgnoreMouseEvents( false );

    return cmd == wxID_OK;
}


int PCB_EDIT_FRAME::BlockCommand( EDA_KEY aKey )
{
    int cmd = 0;

    switch( aKey )
    {
    default:
        cmd = aKey & 0xFF;
        break;

    case 0:
        cmd = BLOCK_MOVE;
        break;

    case GR_KB_SHIFT:
        cmd = BLOCK_DUPLICATE;
        break;

    case GR_KB_CTRL:
        cmd = BLOCK_ROTATE;
        break;

    case GR_KB_SHIFTCTRL:
        cmd = BLOCK_DELETE;
        break;

    case GR_KB_ALT:
        cmd = BLOCK_FLIP;
        break;

    case MOUSE_MIDDLE:
        cmd = BLOCK_ZOOM;
        break;
    }

    return cmd;
}


void PCB_EDIT_FRAME::HandleBlockPlace( wxDC* DC )
{
    GetBoard()->m_Status_Pcb &= ~DO_NOT_SHOW_GENERAL_RASTNEST;

    if( !m_canvas->IsMouseCaptured() )
    {
        DisplayError( this, wxT( "Error in HandleBlockPLace : m_mouseCaptureCallback = NULL" ) );
    }

    GetScreen()->m_BlockLocate.SetState( STATE_BLOCK_STOP );

    const BLOCK_COMMAND_T command = GetScreen()->m_BlockLocate.GetCommand();

    switch( command )
    {
    case BLOCK_IDLE:
        break;

    case BLOCK_DRAG:                // Drag
    case BLOCK_MOVE:                // Move
    case BLOCK_PRESELECT_MOVE:      // Move with preselection list
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        Block_Move();
        GetScreen()->m_BlockLocate.ClearItemsList();
        break;

    case BLOCK_DUPLICATE:     // Duplicate
    case BLOCK_DUPLICATE_AND_INCREMENT:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        Block_Duplicate( command == BLOCK_DUPLICATE_AND_INCREMENT );
        GetScreen()->m_BlockLocate.ClearItemsList();
        break;

    case BLOCK_PASTE:
        break;

    case BLOCK_ZOOM:        // Handled by HandleBlockEnd()
    default:
        break;
    }

    OnModify();

    m_canvas->EndMouseCapture( GetToolId(), m_canvas->GetCurrentCursor(), wxEmptyString, false );
    GetScreen()->ClearBlockCommand();

    if( GetScreen()->m_BlockLocate.GetCount() )
    {
        DisplayError( this, wxT( "Error in HandleBlockPLace some items left in list" ) );
        GetScreen()->m_BlockLocate.ClearItemsList();
    }
}


bool PCB_EDIT_FRAME::HandleBlockEnd( wxDC* DC )
{
    bool nextcmd = false;       // Will be set to true if a block place is needed
    bool cancelCmd = false;
    // If coming here after cancel block, clean up and exit
    if( GetScreen()->m_BlockLocate.GetState() == STATE_NO_BLOCK )
    {
        m_canvas->EndMouseCapture( GetToolId(), m_canvas->GetCurrentCursor(), wxEmptyString,
                                   false );
        GetScreen()->ClearBlockCommand();
        return false;
    }

    // Show dialog if there are no selected items and we're not zooming
    if( !GetScreen()->m_BlockLocate.GetCount()
      && GetScreen()->m_BlockLocate.GetCommand() != BLOCK_ZOOM )
    {
        if( InstallBlockCmdFrame( this, _( "Block Operation" ) ) == false )
        {
            cancelCmd = true;

            // undraw block outline
            if( DC )
                m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );
        }
        else
        {
            DrawAndSizingBlockOutlines( m_canvas, DC, wxDefaultPosition, false );
            Block_SelectItems();

            // Exit if no items found
            if( !GetScreen()->m_BlockLocate.GetCount() )
                cancelCmd = true;
        }
    }

    if( !cancelCmd && m_canvas->IsMouseCaptured() )
    {
        switch( GetScreen()->m_BlockLocate.GetCommand() )
        {
        case BLOCK_IDLE:
            DisplayError( this, wxT( "Error in HandleBlockPLace" ) );
            break;

        case BLOCK_DRAG:                // Drag (not used, for future enhancements)
        case BLOCK_MOVE:                // Move
        case BLOCK_DUPLICATE:           // Duplicate
        case BLOCK_DUPLICATE_AND_INCREMENT:  // Duplicate and increment relevant references
        case BLOCK_PRESELECT_MOVE:      // Move with preselection list
            GetScreen()->m_BlockLocate.SetState( STATE_BLOCK_MOVE );
            nextcmd = true;
            m_canvas->SetMouseCaptureCallback( drawMovingBlock );
            if( DC )
                m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );
            break;

        case BLOCK_DELETE: // Delete
            m_canvas->SetMouseCaptureCallback( NULL );
            GetScreen()->m_BlockLocate.SetState( STATE_BLOCK_STOP );
            Block_Delete();
            break;

        case BLOCK_ROTATE: // Rotation
            m_canvas->SetMouseCaptureCallback( NULL );
            GetScreen()->m_BlockLocate.SetState( STATE_BLOCK_STOP );
            Block_Rotate();
            break;

        case BLOCK_FLIP: // Flip
            m_canvas->SetMouseCaptureCallback( NULL );
            GetScreen()->m_BlockLocate.SetState( STATE_BLOCK_STOP );
            Block_Flip();
            break;

        case BLOCK_COPY: // Copy
            GetScreen()->m_BlockLocate.SetState( STATE_BLOCK_STOP );

            if( GetScreen()->m_BlockLocate.GetCount() )
            {
                // @todo (if useful)         Save_Block( );
            }
            break;

        case BLOCK_PASTE:
            break;

        case BLOCK_ZOOM: // Window Zoom

            // Turn off the redraw block routine now so it is not displayed
            // with one corner at the new center of the screen
            m_canvas->SetMouseCaptureCallback( NULL );
            Window_Zoom( GetScreen()->m_BlockLocate );
            break;

        default:
            break;
        }
    }

    if( ! nextcmd )
    {
        GetBoard()->m_Status_Pcb |= DO_NOT_SHOW_GENERAL_RASTNEST;
        GetScreen()->ClearBlockCommand();
        m_canvas->EndMouseCapture( GetToolId(), m_canvas->GetCurrentCursor(), wxEmptyString,
                                   false );
    }

    return nextcmd;
}


void PCB_EDIT_FRAME::Block_SelectItems()
{
    LSET layerMask;
    bool selectOnlyComplete = GetScreen()->m_BlockLocate.GetWidth() > 0 ;

    GetScreen()->m_BlockLocate.Normalize();

    PICKED_ITEMS_LIST* itemsList = &GetScreen()->m_BlockLocate.GetItems();
    ITEM_PICKER        picker( NULL, UR_UNSPECIFIED );

    // Add modules
    if( blockOpts.includeModules )
    {
        for( MODULE* module = m_Pcb->m_Modules;  module;  module = module->Next() )
        {
            PCB_LAYER_ID layer = module->GetLayer();

            if( module->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete )
                && ( !module->IsLocked() || blockOpts.includeLockedModules ) )
            {
                if( blockOpts.includeItemsOnInvisibleLayers || m_Pcb->IsModuleLayerVisible( layer ) )
                {
                    picker.SetItem ( module );
                    itemsList->PushItem( picker );
                }
            }
        }
    }

    // Add tracks and vias
    for( auto PtStruct : m_Pcb->Tracks() )
    {
        if( !m_Pcb->IsLayerVisible( PtStruct->GetLayer() ) && !blockOpts.includeItemsOnInvisibleLayers )
            continue;

        if( !( blockOpts.includeTracks && PtStruct->Type() == PCB_TRACE_T ) &&
            !( blockOpts.includeVias && PtStruct->Type() == PCB_VIA_T) )
            continue;

        if( PtStruct->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete ) )
        {
            picker.SetItem ( PtStruct );
            itemsList->PushItem( picker );
        }
    }

    // Add graphic items
    layerMask = LSET( Edge_Cuts );

    if( blockOpts.includeItemsOnTechLayers )
        layerMask.set();

    if( !blockOpts.includeBoardOutlineLayer )
        layerMask.set( Edge_Cuts, false );

    for( auto PtStruct : m_Pcb->Drawings() )
    {
        if( !m_Pcb->IsLayerVisible( PtStruct->GetLayer() ) && ! blockOpts.includeItemsOnInvisibleLayers)
            continue;

        bool select_me = false;

        switch( PtStruct->Type() )
        {
        case PCB_LINE_T:
            if( !layerMask[PtStruct->GetLayer()] )
                break;

            if( !PtStruct->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete ) )
                break;

            select_me = true; // This item is in bloc: select it
            break;

        case PCB_TEXT_T:
            if( !blockOpts.includePcbTexts )
                break;

            if( !PtStruct->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete ) )
                break;

            select_me = true; // This item is in bloc: select it
            break;

        case PCB_TARGET_T:
            if( !layerMask[PtStruct->GetLayer()] )
                break;

            if( !PtStruct->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete ) )
                break;

            select_me = true; // This item is in bloc: select it
            break;

        case PCB_DIMENSION_T:
            if( !layerMask[PtStruct->GetLayer()] )
                break;

            if( !PtStruct->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete ) )
                break;

            select_me = true; // This item is in bloc: select it
            break;

        default:
            break;
        }

        if( select_me )
        {
            picker.SetItem ( PtStruct );
            itemsList->PushItem( picker );
        }
    }

    // Add zones
    if( blockOpts.includeZones )
    {
        for( int ii = 0; ii < m_Pcb->GetAreaCount(); ii++ )
        {
            ZONE_CONTAINER* area = m_Pcb->GetArea( ii );

            if( area->HitTest( GetScreen()->m_BlockLocate, selectOnlyComplete ) )
            {
                if( blockOpts.includeItemsOnInvisibleLayers
                  || m_Pcb->IsLayerVisible( area->GetLayer() ) )
                {
                    BOARD_ITEM* zone_c = (BOARD_ITEM*) area;
                    picker.SetItem ( zone_c );
                    itemsList->PushItem( picker );
                }
            }
        }
    }
}


static void drawPickedItems( EDA_DRAW_PANEL* aPanel, wxDC* aDC, wxPoint aOffset )
{
    PICKED_ITEMS_LIST* itemsList = &aPanel->GetScreen()->m_BlockLocate.GetItems();
    PCB_BASE_FRAME* frame = (PCB_BASE_FRAME*) aPanel->GetParent();

    g_Offset_Module = -aOffset;

    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );

        switch( item->Type() )
        {
        case PCB_MODULE_T:
            frame->GetBoard()->m_Status_Pcb &= ~RATSNEST_ITEM_LOCAL_OK;
            ((MODULE*) item)->DrawOutlinesWhenMoving( aPanel, aDC, g_Offset_Module );
            break;

        case PCB_LINE_T:
        case PCB_TEXT_T:
        case PCB_TRACE_T:
        case PCB_VIA_T:
        case PCB_TARGET_T:
        case PCB_DIMENSION_T:    // Currently markers are not affected by block commands
        case PCB_MARKER_T:
            item->Draw( aPanel, aDC, GR_XOR, aOffset );
            break;

        case PCB_ZONE_AREA_T:
            item->Draw( aPanel, aDC, GR_XOR, aOffset );
            ((ZONE_CONTAINER*) item)->DrawFilledArea( aPanel, aDC, GR_XOR, aOffset );
            break;

        default:
            break;
        }
    }

    g_Offset_Module = wxPoint( 0, 0 );
}


static void drawMovingBlock( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                             bool aErase )
{
    BASE_SCREEN* screen = aPanel->GetScreen();

    // do not show local module rastnest in block move, it is not usable.
    auto displ_opts = (PCB_DISPLAY_OPTIONS*)( aPanel->GetDisplayOptions() );
    bool showRats = displ_opts->m_Show_Module_Ratsnest;
    displ_opts->m_Show_Module_Ratsnest = false;

    if( aErase )
    {
        if( screen->m_BlockLocate.GetMoveVector().x || screen->m_BlockLocate.GetMoveVector().y )
        {
            screen->m_BlockLocate.Draw( aPanel, aDC, screen->m_BlockLocate.GetMoveVector(),
                                        GR_XOR, BLOCK_OUTLINE_COLOR );

            if( blockOpts.drawItems )
                drawPickedItems( aPanel, aDC, screen->m_BlockLocate.GetMoveVector() );
        }
    }


    if( screen->m_BlockLocate.GetState() != STATE_BLOCK_STOP )
    {
        screen->m_BlockLocate.SetMoveVector( aPanel->GetParent()->GetCrossHairPosition() -
                                             screen->m_BlockLocate.GetLastCursorPosition() );
    }

    if( screen->m_BlockLocate.GetMoveVector().x || screen->m_BlockLocate.GetMoveVector().y )
    {
        screen->m_BlockLocate.Draw( aPanel, aDC, screen->m_BlockLocate.GetMoveVector(),
                                    GR_XOR, BLOCK_OUTLINE_COLOR );

        if( blockOpts.drawItems )
            drawPickedItems( aPanel, aDC, screen->m_BlockLocate.GetMoveVector() );
    }

    displ_opts->m_Show_Module_Ratsnest = showRats;
}


void PCB_EDIT_FRAME::Block_Delete()
{
    OnModify();
    SetCurItem( NULL );

    PICKED_ITEMS_LIST* itemsList = &GetScreen()->m_BlockLocate.GetItems();
    itemsList->m_Status = UR_DELETED;

    // unlink items and clear flags
    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );
        itemsList->SetPickedItemStatus( UR_DELETED, ii );
        GetBoard()->GetConnectivity()->Remove( item );

        switch( item->Type() )
        {
        case PCB_MODULE_T:
        {
            MODULE* module = (MODULE*) item;
            module->ClearFlags();
            module->UnLink();
            m_Pcb->m_Status_Pcb = 0;
        }
        break;

        case PCB_ZONE_AREA_T:     // a zone area
            m_Pcb->Remove( item );
            break;

        case PCB_LINE_T:          // a segment not on copper layers
        case PCB_TEXT_T:          // a text on a layer
        case PCB_TRACE_T:         // a track segment (segment on a copper layer)
        case PCB_VIA_T:           // a via (like track segment on a copper layer)
        case PCB_DIMENSION_T:     // a dimension (graphic item)
        case PCB_TARGET_T:        // a target (graphic item)
            item->UnLink();
            break;

        // These items are deleted, but not put in undo list
        case PCB_MARKER_T:                  // a marker used to show something
        case PCB_ZONE_T:                     // SEG_ZONE items are now deprecated
            item->UnLink();
            itemsList->RemovePicker( ii );
            ii--;
            item->DeleteStructure();
            break;

        default:
            wxMessageBox( wxT( "PCB_EDIT_FRAME::Block_Delete( ) error: unexpected type" ) );
            break;
        }
    }

    SaveCopyInUndoList( *itemsList, UR_DELETED );

    Compile_Ratsnest( NULL, true );
    m_canvas->Refresh( true );
}


void PCB_EDIT_FRAME::Block_Rotate()
{
    wxPoint centre;                     // rotation cent-re for the rotation transform
    int     rotAngle = m_rotationAngle; // rotation angle in 0.1 deg.

    centre = GetScreen()->m_BlockLocate.Centre();

    OnModify();

    PICKED_ITEMS_LIST* itemsList = &GetScreen()->m_BlockLocate.GetItems();
    itemsList->m_Status = UR_CHANGED;

    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );
        wxASSERT( item );
        itemsList->SetPickedItemStatus( UR_CHANGED, ii );

        switch( item->Type() )
        {
        case PCB_MODULE_T:
            ( (MODULE*) item )->ClearFlags();
            m_Pcb->m_Status_Pcb = 0;
            break;

        // Move and rotate the track segments
        case PCB_TRACE_T:       // a track segment (segment on a copper layer)
        case PCB_VIA_T:         // a via (like track segment on a copper layer)
            m_Pcb->m_Status_Pcb = 0;
            break;

        case PCB_ZONE_AREA_T:
        case PCB_LINE_T:
        case PCB_TEXT_T:
        case PCB_TARGET_T:
        case PCB_DIMENSION_T:
            break;

        // This item is not put in undo list
        case PCB_ZONE_T:         // SEG_ZONE items are now deprecated
            itemsList->RemovePicker( ii );
            ii--;
            break;

        default:
            wxMessageBox( wxT( "PCB_EDIT_FRAME::Block_Rotate( ) error: unexpected type" ) );
            break;
        }
    }

    // Save all the block items in there current state before applying the rotation.
    SaveCopyInUndoList( *itemsList, UR_CHANGED, centre );

    // Now perform the rotation.
    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );
        wxASSERT( item );
        item->Rotate( centre, rotAngle );
        GetBoard()->GetConnectivity()->Update( item );
    }

    Compile_Ratsnest( NULL, true );
    m_canvas->Refresh( true );
}


void PCB_EDIT_FRAME::Block_Flip()
{
#define INVERT( pos ) (pos) = center.y - ( (pos) - center.y )
    wxPoint center; // Position of the axis for inversion of all elements

    OnModify();

    PICKED_ITEMS_LIST* itemsList = &GetScreen()->m_BlockLocate.GetItems();
    itemsList->m_Status = UR_FLIPPED;

    center = GetScreen()->m_BlockLocate.Centre();

    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );
        wxASSERT( item );
        itemsList->SetPickedItemStatus( UR_FLIPPED, ii );
        item->Flip( center );
        GetBoard()->GetConnectivity()->Update( item );

        // If a connected item is flipped, the ratsnest is no more OK
        switch( item->Type() )
        {
        case PCB_MODULE_T:
            item->ClearFlags();
            m_Pcb->m_Status_Pcb = 0;
            break;

        case PCB_TRACE_T:
        case PCB_VIA_T:
            m_Pcb->m_Status_Pcb = 0;
            break;

        case PCB_ZONE_AREA_T:
        case PCB_LINE_T:
        case PCB_TEXT_T:
        case PCB_TARGET_T:
        case PCB_DIMENSION_T:
            break;

        // This item is not put in undo list
        case PCB_ZONE_T:         // SEG_ZONE items are now deprecated
            itemsList->RemovePicker( ii );
            ii--;
            break;


        default:
            wxMessageBox( wxT( "PCB_EDIT_FRAME::Block_Flip( ) error: unexpected type" ) );
            break;
        }
    }

    SaveCopyInUndoList( *itemsList, UR_FLIPPED, center );
    Compile_Ratsnest( NULL, true );
    m_canvas->Refresh( true );
}


void PCB_EDIT_FRAME::Block_Move()
{
    OnModify();

    wxPoint            MoveVector = GetScreen()->m_BlockLocate.GetMoveVector();

    PICKED_ITEMS_LIST* itemsList = &GetScreen()->m_BlockLocate.GetItems();
    itemsList->m_Status = UR_MOVED;

    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );
        itemsList->SetPickedItemStatus( UR_MOVED, ii );
        item->Move( MoveVector );
        GetBoard()->GetConnectivity()->Update( item );
        item->ClearFlags( IS_MOVED );

        switch( item->Type() )
        {
        case PCB_MODULE_T:
            m_Pcb->m_Status_Pcb = 0;
            item->ClearFlags();
            break;

        // Move track segments
        case PCB_TRACE_T:       // a track segment (segment on a copper layer)
        case PCB_VIA_T:         // a via (like a track segment on a copper layer)
            m_Pcb->m_Status_Pcb = 0;
            break;

        case PCB_ZONE_AREA_T:
        case PCB_LINE_T:
        case PCB_TEXT_T:
        case PCB_TARGET_T:
        case PCB_DIMENSION_T:
            break;

        // This item is not put in undo list
        case PCB_ZONE_T:        // SEG_ZONE items are now deprecated
            itemsList->RemovePicker( ii );
            ii--;
            break;

        default:
            wxMessageBox( wxT( "PCB_EDIT_FRAME::Block_Move( ) error: unexpected type" ) );
            break;
        }
    }

    SaveCopyInUndoList( *itemsList, UR_MOVED, MoveVector );

    Compile_Ratsnest( NULL, true );
    m_canvas->Refresh( true );
}


void PCB_EDIT_FRAME::Block_Duplicate( bool aIncrement )
{
    wxPoint MoveVector = GetScreen()->m_BlockLocate.GetMoveVector();

    OnModify();

    PICKED_ITEMS_LIST* itemsList = &GetScreen()->m_BlockLocate.GetItems();

    PICKED_ITEMS_LIST newList;
    newList.m_Status = UR_NEW;

    ITEM_PICKER picker( NULL, UR_NEW );
    BOARD_ITEM* newitem;

    for( unsigned ii = 0; ii < itemsList->GetCount(); ii++ )
    {
        BOARD_ITEM* item = (BOARD_ITEM*) itemsList->GetPickedItem( ii );

        newitem = (BOARD_ITEM*)item->Clone();

        if( item->Type() == PCB_MODULE_T )
            m_Pcb->m_Status_Pcb = 0;

        m_Pcb->Add( newitem );

        if( newitem )
        {
            newitem->Move( MoveVector );
            picker.SetItem ( newitem );
            newList.PushItem( picker );
        }
    }

    if( newList.GetCount() )
        SaveCopyInUndoList( newList, UR_NEW );

    Compile_Ratsnest( NULL, true );
    m_canvas->Refresh( true );
}
